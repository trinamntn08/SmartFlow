#include "tp_utils/AbstractCrossThreadCallback.h"
#include "tp_utils/MutexUtils.h"

#include <utility>

namespace tp_utils
{

//##################################################################################################
AbstractCrossThreadCallback::AbstractCrossThreadCallback(std::function<void()> callback):
  m_callback(std::move(callback)),
  m_callFunctor([this]{call();})
{

}

//##################################################################################################
const std::function<void()>* AbstractCrossThreadCallback::callFunctor() const
{
  return &m_callFunctor;
}

//##################################################################################################
void AbstractCrossThreadCallback::callback() const
{
  m_callback();
}

//##################################################################################################
AbstractCrossThreadCallbackFactory::~AbstractCrossThreadCallbackFactory() = default;

namespace
{
//##################################################################################################
class PolledCrossThreadCallback: public AbstractCrossThreadCallback
{
public:
  //################################################################################################
  PolledCrossThreadCallback(const std::function<void()>& callback_):
    AbstractCrossThreadCallback(callback_)
  {
    poll.setCallback([this]()
    {
      if(m_inCallback)
        return;

      m_inCallback=true;
      TP_CLEANUP([&]{m_inCallback=false;});

      for(;;)
      {
        size_t c = m_mutex.locked(TPMc[&]{return std::exchange(m_count, 0);});

        if(c==0)
          return;

        for(; c; c--)
          callback();
      }
    });
  }

  //################################################################################################
  void call() override
  {
    TP_MUTEX_LOCKER(m_mutex);
    m_count++;
  }

  Callback<void()> poll;

private:
  TPMutex m_mutex{TPM};
  size_t m_count{0};
  bool m_inCallback{false};
};

}

//##################################################################################################
PolledCrossThreadCallbackFactory::PolledCrossThreadCallbackFactory()
{
  poll.setCallback([&]{m_pollAll();});
}

//##################################################################################################
AbstractCrossThreadCallback* PolledCrossThreadCallbackFactory::produce(const std::function<void()>& callback) const
{
  auto c = new PolledCrossThreadCallback(callback);
  c->poll.connect(m_pollAll);
  return c;
}

//##################################################################################################
void blockingCrossThreadCall(TPCTCFactory factory, const std::function<void()>& callback)
{
  if(factory->sameThread())
    return callback();

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  bool called{false};

  TPCrossThreadCallback crossThreadCallback = factory->produceP([&]
  {
    callback();
    TP_MUTEX_LOCKER(mutex);
    called = true;
    waitCondition.wakeAll();
  });

  (*crossThreadCallback)();

  {
    TPMutexLocker lock(mutex);
    if(!called)
      waitCondition.wait(TPMc lock);
  }
}

}
