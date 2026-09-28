#ifndef tp_utils_CallbackCollection_h
#define tp_utils_CallbackCollection_h

#include "tp_utils/Globals.h"

#include <memory>
#include <cassert>

namespace tp_utils
{

//##################################################################################################
template <typename T>
class CallbackCollection;

//##################################################################################################
template<typename T>
class Callback;

//##################################################################################################
template<typename R, typename... Args>
class CallbackCollection<R(Args...)>
{
  TP_NONCOPYABLE(CallbackCollection);
  template<typename> friend class Callback;
  public:
  using T = R(Args...);

  //################################################################################################
  CallbackCollection() = default;

  //################################################################################################
  ~CallbackCollection()
  {
    clear();
  }

  //################################################################################################
  void clear()
  {
    for(auto unrefCallback : m_unrefCallbacks)
      (*unrefCallback)(this, nullptr);

    tpDeleteAll(m_nonRemovableCallbacks);

    m_unrefCallbacks.clear();
    m_nonRemovableCallbacks.clear();
  }

  //################################################################################################
  void addCallback(std::function<T>* callback)
  {
    assert(callback);
    m_callbacks.push_back(callback);
  }

  //################################################################################################
  void addCallback(std::function<T> callback)
  {
    assert(callback);
    m_nonRemovableCallbacks.push_back(new std::function<T>(callback));
    m_callbacks.push_back(m_nonRemovableCallbacks.back());
  }

  //################################################################################################
  void removeCallback(std::function<T>* callback)
  {
    assert(callback);
    tpRemoveOne(m_callbacks, callback);
  }

  //################################################################################################
  R operator()(Args... args) const
  {
    if(m_callbacks.empty())
      return;

    size_t remaining = m_callbacks.size();

    auto decrement = [&](size_t& r)
    {
      r--;
      if(m_callbacks.empty())
      {
        r=0;
        return;
      }

      if(r>=m_callbacks.size())
        r = m_callbacks.size()-1;
    };

    for(size_t r=remaining; r>0; decrement(r))
      if(const auto& c=*m_callbacks.at(m_callbacks.size()-r); c)
        c(args...);

    return; //Force void
  }

  //################################################################################################
  void swap(CallbackCollection<T>& other)
  {
    std::swap(m_nonRemovableCallbacks, other.m_nonRemovableCallbacks);
    std::swap(m_callbacks, other.m_callbacks);
    std::swap(m_unrefCallbacks, other.m_unrefCallbacks);

    for(const auto& callback : m_unrefCallbacks)
      (*callback)(&other, this);

    for(const auto& callback : other.m_unrefCallbacks)
      (*callback)(this, &other);
  }

  private:
  std::vector<std::function<T>*> m_nonRemovableCallbacks;
  std::vector<std::function<T>*> m_callbacks;
  std::vector<std::function<void(CallbackCollection<T>*, CallbackCollection<T>*)>*> m_unrefCallbacks;
};

//##################################################################################################
template<typename R, typename... Args>
class Callback<R(Args...)>
{
  TP_NONCOPYABLE(Callback);
  using C = CallbackCollection<R(Args...)>*;
  public:
  using T = R(Args...);

  //################################################################################################
  Callback()
  {

  }

  //################################################################################################
  Callback(const std::function<T>& callback):
    m_callback(callback)
  {

  }

  //################################################################################################
  template<typename F>
  Callback(const F& callback):
    m_callback(callback)
  {

  }

  //################################################################################################
  ~Callback()
  {
    disconnect();
  }

  //################################################################################################
  void disconnect()
  {
    for(auto c : m_collections)
    {
      tpRemoveOne(c->m_unrefCallbacks, &m_unrefCallback);
      c->removeCallback(&m_callback);
    }
    m_collections.clear();
  }

  //################################################################################################
  void setCallback(const std::function<T>& callback)
  {
    m_callback = callback;
  }

  //################################################################################################
  const std::function<T>& callback()
  {
    return m_callback;
  }

  //################################################################################################
  void connect(CallbackCollection<T>& collection)
  {
    m_collections.push_back(&collection);
    collection.m_unrefCallbacks.push_back(&m_unrefCallback);
    collection.addCallback(&m_callback);
  }

  //################################################################################################
  R operator()(Args... args) const
  {
    if(m_callback)
      m_callback(args...);
    return; //Force void
  }

  private:
  std::function<T> m_callback;
  std::vector<C> m_collections;

  //################################################################################################
  std::function<void(C,C)> m_unrefCallback=[&](C oc, C nc)
  {
    tpRemoveOne(m_collections, oc);
    if(nc)
      m_collections.push_back(nc);
  };
};

}

#endif
