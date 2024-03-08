#include "engine/App.hpp"
#include "engine/Input.hpp"
#include "engine/OrthoCameraController.hpp"

int main (int argc, char *argv[]) {
    App app;
    app.init();

    app.run();

    return 0;
}
