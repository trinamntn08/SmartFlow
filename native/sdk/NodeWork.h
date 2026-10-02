#pragma once
#include <algorithm>
#include <atomic>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace smartflow {
namespace detail {
struct NodeWorkContext {
    size_t threads=1;
    std::function<bool()> cancelled;
};
inline thread_local const NodeWorkContext* nodeWorkContext=nullptr;
// Installed by the adapter, and by the helper for nested serial work.
class NodeWorkScope {
public:
    explicit NodeWorkScope(const NodeWorkContext& context) : previous(nodeWorkContext) { nodeWorkContext=&context; }
    ~NodeWorkScope() { nodeWorkContext=previous; }
    NodeWorkScope(const NodeWorkScope&)=delete;
    NodeWorkScope& operator=(const NodeWorkScope&)=delete;
private:
    const NodeWorkContext* previous;
};
}
inline size_t nodeThreadLimit() { return detail::nodeWorkContext ? detail::nodeWorkContext->threads : 1; }
inline bool nodeWorkCancelled() {
    return detail::nodeWorkContext && detail::nodeWorkContext->cancelled && detail::nodeWorkContext->cancelled();
}

// Partition owned data by index. The caller participates; child workers are
// joined before return/throw. Nested calls run serially, never blocking on the
// graph pool. Returns false if cancelled. Exceptions propagate after joining.
inline bool parallelFor(size_t count, const std::function<void(size_t)>& operation)
{
    if(!count) return !nodeWorkCancelled();
    const size_t threads=std::min(count,nodeThreadLimit());
    detail::NodeWorkContext nested{1,detail::nodeWorkContext ? detail::nodeWorkContext->cancelled : std::function<bool()>{}};
    detail::NodeWorkScope callerScope(nested);
    std::atomic<size_t> next{0};
    std::atomic_bool stop{false};
    std::exception_ptr exception;
    std::mutex exceptionMutex;
    auto work=[&] {
        detail::NodeWorkScope childScope(nested);
        try {
            while(!stop.load() && !nodeWorkCancelled()) {
                const auto index=next.fetch_add(1);
                if(index>=count) break;
                operation(index);
            }
        } catch(...) {
            { std::lock_guard<std::mutex> lock(exceptionMutex); if(!exception) exception=std::current_exception(); }
            stop.store(true);
        }
    };
    std::vector<std::thread> workers;
    try { for(size_t i=1;i<threads;++i) workers.emplace_back(work); }
    catch(...) {
        stop.store(true);
        for(auto& worker : workers) worker.join();
        throw;
    }
    work();
    for(auto& worker : workers) worker.join();
    if(exception) std::rethrow_exception(exception);
    return !nodeWorkCancelled();
}
} // namespace smartflow
