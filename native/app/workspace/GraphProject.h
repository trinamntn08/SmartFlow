#pragma once
#include "project/DocumentHistory.h"
#include <QObject>
#include <map>

namespace smartflow {
// Retained documents are authoritative. step() returns detached inspector
// copies, valid only until the next semantic change.
class GraphProject : public QObject {
    Q_OBJECT
public:
    explicit GraphProject(std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
                          std::vector<NodePresentation> presentations);
    QString title(const tp_utils::StringID& type) const;
    QString category(const tp_utils::StringID& type) const;
    const auto& graph() const { return history.session().executableGraph(); }
    const auto& resultGroups() const { return history.session().resultGroups(); }
    const auto& retained() const { return history.session().document(); }
    const auto& selectedGraph() const { return history.session().selectedGraph(); }
    const auto& diagnostics() const { return history.session().diagnostics(); }
    auto registry() const { return delegates; }
    const auto& nodePresentations() const { return presentations; }
    auto& commands() { return history; }
    quint64 revision() const { return history.revision(); }
    tp_pipeline::StepDetails* step(const tp_utils::StringID& id) const;
    tp_pipeline::StepDetails* create(const tp_utils::StringID& type);
    void remove(const tp_utils::StringID& id);
    bool setParameter(const tp_utils::StringID& id, const tp_pipeline::Parameter& parameter, quint64 group=0);
    bool connectInput(const tp_utils::StringID& target, size_t input,
                      const tp_utils::StringID& source, size_t output);
    void disconnectInput(const tp_utils::StringID& target, size_t input);
    std::string connectionId(const tp_utils::StringID& target, size_t input) const;
Q_SIGNALS:
    void changed();
private:
    void refreshInspection();
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates;
    std::vector<NodePresentation> presentations;
    project::DocumentHistory history;
    std::map<std::string,std::unique_ptr<tp_pipeline::StepDetails>> inspection;
};
std::shared_ptr<tp_pipeline::StepDelegateMap> numericDelegates();
std::vector<NodePresentation> numericPresentations();
QString nodeTitle(const tp_utils::StringID& type);
} // namespace smartflow
