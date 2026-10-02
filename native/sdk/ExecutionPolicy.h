#pragma once

#include <tp_utils/StringID.h>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace smartflow {

// Source-level contracts for trusted native extensions, not a plugin ABI.
// Reentrant delegates use only invocation-local state and owned inputs/outputs.
// Unknown delegates serialize. Resource names coordinate exclusive local work.
struct NodeExecutionPolicy {
    bool reentrant = false;
    std::vector<std::string> exclusiveResources;
    size_t threadLimit = 1;
};
using NodeExecutionPolicies = std::unordered_map<tp_utils::StringID, NodeExecutionPolicy>;

enum class ExecutionMode { Sequential, Parallel };
struct ExecutionOptions {
    ExecutionMode mode = ExecutionMode::Sequential;
    // Total delegate-thread budget, including node-internal work; range 1..64.
    size_t maxThreads = 1;
    NodeExecutionPolicies policies;
};

} // namespace smartflow
