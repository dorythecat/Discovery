#include <cstdlib>
#include <memory>

#include "utils/logger/logger.h"
#include "utils/window/window.h"
#include "utils/device/device.h"
#include "utils/pipeline/pipeline.h"

class HelloTriangleApplication {
public:
    HelloTriangleApplication() {}
    ~HelloTriangleApplication() {
        _device->waitIdle(); // Wait for resources to be freed before deleting them

        // Explicitly destroy all unique pointers and their contents
        _pipeline.reset();
        _device.reset();
        _window.reset();
    }

    void run() const {
        mainLoop();
    }

private:
    // Variables
    std::unique_ptr<Window> _window = std::make_unique<Window>(); // Window class handles the instance, debug messenger, and surface for us
    // Device class handles the physical and logical devices for us, as well as the queues
    std::unique_ptr<Device> _device = std::make_unique<Device>(_window);
    // Pipeline class handles the graphics pipeline, signaling, and rendering to the screen
    std::unique_ptr<Pipeline> _pipeline = std::make_unique<Pipeline>(_device);

    void mainLoop() const {
        while (!_window->shouldClose()) {
            Window::pollEvents();
            _pipeline->renderFrame();
        }
    }
};

int main() {
    try {
        HelloTriangleApplication app;
        app.run();
    } catch (const std::exception& e) {
        Logger::log(FATAL, e.what());
    } return EXIT_SUCCESS;
}