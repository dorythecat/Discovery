#include "buffer.h"

Buffer::Buffer(Device* device,
               const VkDeviceSize size,
               const VkBufferUsageFlags usage,
               const VkMemoryPropertyFlags properties) : _device(device) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device->getDevice(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS)
        Logger::log(FATAL, "Failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device->getDevice(), _buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device->getDevice(), &allocInfo, nullptr, &_memory) != VK_SUCCESS)
        Logger::log(FATAL, "Failed to allocate buffer memory!");

    vkBindBufferMemory(_device->getDevice(), _buffer, _memory, 0);
}

Buffer::~Buffer() {
    vkDestroyBuffer(_device->getDevice(), _buffer, nullptr);
    vkFreeMemory(_device->getDevice(), _memory, nullptr);
}

void Buffer::copyTo(const Buffer* dst, const VkDeviceSize size, const VkCommandPool commandPool) const {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device->getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, _buffer, dst->getBuffer(), 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_device->getGraphicsQueue(), 1, &submitInfo, nullptr);
    vkQueueWaitIdle(_device->getGraphicsQueue());

    vkFreeCommandBuffers(_device->getDevice(), commandPool, 1, &commandBuffer);
}

void Buffer::mapMemory(const VkDeviceSize offset, const VkDeviceSize size, const VkMemoryMapFlags flags, void **ppData) const {
    vkMapMemory(_device->getDevice(), _memory, offset, size, flags, ppData);
}

uint32_t Buffer::findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & 1 << i && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    } Logger::log(FATAL, "Failed to find suitable memory type!");
    return 0; // Avoids compiler warnings
}
