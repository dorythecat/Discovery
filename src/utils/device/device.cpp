#include "device.h"

Device::Device(const std::unique_ptr<Window> &window) : _window(window.get()) {
    pickPhysicalDevice();
    createLogicalDevice();
}

Device::~Device() {
    vkDeviceWaitIdle(_device); // Make sure device isn't in use when destroying it
    vkDestroyDevice(_device, nullptr);
}

void Device::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_window->getInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) Logger::log(FATAL, "Failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_window->getInstance(), &deviceCount, devices.data());

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        uint32_t score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first <= 0) Logger::log(FATAL, "Failed to find a suitable GPU!");

    _physicalDevice = candidates.rbegin()->second;
}

void Device::createLogicalDevice() {
    auto [graphicsFamily, presentFamily] = findQueueFamilies(_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        graphicsFamily.value(),
        presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    constexpr VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;
    if constexpr (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
        Logger::log(FATAL, "Failed to create logical device!");

    vkGetDeviceQueue(_device, graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, presentFamily.value(), 0, &_presentQueue);
}

Device::SwapChainSupportDetails Device::querySwapChainSupport(const VkPhysicalDevice device) const {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _window->getSurface(), &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _window->getSurface(), &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _window->getSurface(), &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _window->getSurface(), &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _window->getSurface(), &presentModeCount, details.presentModes.data());
    }

    return details;
}

Device::QueueFamilyIndices Device::findQueueFamilies(const VkPhysicalDevice device) const {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _window->getSurface(), &presentSupport);

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
        if (presentSupport) indices.presentFamily = i;
        if (indices.isComplete()) break;
        i++;
    } return indices;
}

bool Device::checkDeviceExtensionSupport(const VkPhysicalDevice device) {
    uint32_t extCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);

    std::vector<VkExtensionProperties> availableExts(extCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExts.data());

    std::set<std::string> requiredExts(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto&[name, _] : availableExts) requiredExts.erase(name);
    return requiredExts.empty();
}

uint32_t Device::rateDeviceSuitability(const VkPhysicalDevice device) const {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _window->getSurface(), &surfaceFormatCount, nullptr);

    bool swapChainAdequate = false;
    if (checkDeviceExtensionSupport(device)) {
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    if (!findQueueFamilies(device).isComplete() ||
        !swapChainAdequate ||
        !deviceFeatures.geometryShader) return 0;

    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}