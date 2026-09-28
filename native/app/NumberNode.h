#pragma once
#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QDoubleSpinBox>
#include <QPointer>

class NumberData final : public QtNodes::NodeData
{
public:
    explicit NumberData(double value) : value(value) {}
    QtNodes::NodeDataType type() const override { return {"number", "Number"}; }
    double value;
};

// Temporary non-3D adapter for validating the copied canvas independently.
class NumberNode final : public QtNodes::NodeDelegateModel
{
public:
    QString name() const override { return "Number"; }
    QString caption() const override { return "Number / pass through"; }
    unsigned int nPorts(QtNodes::PortType) const override { return 1; }
    QtNodes::NodeDataType dataType(QtNodes::PortType, QtNodes::PortIndex) const override
    { return {"number", "Number"}; }
    void setInData(std::shared_ptr<QtNodes::NodeData> data, QtNodes::PortIndex) override
    {
        input = std::dynamic_pointer_cast<NumberData>(data);
        if (editor) editor->setEnabled(!input);
        Q_EMIT dataUpdated(0);
    }
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex) override
    { return input ? input : std::make_shared<NumberData>(value); }
    QWidget* embeddedWidget() override
    {
        if (!editor) {
            editor = new QDoubleSpinBox;
            editor->setRange(-1000000, 1000000);
            editor->setValue(value);
            editor->setEnabled(!input);
            connect(editor, &QDoubleSpinBox::valueChanged, this, [this](double next) {
                value = next;
                Q_EMIT dataUpdated(0);
            });
        }
        return editor;
    }
    QJsonObject save() const override
    {
        auto result = NodeDelegateModel::save();
        result["value"] = value;
        return result;
    }
    void load(QJsonObject const& json) override
    {
        value = json["value"].toDouble();
        if (editor) editor->setValue(value);
        Q_EMIT dataUpdated(0);
    }
private:
    double value = 1;
    std::shared_ptr<NumberData> input;
    QPointer<QDoubleSpinBox> editor;
};
