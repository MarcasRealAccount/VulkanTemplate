#include <cstdint>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <vulkan.hpp>

#include <vk_mem_alloc.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define VULKAN_PROGRAM_NAME "VulkanProgram"
#define VULKAN_PROGRAM_VERSION VK_MAKE_API_VERSION(0, 0, 1, 0)
#define VULKAN_ENGINE_NAME "VulkanEngine"
#define VULKAN_ENGINE_VERSION VK_MAKE_API_VERSION(0, 0, 1, 0)
#define VULKAN_MIN_VERSION VK_API_VERSION_1_0

#define VULKAN_VSYNC false
#define VULKAN_MAX_FRAMES_IN_FLIGHT 2

#ifdef _DEBUG
static std::string vulkanGetMessageSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
	switch (severity) {
	case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Verbose";
	case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: return "Info";
	case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warning";
	case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: return "Error";
	default: return "Unknown";
	}
}

static std::string vulkanGetMessageTypes(VkDebugUtilsMessageTypeFlagsEXT types) {
	bool added = false;
	std::string str;
	if (types & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
		added = true;
		str += "General";
		types &= ~VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	}
	if (types & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
		if (added) str += " | ";
		added = true;
		str += "Validation";
		types &= ~VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	}
	if (types & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
		if (added) str += " | ";
		added = true;
		str += "Performance";
		types &= ~VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	}
	if (types != 0) str = "(" + str + ") + " + std::to_string(types);
	return str;
}

static VkBool32 vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::string message = "VK Validation Layer " + vulkanGetMessageSeverity(messageSeverity) + " (" + vulkanGetMessageTypes(messageTypes) + "): " + pCallbackData->pMessage + "\n";
	if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		std::cerr << message;
	else
		std::cout << message;
	return false;
}
#endif

