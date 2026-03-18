#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>

#include "../logger/logger.h"
#include "../device/device.h"

class Buffer {
public:
    Buffer(Device* device,
           VkDeviceSize size,
           VkBufferUsageFlags usage,
           VkMemoryPropertyFlags properties);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    [[nodiscard]] VkBuffer getBuffer() const { return _buffer; }
    [[nodiscard]] VkDeviceMemory getMemory() const { return _memory; }

    void copyTo(const Buffer* dst, VkDeviceSize size, VkCommandPool commandPool) const;
private:
    Device* _device;

    VkBuffer _buffer;
    VkDeviceMemory _memory;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
};

#endif