#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gsl/gsl>
#include <vector>
#include <range/v3/all.hpp>

namespace antartar
{
	class vk {
	private:
		VkInstance instance_;

		inline void initialize_()
		{
			VkApplicationInfo app_info{};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = "antartar";
			app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
			app_info.pEngineName = "no engine";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = VK_API_VERSION_1_0;

			VkInstanceCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = std::addressof(app_info);
			uint32_t glfw_extensions_count = 0;
			create_info.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(std::addressof(glfw_extensions_count));
			create_info.enabledExtensionCount = glfw_extensions_count;
			create_info.enabledLayerCount = 0;

			uint32_t extension_count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, std::addressof(extension_count), nullptr);
			std::pmr::vector<VkExtensionProperties> extensions(extension_count);
			vkEnumerateInstanceExtensionProperties(nullptr, std::addressof(extension_count), extensions.data());

			ranges::for_each(extensions, [](const VkExtensionProperties& prop) {
				fmt::print("\t{}\n", prop.extensionName);
				});


			if (vkCreateInstance(std::addressof(create_info), nullptr, std::addressof(instance_)) != VK_SUCCESS)
			{
				throw std::runtime_error(log_message("failed to create vk instance"));
			}
		}

		inline void cleanup_()
		{
			vkDestroyInstance(instance_, nullptr);
		}

	public:
		inline vk()
		{
			initialize_();
		}
		inline ~vk()
		{
			cleanup_();
		}
	};
}