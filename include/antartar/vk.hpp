#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <antartar/log.hpp>
#include <gsl/gsl>
#include <optional>
#include <range/v3/all.hpp>
#include <set>
#include <string>
#include <vector>

namespace antartar::vk {
constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};

constexpr std::array device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

constexpr bool enable_validation_layers = ANTARTAR_IS_DEBUG;

static inline VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
               VkDebugUtilsMessageTypeFlagsEXT message_type,
               const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
               void* user_data)
{
    fmt::print(stderr, "validation layer: {}\n", callback_data->pMessage);
    return VK_FALSE;
}

static inline VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger)
{
    constexpr auto func_name = "vkCreateDebugUtilsMessengerEXT"sv;
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, func_name.data()));
    if (nullptr != func) {
        return func(instance, create_info, allocator, debug_messenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static inline void
DestroyDebugUtilsMessengerEXT(VkInstance instance,
                              VkDebugUtilsMessengerEXT debug_messenger,
                              const VkAllocationCallbacks* allocator)
{
    constexpr auto func_name = "vkDestroyDebugUtilsMessengerEXT"sv;
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, func_name.data()));
    if (nullptr != func) {
        func(instance, debug_messenger, allocator);
    }
}

struct queue_family_indices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    inline auto is_complete() const
    {
        return graphics_family.has_value() and present_family.has_value();
    }
};

class vk {
  private:
    VkInstance instance_                      = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_                     = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_         = VK_NULL_HANDLE;
    VkDevice device_                          = VK_NULL_HANDLE;
    VkQueue graphics_queue_                   = VK_NULL_HANDLE;
    VkQueue present_queue_                    = VK_NULL_HANDLE;

    inline bool check_validation_layer_support_()
    {
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(std::addressof(layer_count),
                                           nullptr);

        std::pmr::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(std::addressof(layer_count),
                                           available_layers.data());
        return ranges::all_of(validation_layers,
                              [&available_layers](std::string_view layer_name) {
                                  return ranges::contains(
                                      available_layers,
                                      layer_name,
                                      [](const VkLayerProperties& prop) {
                                          return prop.layerName;
                                      });
                              });
    }

    inline auto populate_debug_messenger_create_info_(
        VkDebugUtilsMessengerCreateInfoEXT& create_info)
    {
        create_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
        create_info.pUserData       = nullptr;
    }

