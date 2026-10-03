#include "SceneViewer.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QLineF>
#include <algorithm>

namespace smartflow::scene3d {
SceneViewer::SceneViewer()
{
    setObjectName("sceneViewer"); setMinimumSize(360,270); setFocusPolicy(Qt::StrongFocus);
    setToolTip("Navigate: left drag orbits. Middle/Shift drag pans. Wheel zooms at cursor. G/R/S: move/rotate/scale. Ctrl snaps; Esc cancels.");
    tools=new QWidget(this); auto* outer=new QVBoxLayout(tools); outer->setContentsMargins(8,4,8,4); outer->setSpacing(4);
    auto* layout=new QHBoxLayout; outer->addLayout(layout);
    viewChoice=new QComboBox(tools); viewChoice->setObjectName("sceneView");
    viewChoice->addItems({"Orbit","Front","Right","Top"}); layout->addWidget(viewChoice);
    auto* frame=new QPushButton("Frame",tools); frame->setObjectName("sceneFrame"); layout->addWidget(frame);
    modeChoice=new QComboBox(tools); modeChoice->setObjectName("sceneInteractionMode");
    modeChoice->addItems({"Navigate","Move","Rotate Y","Scale"}); layout->addWidget(modeChoice);
    axisChoice=new QComboBox(tools); axisChoice->setObjectName("sceneMoveAxis"); axisChoice->addItems({"X","Y","Z"});
    axisChoice->setEnabled(false); layout->addWidget(axisChoice);
    auto* selectionRow=new QHBoxLayout; outer->addLayout(selectionRow);
    objectChoice=new QComboBox(tools); objectChoice->setObjectName("sceneObject"); objectChoice->setMaximumWidth(125); selectionRow->addWidget(objectChoice);
    status=new QLabel("Select an object to edit its Transform.",tools); status->setObjectName("sceneInteractionStatus");
    status->setStyleSheet("color: #d0d9e5;");
    status->setTextFormat(Qt::PlainText); status->setWordWrap(true); selectionRow->addWidget(status,1);
    connect(viewChoice,&QComboBox::currentIndexChanged,this,[this](int index) { cancelGesture(); camera.view=CameraView(index); workspaceChanged(); });
    connect(modeChoice,&QComboBox::currentIndexChanged,this,[this](int index) { cancelGesture(); axisChoice->setEnabled(index==1); update(); });
    connect(axisChoice,&QComboBox::currentIndexChanged,this,[this] { cancelGesture(); update(); });
    connect(objectChoice,&QComboBox::currentIndexChanged,this,[this](int index) { cancelGesture(); selectObject(index-1); });
    connect(frame,&QPushButton::clicked,this,[this] { frameScene(); });
    refreshObjects();
}
QRectF SceneViewer::viewport() const { return QRectF(0,90,width(),std::max(1,height()-120)); }
void SceneViewer::resizeEvent(QResizeEvent*) { tools->setGeometry(0,0,width(),86); }
void SceneViewer::hideEvent(QHideEvent*) { cancelGesture(); }
void SceneViewer::workspaceChanged() { update(); if(workspaceStateChanged) workspaceStateChanged(); }
void SceneViewer::cancelGesture() { interaction.cancel(); status->setText("Select an object; choose Move, Rotate Y or Scale."); update(); }
void SceneViewer::projectChanged() { cancelGesture(); }
void SceneViewer::present(std::shared_ptr<const tp_data::Collection> output) { cancelGesture(); snapshot.present(std::move(output)); refreshObjects(); update(); }
const SceneMember* SceneViewer::displayedScene() const { return interaction.preview() ? interaction.preview() : snapshot.scene(); }
void SceneViewer::refreshObjects()
{
    const QSignalBlocker blocker(objectChoice); objectChoice->clear(); objectChoice->addItem("Select object");
    if(snapshot.scene())
        for(int i=0;i<int(snapshot.scene()->objects.size());++i) {
            objectChoice->addItem(QString("Object %1").arg(i+1));
            objectChoice->setItemData(i+1,snapshot.scene()->objects[i].id,Qt::ToolTipRole);
        }
    objectChoice->setCurrentIndex(selectedObject()+1); objectChoice->setEnabled(objectCount()>0);
}
void SceneViewer::selectObject(int index) { snapshot.select(index); refreshObjects(); workspaceChanged(); }
QJsonObject SceneViewer::workspaceState() const
{
    auto state=camera.state(); state["selection"]=selectedObject(); state["selectedObjectId"]=snapshot.selectedId(); return state;
}
void SceneViewer::restoreWorkspaceState(const QJsonObject& state)
{
    camera.restore(state); snapshot.restoreSelection(state);
    cancelGesture(); const QSignalBlocker blocker(viewChoice); viewChoice->setCurrentIndex(int(camera.view)); refreshObjects(); update();
}
QString SceneViewer::describe(const tp_data::Collection& output) const
{
    for(const auto& member : output.members())
        if(const auto* value=dynamic_cast<const SceneMember*>(member.get())) return QString("%1 object(s)").arg(value->objects.size());
    return {};
}
void SceneViewer::frameScene()
{
    cancelGesture(); glm::vec3 low,high; if(snapshot.bounds(low,high)) { camera.frame(low,high); workspaceChanged(); }
}
void SceneViewer::paintEvent(QPaintEvent*)
{
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing); painter.fillRect(rect(),QColor(27,32,41));
    painter.setPen(QColor(208,217,229));
    painter.drawText(12,height()-12,QString("%1 object(s)   Drag: orbit   Shift: pan   Wheel: zoom").arg(objectCount()));
    const auto* scene=displayedScene();
    if(!scene) { painter.drawText(viewport(),Qt::AlignCenter,"No current scene output"); return; }
    painter.save(); painter.setClipRect(viewport());
    renderer.paint(painter,viewport(),camera,*scene,selectedObject());
    const auto selection=selectedObject();
    if(modeChoice->currentIndex()==1 && selection>=0 && scene->objects[selection].transform) {
        const auto origin=scene->objects[selection].transform->parameters.position;
        const auto center=camera.project(origin,viewport());
        const QColor colors[]={QColor(255,110,110),QColor(120,230,140),QColor(120,170,255)};
        const char* names[]={"X","Y","Z"};
        for(int i=0;i<3;++i) {
            glm::vec3 offset(0); offset[i]=camera.span*0.18f;
            const auto end=camera.project(origin+offset,viewport());
            painter.setPen(QPen(colors[i],axisChoice->currentIndex()==i ? 4 : 2)); painter.drawLine(center,end);
            painter.drawText(end+QPointF(4,-4),names[i]); painter.setBrush(colors[i]); painter.drawEllipse(end,4,4);
        }
    }
    painter.restore();
}
int SceneViewer::pickedAxis(QPointF point) const
{
    const auto selection=selectedObject(); const auto* scene=snapshot.scene();
    if(!scene || selection<0 || !scene->objects[selection].transform) return -1;
    const auto origin=scene->objects[selection].transform->parameters.position;
    const auto start=camera.project(origin,viewport()); int picked=-1; double nearest=9;
    for(int i=0;i<3;++i) {
        glm::vec3 offset(0); offset[i]=camera.span*0.18f;
        const auto end=camera.project(origin+offset,viewport()); const auto delta=end-start;
        const auto length=QPointF::dotProduct(delta,delta); if(length<144) continue;
        const auto t=std::clamp(QPointF::dotProduct(point-start,delta)/length,0.0,1.0);
        const auto distance=QLineF(point,start+delta*t).length();
        if(distance<nearest) { nearest=distance; picked=i; }
    }
    return picked;
}
void SceneViewer::mousePressEvent(QMouseEvent* event)
{
    if(event->button()!=Qt::LeftButton && event->button()!=Qt::MiddleButton && event->button()!=Qt::RightButton) return;
    if(event->button()!=Qt::LeftButton || (event->modifiers() & Qt::ShiftModifier)) cancelGesture();
    setFocus(); lastPosition=pressPosition=event->position(); dragged=false;
    if(event->button()==Qt::LeftButton && modeChoice->currentIndex()!=0 && !(event->modifiers() & Qt::ShiftModifier) && snapshot.scene()) {
        const auto axis=modeChoice->currentIndex()==1 ? pickedAxis(event->position()) : -1;
        if(axis>=0) { const QSignalBlocker blocker(axisChoice); axisChoice->setCurrentIndex(axis); }
        else {
            const auto picked=renderer.pick(event->position(),viewport(),camera,*snapshot.scene());
            if(picked>=0 && picked!=selectedObject()) selectObject(picked);
            if(picked<0) { selectObject(-1); return; }
        }
        interaction.begin(*snapshot.scene(),selectedObject(),InteractionMode(modeChoice->currentIndex()),axisChoice->currentIndex(),
                          event->position(),camera,viewport(),commands);
        status->setText(interaction.message()); update();
    }
}
void SceneViewer::mouseMoveEvent(QMouseEvent* event)
{
    if(!(event->buttons() & (Qt::LeftButton|Qt::MiddleButton|Qt::RightButton))) return;
    const auto delta=event->position()-lastPosition;
    dragged=dragged || (event->position()-pressPosition).manhattanLength()>3;
    if(interaction.active() && ((event->buttons() & (Qt::MiddleButton|Qt::RightButton)) || (event->modifiers() & Qt::ShiftModifier))) cancelGesture();
    if(interaction.active()) {
        interaction.update(event->position(),event->modifiers() & Qt::ControlModifier);
        status->setText(interaction.message()); update(); return;
    }
    if((event->buttons() & Qt::LeftButton) && modeChoice->currentIndex()!=0 && !(event->modifiers() & Qt::ShiftModifier)) return;
    if((event->buttons() & Qt::MiddleButton) || (event->modifiers() & Qt::ShiftModifier)) camera.pan(delta,viewport());
    else { camera.orbit(delta); const QSignalBlocker blocker(viewChoice); viewChoice->setCurrentIndex(0); }
    lastPosition=event->position(); workspaceChanged();
}
void SceneViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button()!=Qt::LeftButton) return;
    if(interaction.active()) {
        interaction.update(event->position(),event->modifiers() & Qt::ControlModifier);
        const auto error=interaction.finish(commands); status->setText(error.isEmpty() ? "Transform updated. Undo restores the gesture." : error); update(); return;
    }
    if(dragged || !snapshot.scene() || modeChoice->currentIndex()!=0) return;
    selectObject(renderer.pick(event->position(),viewport(),camera,*snapshot.scene()));
}
void SceneViewer::wheelEvent(QWheelEvent* event) { cancelGesture(); camera.zoom(float(event->angleDelta().y()),event->position(),viewport()); workspaceChanged(); event->accept(); }
void SceneViewer::mouseDoubleClickEvent(QMouseEvent*) { frameScene(); }
void SceneViewer::keyPressEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_F) frameScene();
    else if(event->key()==Qt::Key_Escape) { cancelGesture(); status->setText("Gesture cancelled."); }
    else if(event->key()==Qt::Key_G) modeChoice->setCurrentIndex(1);
    else if(event->key()==Qt::Key_R) modeChoice->setCurrentIndex(2);
    else if(event->key()==Qt::Key_S) modeChoice->setCurrentIndex(3);
    else { OutputViewer::keyPressEvent(event); return; }
    event->accept();
}
}
