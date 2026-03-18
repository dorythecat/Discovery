#include <vulkan/vulkan.h>

#include <cstdlib>
#include <optional>
#include <memory>

#include "utils/logger/logger.h"
#include "utils/window/window.h"
#include "utils/device/device.h"
#include "utils/swapchain/swapchain.h"
#include "utils/pipeline/pipeline.h"

class HelloTriangleApplication {
public:
    HelloTriangleApplication() {}
    ~HelloTriangleApplication() {}

    void run() const {
        mainLoop();
    }

private:
    // Variables
    std::unique_ptr<Window> _window = std::make_unique<Window>(); // Window class handles the instance, debug messenger, and surface for us
    // Device class handles the physical and logical devices for us, as well as the queues
    std::unique_ptr<Device> _device = std::make_unique<Device>(_window);
    // Swapchain class handles the swap chain, and swap chain images, for us
    std::unique_ptr<SwapChain> _swapChain = std::make_unique<SwapChain>(_device);
    // Pipeline class handles the graphics pipeline, signaling, and rendering to the screen
    std::unique_ptr<Pipeline> _pipeline = std::make_unique<Pipeline>(_swapChain);

    void mainLoop() const {
        while (!_window->shouldClose()) {
            glfwPollEvents();
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