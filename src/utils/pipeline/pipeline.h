#ifndef PIPELINE_H
#define PIPELINE_H

#define GLM_FORCE_RADIANS

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <vector>
#include <memory>
#include <fstream>
#include <array>

#include "../logger/logger.h"
#include "../swapchain/swapchain.h"
#include "../device/device.h"
#include "../buffer/buffer.h"

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// TODO(Maybe): Replace GLM for my own vectors and stuff :3
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);



        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);


        return attributeDescriptions;
    }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};

class Pipeline {
public:
    Pipeline(const std::unique_ptr<Device> &device);
    ~Pipeline();

    Pipeline(const Pipeline &pipeline) = delete;
    Pipeline& operator=(const Pipeline &pipeline) = delete;

    void renderFrame();
private:
    std::unique_ptr<SwapChain> _swapChain;
    Device* _device;

    VkRenderPass _renderPass{};
    VkDescriptorSetLayout _descriptorSetLayout;
    VkPipelineLayout _pipelineLayout{};
    VkPipeline _graphicsPipeline{};

    std::unique_ptr<Buffer> _vertexBuffer;
    std::unique_ptr<Buffer> _indexBuffer;

    std::vector<std::unique_ptr<Buffer>> _uniformBuffers;
    std::vector<void*> _uniformBuffersMapped;

    // TODO: Abstract all of these
    VkDescriptorPool _descriptorPool{};
    std::vector<VkDescriptorSet> _descriptorSets;

    VkCommandPool _commandPool{};
    std::vector<VkCommandBuffer> _commandBuffers;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    uint32_t _currentFrame = 0;

    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createCommandPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

    void updateUniformBuffer(uint32_t currentImage);

    // TODO: Move the shader handling to its own class
    static std::vector<char> readFile(const std::string &filename);
    VkShaderModule createShaderModule(const std::vector<char> &code) const;

    void recordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex) const;

    void recreateSwapChain();
};

#endif