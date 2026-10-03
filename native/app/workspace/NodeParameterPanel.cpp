#include "NodeParameterPanel.h"
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QScopedValueRollback>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSlider>
#include <QMouseEvent>
#include <algorithm>
#include <functional>
#include <cmath>

namespace smartflow {
namespace {
class ParameterSlider final : public QSlider {
public:
    explicit ParameterSlider(QWidget* parent) : QSlider(Qt::Horizontal,parent) { setRange(0,10000); }
    std::function<void()> cancel;
    void synchronize(double minimum,double maximum,double step,double value)
    {
        const QSignalBlocker blocker(this);
        if(isSliderDown()) discardPointer=true;
        setSliderDown(false);
        if(!initialized || minimum!=domainMinimum || maximum!=domainMaximum || value<lower || value>upper) {
            domainMinimum=minimum; domainMaximum=maximum; lower=minimum; upper=maximum; initialized=true;
            // Keep huge numeric domains useful for dragging; typing can move the window.
            if(maximum-minimum>10000) {
                const auto span=std::min(maximum-minimum,std::max(200.0,200*step));
                lower=std::clamp(value-span/2,minimum,maximum-span); upper=lower+span;
            }
        }
        const auto span=upper-lower;
        setEnabled(span>0);
        setSingleStep(span>0 ? int(std::clamp(std::round(step/span*10000),1.0,10000.0)) : 1);
        setPageStep(std::min(10000,singleStep()*10));
        setValue(span>0 ? int(std::round((value-lower)/span*10000)) : 0);
        setToolTip(QString("Drag to adjust (%1 to %2). Release to apply; Escape cancels.").arg(lower).arg(upper));
    }
    double number() const { return lower+(upper-lower)*value()/10000.0; }
protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        discardPointer=false; QSlider::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override
    {
        if(discardPointer) event->accept(); else QSlider::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if(discardPointer) {
            const QSignalBlocker blocker(this);
            QSlider::mouseReleaseEvent(event); discardPointer=false;
        } else QSlider::mouseReleaseEvent(event);
    }
    bool event(QEvent* event) override
    {
        if(event->type()==QEvent::ShortcutOverride && static_cast<QKeyEvent*>(event)->key()==Qt::Key_Escape) {
            event->accept(); return true;
        }
        return QSlider::event(event);
    }
    void keyPressEvent(QKeyEvent* event) override
    {
        if(event->key()==Qt::Key_Escape) { if(cancel) cancel(); event->accept(); }
        else QSlider::keyPressEvent(event);
    }
    void wheelEvent(QWheelEvent* event) override
    {
        if(hasFocus()) QSlider::wheelEvent(event); else event->ignore();
    }
private:
    double lower=0,upper=1,domainMinimum=0,domainMaximum=1;
    bool initialized=false,discardPointer=false;
};
class CompactNumber final : public QDoubleSpinBox {
public:
    using QDoubleSpinBox::QDoubleSpinBox;
    QSize sizeHint() const override { return {78,26}; }
    QSize minimumSizeHint() const override { return {60,26}; }
    void setCommittedValue(double next)
    {
        setValue(next);
        lineEdit()->setText(textFromValue(value()));
    }
protected:
    bool event(QEvent* event) override
    {
        if(event->type()==QEvent::ShortcutOverride && static_cast<QKeyEvent*>(event)->key()==Qt::Key_Escape) {
            event->accept(); // Cancel the field draft before the canvas deselect shortcut.
            return true;
        }
        return QDoubleSpinBox::event(event);
    }
    QString textFromValue(double value) const override
    {
        auto text=QDoubleSpinBox::textFromValue(value);
        const auto separator=locale().decimalPoint();
        if(text.contains(separator)) {
            while(text.endsWith('0')) text.chop(1);
            if(text.endsWith(separator)) text.chop(separator.size());
        }
        return text;
    }
    void wheelEvent(QWheelEvent* event) override
    {
        if(hasFocus()) QDoubleSpinBox::wheelEvent(event);
        else event->ignore(); // The canvas can zoom without accidentally editing.
    }
    void keyPressEvent(QKeyEvent* event) override
    {
        if(event->key()==Qt::Key_Escape) {
            lineEdit()->setText(textFromValue(value()));
            event->accept();
        }
        else QDoubleSpinBox::keyPressEvent(event);
    }
};
}
NodeParameterPanel::NodeParameterPanel(GraphProject& document,tp_utils::StringID id)
    : project(&document),node(std::move(id))
{
    setObjectName("nodeParameters");
    setProperty("projectNodeId",QString::fromStdString(node.toString()));
    setFixedWidth(164);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    setStyleSheet("QWidget { color: #e0e6eb; background: transparent; font-size: 12px; }"
                  "QWidget#nodeParameterRow { background: #3e434b; border-radius: 3px; }"
                  "QDoubleSpinBox { border: none; padding: 0; }"
                  "QDoubleSpinBox:focus { background: #4c5969; border-radius: 2px; }"
                  "QLabel#nodeExecutionTime { color: #a5adb8; font-size: 10px; }"
                  "QSlider::groove:horizontal { height: 3px; background: #252a32; border-radius: 1px; }"
                  "QSlider::sub-page:horizontal { background: #7ba8cb; border-radius: 1px; }"
                  "QSlider::handle:horizontal { width: 8px; margin: -3px 0; background: #b5cde0; border-radius: 3px; }"
                  "QDoubleSpinBox:disabled { color: #929aa5; }");
    layout=new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0); layout->setSpacing(8);
    timing=new QLabel("Run: -",this); timing->setObjectName("nodeExecutionTime");
    timing->setAttribute(Qt::WA_TransparentForMouseEvents); timing->setAlignment(Qt::AlignLeft);
    timing->setToolTip("Invocation wall time; excludes ready-queue waiting. Components include their complete body span.");
    layout->addWidget(timing);
    refresh();
}
void NodeParameterPanel::refresh()
{
    const QScopedValueRollback<bool> guard(refreshing,true);
    auto* step=project ? project->step(node) : nullptr;
    if(!step) { setEnabled(false); return; }
    setEnabled(true); revision=project->revision();
    std::vector<tp_pipeline::Parameter> parameters;
    std::vector<std::string> nextShape;
    for(const auto& name : step->orderedParameterNames()) {
        const auto parameter=step->parameter(name);
        parameters.push_back(parameter);
        nextShape.push_back(name.toString()+"/"+parameter.type.toString());
    }
    if(!controls || nextShape!=shape) {
        if(controls) { layout->removeWidget(controls); controls->hide(); controls->deleteLater(); }
        fields.clear(); shape=std::move(nextShape);
        controls=new QWidget(this); auto* rows=new QVBoxLayout(controls);
        rows->setContentsMargins(0,0,0,0); rows->setSpacing(4);
        layout->insertWidget(0,controls);
        for(const auto& parameter : parameters) {
            auto* rowWidget=new QWidget(controls); rowWidget->setObjectName("nodeParameterRow");
            rows->addWidget(rowWidget);
            auto* stack=new QVBoxLayout(rowWidget); stack->setContentsMargins(8,0,8,1); stack->setSpacing(0);
            auto* row=new QHBoxLayout; row->setSpacing(4); stack->addLayout(row);
            const auto name=QString::fromStdString(parameter.name.toString());
            auto title=name; if(!title.isEmpty()) title[0]=title[0].toUpper();
            auto* label=new QLabel(title,rowWidget); label->setTextFormat(Qt::PlainText);
            label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            label->setFixedWidth(66); label->setToolTip(name); row->addWidget(label);
            if(parameter.type!=tp_pipeline::doubleSID() || !std::holds_alternative<double>(parameter.value)) {
                auto* retained=new QLabel("Read-only",rowWidget); retained->setToolTip("Editor unavailable; saved parameter retained.");
                row->addWidget(retained,1); continue;
            }
            auto* editor=new CompactNumber(rowWidget); editor->setObjectName("nodeParameter_"+name);
            editor->setAccessibleName(name); editor->setDecimals(10); editor->setKeyboardTracking(false);
            editor->setMinimumWidth(0); editor->setFixedHeight(20); editor->setAlignment(Qt::AlignRight);
            editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
            editor->setFocusPolicy(Qt::StrongFocus); editor->setGroupSeparatorShown(false);
            editor->setToolTip(name+" — Enter or leave the field to apply; Up/Down adjusts immediately.");
            label->setBuddy(editor); row->addWidget(editor,1);
            auto* slider=new ParameterSlider(rowWidget); slider->setObjectName("nodeSlider_"+name);
            slider->setAccessibleName(title+" slider"); slider->setFixedHeight(10); slider->setFocusPolicy(Qt::StrongFocus);
            stack->addWidget(slider); fields.push_back({parameter.name,editor,slider});
            slider->cancel=[this] { refresh(); };
            connect(slider,&QSlider::valueChanged,this,[this,editor,slider,id=parameter.name](int) {
                if(refreshing) return;
                if(slider->isSliderDown()) {
                    const QSignalBlocker blocker(editor);
                    editor->setCommittedValue(slider->number());
                } else commit(id,slider->number());
            });
            connect(slider,&QSlider::sliderReleased,this,[this,slider,id=parameter.name] { commit(id,slider->number()); });
            connect(editor,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[this,id=parameter.name](double value) { commit(id,value); });
        }
        controls->setVisible(!parameters.empty());
    }
    for(const auto& field : fields) {
        if(!field.editor) continue;
        const auto parameter=step->parameter(field.name);
        const QSignalBlocker blocker(field.editor);
        field.editor->setRange(tpGetVariantValue<double>(parameter.min,-1000000.0),tpGetVariantValue<double>(parameter.max,1000000.0));
        field.editor->setSingleStep(tpGetVariantValue<double>(parameter.step,1.0));
        field.editor->setEnabled(parameter.enabled);
        static_cast<CompactNumber*>(field.editor.data())->setCommittedValue(tpGetVariantValue<double>(parameter.value));
        auto* slider=static_cast<ParameterSlider*>(field.slider.data());
        slider->synchronize(field.editor->minimum(),field.editor->maximum(),field.editor->singleStep(),field.editor->value());
        slider->setEnabled(parameter.enabled && field.editor->minimum()<field.editor->maximum());
    }
    layout->activate(); setFixedHeight(sizeHint().height());
}
void NodeParameterPanel::commit(const tp_utils::StringID& name,double value)
{
    if(refreshing || !project || !std::isfinite(value)) return;
    if(revision!=project->revision()) { refresh(); return; }
    auto* step=project->step(node); if(!step) return;
    auto parameter=step->parameter(name);
    if(!parameter.enabled || parameter.type!=tp_pipeline::doubleSID()) return;
    parameter.value=value;
    if(!project->setParameter(node,parameter)) refresh();
}
void NodeParameterPanel::setTiming(const QString& text) { timing->setText(text); }
}
