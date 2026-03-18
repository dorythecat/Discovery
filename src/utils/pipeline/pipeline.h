#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <fstream>

#include "../logger/logger.h"
#include "../swapchain/swapchain.h"
#include "../device/device.h"

class Pipeline {
public:
    Pipeline(const std::unique_ptr<SwapChain> &swapChain);
    ~Pipeline();

    Pipeline(const Pipeline &pipeline) = delete;
    Pipeline& operator=(const Pipeline &pipeline) = delete;

    void renderFrame();
private:
    SwapChain* _swapChain;
    Device* _device;

    VkRenderPass _renderPass{};
    VkPipelineLayout _pipelineLayout{};
    VkPipeline _graphicsPipeline{};

    std::vector<VkFramebuffer> _swapChainFramebuffers;

    // TODO: Abstract command pools and buffers
    VkCommandPool _commandPool{};
    VkCommandBuffer _commandBuffer{};

    // TODO(maybe): Abstract sync objects
    VkSemaphore _imageAvailableSemaphore{};
    VkSemaphore _renderFinishedSemaphore{};
    VkFence _inFlightFence{};

    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    // TODO: Move the shader handling to its own class
    static std::vector<char> readFile(const std::string &filename);
    VkShaderModule createShaderModule(const std::vector<char> &code) const;

    void recordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex) const;
};

#endif