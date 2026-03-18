#ifndef WINDOW_H
#define WINDOW_H

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstring>
#include <vector>

#include "../logger/logger.h"

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr uint32_t SCR_WIDTH = 800;
constexpr uint32_t SCR_HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class Window {
public:
    Window();
    ~Window();

    Window(const Window &window) = delete;
    Window& operator=(const Window &window) = delete;

    // Getter functions
    [[nodiscard]] VkInstance getInstance() const {
        if (_instance == nullptr)
            Logger::log(FATAL, "Tried to access instance that hasn't been created yet!");
        return _instance;
    }
    [[nodiscard]] VkDebugUtilsMessengerEXT getDebugMesenger() const {
        if (_debugMessenger == nullptr)
            Logger::log(FATAL, "Tried to access debug messenger that hasn't been created yet!");
        return _debugMessenger;
    }
    [[nodiscard]] VkSurfaceKHR getSurface() const {
        if (_surface == nullptr)
            Logger::log(FATAL, "Tried to access surface that hasn't been created yet!");
        return _surface;
    }
    [[nodiscard]] VkExtent2D getExtent() const {
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }
    [[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(_window); }
    static void pollEvents() { glfwPollEvents(); }

    static std::vector<const char*> getGLFWExtensions();
private:
    GLFWwindow* _window;
    VkInstance _instance = nullptr;
    VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
    VkSurfaceKHR _surface = nullptr;

    void createInstance();
    void setupDebugMessenger();
    void createSurface();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData) {
        Logger::log( VALIDATION, pCallbackData->pMessage);
        return VK_FALSE;
    }
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    static bool checkValidationLayerSupport();
    static std::vector<const char*> getRequiredExtensions();
    static VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void DestroyDebugUtilsMessengerEXT(const VkInstance instance,
                                              const VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator);
};


#endif