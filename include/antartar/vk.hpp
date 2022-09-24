#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gsl/gsl>
#include <vector>
#include <range/v3/all.hpp>

namespace antartar
{
	const std::array<const char*, 1> validation_layers = {
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

		inline	void log_available_extensions_() const
		{
			uint32_t extension_count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, std::addressof(extension_count), nullptr);
			std::pmr::vector<VkExtensionProperties> extensions(extension_count);
			vkEnumerateInstanceExtensionProperties(nullptr, std::addressof(extension_count), extensions.data());

			ranges::for_each(extensions, [](const VkExtensionProperties& prop) {
				fmt::print("\t{}\n", prop.extensionName);
				});
		}

		inline	void init_app_info_(VkApplicationInfo& app_info) const
		{
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = "antartar";
			app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
			app_info.pEngineName = "no engine";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = VK_API_VERSION_1_0;
		}

		inline void create_info_(VkApplicationInfo& app_info, VkInstanceCreateInfo& create_info) const
		{
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = std::addressof(app_info);
			uint32_t glfw_extensions_count = 0;
			create_info.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(std::addressof(glfw_extensions_count));
			create_info.enabledExtensionCount = glfw_extensions_count;
			create_info.enabledLayerCount = 0;
		}

		inline void configure_validation_layers_(VkInstanceCreateInfo& create_info)
		{
			if (enable_validation_layers && !check_validation_layer_support()) {
				throw std::runtime_error(log_message("validation layers requested, but not available!"));
			}

			// NOTE: this probably overrides previous layers settings (if there is any), but for now it's fine
			if (enable_validation_layers) {
				create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
				create_info.ppEnabledLayerNames = validation_layers.data();
			}
			else {
				create_info.enabledLayerCount = 0;
			}

		}

	public:
		inline vk()
		{
			VkApplicationInfo app_info{};
			init_app_info_(app_info);
			VkInstanceCreateInfo create_info{};
			create_info_(app_info, create_info);
			log_available_extensions_();
			configure_validation_layers_(create_info);


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