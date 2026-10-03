#include "ObjectInteractionController.h"
#include <algorithm>
#include <cmath>

namespace smartflow::scene3d {
namespace {
const char* positionNames[]={"x","y","z"};
const char* scaleNames[]={"scale X","scale Y","scale Z"};
}
void ObjectInteractionController::cancel() { original.reset(); current.reset(); bindings.clear(); valid=false; }
bool ObjectInteractionController::begin(const SceneMember& scene,int selection,InteractionMode nextMode,int nextAxis,
                                        QPointF cursor,const CameraController& view,QRectF area,const ViewerCommands& commands)
{
    cancel(); status.clear();
    if(selection<0 || selection>=int(scene.objects.size())) { status="Select an object first."; return false; }
    const auto& object=scene.objects[selection];
    if(!object.transform) { status="Add a Transform node to edit this object."; return false; }
    if(!commands.revision || !commands.parameter || !commands.apply) { status="This viewer is read-only."; return false; }
    if(nextMode==InteractionMode::Navigate || nextAxis<0 || nextAxis>2) return false;
    source=*object.transform; mode=nextMode; axis=nextAxis; camera=view; viewport=area; start=cursor;
    request={commands.revision(),mode==InteractionMode::Move ? "Move scene object" :
        mode==InteractionMode::Rotate ? "Rotate scene object" : "Scale scene object",{}};
    const auto node=QString::fromStdString(source.node.toString());
    const auto add=[&](const char* name,double originalValue) {
        const auto binding=commands.parameter(node,name);
        if(!binding) { status=QString("Expose '%1' to enable this edit.").arg(name); return false; }
        if(std::abs(binding->value-originalValue)>1e-5*std::max(1.0,std::abs(binding->value))) {
            status="Scene output is stale. Run the graph again."; return false;
        }
        bindings.push_back(*binding); request.parameters.push_back({node,name,binding->value}); return true;
    };
    if(mode==InteractionMode::Move && !add(positionNames[axis],source.parameters.position[axis])) return false;
    if(mode==InteractionMode::Rotate && !add("rotation Y",source.parameters.rotation.y)) return false;
    if(mode==InteractionMode::Scale)
        for(int i=0;i<3;++i) {
            if(!add(scaleNames[i],source.parameters.scale[i])) return false;
            if(bindings.back().value<=0 || bindings.back().minimum<=0) { status="Scale must be positive."; return false; }
        }
    axisDirection=glm::vec3(0); axisDirection[axis]=1;
    if(mode==InteractionMode::Move) {
        const auto projected=axisDirection-camera.forward()*glm::dot(axisDirection,camera.forward());
        if(glm::dot(projected,projected)<0.01f) { status="Axis points into the camera. Choose another view."; return false; }
        const auto point=camera.pointOnPlane(cursor,viewport,source.parameters.position,camera.forward());
        if(!point) { status="Cannot project this gesture."; return false; }
        startPoint=*point;
    }
    original=std::make_shared<SceneMember>(); original->objects=scene.objects;
    const auto count=std::count_if(scene.objects.begin(),scene.objects.end(),[&](const auto& item) {
        return item.transform && item.transform->node==source.node;
    });
    status=QString("Editing Transform: %1 object(s). Ctrl snaps. Esc cancels.").arg(count);
    valid=true; update(cursor,false); return true;
}
void ObjectInteractionController::update(QPointF cursor,bool snap)
{
    if(!original) return;
    auto transform=source.parameters;
    if(mode==InteractionMode::Move) {
        const auto point=camera.pointOnPlane(cursor,viewport,source.parameters.position,camera.forward());
        if(!point) { valid=false; status="Cannot project this gesture."; return; }
        const auto projected=axisDirection-camera.forward()*glm::dot(axisDirection,camera.forward());
        double amount=glm::dot(*point-startPoint,axisDirection)/glm::dot(projected,projected);
        if(snap) amount=std::round(amount*10.0)/10.0;
        request.parameters[0].value=std::clamp(bindings[0].value+amount,bindings[0].minimum,bindings[0].maximum);
        transform.position[axis]=float(request.parameters[0].value);
    } else if(mode==InteractionMode::Rotate) {
        double angle=(cursor.x()-start.x())*0.5;
        if(snap) angle=std::round(angle/5)*5;
        request.parameters[0].value=std::clamp(bindings[0].value+angle,bindings[0].minimum,bindings[0].maximum);
        transform.rotation.y=float(request.parameters[0].value);
    } else {
        double factor=std::exp(std::clamp((cursor.x()-start.x())/100.0,-8.0,8.0));
        if(snap) factor=std::max(0.1,std::round(factor*10)/10);
        // Clamp one factor, preserving the original nonuniform scale ratio.
        double low=0,high=INFINITY;
        for(const auto& binding : bindings) { low=std::max(low,binding.minimum/binding.value); high=std::min(high,binding.maximum/binding.value); }
        factor=std::clamp(factor,low,high);
        for(int i=0;i<3;++i) { request.parameters[i].value=bindings[i].value*factor; transform.scale[i]=float(request.parameters[i].value); }
    }
    const auto delta=transform.calculateModelMatrix()*glm::inverse(source.parameters.calculateModelMatrix());
    auto preview=std::make_shared<SceneMember>(); preview->objects=original->objects;
    for(auto& object : preview->objects) {
        if(!object.transform || object.transform->node!=source.node) continue;
        object.geometry.transform(delta); object.geometry.calculateFaceNormals(); object.transform->parameters=transform;
        for(const auto& vertex : object.geometry.verts)
            for(int i=0;i<3;++i)
                if(!std::isfinite(vertex.vert[i]) || std::abs(vertex.vert[i])>100000.0f) {
                    valid=false; current.reset(); status="Preview exceeds the supported scene bounds."; return;
                }
    }
    current=std::move(preview); valid=true;
}
QString ObjectInteractionController::finish(const ViewerCommands& commands)
{
    if(!original) return status;
    if(!valid) { const auto error=status; cancel(); return error; }
    const auto edits=request;
    bool changed=false;
    for(size_t i=0;i<bindings.size();++i) changed=changed || edits.parameters[i].value!=bindings[i].value;
    cancel();
    status=changed ? (commands.apply ? commands.apply(edits) : "This viewer is read-only.") : QString();
    return status;
}
}
