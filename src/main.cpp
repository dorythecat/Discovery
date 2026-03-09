#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>

constexpr uint32_t SCR_WIDTH = 800;
constexpr uint32_t SCR_HEIGHT = 600;

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // Variables
    GLFWwindow* window;
    VkInstance instance;

    // Main functions
    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO: Make resizable

        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Testing", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    // Vulkan functions
    void createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Testing";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
        appInfo.pEngineName = "Testing Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_4;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> requiredExtensions;
        for(uint32_t i = 0; i < glfwExtensionCount; i++) requiredExtensions.emplace_back(glfwExtensions[i]);

        // Check what extensions we have
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

#ifndef NDEBUG
        std::cout << "Available extensions:" << std::endl;

        for (const auto& extension : extensions)
            std::cout << '\t' << "- " << extension.extensionName << std::endl;

        std::cout << "Required extensions:" << std::endl;

        for (const auto& extension : requiredExtensions)
            std::cout << '\t' << "- " << extension << std::endl;
#endif

        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        // Check that we have all extensions GLFW needs
        for (const auto& requiredExtension : requiredExtensions) {
            bool found = false;
            for (const auto& [extensionName, _] : extensions) {
                if (strcmp(extensionName, requiredExtension) != 0) continue;
                found = true;
                break;
            } if (!found) throw std::runtime_error("Lacking required Vulkan extension(s)!");
        }

        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create Vulkan instance!");
    }
};

int main() {
    try {
        HelloTriangleApplication app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } return EXIT_SUCCESS;
}