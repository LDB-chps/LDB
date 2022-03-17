#include "QtSignalHandler.h"
#include <QThread>
#include <iostream>
#include <thread>

namespace ldb::gui {

  QtSignalHandler::QtSignalHandler(Process* process)
      : SignalHandler(process), worker_thread(nullptr) {
    connect(this, &QtSignalHandler::ignoredEvent, this, &QtSignalHandler::resumeTracee);
    worker_thread = QThread::create(&QtSignalHandler::workerLoop, this);
    worker_thread->start();
  }

  void QtSignalHandler::reset(Process* p) {
    if (worker_thread and worker_thread->isRunning()) { worker_thread->exit(0); }
    process = p;
    worker_thread = QThread::create(&QtSignalHandler::workerLoop, this);
    worker_thread->start();
  }

  QtSignalHandler::~QtSignalHandler() {
    if (worker_thread and worker_thread->isRunning()) { worker_thread->terminate(); }
  }

  SignalEvent QtSignalHandler::handleEvent(const SignalEvent& event) {
    if (process->getStatus() == Process::Status::kStopped and
        ignored_signals[static_cast<size_t>(event.getSignal())] and
        event.getSignal() != Signal::kSIGCONT) {
      process->updateStatus(Process::Status::kRunning);
      emit ignoredEvent(event);
      return {event.getSignal(), event.getStatus(), true, false};
    }
    emit signalReceived(event);
    return event;
  }

  void QtSignalHandler::resumeTracee(SignalEvent event) {
    process->resume();
  }

  void QtSignalHandler::workerLoop() {
    while (true) {
      SignalEvent event = nextSignal();
      handleEvent(event);
      if (event.getStatus() == Process::Status::kDead or
          event.getStatus() == Process::Status::kExited or
          event.getStatus() == Process::Status::kKilled) {
        break;
      }
    }
  }

}// namespace ldb::gui