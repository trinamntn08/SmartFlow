#ifndef tp_utils_MutexUtils_h
#define tp_utils_MutexUtils_h

#include "tp_utils/Globals.h"

#ifdef TP_ENABLE_MUTEX_TIME
#include "tp_utils/TimeUtils.h"
#endif

#include <mutex>

#ifndef TP_ENABLE_MUTEX_TIME

#define TPM
#define cTPM
#define TPM_A
#define TPM_B
#define TPMc
#define TPM_Ac
#define TPM_Bc

//#define TP_MUTEX_LOCKER(mutex)TPMutexLocker TP_CONCAT(locker, __LINE__)(mutex); TP_UNUSED(TP_CONCAT(locker, __LINE__))
#define TP_MUTEX_LOCKER(m)std::lock_guard<std::mutex> TP_CONCAT(locker, __LINE__)(m); TP_UNUSED(TP_CONCAT(locker, __LINE__))
#define TP_MUTEX_UNLOCKER(mutex)TPMutexUnlocker TP_CONCAT(locker, __LINE__)(&mutex); TP_UNUSED(TP_CONCAT(locker, __LINE__))

class TP_UTILS_EXPORT TPMutex: public std::mutex
{
public:
  //################################################################################################
  template<typename T>
  auto locked(TPM_Ac const T& callback)
  {
    lock(TPM_B);
    TP_CLEANUP([&]{unlock(TPM_B);});
    return callback();
  }
};

//##################################################################################################
typedef std::unique_lock<std::mutex> TPMutexLocker;

//##################################################################################################
template<typename T>
class TP_UTILS_EXPORT TPMutexUnlocker
{
  TP_NONCOPYABLE(TPMutexUnlocker);
  T* m_mutex;
public:

  //################################################################################################
  TPMutexUnlocker(T* mutex):
    m_mutex(mutex)
  {
    m_mutex->unlock();
  }

  //################################################################################################
  ~TPMutexUnlocker()
  {
    m_mutex->lock();
  }
};

#else

#define TPM __FILE__, __LINE__
#define cTPM ,TPM
#define TPM_A const char* file_tpm, int line_tpm
#define TPM_B file_tpm, line_tpm
#define TPMc TPM,
#define TPM_Ac TPM_A,
#define TPM_Bc TPM_B,
#define TP_MUTEX_LOCKER(mutex)TPMutexLocker TP_CONCAT(locker, __LINE__)(&mutex, TPM); TP_UNUSED(TP_CONCAT(locker, __LINE__))
#define TP_MUTEX_UNLOCKER(mutex)TPMutexUnlocker TP_CONCAT(locker, __LINE__)(&mutex, TPM); TP_UNUSED(TP_CONCAT(locker, __LINE__))

namespace tp_utils
{

//##################################################################################################
//! This is an internal class used for recording lock statistics
/*!
The only method on this that is of use externally is takeResults() this returns a table of results
that can be used to debug mutex contention. The intended use of this is to write the stats out to
file every few seconds, then someone who is debugging the system can watch the file.
*/
class TP_UTILS_EXPORT LockStats
{
public:
  //################################################################################################
  static int init(const char* type, const char* file, int line);

  //################################################################################################
  static void destroy(int id);

  //################################################################################################
  static size_t waiting(int id, const char* file, int line);

  //################################################################################################
  static void locked(int id, const char* file, int line, int elapsedWaiting, size_t blockingID);

  //################################################################################################
  static void tryLock(int id, const char* file, int line, int elapsedWaiting, size_t blockingID, bool got);

  //################################################################################################
  static void unlock(int id, const char* file, int line);

  //################################################################################################
  //! Present the current stats
  /*!

  TPMutex:Name                    Totals(inst:0000000000,lck:0000000000,wt:0000000000,hld:0000000000)
  ID |Lock name              |Lock count|Wait (ms) |Held (ms) |Held avg  |Blocked by (f: tryLock fail)
  000|Name                   |0000000000|0000000000|0000000000|0000000000|f:000,ID=ms,ID=ms,...
  ID |Unlock name            |Unlock cnt|Waitin cnt|Held (ms) |Held avg  |Held max  |Held rc |Count rc
  000|Name                   |0000000000|0000000000|0000000000|0000000000|0000000000|00000000|00000000

  All times in ms

  inst       = The total number of instances of this mutex
  lck        = The total bumber of times that this mutex has been locked across all instances
  wt         = The total time spent waiting to acquire locks on this mutex across all instances in ms
  hld        = The total amout of time that this mutex was held for across all instances in ms
  Lock count = The number of times a lock method has been called from a particular location (no fails)
  Wait (ms)  = The total time spend waiting on a lock from a particular location
  Held (ms)  = The total time that this mutex has been held by calls to this particular lock
  Held avg   = The average time that calls to this lock method hold the lock (see below for unlock) ms
  Blocked by = The lock ID's that have blocked this call, sorted by wait time
  f:         = If this is a tryLock f: will be present with a count of fails
  Unlock cnt = The number of times this unlock method has been called
  Waitin cnt = The number of threads waiting on this lock when it was released from this location
  Held (ms)  = The total held time for locks released at this location (see above for lock)
  Held avg   = The average time that locks unlocked at this location were held for(see above for lock)
  Held max   = The longest held time that was released at this location in ms
  Held rc    = The recent accumulated held time reset every time takeResults is called in ms
  Count rc   = The recent unlock count reset every time takeResults is called
  */
  static std::string takeResults();

private:
  struct Instance;
  static Instance* instance();
};

}

