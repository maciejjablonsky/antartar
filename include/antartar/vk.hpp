#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gsl/gsl>
#include <vector>
#include <range/v3/all.hpp>

namespace antartar
{
	const std::array<std::string_view, 1> validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	constexpr bool enable_validation_layers = ANTARTAR_IS_DEBUG;


	class vk {
	private:
		VkInstance instance_;

		inline bool check_validation_layer_support()
		{
			uint32_t layer_count = 0;
			vkEnumerateInstanceLayerProperties(std::addressof(layer_count), nullptr);

			std::pmr::vector<VkLayerProperties> available_layers(layer_count);
			vkEnumerateInstanceLayerProperties(std::addressof(layer_count), available_layers.data());
			return  ranges::all_of(validation_layers, [&available_layers](std::string_view layer_name) {
				return ranges::contains(available_layers, layer_name, [](const VkLayerProperties& prop) {
					return prop.layerName; });
				});
		}

		inline	void log_available_extensions() const
		{
			uint32_t extension_count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, std::addressof(extension_count), nullptr);
			std::pmr::vector<VkExtensionProperties> extensions(extension_count);
			vkEnumerateInstanceExtensionProperties(nullptr, std::addressof(extension_count), extensions.data());

			ranges::for_each(extensions, [](const VkExtensionProperties& prop) {
				fmt::print("\t{}\n", prop.extensionName);
				});
		}

		VkApplicationInfo app_info_() const
		{
			VkApplicationInfo app_info{};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = "antartar";
			app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
			app_info.pEngineName = "no engine";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = VK_API_VERSION_1_0;
			return app_info;
		}

		VkInstanceCreateInfo create_info_() const
		{
			auto app_info = app_info_();
			VkInstanceCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = std::addressof(app_info);
			uint32_t glfw_extensions_count = 0;
			create_info.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(std::addressof(glfw_extensions_count));
			create_info.enabledExtensionCount = glfw_extensions_count;
			create_info.enabledLayerCount = 0;
			return create_info;
		}

	public:
		inline vk()
		{
			auto create_info = create_info_();
			log_available_extensions();
			if (enable_validation_layers && !check_validation_layer_support()) {
				throw std::runtime_error(log_message("validation layers requested, but not available!"));
			}


			if (vkCreateInstance(std::addressof(create_info), nullptr, std::addressof(instance_)) != VK_SUCCESS)
			{
				throw std::runtime_error(log_message("failed to create vk instance"));
			}
		}

		inline ~vk()
		{
			vkDestroyInstance(instance_, nullptr);
		}
	};
}