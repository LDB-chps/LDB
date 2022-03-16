#pragma once
#include "Process.h"
#include "SignalHandler.h"
#include <QtCore/QObject>

namespace ldb::gui {

  class QtSignalHandler : public SignalHandler {
    Q_OBJECT
  public:
    QtSignalHandler();
    QtSignalHandler(Process* process);


    void handleSignal(SignalEvent& event) override;

  signals:

    void signalReceived(SignalEvent event);
    void processExited();
  };

}// namespace ldb::gui
