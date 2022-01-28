#include <iostream>
#include "ldbapp.h"

int main(int argc, char** argv) {
    ldb::LDBApp app;
    app.run(argc, argv);
    return 0;
}