#pragma once

#include "tp_utils/CallbackCollection.h"
#include "tp_utils/MutexUtils.h"

#include <thread>

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_EXPORT AbstractCrossThreadCallback
{
  TP_NONCOPYABLE(AbstractCrossThreadCallback);
public:
  //################################################################################################
  AbstractCrossThreadCallback(std::function<void()> callback);

  //################################################################################################
  virtual ~AbstractCrossThreadCallback()=default;

  //################################################################################################
  virtual void call() = 0;

  //################################################################################################
  const std::function<void()>* callFunctor() const;

  //################################################################################################
  void operator()()
  {
    call();
  }

  //################################################################################################
  // Only call this from the target thread.
  void callDirectly()
  {
    callback();
  }

protected:
  //################################################################################################
  void callback() const;

private:
  std::function<void()> m_callback;
  const std::function<void()> m_callFunctor;
};
}

//##################################################################################################
typedef std::unique_ptr<tp_utils::AbstractCrossThreadCallback> TPCrossThreadCallback;

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_EXPORT AbstractCrossThreadCallbackFactory
{
  TP_NONCOPYABLE(AbstractCrossThreadCallbackFactory);
  std::thread::id m_mainThreadID{std::this_thread::get_id()};
public:

  //################################################################################################
  AbstractCrossThreadCallbackFactory()=default;

  //################################################################################################
  virtual ~AbstractCrossThreadCallbackFactory();

protected:
  //################################################################################################
  [[nodiscard]] virtual AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const = 0;

public:
  //################################################################################################
  [[nodiscard]] TPCrossThreadCallback produceP(const std::function<void()>& callback) const
  {
    return std::unique_ptr<tp_utils::AbstractCrossThreadCallback>(produce(callback));
  }

  //################################################################################################
  bool sameThread()
  {
    return m_mainThreadID == std::this_thread::get_id();
  }
};

//##################################################################################################
template<typename T>
class CrossThreadCallbackFactoryTemplate : public AbstractCrossThreadCallbackFactory
{
protected:
  AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const override
  {
    return new T(callback);
  }
};

//##################################################################################################
class TP_UTILS_EXPORT PolledCrossThreadCallbackFactory: public AbstractCrossThreadCallbackFactory
{
public:
  //################################################################################################
  PolledCrossThreadCallbackFactory();

protected:
  //################################################################################################
  AbstractCrossThreadCallback* produce(const std::function<void()>& callback) const override;

public:
  //################################################################################################
  Callback<void()> poll;

private:
  mutable CallbackCollection<void()> m_pollAll;
};

//##################################################################################################
void blockingCrossThreadCall(TPCTCFactory factory, const std::function<void()>& callback);

//##################################################################################################
template<typename T>
class CrossThreadCallbackWithPayload
{
  TPMutex m_mutex{TPM};
  std::vector<T> m_payloads;
  std::function<void(T)> m_callback;
  TPCrossThreadCallback m_crossThreadCallback;
public:
  //################################################################################################
  CrossThreadCallbackWithPayload(TPCTCFactory factory, const std::function<void(T)>& callback):
    m_callback(callback)
  {
    m_crossThreadCallback = factory->produceP([&]
    {
      m_mutex.lock(TPM);
      T payload = tpTakeFirst(m_payloads);
      m_mutex.unlock(TPM);
      m_callback(payload);
    });
  }

  //################################################################################################
  void call(const T& payload)
  {
    m_mutex.lock(TPM);
    m_payloads.push_back(payload);
    m_mutex.unlock(TPM);
    (*m_crossThreadCallback)();
  }

  //################################################################################################
  void operator()(const T& payload)
  {
    call(payload);
  }
};

}
