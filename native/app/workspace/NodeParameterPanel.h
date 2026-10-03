#pragma once
#include "GraphProject.h"
#include <QPointer>
#include <QWidget>
#include <vector>

class QDoubleSpinBox;
class QSlider;
class QLabel;
class QVBoxLayout;
namespace smartflow {
// App-owned controls. No StepDetails pointers survive a semantic revision.
class NodeParameterPanel final : public QWidget {
public:
    NodeParameterPanel(GraphProject& project,tp_utils::StringID node);
    void refresh();
    void setTiming(const QString& text);
private:
    void commit(const tp_utils::StringID& name,double value,quint64 group=0);
    struct Field { tp_utils::StringID name; QPointer<QDoubleSpinBox> editor; QPointer<QSlider> slider; };
    QPointer<GraphProject> project;
    tp_utils::StringID node;
    QWidget* controls=nullptr;
    QVBoxLayout* layout;
    QLabel* timing;
    std::vector<Field> fields;
    std::vector<std::string> shape;
    quint64 revision=0;
    bool refreshing=false;
    bool applyingSlider=false;
};
}
