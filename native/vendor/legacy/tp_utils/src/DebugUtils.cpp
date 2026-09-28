#include "tp_utils/DebugUtils.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/StackTrace.h"
#include "tp_utils/FileUtils.h"
#include "tp_utils/detail/log_stats/virtual_memory.h"

#include "date/date.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <atomic>
#include <memory>

#ifdef TP_ANDROID
#include <android/log.h>
#endif

namespace tp_utils
{
namespace
{
TPMutex debugMutex{TPM};
std::function<void(MessageType, const std::string&)> debugCallback;
std::function<void(const std::string&, DebugType, const std::string&)> tableCallback;

//##################################################################################################
std::unordered_map<std::string, std::unordered_map<int, bool>>& enabledDebugModeObjects()
{
  static std::unordered_map<std::string, std::unordered_map<int, bool>> s;
  return s;
}

//##################################################################################################
std::vector<DebugMode*>& debugModeObjects()
{
  static std::vector<DebugMode*> s;
  return s;
}

//##################################################################################################
void handleSignal(int signum)
{
  tpWarning() << "Signal caught: " << signum;
  printStackTrace();
}
}

//##################################################################################################
std::string getCurrentTimestamp()
{
  using std::chrono::system_clock;
  auto currentTime = std::chrono::system_clock::now();
  return date::format("[%F %H:%M:%S]", currentTime);
}

//##################################################################################################
std::string getCurrentTimestampPath()
{
  using std::chrono::system_clock;
  auto currentTime = std::chrono::system_clock::now();
  return date::format("%F %H_%M_%S", currentTime);
}

//##################################################################################################
std::string getCurrentTimestamp_notThreadSafe()
{
#ifdef TP_WIN32
  return getCurrentTimestamp();
#else
  using std::chrono::system_clock;
  auto currentTime = std::chrono::system_clock::now();

  auto transformed = currentTime.time_since_epoch().count() / 1000000;

  auto millis = transformed % 1000;

  time_t tt = system_clock::to_time_t( currentTime );
  auto timeinfo = localtime(&tt);

  char bufferA[80];

#ifdef TP_WIN32_MINGW
  strftime(bufferA, 80, "[%H:%M:%S", timeinfo);
#else
  strftime(bufferA, 80, "[%F %H:%M:%S", timeinfo);
#endif

  char bufferB[100];
  snprintf(bufferB, sizeof(bufferB),  "%s:%03d] ", bufferA, int(millis));

  return std::string(bufferB);
#endif
}

//##################################################################################################
void installSignalHandler()
{
  signal(SIGABRT, &handleSignal);
}

//##################################################################################################
std::function<void(MessageType, const std::string&)> installMessageHandler(const std::function<void(MessageType, const std::string&)>& callback)
{
  TP_MUTEX_LOCKER(debugMutex);
  auto previous = debugCallback;
  debugCallback = callback;
  return previous;
}

//##################################################################################################
void installDateTimeMessageHandler()
{
  installMessageHandler([](tp_utils::MessageType, const std::string& message)
  {
    std::cout << getCurrentTimestamp() << message;
    std::cout.flush();
  });
}

//##################################################################################################
void installDateTimeMessageHandler_notThreadSafe()
{
  installMessageHandler([](tp_utils::MessageType, const std::string& message)
  {
    std::cout << getCurrentTimestamp_notThreadSafe() << message;
    std::cout.flush();
  });
}

//##################################################################################################
void installDateTimeMemoryMessageHandler()
{
  installMessageHandler([](tp_utils::MessageType, const std::string& message)
  {
    tp_utils::detail::VirtualMemory vm;
    std::cout << getCurrentTimestamp() << "(" << int(vm.VmHWM/1024) << "MB)" << message;
    std::cout.flush();
  });
}


//##################################################################################################
TeeMessageHandler::TeeMessageHandler(const std::string& path, bool withTime)
{
  auto closure = [this, withTime, path](tp_utils::MessageType type, const std::string& message)
  {
    if(withTime)
      writeTextFile(path, getCurrentTimestamp() + message, TPAppend::Yes);
    else
      writeTextFile(path, message, TPAppend::Yes);

    if(m_previous)
      m_previous(type, message);
  };

  m_previous = installMessageHandler(closure);
}

//##################################################################################################
TeeMessageHandler::~TeeMessageHandler()
{
  installMessageHandler(m_previous);
}

//##################################################################################################
StringMessageHandler::StringMessageHandler(std::string& result, bool withTime)
{
  auto closure = [this, withTime, &result](tp_utils::MessageType type, const std::string& message)
  {
    if(withTime)
      result += getCurrentTimestamp() + message;
    else
      result += message;

    if(m_previous)
      m_previous(type, message);
  };

  m_previous = installMessageHandler(closure);
}

//##################################################################################################
StringMessageHandler::~StringMessageHandler()
{
  installMessageHandler(m_previous);
}


//##################################################################################################
struct DebugMode::Private
{
  TP_NONCOPYABLE(Private);

  const std::string& classPath;
  DebugType debugType;
  std::atomic_bool enabled{false};