int main(int argc, char** argv) {
	try {
		// Initialize GLFW
		if (!glfwInit()) {
			std::cerr << "GLFW failed to initialize!" << std::endl;
			return EXIT_FAILURE;
		}

		// Set window hints
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // INFO: Disable resizing, enable once you can recreate the swapchain without lots of code. :)

		// Create window
		GLFWwindow* windowPtr = glfwCreateWindow(1280, 720, VULKAN_PROGRAM_NAME, nullptr, nullptr);

		// Create Vulkan Instance
		vk::Instance vulkanInstance;
		{
			vk::ApplicationInfo appInfo = { VULKAN_PROGRAM_NAME, VULKAN_PROGRAM_VERSION, VULKAN_ENGINE_NAME, VULKAN_ENGINE_VERSION, VULKAN_MIN_VERSION };

			std::vector<const char*> enabledLayerNames;
			std::vector<const char*> enabledExtensionNames;

#ifdef _DEBUG
			// Add validation layer if in Debug mode
			enabledLayerNames.push_back("VK_LAYER_KHRONOS_validation");
#endif

			// Get required Instance extensions from GLFW
			std::uint32_t glfwExtensionCount;
			const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			// Add the extensions
			enabledExtensionNames.resize(glfwExtensionCount);
			for (std::size_t i = 0; i < glfwExtensionCount; ++i)
				enabledExtensionNames[i] = glfwExtensions[i];

#ifdef _DEBUG
			// Add debug utils extension in Debug mode
			enabledExtensionNames.push_back("VK_EXT_debug_utils");
#endif

			vk::InstanceCreateInfo createInfo = { {}, &appInfo, enabledLayerNames, enabledExtensionNames };

#ifdef _DEBUG
			vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = { {}, vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError, vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance, &vulkanDebugCallback };

			createInfo.pNext = &debugCreateInfo;
#endif

			vulkanInstance = vk::createInstance(createInfo);
		}

		// Get API Version
		std::uint32_t vulkanAPIVersion;
		if (vk::enumerateInstanceVersion(&vulkanAPIVersion) != vk::Result::eSuccess) vulkanAPIVersion = VULKAN_MIN_VERSION;

#ifdef _DEBUG
		// Create Vulkan Debug Messenger
		vk::DebugUtilsMessengerEXT vulkanDebugMessenger = vulkanInstance.createDebugUtilsMessengerEXT({ {}, /*vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | */ vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError, vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance, &vulkanDebugCallback });
#endif

		// Create surface from window
		vk::SurfaceKHR vulkanSurface;
		{
			VkSurfaceKHR surface;
			vulkanSurface = vk::createResultValue(static_cast<vk::Result>(glfwCreateWindowSurface(vulkanInstance, windowPtr, nullptr, &surface)), surface, "glfwCreateWindowSurface");
		}

		// Pick the best physical device
		vk::PhysicalDevice vulkanPhysicalDevice;
		{
			std::size_t bestScore = 0L;

			// Enumerate all physical devices from the Vulkan Instance and iterate through them
			for (auto& physicalDevice : vulkanInstance.enumeratePhysicalDevices()) {
				std::size_t score = 0L;

				// Get properties and features from the current physical device
				auto properties = physicalDevice.getProperties();
				auto features   = physicalDevice.getFeatures();

				// Prioritize better gpus
				switch (properties.deviceType) {
				case vk::PhysicalDeviceType::eCpu: score += 1L; break;
				case vk::PhysicalDeviceType::eOther: score += 2L; break;
				case vk::PhysicalDeviceType::eVirtualGpu: score += 3L; break;
				case vk::PhysicalDeviceType::eIntegratedGpu: score += 4L; break;
				case vk::PhysicalDeviceType::eDiscreteGpu: score += 5L; break;
				}

				// If this physical device scored better than any previous ones, then select this one
				if (bestScore == 0L || score > bestScore) {
					bestScore            = score;
					vulkanPhysicalDevice = physicalDevice;
				}
			}
		}

		// Find Graphics & Present queue family index
		std::uint32_t graphicsFamilyIndex = 0;
		{
			auto queueFamilies = vulkanPhysicalDevice.getQueueFamilyProperties();

			std::uint32_t i = 0;
			for (auto& queueFamily : queueFamilies) {
				if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && vulkanPhysicalDevice.getSurfaceSupportKHR(i, vulkanSurface)) {
					graphicsFamilyIndex = i;
					break;
				}

				++i;
			}
		}

		// Create Vulkan Device and get graphics queue
		vk::Device vulkanDevice;
		vk::Queue vulkanGraphicsQueue;
		{
			std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
			std::vector<float> queuePriorities = { 1.0f };
			deviceQueueCreateInfos.push_back({ {}, graphicsFamilyIndex, queuePriorities });

			std::vector<const char*> enabledLayerNames;

#ifdef _DEBUG
			enabledLayerNames.push_back("VK_LAYER_KHRONOS_validation");
#endif

			std::vector<const char*> enabledExtensionNames = { "VK_KHR_swapchain" };

			vk::PhysicalDeviceFeatures enabledFeatures = {};

			vulkanDevice        = vulkanPhysicalDevice.createDevice({ {}, deviceQueueCreateInfos, enabledLayerNames, enabledExtensionNames, &enabledFeatures });
			vulkanGraphicsQueue = vulkanDevice.getQueue(graphicsFamilyIndex, 0);
		}

		// Create a Vulkan Memory Allocator instance
		VmaAllocator vmaAllocator;
		{
			VmaAllocatorCreateInfo createInfo = {};
			createInfo.vulkanApiVersion       = vulkanAPIVersion;
			createInfo.instance               = vulkanInstance;
			createInfo.physicalDevice         = vulkanPhysicalDevice;
			createInfo.device                 = vulkanDevice;

			vmaCreateAllocator(&createInfo, &vmaAllocator);
		}

		// Create Vulkan Command Pools for graphics following formula 'F * T', F = Frames In Flight, T = Number of Threads
		// Indexed 'F + T * NT', F = Current Frame, T = Current Thread, NT = Number of Threads
		std::vector<vk::CommandPool> vulkanCommandPools;
		std::vector<std::vector<vk::CommandBuffer>> vulkanCommandBuffers;
		{
			std::size_t threadCount = 1; // Here we won't go into multithreading so we just use 1 as the thread count.
			vulkanCommandPools.resize(VULKAN_MAX_FRAMES_IN_FLIGHT * threadCount);
			vulkanCommandBuffers.resize(VULKAN_MAX_FRAMES_IN_FLIGHT * threadCount);
			for (std::size_t i = 0; i < vulkanCommandPools.size(); ++i) {
				// Create a command pool
				vulkanCommandPools[i] = vulkanDevice.createCommandPool({ {}, graphicsFamilyIndex });

				// Allocate command buffers for the pool, currently that's just 1 primary buffer
				vulkanCommandBuffers[i] = vulkanDevice.allocateCommandBuffers({ vulkanCommandPools[i], vk::CommandBufferLevel::ePrimary, 1 });
			}
		}

		// Create synchronization objects
		std::vector<vk::Semaphore> vulkanImageAvailableSemaphores;
		std::vector<vk::Semaphore> vulkanRenderFinishedSemaphores;
		std::vector<vk::Fence> vulkanInFlightFences;
		{
			vulkanImageAvailableSemaphores.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
			vulkanRenderFinishedSemaphores.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
			vulkanInFlightFences.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);

			// Create two semaphores and one fence for every frame in flight
			for (std::size_t i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; ++i) {
				vulkanImageAvailableSemaphores[i] = vulkanDevice.createSemaphore({ vk::SemaphoreCreateFlags {} });
				vulkanRenderFinishedSemaphores[i] = vulkanDevice.createSemaphore({ vk::SemaphoreCreateFlags {} });
				vulkanInFlightFences[i]           = vulkanDevice.createFence({ vk::FenceCreateFlagBits::eSignaled });
			}
		}

		// Create Vulkan Swapchain
		vk::Format vulkanSwapchainFormat                       = vk::Format::eUndefined;
		vk::ColorSpaceKHR vulkanSwapchainColorSpace            = vk::ColorSpaceKHR::eSrgbNonlinear;
		vk::PresentModeKHR vulkanSwapchainPresentMode          = vk::PresentModeKHR::eFifo;
		vk::SurfaceCapabilitiesKHR vulkanSwapchainCapabilities = {};
		vk::Extent2D vulkanSwapchainExtent                     = {};
		vk::SwapchainKHR vulkanSwapchain;
		vk::RenderPass vulkanRenderPass;
		std::vector<vk::Image> vulkanSwapchainImages;
		std::vector<vk::Image> vulkanSwapchainDepthImages;
		std::vector<VmaAllocation> vulkanSwapchainDepthImageAllocations;
		std::vector<vk::ImageView> vulkanSwapchainImageViews;
		std::vector<vk::ImageView> vulkanSwapchainDepthImageViews;
		std::vector<vk::Framebuffer> vulkanSwapchainFramebuffers;
		std::vector<vk::Fence> vulkanImagesInFlight;
		std::size_t currentImage = 0;
		std::size_t currentFrame = 0;
		{
			// INFO: Most of this should be able to be moved into a separate function to support recreating the swapchain when the window resizes

			std::uint32_t imageCount = 0;

			// Get Vulkan Swapchain Details
			{
				vulkanSwapchainCapabilities = vulkanPhysicalDevice.getSurfaceCapabilitiesKHR(vulkanSurface);

				// Get swapchain extent
				if (vulkanSwapchainCapabilities.currentExtent.width != -1) vulkanSwapchainExtent = vulkanSwapchainCapabilities.currentExtent;

				std::int32_t fw, fh;
				glfwGetFramebufferSize(windowPtr, &fw, &fh);
				vulkanSwapchainExtent.width  = std::clamp(static_cast<std::uint32_t>(fw), vulkanSwapchainCapabilities.minImageExtent.width, vulkanSwapchainCapabilities.maxImageExtent.width);
				vulkanSwapchainExtent.height = std::clamp(static_cast<std::uint32_t>(fh), vulkanSwapchainCapabilities.minImageExtent.height, vulkanSwapchainCapabilities.maxImageExtent.height);

				// Get image count
				imageCount = std::min(vulkanSwapchainCapabilities.minImageCount + 1, vulkanSwapchainCapabilities.maxImageCount);

				// Get swapchain format and color space
				auto formats = vulkanPhysicalDevice.getSurfaceFormatsKHR(vulkanSurface);
				for (auto& format : formats) {
					if (format.format == vk::Format::eB8G8R8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
						vulkanSwapchainFormat     = format.format;
						vulkanSwapchainColorSpace = format.colorSpace;
						break;
					}
				}
				vulkanSwapchainFormat     = formats[0].format;
				vulkanSwapchainColorSpace = formats[0].colorSpace;

				// Get swapchain present mode
				// INFO: I do not recomment using a macro for wether the app is using vsync or not.
				//       It should instead be a variable, but as I couldn't care to add that...
				auto presentModes = vulkanPhysicalDevice.getSurfacePresentModesKHR(vulkanSurface);
				for (auto& presentMode : presentModes) {
#ifdef VULKAN_VSYNC
					if (presentMode == vk::PresentModeKHR::eFifo)
						vulkanSwapchainPresentMode = presentMode;
#else
					if (presentMode == vk::PresentModeKHR::eMailbox)
						vulkanSwapchainPresentMode = presentMode;
#endif
				}
				vulkanSwapchainPresentMode = vk::PresentModeKHR::eFifo;
			}

			// Create Vulkan Render Pass
			{
				std::vector<vk::AttachmentDescription> attachments;
				std::vector<vk::SubpassDescription> subpasses;
				std::vector<vk::SubpassDependency> dependencies;

				// Add Color attachment
				attachments.push_back({ {}, vulkanSwapchainFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR });

				// Add Depth attachment
				attachments.push_back({ {}, vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal });

				// Add subpass
				std::vector<vk::AttachmentReference> colorAttachments;
				vk::AttachmentReference depthStencilAttachment;
				colorAttachments.push_back({ 0, vk::ImageLayout::eColorAttachmentOptimal });
				depthStencilAttachment = { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };
				subpasses.push_back({ {}, vk::PipelineBindPoint::eGraphics, {}, colorAttachments, {}, &depthStencilAttachment, {} });

				// Add dependency
				dependencies.push_back({ ~0U, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite, {} });

				vulkanRenderPass = vulkanDevice.createRenderPass({ {}, attachments, subpasses, dependencies });
			}

			// Create Vulkan Swapchain
			{
				std::vector<std::uint32_t> swapchainIndices = { graphicsFamilyIndex };

				vulkanSwapchain       = vulkanDevice.createSwapchainKHR({ {}, vulkanSurface, imageCount, vulkanSwapchainFormat, vulkanSwapchainColorSpace, vulkanSwapchainExtent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, swapchainIndices, vulkanSwapchainCapabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, vulkanSwapchainPresentMode, true, vulkanSwapchain });
				vulkanSwapchainImages = vulkanDevice.getSwapchainImagesKHR(vulkanSwapchain);

				// Create swapchain images, image views and framebuffers
				vulkanSwapchainImageViews.resize(imageCount);
				vulkanSwapchainDepthImages.resize(imageCount);
				vulkanSwapchainDepthImageAllocations.resize(imageCount);
				vulkanSwapchainDepthImageViews.resize(imageCount);
				vulkanSwapchainFramebuffers.resize(imageCount);
				for (std::size_t i = 0; i < imageCount; ++i) {
					// Create Image View of current swapchain image
					vulkanSwapchainImageViews[i] = vulkanDevice.createImageView({ {}, vulkanSwapchainImages[i], vk::ImageViewType::e2D, vulkanSwapchainFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } });

					// Create depth image for current swapchain image
					{
						vk::ImageCreateInfo depthImageCreateInfo         = { {}, vk::ImageType::e2D, vk::Format::eD32Sfloat, { vulkanSwapchainExtent.width, vulkanSwapchainExtent.height, 1 }, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, nullptr, vk::ImageLayout::eUndefined };
						VkImageCreateInfo depthImageCreateInfo_          = depthImageCreateInfo;
						VmaAllocationCreateInfo depthImageAllocationInfo = { {}, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, 0, 0, 0, 0.0f };
						VkImage depthImage_;
						VmaAllocation depthImageAllocation;
						// This both creates an image and allocates memory for it
						vulkanSwapchainDepthImages[i]           = vk::createResultValue(static_cast<vk::Result>(vmaCreateImage(vmaAllocator, &depthImageCreateInfo_, &depthImageAllocationInfo, &depthImage_, &depthImageAllocation, nullptr)), depthImage_, "vmaCreateImage");
						vulkanSwapchainDepthImageAllocations[i] = depthImageAllocation;
					}

					// Create image view for current depth image
					vulkanSwapchainDepthImageViews[i] = vulkanDevice.createImageView({ {}, vulkanSwapchainDepthImages[i], vk::ImageViewType::e2D, vk::Format::eD32Sfloat, {}, { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 } });

					// Create framebuffer for current swapchain image
					std::vector<vk::ImageView> framebufferAttachments = { vulkanSwapchainImageViews[i], vulkanSwapchainDepthImageViews[i] };

					vulkanSwapchainFramebuffers[i] = vulkanDevice.createFramebuffer({ {}, vulkanRenderPass, framebufferAttachments, vulkanSwapchainExtent.width, vulkanSwapchainExtent.height, 1 });
				}
			}

			// Create fences for images currently in flight
			vulkanImagesInFlight.resize(imageCount);
			for (std::size_t i = 0; i < imageCount; ++i) vulkanImagesInFlight[i] = nullptr;

			// Change image layout of depth images to their optimal layout, tbh I don't really know what this does...
			{
				vk::CommandPool currentCommandPool = vulkanCommandPools[currentFrame];
				// Reset current command pool before we use it
				vulkanDevice.resetCommandPool(currentCommandPool);
				vk::CommandBuffer currentCommandBuffer = vulkanCommandBuffers[currentFrame][0];

				vk::CommandBufferBeginInfo beginInfo = {};
				currentCommandBuffer.begin(beginInfo);

				std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers;
				imageMemoryBarriers.resize(imageCount);
				for (std::size_t i = 0; i < imageCount; ++i)
					imageMemoryBarriers[i] = { {}, vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, ~0U, ~0U, vulkanSwapchainDepthImages[i], { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 } };

				currentCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eEarlyFragmentTests, {}, {}, {}, imageMemoryBarriers);

				currentCommandBuffer.end();

				// Submit current command buffer and wait until it finishes
				std::vector<vk::CommandBuffer> submitCommandBuffers = { currentCommandBuffer };
				vulkanGraphicsQueue.submit({ { {}, {}, submitCommandBuffers, {} } });
				vulkanGraphicsQueue.waitIdle();
			}
		}

		// ------------------
		// -- Dynamic data --

		// Create Graphics Pipeline
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::PipelineLayout graphicsPipelineLayout;
		vk::Pipeline graphicsPipeline;
		vk::DescriptorPool descriptorPool;
		{
			// Create shader modules
			vk::ShaderModule vertexShaderModule;
			vk::ShaderModule fragmentShaderModule;
			{
				// Load vertex shader
				std::string vertexFileContent;
				{
					std::ifstream vertexFile = std::ifstream("shaders/vert.spv", std::ios::binary | std::ios::ate);
					if (vertexFile.is_open()) {
						auto size = vertexFile.tellg();
						vertexFileContent.resize(static_cast<std::size_t>(size));
						vertexFile.seekg(0);
						vertexFile.read(vertexFileContent.data(), size);
						vertexFile.close();
					}
				}
				vertexShaderModule = vulkanDevice.createShaderModule({ {}, vertexFileContent.size(), reinterpret_cast<std::uint32_t*>(vertexFileContent.data()) });

				// Load fragment shader
				std::string fragmentFileContent;
				{
					std::ifstream fragmentFile = std::ifstream("shaders/frag.spv", std::ios::binary | std::ios::ate);
					if (fragmentFile.is_open()) {
						auto size = fragmentFile.tellg();
						fragmentFileContent.resize(static_cast<std::size_t>(size));
						fragmentFile.seekg(0);
						fragmentFile.read(fragmentFileContent.data(), size);
						fragmentFile.close();
					}
				}
				fragmentShaderModule = vulkanDevice.createShaderModule({ {}, fragmentFileContent.size(), reinterpret_cast<std::uint32_t*>(fragmentFileContent.data()) });
			}

			// Create descriptor set layout
			{
				std::vector<vk::DescriptorSetLayoutBinding> bindings = {
					{ 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr },
					{ 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr }
				};

				descriptorSetLayout = vulkanDevice.createDescriptorSetLayout({ {}, bindings });
			}

			// Create graphics pipeline layout
			graphicsPipelineLayout = vulkanDevice.createPipelineLayout({ {}, descriptorSetLayout, {} });

			// Create graphics pipeline
			{
				std::vector<vk::PipelineShaderStageCreateInfo> stages;
				stages.push_back({ {}, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main", nullptr });
				stages.push_back({ {}, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main", nullptr });

				std::vector<vk::VertexInputBindingDescription> vertexBindings     = { { 0, 24, vk::VertexInputRate::eVertex } };
				std::vector<vk::VertexInputAttributeDescription> vertexAttributes = { { 0, 0, vk::Format::eR32G32B32A32Sfloat, 0 }, { 1, 0, vk::Format::eR32G32Sfloat, 16 } };

				std::vector<vk::Viewport> viewports = { { 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f } };
				std::vector<vk::Rect2D> scissors    = { { { 0, 0 }, { 1, 1 } } };

				std::vector<vk::PipelineColorBlendAttachmentState> blendAttachments = { { false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA } };

				std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eLineWidth };

				vk::PipelineVertexInputStateCreateInfo vertexInputState     = { {}, vertexBindings, vertexAttributes };
				vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = { {}, vk::PrimitiveTopology::eTriangleList, false };
				vk::PipelineViewportStateCreateInfo viewportState           = { {}, viewports, scissors };
				vk::PipelineRasterizationStateCreateInfo rasterizationState = { {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f };
				vk::PipelineMultisampleStateCreateInfo multisampleState     = { {}, vk::SampleCountFlagBits::e1, false, 1.0f, nullptr, false, false };
				vk::PipelineDepthStencilStateCreateInfo depthStencilState   = { {}, true, true, vk::CompareOp::eLess, false, false, { vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep, vk::CompareOp::eLess, ~0U, ~0U, ~0U }, { vk::StencilOp::eKeep, vk::StencilOp::eReplace, vk::StencilOp::eKeep, vk::CompareOp::eLess, ~0U, ~0U, ~0U }, 0.0f, 1.0f };
				vk::PipelineColorBlendStateCreateInfo colorBlendState       = { {}, false, vk::LogicOp::eCopy, blendAttachments, {} };
				vk::PipelineDynamicStateCreateInfo dynamicState             = { {}, dynamicStates };

				graphicsPipeline = vulkanDevice.createGraphicsPipeline(nullptr, { {}, stages, &vertexInputState, &inputAssemblyState, nullptr, &viewportState, &rasterizationState, &multisampleState, &depthStencilState, &colorBlendState, &dynamicState, graphicsPipelineLayout, vulkanRenderPass, 0, nullptr, 0 }).value; // .value is apparently required here, as it gives an error with multiple cast functions available.
			}

			// Create descriptor pool
			{
				std::vector<vk::DescriptorPoolSize> poolSizes = { { vk::DescriptorType::eUniformBuffer, VULKAN_MAX_FRAMES_IN_FLIGHT }, { vk::DescriptorType::eCombinedImageSampler, VULKAN_MAX_FRAMES_IN_FLIGHT } };

				descriptorPool = vulkanDevice.createDescriptorPool({ {}, VULKAN_MAX_FRAMES_IN_FLIGHT, poolSizes });
			}

			// Destroy shader modules
			vulkanDevice.destroyShaderModule(vertexShaderModule);
			vulkanDevice.destroyShaderModule(fragmentShaderModule);
		}

		// Create Mesh Buffer and image
		vk::Buffer meshBuffer;
		VmaAllocation meshBufferAllocation;
		vk::Image image;
		VmaAllocation imageAllocation;
		vk::ImageView imageView;
		vk::Sampler imageSampler;
		{
			// Create mesh buffer
			std::size_t meshBufferSize           = 240;
			vk::BufferCreateInfo createInfo      = { {}, meshBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive, {} };
			VkBufferCreateInfo createInfo_       = createInfo;
			VmaAllocationCreateInfo allocateInfo = { {}, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, 0, 0, 0, 0.0f };

			VkBuffer buffer;
			meshBuffer = vk::createResultValue(static_cast<vk::Result>(vmaCreateBuffer(vmaAllocator, &createInfo_, &allocateInfo, &buffer, &meshBufferAllocation, nullptr)), buffer, "vmaCreateBuffer");

			// Create image
			vk::ImageCreateInfo imageCreateInfo = { {}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, { 2, 2, 1 }, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined };
			VkImageCreateInfo imageCreateInfo_  = imageCreateInfo;
			VkImage image_;
			image = vk::createResultValue(static_cast<vk::Result>(vmaCreateImage(vmaAllocator, &imageCreateInfo_, &allocateInfo, &image_, &imageAllocation, nullptr)), image_, "vmaCreateImage");

			// Create image view
			imageView = vulkanDevice.createImageView({ {}, image, vk::ImageViewType::e2D, imageCreateInfo.format, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } });

			// Create image sampler
			imageSampler = vulkanDevice.createSampler({ {}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, false, 0.0f, false, vk::CompareOp::eAlways, 0.0f, 1.0f, vk::BorderColor::eIntOpaqueBlack, false });

			// Create staging buffer
			createInfo   = { {}, meshBufferSize + (2 * 2 * 4), vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, {} };
			createInfo_  = createInfo;
			allocateInfo = { {}, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY, 0, 0, 0, 0, 0, 0.0f };

			VkBuffer stagingBuffer;
			VmaAllocation stagingBufferAllocation;
			vk::Result result = static_cast<vk::Result>(vmaCreateBuffer(vmaAllocator, &createInfo_, &allocateInfo, &stagingBuffer, &stagingBufferAllocation, nullptr));
			if (result != vk::Result::eSuccess)
				vk::throwResultException(result, "vmaCreateBuffer");

			// Map staging buffer and copy mesh data into it.
			void* pData;
			vmaMapMemory(vmaAllocator, stagingBufferAllocation, &pData);
			float vertices[]        = { -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, -0.3f, -0.3f, 0.0f, 1.0f, 1.0f, 1.0f, 0.3f, -0.3f, 0.0f, 1.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.0f, 1.0f, 0.0f, 0.0f, -0.3f, 0.3f, 0.0f, 1.0f, 1.0f, 0.0f };
			std::uint32_t indices[] = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };
			std::uint8_t pixels[]   = { 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF };
			std::uintptr_t dataPtr  = reinterpret_cast<std::uintptr_t>(pData);
			std::memcpy(reinterpret_cast<void*>(dataPtr), vertices, sizeof(vertices));
			std::memcpy(reinterpret_cast<void*>(dataPtr + sizeof(vertices)), indices, sizeof(indices));
			std::memcpy(reinterpret_cast<void*>(dataPtr + sizeof(vertices) + sizeof(indices)), pixels, sizeof(pixels));
			vmaUnmapMemory(vmaAllocator, stagingBufferAllocation);

			// Copy data from staging buffer into mesh buffer
			vk::CommandPool currentCommandPool = vulkanCommandPools[currentFrame];
			vulkanDevice.resetCommandPool(currentCommandPool);
			vk::CommandBuffer currentCommandBuffer = vulkanCommandBuffers[currentFrame][0];

			vk::CommandBufferBeginInfo beginInfo = {};
			currentCommandBuffer.begin(beginInfo);
			currentCommandBuffer.copyBuffer(stagingBuffer, meshBuffer, { { 0, 0, meshBufferSize } });

			vk::ImageMemoryBarrier imageMemoryBarrier = { {}, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, ~0U, ~0U, image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
			currentCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imageMemoryBarrier);

			std::vector<vk::BufferImageCopy> bufferImageCopies = { { meshBufferSize, 0, 0, { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, { 0, 0, 0 }, imageCreateInfo.extent } };
			currentCommandBuffer.copyBufferToImage(stagingBuffer, image, vk::ImageLayout::eTransferDstOptimal, bufferImageCopies);

			imageMemoryBarrier = { vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, ~0U, ~0U, image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
			currentCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imageMemoryBarrier);

			currentCommandBuffer.end();
			vulkanGraphicsQueue.submit({ { {}, {}, currentCommandBuffer, {} } });
			vulkanGraphicsQueue.waitIdle();

			// Destroy staging buffer
			vmaDestroyBuffer(vmaAllocator, stagingBuffer, stagingBufferAllocation);
		}

		// Create Uniform Buffer
		vk::Buffer uniformBuffer;
		VmaAllocation uniformBufferAllocation;
		{
			vk::BufferCreateInfo createInfo      = { {}, 128 * VULKAN_MAX_FRAMES_IN_FLIGHT, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive, {} };
			VkBufferCreateInfo createInfo_       = createInfo;
			VmaAllocationCreateInfo allocateInfo = { {}, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, 0, 0, 0, 0.0f };

			VkBuffer buffer;
			uniformBuffer = vk::createResultValue(static_cast<vk::Result>(vmaCreateBuffer(vmaAllocator, &createInfo_, &allocateInfo, &buffer, &uniformBufferAllocation, nullptr)), buffer, "vmaCreateBuffer");
		}

		// Create descriptor sets and write to them
		std::vector<vk::DescriptorSet> descriptorSets;
		{
			std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(VULKAN_MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
			descriptorSets = vulkanDevice.allocateDescriptorSets({ descriptorPool, descriptorSetLayouts });

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
			vk::DescriptorImageInfo imageInfo = { imageSampler, imageView, vk::ImageLayout::eShaderReadOnlyOptimal };
			std::vector<vk::DescriptorBufferInfo> bufferInfos;
			writeDescriptorSets.resize(VULKAN_MAX_FRAMES_IN_FLIGHT * 2);
			bufferInfos.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
			for (size_t i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; ++i) {
				bufferInfos[i]         = vk::DescriptorBufferInfo { uniformBuffer, 128 * i, 128 };
				writeDescriptorSets[i] = { descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfos[i], nullptr };
			}
			for (size_t i = VULKAN_MAX_FRAMES_IN_FLIGHT; i < VULKAN_MAX_FRAMES_IN_FLIGHT * 2; ++i)
				writeDescriptorSets[i] = { descriptorSets[i - VULKAN_MAX_FRAMES_IN_FLIGHT], 1, 0, vk::DescriptorType::eCombinedImageSampler, imageInfo, {}, {} };

			vulkanDevice.updateDescriptorSets(writeDescriptorSets, {});
		}

		// -- Dynamic Data --
		// ------------------

		// Poll for all window events and wait until window should be closed (Pressed X button)
		while (!glfwWindowShouldClose(windowPtr)) {
			glfwPollEvents();

			// Begin frame
			vk::Result result = vulkanDevice.waitForFences({ vulkanInFlightFences[currentFrame] }, true, ~0ULL);
			vulkanDevice.resetFences({ vulkanInFlightFences[currentFrame] });

			std::uint32_t imageIndex;
			result = vulkanDevice.acquireNextImageKHR(vulkanSwapchain, -1, vulkanImageAvailableSemaphores[currentFrame], nullptr, &imageIndex);

			if (result == vk::Result::eErrorOutOfDateKHR) {
				// Recreate swapchain.
				// Begin Frame again.
				continue;
			} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
				vk::throwResultException(result, "vk::Device::acquireNextImageKHR");
			}

			currentImage = imageIndex;

			if (vulkanImagesInFlight[currentImage])
				result = vulkanDevice.waitForFences({ vulkanImagesInFlight[currentImage] }, true, ~0ULL);
			vulkanImagesInFlight[currentImage] = vulkanInFlightFences[currentFrame];

			vulkanDevice.resetCommandPool(vulkanCommandPools[currentFrame]);
			vulkanDevice.resetFences({ vulkanInFlightFences[currentFrame] });

			// Collect commands
			vk::CommandBuffer currentCommandBuffer = vulkanCommandBuffers[currentFrame][0];

			vk::CommandBufferBeginInfo beginInfo = {};
			currentCommandBuffer.begin(beginInfo);

			std::vector<vk::ClearValue> renderPassClearValues = { { std::array<float, 4> { 0.1f, 0.1f, 0.1f, 1.0f } }, { { 1.0f, 0 } } };
			currentCommandBuffer.beginRenderPass({ vulkanRenderPass, vulkanSwapchainFramebuffers[currentImage], { { 0, 0 }, vulkanSwapchainExtent }, renderPassClearValues }, vk::SubpassContents::eInline);

			// ------------------
			// -- Dynamic data --

			currentCommandBuffer.setViewport(0, { { 0.0f, 0.0f, static_cast<float>(vulkanSwapchainExtent.width), static_cast<float>(vulkanSwapchainExtent.height), 0.0f, 1.0f } });
			currentCommandBuffer.setScissor(0, { { { 0, 0 }, vulkanSwapchainExtent } });
			currentCommandBuffer.setLineWidth(1.0f);
			currentCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
			currentCommandBuffer.bindVertexBuffers(0, meshBuffer, 0ULL);
			currentCommandBuffer.bindIndexBuffer(meshBuffer, 192, vk::IndexType::eUint32);
			currentCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0, descriptorSets[currentFrame], {});
			currentCommandBuffer.drawIndexed(12, 1, 0, 0, 0);

			// -- Dynamic Data --
			// ------------------

			currentCommandBuffer.endRenderPass();
			currentCommandBuffer.end();

			// End frame
			std::vector<vk::Semaphore> waitSemaphores            = { vulkanImageAvailableSemaphores[currentFrame] };
			std::vector<vk::Semaphore> signalSemaphores          = { vulkanRenderFinishedSemaphores[currentFrame] };
			std::vector<vk::PipelineStageFlags> waitDstStageMask = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
			std::vector<vk::CommandBuffer>& submitCommandBuffers = vulkanCommandBuffers[currentFrame];
			vulkanGraphicsQueue.submit({ { waitSemaphores, waitDstStageMask, submitCommandBuffers, signalSemaphores } }, vulkanInFlightFences[currentFrame]);

			std::vector<vk::SwapchainKHR> presentSwapchains = { vulkanSwapchain };
			std::vector<std::uint32_t> presentImageIndices  = { static_cast<std::uint32_t>(currentImage) };

			result = vulkanGraphicsQueue.presentKHR({ signalSemaphores, presentSwapchains, presentImageIndices });
			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
				// Recreate swapchain.
				continue;
			} else if (result != vk::Result::eSuccess) {
				vk::throwResultException(result, "vk::Queue::presentKHR");
			}

			vulkanGraphicsQueue.waitIdle();

			currentFrame = (currentFrame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;
		}

		// ------------------
		// -- Dynamic data --

		// Destroy Uniform Buffer
		vmaDestroyBuffer(vmaAllocator, uniformBuffer, uniformBufferAllocation);

		// Destroy Mesh Buffer
		vmaDestroyBuffer(vmaAllocator, meshBuffer, meshBufferAllocation);

		// Destroy Image
		vmaDestroyImage(vmaAllocator, image, imageAllocation);

		// Destroy Image View
		vulkanDevice.destroyImageView(imageView);

		// Destroy Image Sampler
		vulkanDevice.destroySampler(imageSampler);

		// Destroy Descriptor Pool
		vulkanDevice.destroyDescriptorPool(descriptorPool);

		// Destroy Graphics Pipeline
		vulkanDevice.destroyPipeline(graphicsPipeline);

		// Destroy Graphics Pipeline Layout
		vulkanDevice.destroyPipelineLayout(graphicsPipelineLayout);

		// Destroy Descriptor Set Layout
		vulkanDevice.destroyDescriptorSetLayout(descriptorSetLayout);

		// -- Dynamic Data --
		// ------------------

		// Destroy Vulkan Swapchain
		{
			// INFO: Most of this should be able to be moved into a separate function to support recreating the swapchain when the window resizes

			// Destroy framebuffers
			for (auto& framebuffer : vulkanSwapchainFramebuffers)
				vulkanDevice.destroyFramebuffer(framebuffer);

			// Destroy depth image views
			for (auto& imageView : vulkanSwapchainDepthImageViews)
				vulkanDevice.destroyImageView(imageView);

			// Destroy depth images
			for (std::size_t i = 0; i < vulkanSwapchainDepthImages.size(); ++i)
				vmaDestroyImage(vmaAllocator, vulkanSwapchainDepthImages[i], vulkanSwapchainDepthImageAllocations[i]);

			// Destroy Vulkan Image Views
			for (auto& imageView : vulkanSwapchainImageViews)
				vulkanDevice.destroyImageView(imageView);

			// Destroy Vulkan Render Pass
			vulkanDevice.destroyRenderPass(vulkanRenderPass);

			// Destroy Vulkan Swapchain
			vulkanDevice.destroySwapchainKHR(vulkanSwapchain);
		}

		// Destroy all Vulkan Semaphores
		for (auto& semaphore : vulkanImageAvailableSemaphores) vulkanDevice.destroySemaphore(semaphore);
		for (auto& semaphore : vulkanRenderFinishedSemaphores) vulkanDevice.destroySemaphore(semaphore);

		// Destroy all Vulkan Fences
		for (auto& fence : vulkanInFlightFences) vulkanDevice.destroyFence(fence);

		// Destroy all Vulkan Command Pools
		for (auto& commandPool : vulkanCommandPools) vulkanDevice.destroyCommandPool(commandPool);

		// Destroy Vulkan Memory Allocator
		vmaDestroyAllocator(vmaAllocator);

		// Destroy Vulkan Device
		vulkanDevice.destroy();

		// Destroy Vulkan Surface
		vulkanInstance.destroySurfaceKHR(vulkanSurface);

#ifdef _DEBUG
		// Destroy Vulkan Debug Messenger
		vulkanInstance.destroyDebugUtilsMessengerEXT(vulkanDebugMessenger);
#endif

		// Destroy Vulkan Instance
		vulkanInstance.destroy();

		// Destroy window and terminate GLFW
		glfwDestroyWindow(windowPtr);
		glfwTerminate();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

#if _WIN32
	#undef APIENTRY
	#include <Windows.h>

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	return main(__argc, __argv);
}
#endif