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
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    static bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        return std::ranges::all_of(validationLayers, [&availableLayers](const char* layerName) {
            return std::ranges::any_of(availableLayers, [layerName](const VkLayerProperties& props) {
                return strcmp(layerName, props.layerName) == 0;
            });
        });
    }

    static std::vector<const char*> getRequiredExtensions() {
        std::vector<const char*> requiredExtensions = Window::getGLFWExtensions();

        // Check what extensions we have
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

#ifndef NDEBUG
        Logger::log(DEBUG, "Available extensions:");

        for (const auto&[extensionName, _] : extensions)
            std::cout << '\t' << "- " << extensionName << std::endl;

        Logger::log(DEBUG, "GLFW required extensions:");

        for (const auto& extension : requiredExtensions)
            std::cout << '\t' << "- " << extension << std::endl;
#endif

        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        if constexpr (enableValidationLayers) requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Check that we have all extensions GLFW needs
        if (!std::ranges::all_of(requiredExtensions, [&extensions](const char* reqExt) {
            return std::ranges::any_of(extensions, [reqExt](const auto& ext) {
                return strcmp(reqExt, ext.extensionName) == 0;
            });
        })) Logger::log(FATAL, "Lacking required Vulkan extension(s)!");
        return requiredExtensions;
    }

    static VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkDebugUtilsMessengerEXT* pDebugMessenger) {
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
        );
        if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    static void DestroyDebugUtilsMessengerEXT(const VkInstance instance,
                                              const VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator) {
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
        );
        if (func != nullptr) func(instance, debugMessenger, pAllocator);
    }
};


#endif