  //################################################################################################
  Private(const std::string& classPath_, DebugType debugType_):
    classPath(classPath_),
    debugType(debugType_)
  {

  }
};

//##################################################################################################
DebugMode::DebugMode(const std::string& classPath, DebugType debugType):
  d(new Private(classPath, debugType))
{
  TP_MUTEX_LOCKER(debugMutex);
  debugModeObjects().push_back(this);
  auto i = enabledDebugModeObjects().find(classPath);
  if(i != enabledDebugModeObjects().end())
  {
    auto ii = i->second.find(int(debugType));
    if(ii != i->second.end())
      d->enabled = ii->second;
  }
}

//##################################################################################################
DebugMode::~DebugMode()
{
  TP_MUTEX_LOCKER(debugMutex);
  tpRemoveOne(debugModeObjects(), this);
}

//##################################################################################################
bool DebugMode::operator()()
{
  return d->enabled;
}

//##################################################################################################
void DebugMode::setTable(const std::string& table)
{
  if(d->enabled)
  {
    TP_MUTEX_LOCKER(debugMutex);
    if(tableCallback)
      tableCallback(d->classPath, d->debugType, table);
  }
}

//##################################################################################################
void DebugMode::installTableCallback(std::function<void(const std::string&, DebugType, const std::string&)> callback)
{
  TP_MUTEX_LOCKER(debugMutex);
  tableCallback = std::move(callback);
}

//##################################################################################################
void DebugMode::enable(const std::string& classPath, DebugType debugType, bool enabled)
{
  TP_MUTEX_LOCKER(debugMutex);
  enabledDebugModeObjects()[classPath][int(debugType)] = enabled;
  for(auto dm : debugModeObjects())
    if(dm->d->classPath == classPath && dm->d->debugType == debugType)
      dm->d->enabled = enabled;
}

//##################################################################################################
std::vector<std::string> DebugMode::classPaths(DebugType debugType)
{
  TP_MUTEX_LOCKER(debugMutex);
  std::vector<std::string> classPaths;

  for(DebugMode* dm : debugModeObjects())
    if(dm->d->debugType == debugType)
      classPaths.push_back(dm->d->classPath);

  return classPaths;
}

//##################################################################################################
int DebugBuffer::sync()
{
  TP_MUTEX_LOCKER(debugMutex);
  if(debugCallback)
    debugCallback(m_type, str());
  else
  {
    std::cout << str();
    std::cout.flush();
  }

  DBG::Manager::instance().debugCallbacks(m_type, str());

  str("");
  return 0;
}

namespace DBG
{

//##################################################################################################
struct Default : public Base
{
  TP_NONCOPYABLE(Default);

  Default(MessageType type);
  ~Default() override;
  std::ostream& operator()() override;

  DebugBuffer m_buffer;
  std::ostream m_stream;
};

//##################################################################################################
struct DefaultWarning : public Default
{
  DefaultWarning():
    Default(MessageType::Warning)
  {

  }
};

//##################################################################################################
struct DefaultDebug : public Default
{
  DefaultDebug():
    Default(MessageType::Debug)
  {

  }
};

//##################################################################################################
using DefaultWarningFactory = FactoryTemplate<DefaultWarning>;
using DefaultDebugFactory = FactoryTemplate<DefaultDebug>;

//##################################################################################################
Default::Default(MessageType type):
  m_stream(&m_buffer)
{
  m_buffer.m_type = type;
}

//##################################################################################################
Default::~Default()
{
  m_stream << std::endl;
}

//##################################################################################################
std::ostream& Default::operator()()
{
  return m_stream;
}

//##################################################################################################
struct Manager::Private
{
  TP_NONCOPYABLE(Private);
  Private() = default;

  std::mutex mutex;
  std::unique_ptr<FactoryBase> warningFactory{new DefaultWarningFactory()};
  std::unique_ptr<FactoryBase> debugFactory{new DefaultDebugFactory()};
};

//##################################################################################################
Manager::Manager():
  d(new Private())
{

}

//##################################################################################################
Manager::~Manager()
{
  delete d;
}

//##################################################################################################
void Manager::setWarning(FactoryBase* warningFactory)
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  d->warningFactory.reset(warningFactory);
}

//##################################################################################################
Base* Manager::produceWarning()
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  return d->warningFactory->produce();
}

//##################################################################################################
void Manager::setDebug(FactoryBase* debugFactory)
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  d->debugFactory.reset(debugFactory);
}

//##################################################################################################
Base* Manager::produceDebug()
{
  std::lock_guard<std::mutex> lg(d->mutex);
  TP_UNUSED(lg);
  return d->debugFactory->produce();
}

//##################################################################################################
Manager& Manager::instance()
{
  static Manager instance;
  return instance;
}
}

//##################################################################################################
DebugHelper::DebugHelper(DBG::Base* dbg):
  m_dbg(dbg)
{

}

//##################################################################################################
DebugHelper::~DebugHelper()
{
  delete m_dbg;
}

//##################################################################################################
std::ostream& DebugHelper::operator()()
{
  return (*m_dbg)();
}

//##################################################################################################
//## Platform Abstractions #########################################################################
//##################################################################################################

#ifdef TP_ANDROID
namespace
{
//##################################################################################################
void messageHandler(tp_utils::MessageType messageType, const std::string& message)
{
  const char* tag="tpDebug";
  switch(messageType)
  {
    case tp_utils::MessageType::Debug:   tag="tpDebug";   break;
    case tp_utils::MessageType::Warning: tag="tpWarning"; break;
  }
  __android_log_print(ANDROID_LOG_DEBUG, tag, "%s", message.c_str());
}
}
#endif

//##################################################################################################
void installDefaultMessageHandler()
{
#ifdef TP_ANDROID
  tp_utils::installMessageHandler(messageHandler);
#endif
}

}
