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

    if (vkAllocateMemory(_device->getDevice(), &allocInfo, nullptr, &_memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(_device->getDevice(), _buffer, _memory, 0);
}

Buffer::~Buffer() {
    vkDestroyBuffer(_device->getDevice(), _buffer, nullptr);
    vkFreeMemory(_device->getDevice(), _memory, nullptr);
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