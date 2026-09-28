#ifndef tp_utils_Progress_h
#define tp_utils_Progress_h

#include "tp_utils/CallbackCollection.h"
#include "tp_utils/TPPixel.h"
#include "tp_utils/JSONUtils.h" // IWYU pragma: keep

namespace tp_utils
{
class AbstractCrossThreadCallbackFactory;
class AbstractProgressStore;

//##################################################################################################
struct ProgressMessage
{
  std::string message;
  bool error{false};
  size_t indentation{0};

  //################################################################################################
  ProgressMessage() = default;

  //################################################################################################
  ProgressMessage(const std::string& message_, bool error_, size_t indentation_):
    message(message_),
    error(error_),
    indentation(indentation_)
  {

  }
};

//##################################################################################################
struct ProgressEvent
{
  AbstractProgressStore* store{nullptr};

  size_t id{0};
  size_t parentId{0};
  std::string name;

  int64_t start{0};
  int64_t end{0};
  float fraction{0.0f};
  TPPixel color{176, 215, 136};
  bool active{true};

  //################################################################################################
  void saveState(JSON& j) const;

  //################################################################################################
  void loadState(const JSON& j);
};

//##################################################################################################
class AbstractProgressStore
{
public:
  //################################################################################################
  AbstractProgressStore();

  //################################################################################################
  virtual ~AbstractProgressStore();

  //################################################################################################
  //! Add the progress event to the list and set its id.
  virtual void initProgressEvent(ProgressEvent& progressEvent)=0;

  //################################################################################################
  virtual void updateProgressEvent(const ProgressEvent& progressEvent)=0;
};

//##################################################################################################
class RAMProgressStore : public AbstractProgressStore
{
  TP_NONCOPYABLE(RAMProgressStore);
  TP_DQ;
public:
  //################################################################################################
  RAMProgressStore();

  //################################################################################################
  ~RAMProgressStore() override;

  //################################################################################################
  void initProgressEvent(ProgressEvent& progressEvent) override;

  //################################################################################################
  void updateProgressEvent(const ProgressEvent& progressEvent) override;

  //################################################################################################
  void viewProgressEvents(const std::function<void(const std::vector<ProgressEvent>&)>& closure);

  //################################################################################################  
  void saveState(JSON& j) const;

  //################################################################################################
  static std::vector<ProgressEvent> loadState(const JSON& j);
};

//##################################################################################################
extern AbstractProgressStore* globalProgressStore_;

//##################################################################################################
//! A class for recording the progress of an operation.
/*!

*/
class TP_UTILS_EXPORT Progress
{
  TP_NONCOPYABLE(Progress);
  TP_DQ;
public:
  //################################################################################################
  //! Thread safe constructor.
  Progress(TPCTCFactory ctcFactory,
           const std::string& message,
           AbstractProgressStore* progressStore=nullptr);

  //################################################################################################
  //! Blocking operation constructor.
  Progress(const std::function<bool()>& poll,
           const std::string& message,
           AbstractProgressStore* progressStore=nullptr);

  //################################################################################################
  //! Child step constructor.
  Progress(Progress* parent, const std::string& message);

  //################################################################################################
  virtual ~Progress();

  //################################################################################################
  void setPrintToConsole(bool printToConsole);

  //################################################################################################
  void setPrintToFile(const std::string& printToFile);

  //################################################################################################
  //! Set the progress
  /*!
  \param fraction of the way through this task (0 to 1)
  */
  void setProgress(float fraction);

  //################################################################################################
  //! Set the progress and the description of what is being processes
  /*!
  \param fraction of the way through this task (0 to 1)
  */
  void setProgress(float fraction, const std::string& description);

  //################################################################################################
  //! Returns the current progress as a fraction
  float progress() const;

  //################################################################################################
  //! Sets the description of the task that is being performed
  /*!
  This should be updated to describe what is happening at each stage of a process, this should be a
  short string that will be hisplayed to the user.

  \param description - The new description
  */
  void setDescription(const std::string& description);

  //################################################################################################
  //! Returns the descripton of the current stage of the task.
  std::string description() const;

  //################################################################################################
  /*!
  \param description of the child step.
  \param completeFraction the progress of this once the child step is complete.
  \return A pointer to new Progress object, owned by this.
  */
  TP_NODISCARD Progress* addChildStep(const std::string& message, float completeFraction);

  //################################################################################################
  //! Log a message that will be kept in the tree of messages.
  void addMessage(const std::string& message);

  //################################################################################################
  //! Log a message that will be kept in the tree of messages.
  void addError(const std::string& error);

  //################################################################################################
  //! Return the tree of messages.
  std::vector<ProgressMessage> allMessages() const;

  //################################################################################################
  //! Return the tree of error messages.
  std::vector<ProgressMessage> errorMessages() const;

  //################################################################################################
  std::string compileErrors() const;

  //################################################################################################
  std::vector<Progress*> childSteps() const;

  //################################################################################################
  //! Returs true if this task should stop
  bool shouldStop() const;

  //################################################################################################
  //! Use this to stop the task
  void stop(bool shouldStop);

  //################################################################################################
  bool poll();

  //################################################################################################
  AbstractProgressStore* progressStore() const;

  //################################################################################################
  void updateProgressEvent(const std::function<void(ProgressEvent&)>& closure) const;

  //################################################################################################
  void viewProgressEvent(const std::function<void(const ProgressEvent&)>& closure) const;

  //################################################################################################
  void copyChildSteps(Progress* progress);

  //################################################################################################
  CallbackCollection<void()> changed;

  //################################################################################################
  CallbackCollection<void()> pollCallbacks;

protected:
  //################################################################################################
  void callChanged();

  //################################################################################################
  void getAllMessages(size_t indentation, std::vector<ProgressMessage>& messages) const;

  //################################################################################################
  bool getErrors(size_t indentation, std::vector<ProgressMessage>& messages) const;
};

//##################################################################################################
/*!
~~~~~~~~~~~~~{.cpp}
ParrallelProgress parrallelProgress(progress);

std::thread t1([&]
{
  auto pp = parrallelProgress->addChildStep("Task 1");
  ...
});

std::thread t2([&]
{
  auto pp = parrallelProgress->addChildStep("Task 2");
  ...
});

//Optional
parrallelProgress->childStepFinished()

t1.join();
t2.join();

// ~ParrallelProgress will write child steps to progress sequentially, or in the order that
childStepFinished is called.
~~~~~~~~~~~~~
*/
class TP_UTILS_EXPORT ParrallelProgress
{
  TP_NONCOPYABLE(ParrallelProgress);
  TP_DQ;
public:

  //################################################################################################
  ParrallelProgress(Progress* progress);

  //################################################################################################
  ~ParrallelProgress();

  //################################################################################################
  //! Get a thread safe progress object.
  TP_NODISCARD Progress* addChildStep(const std::string& message);

  //################################################################################################
  //! Must be called from the main thread.
  void childStepFinished(Progress* progress, float fraction);
};


}

#endif
