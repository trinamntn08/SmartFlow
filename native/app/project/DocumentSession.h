#pragma once
#include "ProjectFile.h"
#include <WorkspaceExtension.h>
#include <tp_pipeline/PipelineDetails.h>

namespace smartflow::project {
// Retains the complete file independently of the executable projection.
// Construct a replacement session before discarding the current one.
// Delegate registrations must remain immutable for the session's lifetime.
class DocumentSession {
public:
    DocumentSession(Document source, std::string graphId,
                    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
                    std::vector<NodePresentation> registrations);
    const Document& document() const { return source; }
    const std::vector<std::string>& diagnostics() const { return issues; }
    // Never return a partial graph that silently excludes unsupported content.
    const tp_pipeline::PipelineDetails& executableGraph() const;
    // Explicit single-field edit. All other JSON, including extra graphs and
    // workspace state, remains unchanged. Unsupported parameter codecs reject.
    void setParameter(const std::string& nodeId, const std::string& name, const Document& value);
private:
    void compile();
    Document source;
    size_t graphIndex = 0;
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates;
    std::vector<NodePresentation> registrations;
    std::unique_ptr<tp_pipeline::PipelineDetails> compiled;
    std::vector<std::string> issues;
};
} // namespace smartflow::project
