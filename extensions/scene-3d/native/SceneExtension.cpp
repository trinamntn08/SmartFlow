#include "SceneExtension.h"
#include "SceneViewer.h"
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/AbstractMemberFactory.h>
#include <tp_math_utils/materials/OpenGLMaterial.h>
#include <cmath>
#include <stdexcept>

namespace smartflow::scene3d {
using namespace tp_pipeline;
using tp_utils::StringID;

const StringID& sceneType()
{
    static const StringID value("smartflow.scene-3d.scene@1");
    return value;
}

namespace {
class SceneFactory final : public tp_data::AbstractMemberFactory {
public:
    SceneFactory() : AbstractMemberFactory(sceneType(), "scene", {80, 160, 220}) {}
    std::shared_ptr<tp_data::AbstractMember> clone(std::string& error, const tp_data::AbstractMember& member) const override
    {
        const auto* scene = dynamic_cast<const SceneMember*>(&member);
        if(!scene) { error = "Expected scene output"; return {}; }
        auto copy = std::make_shared<SceneMember>();
        copy->objects = scene->objects;
        copy->setName(scene->name());
        return copy;
    }
    void save(std::string& error, const tp_data::AbstractMember&, std::string&) const override
    { error = "Scene output persistence is not implemented"; }
    std::shared_ptr<tp_data::AbstractMember> load(std::string& error, const std::string&) const override
    { error = "Scene output persistence is not implemented"; return {}; }
};

enum class Operation { Cube, Transform, Material, Scene, Merge };
struct Field { const char* name; double value, min, max; };
std::vector<Field> fields(Operation operation)
{
    switch(operation) {
    case Operation::Cube: return {{"size", 2, 0.01, 100}};
    case Operation::Transform: return {
        {"x", 0, -100, 100}, {"y", 0, -100, 100}, {"z", 0, -100, 100},
        {"rotation Y", 25, -360, 360}, {"scale X", 1, 0.01, 10},
        {"scale Y", 1, 0.01, 10}, {"scale Z", 1, 0.01, 10}};
    case Operation::Material: return {{"red", 0.18, 0, 1}, {"green", 0.58, 0, 1}, {"blue", 0.88, 0, 1}};
    default: return {};
    }
}

tp_math_utils::Geometry3D cube(float size)
{
    tp_math_utils::Geometry3D geometry;
    const float half = size / 2;
    for(const auto& v : std::vector<glm::vec3>{
        {-half,-half,-half}, {half,-half,-half}, {half,half,-half}, {-half,half,-half},
        {-half,-half,half}, {half,-half,half}, {half,half,half}, {-half,half,half}})
        geometry.verts.push_back({v, {}, {}});
    geometry.indexes.push_back({TP_TRIANGLES, {
        0,2,1, 0,3,2, 4,5,6, 4,6,7, 0,1,5, 0,5,4,
        3,7,6, 3,6,2, 0,4,7, 0,7,3, 1,2,6, 1,6,5}});
    geometry.breakApartTriangles();
    geometry.calculateFaceNormals();
    geometry.material.findOrAddOpenGL()->albedo = {0.65f, 0.68f, 0.72f};
    return geometry;
}

class SceneDelegate final : public StepDelegate {
public:
    SceneDelegate(const char* type, Operation operation)
        : StepDelegate(type, {"Scene 3D"},
              operation == Operation::Cube ? std::vector<PortDetails>{} :
              operation == Operation::Merge ? std::vector<PortDetails>{{"first", sceneType()}, {"second", sceneType()}} :
                                             std::vector<PortDetails>{{"in", sceneType()}},
              {{"out", sceneType()}}), operation(operation) {}
    void fixupParameters(StepDetails* step, std::vector<StringID>& names) const override
    {
        for(const auto& field : fields(operation)) {
            Parameter p;
            p.name = field.name; p.type = doubleSID(); p.value = field.value;
            p.min = field.min; p.max = field.max; p.step = 0.1;
            step->setParamerter(p);
            names.push_back(p.name);
        }
    }
    bool executeStep(StepContext* context) const override
    {
        for(const auto& field : fields(operation)) {
            const auto parameter = context->stepDetails->parameter(field.name);
            const auto* value = std::get_if<double>(&parameter.value);
            if(!value || !std::isfinite(*value) || *value < field.min || *value > field.max)
                throw std::runtime_error(std::string("Invalid scene parameter: ") + field.name);
        }
        auto value = [&](const char* name) { return float(context->stepDetails->parameterValue<double>(name)); };
        auto output = std::make_shared<SceneMember>();
        if(operation == Operation::Cube) {
            output->objects.push_back({context->stepDetails->id(), cube(value("size"))});
        } else {
            auto* input = context->memberCast<SceneMember>(operation == Operation::Merge ? "first" : "in");
            if(!input) return false;
            output->objects = input->objects; // Value-copy; never mutate a published upstream result.
            if(operation == Operation::Merge) {
                auto* second = context->memberCast<SceneMember>("second");
                if(!second || output->objects.size() + second->objects.size() > 64)
                    throw std::runtime_error("Scene preview supports at most 64 objects");
                output->objects.insert(output->objects.end(), second->objects.begin(), second->objects.end());
            }
            if(operation == Operation::Transform) {
                tp_math_utils::MeshKeyFrame transform;
                transform.position = {value("x"), value("y"), value("z")};
                transform.rotation.y = value("rotation Y");
                transform.scale = {value("scale X"), value("scale Y"), value("scale Z")};
                const auto matrix = transform.calculateModelMatrix();
                for(auto& object : output->objects) {
                    object.geometry.transform(matrix);
                    // Legacy transform rotates normals only. Recompute after
                    // nonuniform scale so lighting follows the transformed faces.
                    object.geometry.calculateFaceNormals();
                }
            }
            if(operation == Operation::Material)
                for(auto& object : output->objects)
                    object.geometry.material.findOrAddOpenGL()->albedo = {value("red"), value("green"), value("blue")};
        }
        for(const auto& object : output->objects)
            for(const auto& vertex : object.geometry.verts)
                for(int axis=0; axis<3; ++axis)
                    if(!std::isfinite(vertex.vert[axis]) || std::abs(vertex.vert[axis]) > 100000.0f)
                        throw std::runtime_error("Scene preview coordinates exceed supported bounds");
        return context->stepOutput->addSharedMember("out", output, context->progress);
    }
private:
    Operation operation;
};
}

void contribute(WorkspaceConfiguration& configuration)
{
    const std::vector<std::pair<const char*, Operation>> operations = {
        {"cube", Operation::Cube}, {"transform", Operation::Transform},
        {"material", Operation::Material}, {"scene", Operation::Scene}, {"merge", Operation::Merge}};
    for(const auto& [name, operation] : operations) {
        const auto type = QString("smartflow.scene-3d.%1@1").arg(name);
        QString title(name); title[0] = title[0].toUpper();
        configuration.delegates->addStepDelegate(new SceneDelegate(type.toStdString().c_str(), operation));
        configuration.nodes.push_back({type, title, "Scene 3D", "smartflow.scene-3d", name, 1});
    }
    configuration.factory->addMemberFactory(new SceneFactory);
    configuration.createViewer = [] { return new SceneViewer; };
}

WorkspaceConfiguration sceneConfiguration()
{
    WorkspaceConfiguration config;
    config.delegates = std::make_shared<StepDelegateMap>();
    config.factory = std::make_shared<tp_data::CollectionFactory>();
    tp_data::createCollectionFactories(*config.factory);
    contribute(config);
    config.factory->finalize();
    config.preset = {{"smartflow.scene-3d.cube@1", {0,0}, {}},
                     {"smartflow.scene-3d.transform@1", {190,0}, {}},
                     {"smartflow.scene-3d.material@1", {380,0}, {}},
                     {"smartflow.scene-3d.scene@1", {570,0}, {}}};
    config.connections = {{0,0,1,0}, {1,0,2,0}, {2,0,3,0}};
    return config;
}
} // namespace smartflow::scene3d
