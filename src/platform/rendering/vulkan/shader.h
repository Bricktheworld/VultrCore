#pragma once
#include <vulkan/vulkan.h>
#include "device.h"
#include <filesystem/filesystem.h>
#include <platform/rendering.h>
#include "descriptor_set.h"

namespace Vultr
{
	namespace Vulkan
	{
		enum struct UniformType
		{
			Vec2,
			Vec3,
			Vec4,
			Mat3,
			Mat4,
			f32,
			f64,
			s8,
			s16,
			s32,
			s64,
			u8,
			u16,
			u32,
			u64,
		};

		struct UniformMember
		{
			UniformType type = UniformType::Vec3;
			u32 binding      = 0;
			u32 offset       = 0;
		};
	} // namespace Vulkan
#define MAX_DESCRIPTOR_SETS 128
	namespace Platform
	{
		struct Shader
		{
			VkShaderModule vert_module = nullptr;
			VkShaderModule frag_module = nullptr;

			Vector<Vulkan::UniformMember> uniform_members{};
			u32 uniform_size = 0;
			Hashmap<String, u32> uniform_member_names{};
			Hashmap<String, u32> sampler_bindings{};
			VkDescriptorSetLayout vk_custom_layout = nullptr;

			Vector<VkDescriptorPool, Vulkan::MAX_FRAMES_IN_FLIGHT> vk_descriptor_pools{};

			DescriptorSet descriptor_set_pool[MAX_DESCRIPTOR_SETS]{};
			Queue<DescriptorSet *, MAX_DESCRIPTOR_SETS> free_descriptor_sets{};
			Vector<DescriptorSet *, MAX_DESCRIPTOR_SETS> allocated_descriptor_sets{};
			Platform::Mutex mutex{};
		};
	} // namespace Platform
} // namespace Vultr