//##################################################################################################
class TP_UTILS_EXPORT TPMutex: public std::timed_mutex
{
  int m_id;
public:

  //################################################################################################
  TPMutex(const char* file, int line):
    m_id(tp_utils::LockStats::init("TPMutex", file, line))
  {

  }

  //################################################################################################
  ~TPMutex()
  {
    tp_utils::LockStats::destroy(m_id);
  }

  //################################################################################################
  void lock(const char* file, int line)
  {
    tp_utils::ElapsedTimer timer;
    timer.start();
    size_t blockingID=tp_utils::LockStats::waiting(m_id, file, line);
    std::timed_mutex::lock();
    tp_utils::LockStats::locked(m_id, file, line, int(timer.elapsed()), blockingID);
  }

  //################################################################################################
  bool tryLock(const char* file, int line, int timeout = 0)
  {
    tp_utils::ElapsedTimer timer;
    timer.start();
    size_t blockingID=tp_utils::LockStats::waiting(m_id, file, line);
    bool got=std::timed_mutex::try_lock_for(std::chrono::milliseconds(timeout));
    tp_utils::LockStats::tryLock(m_id, file, line, int(timer.elapsed()), blockingID, got);
    return got;
  }

  //################################################################################################
  void unlock(const char* file, int line)
  {
    tp_utils::LockStats::unlock(m_id, file, line);
    std::timed_mutex::unlock();
  }

  //################################################################################################
  template<typename T>
  auto locked(TPM_Ac const T& callback)
  {
    lock(TPM_B);
    TP_CLEANUP([&]{unlock(TPM_B);});
    return callback();
  }
};

//##################################################################################################
class TPMutexLocker
{
  TPMutex* m_mutex;
  const char* m_file;
  int m_line;
  bool m_ownsLock{true};
public:

  //################################################################################################
  TPMutexLocker(TPMutex* mutex, const char* file="", int line=0):
    m_mutex(mutex),
    m_file(file),
    m_line(line)
  {
    m_mutex->lock(m_file, m_line);
  }

  //################################################################################################
  TPMutexLocker(TPMutex& mutex, const char* file="", int line=0):
    m_mutex(&mutex),
    m_file(file),
    m_line(line)
  {
    m_mutex->lock(m_file, m_line);
  }

  //################################################################################################
  ~TPMutexLocker()
  {
    m_mutex->unlock(m_file, m_line);
  }

  //################################################################################################
  TPMutex* mutex() const
  {
    return m_mutex;
  }

  //################################################################################################
  void lock(const char* file, int line)
  {
    m_ownsLock = true;
    m_mutex->lock(file, line);
  }

  //################################################################################################
  void unlock(const char* file, int line)
  {
    m_ownsLock = false;
    m_mutex->unlock(file, line);
  }

  //################################################################################################
  bool owns_lock() const
  {
    return m_ownsLock;
  }
};

//##################################################################################################
class TP_UTILS_EXPORT TPMutexUnlocker
{
  TPMutex* m_mutex;
  TPMutexLocker* m_locker{nullptr};
  const char* m_file;
  int m_line;
public:

  //################################################################################################
  TPMutexUnlocker(TPMutex* mutex, const char* file="", int line=0):
    m_mutex(mutex),
    m_file(file),
    m_line(line)
  {
    m_mutex->unlock(m_file, m_line);
  }

  //################################################################################################
  TPMutexUnlocker(TPMutexLocker* locker, const char* file="", int line=0):
    m_mutex(locker->mutex()),
    m_locker(locker),
    m_file(file),
    m_line(line)
  {
    m_locker->unlock(m_file, m_line);
  }

  //################################################################################################
  ~TPMutexUnlocker()
  {
    if(m_locker)
      m_locker->lock(m_file, m_line);
    else
      m_mutex->lock(m_file, m_line);
  }
};

#endif

//##################################################################################################
class TP_UTILS_EXPORT TPWaitCondition
{
  TP_NONCOPYABLE(TPWaitCondition);
  TP_DQ;
public:
  //################################################################################################
  TPWaitCondition();

  //################################################################################################
  ~TPWaitCondition();

  //################################################################################################
  bool wait(TPM_Ac TPMutexLocker& lockedMutex, int64_t ms = INT64_MAX) noexcept;

  //################################################################################################
  void wakeOne();

  //################################################################################################
  void wakeAll();
};

#endif











