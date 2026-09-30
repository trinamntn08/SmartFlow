#pragma once
#include <tp_pipeline/PipelineDetails.h>
#include <tp_pipeline/StepDelegateMap.h>
#include <QObject>
#include <QJsonObject>
#include <memory>

namespace smartflow {

// Semantic project state. Canvas identities, positions and execution results
// are deliberately kept outside this model. Not a persisted project format.
class GraphProject : public QObject {
    Q_OBJECT
public:
    explicit GraphProject(std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates);
    const tp_pipeline::PipelineDetails& graph() const { return document; }
    auto registry() const { return delegates; }
    quint64 revision() const { return currentRevision; }
    tp_pipeline::StepDetails* step(const tp_utils::StringID& id) const;
    tp_pipeline::StepDetails* create(const tp_utils::StringID& type);
    void remove(const tp_utils::StringID& id);
    bool setParameter(const tp_utils::StringID& id, const tp_pipeline::Parameter& parameter);
    void connectInput(const tp_utils::StringID& target, size_t input,
                      const tp_utils::StringID& source, size_t output);
    void disconnectInput(const tp_utils::StringID& target, size_t input);
    QJsonObject capture(const tp_utils::StringID& id) const;
    tp_pipeline::StepDetails* restore(const QJsonObject& snapshot);
Q_SIGNALS:
    void changed();
private:
    void modified();
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates;
    tp_pipeline::PipelineDetails document;
    quint64 currentRevision = 0;
};

std::shared_ptr<tp_pipeline::StepDelegateMap> numericDelegates();
QString nodeTitle(const tp_utils::StringID& type);

} // namespace smartflow
