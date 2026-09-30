#include "SceneViewer.h"
#include <tp_math_utils/materials/OpenGLMaterial.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <algorithm>
#include <cmath>

namespace smartflow::scene3d {
SceneViewer::SceneViewer()
{
    setObjectName("sceneViewer");
    setMinimumSize(360, 270);
    setFocusPolicy(Qt::StrongFocus);
    setToolTip("Drag to orbit. Wheel to zoom. Double-click to frame. Click an object to select it.");
}

void SceneViewer::present(std::shared_ptr<const tp_data::Collection> output)
{
    collection = std::move(output);
    scene = nullptr;
    if(collection)
        for(const auto& member : collection->members())
            if(const auto* found = dynamic_cast<const SceneMember*>(member.get())) { scene = found; break; }
    if(!scene || selection >= int(scene->objects.size())) selection = -1;
    pickFaces.clear();
    update();
}

size_t SceneViewer::objectCount() const { return scene ? scene->objects.size() : 0; }

QString SceneViewer::describe(const tp_data::Collection& output) const
{
    for(const auto& member : output.members())
        if(const auto* value = dynamic_cast<const SceneMember*>(member.get()))
            return QString("%1 object(s)").arg(value->objects.size());
    return {};
}

void SceneViewer::frameScene()
{
    if(!scene || scene->objects.empty()) return;
    glm::vec3 low(INFINITY), high(-INFINITY);
    for(const auto& object : scene->objects)
        for(const auto& vertex : object.geometry.verts) {
            low = glm::min(low, vertex.vert);
            high = glm::max(high, vertex.vert);
        }
    target = (low + high) / 2.0f;
    span = std::max(0.1f, glm::length(high-low) * 1.4f);
    update();
}

void SceneViewer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(27, 32, 41));
    painter.setPen(QColor(208, 217, 229));
    painter.drawText(14, 23, "3D preview");
    painter.drawText(14, height()-14, "Drag: orbit   Wheel: zoom   Double-click: frame   Click: select");
    pickFaces.clear();
    if(!scene) {
        painter.drawText(rect(), Qt::AlignCenter, "No current scene output");
        return;
    }
    const float y = glm::radians(yaw), p = glm::radians(pitch);
    const glm::vec3 forward(std::sin(y)*std::cos(p), std::sin(p), std::cos(y)*std::cos(p));
    const glm::vec3 right(std::cos(y), 0, -std::sin(y));
    const glm::vec3 up = glm::cross(forward, right);
    const float pixels = float(std::min(width(), height()-60)) / span;
    const QPointF center(width()/2.0, height()/2.0);
    auto project = [&](glm::vec3 v) {
        v -= target;
        return center + QPointF(glm::dot(v,right)*pixels, -glm::dot(v,up)*pixels);
    };
    painter.setPen(QPen(QColor(48, 57, 69), 1));
    for(int i=-5; i<=5; ++i) {
        painter.drawLine(project({float(i),-1,-5}), project({float(i),-1,5}));
        painter.drawLine(project({-5,-1,float(i)}), project({5,-1,float(i)}));
    }
    struct Face { QPolygonF polygon; QColor color; float depth; int object; };
    std::vector<Face> faces;
    int index = 0;
    for(const auto& object : scene->objects) {
        glm::vec3 albedo(0.65f);
        object.geometry.material.viewOpenGL([&](const auto& material) { albedo = material.albedo; });
        object.geometry.forEachTriangle([&](const auto& a, const auto& b, const auto& c, int, int, int) {
            auto normal = glm::cross(b.vert-a.vert, c.vert-a.vert);
            if(glm::length(normal) < 1e-7f) return;
            normal = glm::normalize(normal);
            if(glm::dot(normal, forward) <= 0) return;
            const float light = 0.28f + 0.72f * std::max(0.0f, glm::dot(normal, glm::normalize(glm::vec3(-1,2,3))));
            const auto color = glm::clamp(albedo * light, glm::vec3(0), glm::vec3(1));
            faces.push_back({QPolygonF({project(a.vert), project(b.vert), project(c.vert)}),
                QColor::fromRgbF(color.r, color.g, color.b),
                glm::dot((a.vert+b.vert+c.vert)/3.0f-target, forward), index});
        });
        ++index;
    }
    // Painter ordering is suitable for the small nonintersecting primitive
    // preview. Intersecting surfaces require a future depth-buffered renderer.
    std::stable_sort(faces.begin(), faces.end(), [](const auto& a, const auto& b) { return a.depth < b.depth; });
    for(const auto& face : faces) {
        painter.setBrush(face.color);
        painter.setPen(QPen(face.object == selection ? QColor(255,207,94) : face.color.darker(115),
                            face.object == selection ? 2 : 1));
        painter.drawPolygon(face.polygon);
        pickFaces.push_back({face.polygon, face.object});
    }
    painter.setPen(QColor(208, 217, 229));
    painter.drawText(14, 44, QString("%1 object(s)%2").arg(objectCount()).arg(
        selection >= 0 ? QString(" - selected object %1").arg(selection+1) : QString()));
}

void SceneViewer::mousePressEvent(QMouseEvent* event)
{
    if(event->button() != Qt::LeftButton) return;
    lastPosition = pressPosition = event->position();
    dragged = false;
}
void SceneViewer::mouseMoveEvent(QMouseEvent* event)
{
    if(!(event->buttons() & Qt::LeftButton)) return;
    const auto delta = event->position() - lastPosition;
    dragged = dragged || (event->position()-pressPosition).manhattanLength() > 3;
    yaw += float(delta.x()) * 0.5f;
    pitch = std::clamp(pitch + float(delta.y()) * 0.5f, -85.0f, 85.0f);
    lastPosition = event->position();
    update();
}
void SceneViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() != Qt::LeftButton || dragged) return;
    selection = -1;
    for(auto it=pickFaces.rbegin(); it!=pickFaces.rend(); ++it)
        if(it->polygon.containsPoint(event->position(), Qt::OddEvenFill)) { selection=it->object; break; }
    update();
}
void SceneViewer::wheelEvent(QWheelEvent* event)
{
    const float factor = std::exp(std::clamp(-float(event->angleDelta().y())/1000.0f, -1.0f, 1.0f));
    span = std::clamp(span * factor, 0.1f, 1000000.0f);
    update();
    event->accept();
}
void SceneViewer::mouseDoubleClickEvent(QMouseEvent*) { frameScene(); }
} // namespace smartflow::scene3d
