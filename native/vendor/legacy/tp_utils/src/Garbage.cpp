#include "tp_utils/Garbage.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/detail/StaticState.h"

#include "lib_platform/SetThreadName.h"

#include <thread>
#include <queue>

namespace tp_utils
{

#ifndef TP_NO_THREADS
namespace
{
//##################################################################################################
StaticState::Garbage& getStaticState()
{
  return StaticState::instance()->garbage;
}
}
#endif

//##################################################################################################
struct Garbage::Private
{
  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool finish{false};
  std::vector<std::thread*> threads;
  std::queue<std::function<void()>> queue;

  //################################################################################################
  void addThread()
  {
    threads.push_back(new std::thread([&]
    {
      lib_platform::setThreadName("Garbage");
      TPMutexLocker lock(mutex);
      while(!finish || !queue.empty())
      {
        if(queue.empty())
          waitCondition.wait(TPMc lock);
        else
        {
          auto closure = queue.front();
          queue.pop();
          {
            TP_MUTEX_UNLOCKER(lock);
            closure();
          }
        }
      }
    }));
  }

  //################################################################################################
  void setThreads(size_t nThreads)
  {
    while(threads.size()<nThreads)
      addThread();
  }
};

//##################################################################################################
Garbage::Garbage():
  d(new Private)
{
#ifndef TP_NO_THREADS
  auto& staticState = getStaticState();
  staticState.instanceMutex.locked(TPMc [&]
  {
    if(!staticState.instance)
      staticState.instance = this;
  });
#endif

  d->setThreads(4);
}

//##################################################################################################
Garbage::~Garbage()
{
#ifndef TP_NO_THREADS
  auto& staticState = getStaticState();
  staticState.instanceMutex.locked(TPMc [&]{staticState.instance = nullptr;});
#endif

  d->mutex.locked(TPMc [&]{d->finish = true;});
  d->waitCondition.wakeAll();
  while(!d->threads.empty())
  {
    auto thread = tpTakeLast(d->threads);
    thread->join();
    delete thread;
  }

  delete d;
}

//##################################################################################################
void Garbage::garbage(const std::function<void()>& closure)
{
  d->mutex.locked(TPMc [&]{d->queue.push(closure);});
  d->waitCondition.wakeOne();
}

//##################################################################################################
void Garbage::increaseThreads(size_t nThreads)
{
  d->setThreads(nThreads);
}

//##################################################################################################
void garbage(const std::function<void()>& closure)
{
#ifndef TP_NO_THREADS
  auto& staticState = getStaticState();
  {
    TP_MUTEX_LOCKER(staticState.instanceMutex);
    if(staticState.instance)
    {
      staticState.instance->garbage(closure);
      return;
    }
  }
#endif

  closure();
}

}
