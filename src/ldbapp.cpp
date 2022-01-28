//
// Created by thukisdo on 28/01/2022.
//

#include "ldbapp.h"
#include <iostream>
#include <QApplication>
#include "MainWindow.h"

namespace ldb {

  std::pair<std::string, std::vector<std::string>> parse_command(int argc, char** argv) {

    std::string command = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++) {
      args.emplace_back(argv[i]);
    }
    return {command, args};
  }

  void LDBApp::run(int argc, char **argv) {
    // Create the GUI here
    // Start the gui
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Le debugger");
    gui::MainWindow main_window;
    main_window.show();

    // If the user has specified a command to trace, open it
    if (argc > 2) {
      auto [command, args] = parse_command(argc, argv);
      main_window.startCommand(command, args);
    }
    app.exec();
  }
}