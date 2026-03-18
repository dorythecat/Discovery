#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <fstream>

#include "../logger/logger.h"
#include "../swapchain/swapchain.h"
#include "../device/device.h"

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class Pipeline {
public:
    Pipeline(const std::unique_ptr<Device> &device);
    ~Pipeline();

    Pipeline(const Pipeline &pipeline) = delete;
    Pipeline& operator=(const Pipeline &pipeline) = delete;

    void renderFrame() const;
private:
    std::unique_ptr<SwapChain> _swapChain;
    Device* _device;

    VkRenderPass _renderPass{};
    VkPipelineLayout _pipelineLayout{};
    VkPipeline _graphicsPipeline{};

    // TODO: Abstract command pools and buffers
    VkCommandPool _commandPool{};
    std::vector<VkCommandBuffer> _commandBuffers;

    // TODO(maybe): Abstract sync objects
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    uint32_t _currentFrame = 0;

    void createRenderPass();
    void createGraphicsPipeline();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    // TODO: Move the shader handling to its own class
    static std::vector<char> readFile(const std::string &filename);
    VkShaderModule createShaderModule(const std::vector<char> &code) const;

    void recordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex) const;
};

#endif