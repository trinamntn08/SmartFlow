#include "SceneViewer.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSignalBlocker>

namespace smartflow::scene3d {
SceneViewer::SceneViewer()
{
    setObjectName("sceneViewer"); setMinimumSize(360,270); setFocusPolicy(Qt::StrongFocus);
    setToolTip("Left drag: orbit. Middle drag or Shift-drag: pan. Wheel: zoom at cursor. F: frame. Click: select.");
    tools=new QWidget(this); auto* layout=new QHBoxLayout(tools); layout->setContentsMargins(8,4,8,4);
    viewChoice=new QComboBox(tools); viewChoice->setObjectName("sceneView");
    viewChoice->addItems({"Orbit","Front","Right","Top"}); layout->addWidget(viewChoice);
    auto* frame=new QPushButton("Frame",tools); frame->setObjectName("sceneFrame"); layout->addWidget(frame); layout->addStretch();
    connect(viewChoice,&QComboBox::currentIndexChanged,this,[this](int index) { camera.view=CameraView(index); workspaceChanged(); });
    connect(frame,&QPushButton::clicked,this,[this] { frameScene(); });
}
QRectF SceneViewer::viewport() const { return QRectF(0,46,width(),std::max(1,height()-76)); }
void SceneViewer::resizeEvent(QResizeEvent*) { tools->setGeometry(0,0,width(),40); }
void SceneViewer::workspaceChanged() { update(); if(workspaceStateChanged) workspaceStateChanged(); }
void SceneViewer::present(std::shared_ptr<const tp_data::Collection> output) { snapshot.present(std::move(output)); update(); }
QJsonObject SceneViewer::workspaceState() const
{
    auto state=camera.state(); state["selection"]=selectedObject(); state["selectedObjectId"]=snapshot.selectedId(); return state;
}
void SceneViewer::restoreWorkspaceState(const QJsonObject& state)
{
    camera.restore(state); snapshot.restoreSelection(state);
    const QSignalBlocker blocker(viewChoice); viewChoice->setCurrentIndex(int(camera.view)); update();
}
QString SceneViewer::describe(const tp_data::Collection& output) const
{
    for(const auto& member : output.members())
        if(const auto* value=dynamic_cast<const SceneMember*>(member.get())) return QString("%1 object(s)").arg(value->objects.size());
    return {};
}
void SceneViewer::frameScene()
{
    glm::vec3 low,high; if(snapshot.bounds(low,high)) { camera.frame(low,high); workspaceChanged(); }
}
void SceneViewer::paintEvent(QPaintEvent*)
{
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing); painter.fillRect(rect(),QColor(27,32,41));
    painter.setPen(QColor(208,217,229));
    painter.drawText(12,height()-12,QString("%1 object(s)   Drag: orbit   Shift: pan   Wheel: zoom").arg(objectCount()));
    if(!snapshot.scene()) { painter.drawText(viewport(),Qt::AlignCenter,"No current scene output"); return; }
    painter.save(); painter.setClipRect(viewport());
    renderer.paint(painter,viewport(),camera,*snapshot.scene(),selectedObject()); painter.restore();
}
void SceneViewer::mousePressEvent(QMouseEvent* event)
{
    if(event->button()!=Qt::LeftButton && event->button()!=Qt::MiddleButton && event->button()!=Qt::RightButton) return;
    setFocus(); lastPosition=pressPosition=event->position(); dragged=false;
}
void SceneViewer::mouseMoveEvent(QMouseEvent* event)
{
    if(!(event->buttons() & (Qt::LeftButton|Qt::MiddleButton|Qt::RightButton))) return;
    const auto delta=event->position()-lastPosition;
    dragged=dragged || (event->position()-pressPosition).manhattanLength()>3;
    if((event->buttons() & Qt::MiddleButton) || (event->modifiers() & Qt::ShiftModifier)) camera.pan(delta,viewport());
    else { camera.orbit(delta); const QSignalBlocker blocker(viewChoice); viewChoice->setCurrentIndex(0); }
    lastPosition=event->position(); workspaceChanged();
}
void SceneViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button()!=Qt::LeftButton || dragged || !snapshot.scene()) return;
    snapshot.select(renderer.pick(event->position(),viewport(),camera,*snapshot.scene())); workspaceChanged();
}
void SceneViewer::wheelEvent(QWheelEvent* event) { camera.zoom(float(event->angleDelta().y()),event->position(),viewport()); workspaceChanged(); event->accept(); }
void SceneViewer::mouseDoubleClickEvent(QMouseEvent*) { frameScene(); }
void SceneViewer::keyPressEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_F) { frameScene(); event->accept(); } else OutputViewer::keyPressEvent(event);
}
}