    inline auto get_required_extensions_()
    {
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions  = nullptr;
        glfw_extensions               = glfwGetRequiredInstanceExtensions(
            std::addressof(glfw_extension_count));
        std::vector<const char*> extensions(glfw_extensions,
                                            glfw_extensions
                                                + glfw_extension_count);
        if (enable_validation_layers) {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }

    inline auto setup_debug_messenger_()
    {
        if (not enable_validation_layers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT create_info{};
        populate_debug_messenger_create_info_(create_info);

        if (VK_SUCCESS
            != CreateDebugUtilsMessengerEXT(instance_,
                                            std::addressof(create_info),
                                            nullptr,
                                            std::addressof(debug_messenger_))) {
            throw std::runtime_error(
                log_message("failed to set up debug messenger!"));
        }
    }

    inline auto find_queue_families_(VkPhysicalDevice device)
    {
        queue_family_indices indices;

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            std::addressof(queue_family_count),
            nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            std::addressof(queue_family_count),
            queue_families.data());

        for (const auto& [i, family] :
             queue_families | ranges::view::enumerate) {
            // pick graphics family
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics_family = static_cast<uint32_t>(i);
            }
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                device,
                i,
                surface_,
                std::addressof(present_support));
            if (present_support) {
                indices.present_family = static_cast<uint32_t>(i);
            }
        }
        return indices;
    }

    inline auto check_device_extension_support_(VkPhysicalDevice device)
    {
        uint32_t extensions_count = 0;
        vkEnumerateDeviceExtensionProperties(device,
                                             nullptr,
                                             std::addressof(extensions_count),
                                             nullptr);
        std::vector<VkExtensionProperties> available_extensions(
            extensions_count);
        vkEnumerateDeviceExtensionProperties(device,
                                             nullptr,
                                             std::addressof(extensions_count),
                                             available_extensions.data());

        std::set<std::string_view> required_extensions(
            std::begin(device_extensions),
            std::end(device_extensions));

        for (const auto& ext : available_extensions) {
            required_extensions.erase(ext.extensionName);
        }
        return required_extensions.empty();
    }

    inline auto is_device_suitable_(VkPhysicalDevice device)
    {
        auto indices              = find_queue_families_(device);
        auto extensions_supported = check_device_extension_support_(device);
        return indices.is_complete() && extensions_supported;
    }

    inline auto pick_physical_device_()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_,
                                   std::addressof(device_count),
                                   nullptr);
        if (0 == device_count) {
            throw std::runtime_error(
                log_message("failed to find GPUs with Vulkan support!"));
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_,
                                   std::addressof(device_count),
                                   devices.data());

        auto find_physical_device = [this](const auto& devices) {
            return ranges::find_if(devices, [this](VkPhysicalDevice device) {
                return is_device_suitable_(device);
            });
        };

        if (auto result = find_physical_device(devices);
            result != devices.end()) {
            physical_device_ = *result;
        }
        else {
            throw std::runtime_error(
                log_message("failed to find suitable GPU!"));
        }
    }

    void create_logical_device_()
    {
        auto indices = find_queue_families_(physical_device_);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = {*indices.graphics_family,
                                                    *indices.present_family};
        auto queue_priority                      = 1.0f;

        for (uint32_t queue_family : unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType =
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount       = 1;
            queue_create_info.pQueuePriorities = std::addressof(queue_priority);
            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.queueCreateInfoCount =
            static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();

        create_info.pEnabledFeatures      = std::addressof(device_features);
        create_info.enabledExtensionCount = 0;

        if (enable_validation_layers) {
            create_info.enabledLayerCount =
                static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        }
        else {
            create_info.enabledLayerCount = 0;
        }

        if (VK_SUCCESS
            != vkCreateDevice(physical_device_,
                              std::addressof(create_info),
                              nullptr,
                              std::addressof(device_))) {
            throw std::runtime_error(
                log_message("failed to create logical device!"));
        }
        vkGetDeviceQueue(device_,
                         indices.graphics_family.value(),
                         0,
                         std::addressof(graphics_queue_));
        vkGetDeviceQueue(device_,
                         indices.graphics_family.value(),
                         0,
                         std::addressof(present_queue_));
    }

    inline auto create_surface_(auto& window)
    {
        if (VK_SUCCESS
            != glfwCreateWindowSurface(instance_,
                                       window,
                                       nullptr,
                                       std::addressof(surface_))) {
            throw std::runtime_error(
                log_message("failed to create window surface!"));
        }
    }

    inline auto create_vk_instance_()
    {
        if (enable_validation_layers && !check_validation_layer_support_()) {
            throw std::runtime_error(
                log_message("validation layers requested, but not available!"));
        }

        VkApplicationInfo app_info{};
        app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName   = "antartar";
        app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        app_info.pEngineName        = "no engine";
        app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion         = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info{};
        create_info.sType             = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo  = std::addressof(app_info);
        create_info.enabledLayerCount = 0;

        auto extensions = get_required_extensions_();
        create_info.enabledExtensionCount =
            static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (enable_validation_layers) {
            create_info.enabledLayerCount =
                static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populate_debug_messenger_create_info_(debug_create_info);
            create_info.pNext = std::addressof(debug_create_info);
        }
        else {
            create_info.enabledLayerCount = 0;
            create_info.pNext             = nullptr;
        }

        if (VK_SUCCESS
            != vkCreateInstance(std::addressof(create_info),
                                nullptr,
                                std::addressof(instance_))) {
            throw std::runtime_error(
                log_message("failed to create vk instance"));
        }
    }

  public:
    inline vk(auto& window)
    {
        create_vk_instance_();
        setup_debug_messenger_();
        create_surface_(window);
        pick_physical_device_();
        create_logical_device_();
    }

    inline ~vk()
    {
        vkDestroyDevice(device_, nullptr);
        if (enable_validation_layers) {
            DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
        }
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }
};
} // namespace antartar::vk