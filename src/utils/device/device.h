#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.h>

#include <vector>
#include <map>
#include <set>
#include <optional>

#include "../logger/logger.h"
#include "../window/window.h"

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class Device {
public:
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    explicit Device(const Window &window);
    ~Device();

    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const {
        if (_device == nullptr)
            Logger::log(FATAL, "Tried to access physical device that hasn't been created yet!");
        return _physicalDevice;
    }
    [[nodiscard]] VkDevice getDevice() const {
        if (_device == nullptr)
            Logger::log(FATAL, "Tried to access device that hasn't been created yet!");
        return _device;
    }
    [[nodiscard]] VkQueue getGraphicsQueue() const {
        if (_graphicsQueue == nullptr)
            Logger::log(FATAL, "Tried to access graphics queue that hasn't been created yet!");
        return _graphicsQueue;
    }
    [[nodiscard]] VkQueue getPresentQueue() const {
        if (_presentQueue == nullptr)
            Logger::log(FATAL, "Tried to access present queue that hasn't been created yet!");
        return _presentQueue;
    }

    SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device) const {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _window.getSurface(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _window.getSurface(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _window.getSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _window.getSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _window.getSurface(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _window.getSurface(), &presentSupport);

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
            if (presentSupport) indices.presentFamily = i;
            if (indices.isComplete()) break;
            i++;
        } return indices;
    }
private:
    Window _window;
    VkPhysicalDevice _physicalDevice = nullptr;
    VkDevice _device = nullptr;

    VkQueue _graphicsQueue = nullptr;
    VkQueue _presentQueue = nullptr;

    void pickPhysicalDevice();
    void createLogicalDevice();

    static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);

        std::vector<VkExtensionProperties> availableExts(extCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExts.data());

        std::set<std::string> requiredExts(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto&[name, _] : availableExts) requiredExts.erase(name);
        return requiredExts.empty();
    }

    uint32_t rateDeviceSuitability(const VkPhysicalDevice device) const {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _window.getSurface(), &surfaceFormatCount, nullptr);

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
};


#endif