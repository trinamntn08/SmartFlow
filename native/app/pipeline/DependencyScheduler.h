#pragma once
#include "PipelineExecution.h"
#include <functional>

namespace smartflow {
ExecutionResult executeScheduled(tp_pipeline::PipelineDetails& graph,
    const tp_pipeline::StepDelegateMap& delegates, const tp_data::CollectionFactory& factory,
    const std::vector<ResultGroup>& groups, const ExecutionOptions& options,
    const std::function<bool()>& cancelled);
}
