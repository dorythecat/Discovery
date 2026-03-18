#include "swapchain.h"

SwapChain::SwapChain(const std::unique_ptr<Device> &device) : _device(device.get()) {
    createSwapChain();
    createImageViews();
}

SwapChain::~SwapChain() {
    for (const auto imageView : _imageViews) vkDestroyImageView(_device->getDevice(), imageView, nullptr);
    vkDestroySwapchainKHR(_device->getDevice(), _swapChain, nullptr);
}

void SwapChain::createSwapChain() {
    const Device::SwapChainSupportDetails swapChainSupport = _device->querySwapChainSupport(_device->getPhysicalDevice());

    auto [format, colorSpace] = chooseSwapSurfaceFormat(swapChainSupport.formats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    const VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _device->getWindow()->getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Device::QueueFamilyIndices indices = _device->findQueueFamilies(_device->getPhysicalDevice());
    const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device->getDevice(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
        Logger::log(FATAL, "Failed to create swap chain!");

    vkGetSwapchainImagesKHR(_device->getDevice(), _swapChain, &imageCount, nullptr);
    _images.resize(imageCount);
    vkGetSwapchainImagesKHR(_device->getDevice(),
                            _swapChain,
                            &imageCount,
                            _images.data());

    _imageFormat = format;
    _extent = extent;
}

void SwapChain::createImageViews() {
    _imageViews.resize(_images.size());

    for (size_t i = 0; i < _images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _images[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _imageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device->getDevice(),
                                    &createInfo,
                                    nullptr,
                                    &_imageViews[i]) != VK_SUCCESS)
            Logger::log(FATAL, "Failed to create swap chain image views!");
    }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format != VK_FORMAT_B8G8R8A8_SRGB ||
            availableFormat.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) continue;
        return availableFormat;
    } return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode != VK_PRESENT_MODE_MAILBOX_KHR) continue;
        return availablePresentMode;
    } return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

    VkExtent2D actualExtent = _device->getWindow()->getExtent();
    actualExtent.width = std::clamp(actualExtent.width,
                                     capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
                                      capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);
    return actualExtent;
}
