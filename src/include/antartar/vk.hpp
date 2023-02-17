#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <antartar/file.hpp>
#include <antartar/log.hpp>
#include <gsl/gsl>
#include <optional>
#include <range/v3/all.hpp>
#include <set>
#include <string>
#include <vector>

namespace antartar::vk {
auto equals(auto lhs, auto rhs) -> bool
    requires std::equality_comparable_with<std::remove_cvref_t<decltype(lhs)>,
                                           std::remove_cvref_t<decltype(rhs)>>
{
    return lhs == rhs;
}

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
    if (not equals(nullptr, func)) {
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
    if (equals(nullptr, func)) {
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

struct swap_chain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
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
    VkSwapchainKHR swap_chain_                = VK_NULL_HANDLE;
    std::pmr::vector<VkImage> swap_chain_images_{};
    VkFormat swap_chain_image_format_ = VkFormat::VK_FORMAT_UNDEFINED;
    VkExtent2D swap_chain_extent_{};
    std::pmr::vector<VkImageView> swap_chain_image_views_{};

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

    inline auto find_queue_families_(VkPhysicalDevice device) const
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

    inline auto check_device_extension_support_(VkPhysicalDevice device) const
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

    inline auto query_swap_chain_support_(VkPhysicalDevice device) const
    {
        swap_chain_support_details details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            device,
            surface_,
            std::addressof(details.capabilities));

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                             surface_,
                                             std::addressof(format_count),
                                             nullptr);
        if (not equals(format_count, 0)) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                                 surface_,
                                                 std::addressof(format_count),
                                                 details.formats.data());
        }

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface_,
            std::addressof(present_mode_count),
            nullptr);
        if (not equals(present_mode_count, 0)) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface_,
                std::addressof(present_mode_count),
                details.present_modes.data());
        }
        return details;
    }

    inline auto is_device_suitable_(VkPhysicalDevice device) const
    {
        auto indices = find_queue_families_(device);
        const auto extensions_supported =
            check_device_extension_support_(device);
        auto swap_chain_adequate = false;
        if (extensions_supported) {
            auto swap_chain_support = query_swap_chain_support_(device);
            swap_chain_adequate =
                (not swap_chain_support.formats.empty())
                and (not swap_chain_support.present_modes.empty());
        }

        return indices.is_complete() && extensions_supported
               && swap_chain_adequate;
    }

    inline auto pick_physical_device_()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_,
                                   std::addressof(device_count),
                                   nullptr);
        if (equals(0, device_count)) {
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

        create_info.pEnabledFeatures = std::addressof(device_features);
        create_info.enabledExtensionCount =
            static_cast<uint32_t>(device_extensions.size());
        create_info.ppEnabledExtensionNames = device_extensions.data();

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

    inline VkSurfaceFormatKHR choose_swap_surface_format_(
        const std::vector<VkSurfaceFormatKHR>& available_formats) const
    {
        auto srgb_supported = [](const VkSurfaceFormatKHR& format) {
            return VK_FORMAT_B8G8R8A8_SRGB == format.format
                   && VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == format.colorSpace;
        };
        if (auto format = ranges::find_if(available_formats, srgb_supported);
            format != std::end(available_formats)) {
            return *format;
        }
        // settle with first available
        return available_formats.front();
    }

    inline VkPresentModeKHR choose_swap_present_mode_(
        const std::vector<VkPresentModeKHR>& available_present_modes) const
    {
        auto mailbox_supported = [](const VkPresentModeKHR& mode) {
            return VK_PRESENT_MODE_MAILBOX_KHR == mode;
        };
        if (auto mode =
                ranges::find_if(available_present_modes, mailbox_supported);
            mode != std::end(available_present_modes)) {
            return *mode;
        }
        // this is always present
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    inline VkExtent2D
    choose_swap_extent_(const VkSurfaceCapabilitiesKHR& capabilities,
                        auto& window) const
    {
        if (capabilities.currentExtent.width
            != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window,
                                   std::addressof(width),
                                   std::addressof(height));
            VkExtent2D actual_extent = {.width  = static_cast<uint32_t>(height),
                                        .height = static_cast<uint32_t>(width)};
            actual_extent.width      = std::clamp(actual_extent.width,
                                             capabilities.minImageExtent.width,
                                             capabilities.maxImageExtent.width);
            actual_extent.height =
                std::clamp(actual_extent.height,
                           capabilities.minImageExtent.height,
                           capabilities.maxImageExtent.height);
            return actual_extent;
        }
    }

    inline auto create_swap_chain_(auto& window)
    {
        auto swap_chain_support = query_swap_chain_support_(physical_device_);

        auto surface_format =
            choose_swap_surface_format_(swap_chain_support.formats);
        auto present_mode =
            choose_swap_present_mode_(swap_chain_support.present_modes);
        auto extent =
            choose_swap_extent_(swap_chain_support.capabilities, window);

        // minimum + one more so we dont wait on the driver
        auto image_count = swap_chain_support.capabilities.minImageCount + 1;
        // make sure we don't exceed maxImageCount, where 0 means it's unbounded
        if (swap_chain_support.capabilities.maxImageCount > 0
            && image_count > swap_chain_support.capabilities.maxImageCount) {
            image_count = swap_chain_support.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = surface_;

        create_info.minImageCount    = image_count;
        create_info.imageFormat      = surface_format.format;
        create_info.imageColorSpace  = surface_format.colorSpace;
        create_info.imageExtent      = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto indices = find_queue_families_(physical_device_);
        std::array queue_family_indices = {indices.graphics_family.value(),
                                           indices.present_family.value()};
        if (indices.graphics_family != indices.present_family) {
            create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices   = queue_family_indices.data();
        }
        else {
            create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0;       // optional
            create_info.pQueueFamilyIndices   = nullptr; // optional
        }

        create_info.preTransform =
            swap_chain_support.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode    = present_mode;
        create_info.clipped        = VK_TRUE;
        create_info.oldSwapchain   = VK_NULL_HANDLE;

        if (VK_SUCCESS
            != vkCreateSwapchainKHR(device_,
                                    std::addressof(create_info),
                                    nullptr,
                                    std::addressof(swap_chain_))) {
            throw std::runtime_error(
                log_message("failed to create swap chain!"));
        }

        vkGetSwapchainImagesKHR(device_,
                                swap_chain_,
                                std::addressof(image_count),
                                nullptr);
        swap_chain_images_.resize(image_count);
        vkGetSwapchainImagesKHR(device_,
                                swap_chain_,
                                std::addressof(image_count),
                                swap_chain_images_.data());
        swap_chain_image_format_ = surface_format.format;
        swap_chain_extent_       = extent;
    }

    inline auto create_image_views_()
    {
        swap_chain_image_views_.resize(swap_chain_images_.size());
        for (const auto& [i, image] :
             swap_chain_images_ | ranges::views::enumerate) {
            VkImageViewCreateInfo create_info{};
            create_info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image        = swap_chain_images_.at(i);
            create_info.viewType     = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format       = swap_chain_image_format_;
            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel   = 0;
            create_info.subresourceRange.levelCount     = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount     = 1;

            if (VK_SUCCESS
                != vkCreateImageView(
                    device_,
                    std::addressof(create_info),
                    nullptr,
                    std::addressof(swap_chain_image_views_.at(i)))) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    inline VkShaderModule create_shader_module_(const auto& code)
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code->size();
        create_info.pCode    = reinterpret_cast<const uint32_t*>(code->data());

        VkShaderModule shader_module;
        if (VK_SUCCESS
            != vkCreateShaderModule(device_,
                                    std::addressof(create_info),
                                    nullptr,
                                    std::addressof(shader_module))) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shader_module;
    }

    inline auto create_graphics_pipeline_()
    {
        auto vert_shader_code = file::read(
            file::path::join(ANTARTAR_SHADERS_DIRECTORY, "shader.vert.spv"));
        auto vert_shader_module = create_shader_module_(vert_shader_code);
        auto frag_shader_code   = file::read(
            file::path::join(ANTARTAR_SHADERS_DIRECTORY, "shader.frag.spv"));
        auto frag_shader_module = create_shader_module_(frag_shader_code);

        VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
        vert_shader_stage_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_stage_info.module = vert_shader_module;
        vert_shader_stage_info.pName  = "main";

        VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
        frag_shader_stage_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_stage_info.module = frag_shader_module;
        frag_shader_stage_info.pName  = "main";

        std::array shader_stages = {vert_shader_stage_info,
                                    frag_shader_stage_info};

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount   = 0;
        vertex_input_info.pVertexBindingDescriptions      = nullptr;
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        vertex_input_info.pVertexAttributeDescriptions    = nullptr;

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(swap_chain_extent_.width);
        viewport.height   = static_cast<float>(swap_chain_extent_.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swap_chain_extent_;

        vkDestroyShaderModule(device_, frag_shader_module, nullptr);
        vkDestroyShaderModule(device_, vert_shader_module, nullptr);

        std::array dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount =
            static_cast<uint32_t>(dynamic_states.size());
        dynamic_state.pDynamicStates = dynamic_states.data();

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount  = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth               = 1.0f;
        rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable         = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp          = 0.0f;
        rasterizer.depthBiasSlopeFactor    = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable   = VK_FALSE;
        multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading      = 1.0f;
        multisampling.pSampleMask           = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable      = VK_FALSE;
    }

  public:
    inline vk(auto& window)
    {
        create_vk_instance_();
        setup_debug_messenger_();
        create_surface_(window);
        pick_physical_device_();
        create_logical_device_();
        create_swap_chain_(window);
        create_image_views_();
        create_graphics_pipeline_();
    }

    inline ~vk()
    {
        ranges::for_each(swap_chain_image_views_,
                         [this](const VkImageView& image_view) {
                             vkDestroyImageView(device_, image_view, nullptr);
                         });
        vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
        vkDestroyDevice(device_, nullptr);
        if (enable_validation_layers) {
            DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
        }
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }
};
} // namespace antartar::vk
