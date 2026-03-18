#include "device.h"

Device::Device(const Window &window) : _window(window) {
    pickPhysicalDevice();
    createLogicalDevice();
}

Device::~Device() {
    vkDeviceWaitIdle(_device); // Make sure device isn't in use when destroying it
    vkDestroyDevice(_device, nullptr);
}

void Device::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_window.getInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) Logger::log(FATAL, "Failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_window.getInstance(), &deviceCount, devices.data());

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