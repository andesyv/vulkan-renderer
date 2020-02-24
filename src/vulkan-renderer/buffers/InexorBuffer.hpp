#pragma once

// Vulkan Memory Allocator.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorStagingBuffer
	/// @brief An abstraction class for the creation of staging buffers.
	/// A staging buffer is a buffer that is used for efficient data transfer to GPU.
	/// It will be deleted immediately after usage.
	struct InexorBuffer
	{
		InexorBuffer()
		{
		}

		InexorBuffer(const std::size_t& buffer_size)
		{	
			size = buffer_size;
		}
				
		std::size_t size;
				
		VkBuffer buffer = VK_NULL_HANDLE;

		VmaAllocation allocation = VK_NULL_HANDLE;

		VmaAllocationInfo allocation_info = {};

		VkBufferCreateInfo create_info = {};
				
		VmaAllocationCreateInfo allocation_create_info = {};				
	};


};
};