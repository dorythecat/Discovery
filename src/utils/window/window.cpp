#include "window.h"

Window::Window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Testing", nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);

    createInstance();
    setupDebugMessenger();
    createSurface();
}

Window::~Window() {
    // Clean everything up
    vkDestroySurfaceKHR(_instance, _surface, nullptr);

    if constexpr (enableValidationLayers) DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);

    vkDestroyInstance(_instance, nullptr);

    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Window::createInstance() {
    if constexpr (enableValidationLayers)
        if (!checkValidationLayerSupport())
            Logger::log(FATAL, "Validation layers requested, but not available!");

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
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
    if constexpr (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
        Logger::log(FATAL, "Failed to create Vulkan instance!");
}

void Window::setupDebugMessenger() {
    if constexpr (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
        Logger::log(FATAL, "Failed to set up debug messenger!");
}

void Window::createSurface() {
    if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS)
        Logger::log(FATAL, "Failed to create window surface!");
}

std::vector<const char *> Window::getGLFWExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (glfwExtensions == nullptr) Logger::log(FATAL, "GLFW extensions not found!");

    std::vector<const char*> requiredExtensions;
    for(uint32_t i = 0; i < glfwExtensionCount; i++) requiredExtensions.emplace_back(glfwExtensions[i]);

    return requiredExtensions;
}

bool Window::checkValidationLayerSupport() {
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

std::vector<const char*> Window::getRequiredExtensions() {
    std::vector<const char*> requiredExtensions = getGLFWExtensions();

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

VkResult Window::CreateDebugUtilsMessengerEXT(const VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkDebugUtilsMessengerEXT* pDebugMessenger) {
    const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Window::DestroyDebugUtilsMessengerEXT(const VkInstance instance,
                                              const VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator) {
    const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (func != nullptr) func(instance, debugMessenger, pAllocator);
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto _window = static_cast<Window*>(glfwGetWindowUserPointer(window));
    _window->framebufferResized = true;
}
