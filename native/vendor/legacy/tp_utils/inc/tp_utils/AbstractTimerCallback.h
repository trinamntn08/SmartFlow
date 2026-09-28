#ifndef tp_utils_AbstractTimerCallback_h
#define tp_utils_AbstractTimerCallback_h

#include "tp_utils/CallbackCollection.h"

namespace tp_utils
{

//##################################################################################################
class TP_UTILS_EXPORT AbstractTimerCallback
{
  TP_NONCOPYABLE(AbstractTimerCallback);
public:
  //################################################################################################
  AbstractTimerCallback(std::function<void()> callback, int64_t timeOutMS);

  //################################################################################################
  virtual ~AbstractTimerCallback();

  //################################################################################################
  virtual void setTimeOutMS(int64_t timeOutMS);

  //################################################################################################
  int64_t timeOutMS();

protected:
  //################################################################################################
  void callback() const;

private:
  std::function<void()> m_callback;
  int64_t m_timeOutMS;
};

//##################################################################################################
class TP_UTILS_EXPORT AbstractTimerCallbackFactory
{
  TP_NONCOPYABLE(AbstractTimerCallbackFactory);
public:

  //################################################################################################
  AbstractTimerCallbackFactory()=default;

  //################################################################################################
  virtual ~AbstractTimerCallbackFactory();

  //################################################################################################
  virtual AbstractTimerCallback* produce(const std::function<void()>& callback, int64_t timeOutMS) const=0;
};

//##################################################################################################
template<typename T>
class TimerCallbackFactoryTemplate : public AbstractTimerCallbackFactory
{
  AbstractTimerCallback* produce(const std::function<void()>& callback, int64_t timeOutMS) const override
  {
    return new T(callback, timeOutMS);
  }
};

//##################################################################################################
class TP_UTILS_EXPORT PolledTimerCallbackFactory: public AbstractTimerCallbackFactory
{
public:
  //################################################################################################
  PolledTimerCallbackFactory();

  //################################################################################################
  AbstractTimerCallback* produce(const std::function<void()>& callback, int64_t timeOutMS) const override;

  //################################################################################################
  Callback<void()> poll;

private:
  mutable CallbackCollection<void()> m_pollAll;
};

}

#endif
