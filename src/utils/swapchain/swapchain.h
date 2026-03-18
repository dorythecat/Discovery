#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vulkan/vulkan.h>

#include <limits>
#include <vector>
#include <memory>

#include "../logger/logger.h"
#include "../device/device.h"

class SwapChain {
public:
    explicit SwapChain(const std::unique_ptr<Device> &device);
    ~SwapChain();

    SwapChain(const SwapChain &swapChain) = delete;
    SwapChain& operator=(const SwapChain &swapChain) = delete;

    Device* getDevice() const { return _device; }
    VkSwapchainKHR getSwapChain() const {
        if (_swapChain == nullptr)
            Logger::log(FATAL, "Tried to access swap chain before creation!");
        return _swapChain;
    }
    VkFormat getImageFormat() const { return _imageFormat; }
    VkExtent2D getExtent() const { return _extent; }
    std::vector<VkImage> getImages() const { return _images; }
    std::vector<VkImageView> getImageViews() const { return _imageViews; }
    std::vector<VkFramebuffer> getFramebuffers() const { return _framebuffers; }

    void createFramebuffers(VkRenderPass renderPass);
private:
    Device* _device;

    VkSwapchainKHR _swapChain = nullptr;
    VkFormat _imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D _extent{};
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;
    std::vector<VkFramebuffer> _framebuffers;

    void createSwapChain();
    void createImageViews();

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
};


#endif