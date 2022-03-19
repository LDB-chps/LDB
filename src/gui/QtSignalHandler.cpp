#include "QtSignalHandler.h"
#include <QThread>
#include <iostream>
#include <thread>

namespace ldb::gui {

  QtSignalHandler::QtSignalHandler(Process* process, BreakPointHandler* bph)
      : SignalHandler(process, bph), worker_thread(nullptr) {
    connect(this, &QtSignalHandler::ignoredEvent, this, &QtSignalHandler::resumeTracee);
    worker_thread = QThread::create(&QtSignalHandler::workerLoop, this);
    worker_thread->start();
  }

  QtSignalHandler::~QtSignalHandler() {
    stopThread();
  }

  void QtSignalHandler::stopThread() {
    if (not worker_thread or not worker_thread->isRunning()) return;

    worker_exit = true;
    is_muted = true;
    update_cv.notify_all();
    worker_thread->wait();
    worker_exit = false;
    is_muted = false;
  }

  void QtSignalHandler::reset(Process* p) {
    stopThread();
    is_muted = false;
    process = p;
    worker_thread = QThread::create(&QtSignalHandler::workerLoop, this);
    worker_thread->start();
  }

  void QtSignalHandler::mute() {

    std::cout << "Muting" << std::endl;
    if (is_muted or not worker_thread or not worker_thread->isRunning()) return;

    is_muted = true;

    std::unique_lock<std::mutex> l(mutex);

    // Wait until the thread signals that it has finished
    update_cv.wait(l, [this]() -> bool { return worker_waiting; });
    std::cout << "Done muting" << std::endl;
  }

  void QtSignalHandler::unmute() {
    if (not is_muted or not worker_thread or not worker_thread->isRunning()) return;
    update_cv.notify_one();

    is_muted = false;
  }

  SignalEvent QtSignalHandler::handleEvent(const SignalEvent& event) {
    if (event.getSignal() == Signal::kSIGTRAP and breakpoint_handler->isAtBreakpoint()) {
      breakpoint_handler->resetBreakpoint();
    }
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
    while (not worker_exit) {
      // Wait for 100ms
      std::optional<SignalEvent> event = waitEvent(10000000);
      if (event) {
        handleEvent(*event);
        if (event->getStatus() == Process::Status::kDead or
            event->getStatus() == Process::Status::kExited or
            event->getStatus() == Process::Status::kKilled) {
          break;
        }
      }
      if (is_muted and not worker_exit) {
        std::unique_lock<std::mutex> l(mutex);
        std::cout << "Muted !" << std::endl;
        worker_waiting = true;
        update_cv.notify_one();
        update_cv.wait(l);
        std::cout << "Unmuted !" << std::endl;
        worker_waiting = false;
      }
    }
  }

}// namespace ldb::gui