#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.h>

#include <vector>
#include <map>
#include <set>
#include <optional>
#include <memory>

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

    explicit Device(const std::unique_ptr<Window> &window);
    ~Device();

    Device(const Device &device) = delete;
    Device& operator=(const Device &device) = delete;

    [[nodiscard]] Window* getWindow() const { return _window; }
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

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
private:
    Window* _window;
    VkPhysicalDevice _physicalDevice = nullptr;
    VkDevice _device = nullptr;

    VkQueue _graphicsQueue = nullptr;
    VkQueue _presentQueue = nullptr;

    void pickPhysicalDevice();
    void createLogicalDevice();

    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    uint32_t rateDeviceSuitability(VkPhysicalDevice device) const;
};


#endif