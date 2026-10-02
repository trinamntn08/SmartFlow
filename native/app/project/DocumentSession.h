#pragma once
#include "ProjectFile.h"
#include "GraphComponent.h"
#include <WorkspaceExtension.h>
#include <tp_pipeline/PipelineDetails.h>
#include "pipeline/PipelineExecution.h"
#include <map>

namespace smartflow::project {
// Retains the complete file independently of the executable projection.
// Construct a replacement session before discarding the current one.
// Delegate registrations must remain immutable for the session's lifetime.
class DocumentSession {
public:
    DocumentSession(Document source, std::string graphId,
                    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
                    std::vector<NodePresentation> registrations);
    static DocumentSession empty(const std::string& projectId, const std::string& graphId,
                    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
                    std::vector<NodePresentation> registrations);
    const Document& document() const { return source; }
    const std::vector<std::string>& diagnostics() const { return issues; }
    const Document& selectedGraph() const { return source["project"]["graphs"][graphIndex]; }
    // Detached inspector copy; never exposes the mutable compiled projection.
    std::unique_ptr<tp_pipeline::StepDetails> inspectNode(const std::string& id) const;
    // Never return a partial graph that silently excludes unsupported content.
    const tp_pipeline::PipelineDetails& executableGraph() const;
    const auto& resultGroups() const { return groups; }
    // Explicit single-field edit. All other JSON, including extra graphs and
    // workspace state, remains unchanged. Unsupported parameter codecs reject.
    void setParameter(const std::string& nodeId, const std::string& name, const Document& value);
    // Callers allocate stable IDs once, so command redo can reuse them.
    void createNode(const std::string& nodeId, const std::string& registeredType);
    // Explicit deletion removes this node and all incident edges, including
    // unavailable ones. Unrelated opaque content is left untouched.
    void removeNode(const std::string& nodeId);
    void connect(const std::string& connectionId,
                 const std::string& sourceNode, const std::string& sourcePort,
                 const std::string& targetNode, const std::string& targetPort);
    void disconnect(const std::string& connectionId);
    void catalogComponent(const GraphComponent& component);
    ComponentBindings instantiateComponent(const GraphComponent& component, const std::string& instanceId,
        const Document& controls, const Document& inputSources, bool collapsed = false);
    // Replace only an explicitly owned top-level workspace field. This does not
    // recompile the graph or alter its transient output identities.
    void setWorkspaceField(const std::string& name, const Document& value);
private:
    void replace(Document replacement);
    void compile();
    Document source;
    size_t graphIndex = 0;
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates;
    std::vector<NodePresentation> registrations;
    std::unique_ptr<tp_pipeline::PipelineDetails> compiled;
    std::vector<std::string> issues;
    std::map<std::string,std::unique_ptr<tp_pipeline::StepDetails>> components;
    std::vector<ResultGroup> groups;
};
} // namespace smartflow::project
