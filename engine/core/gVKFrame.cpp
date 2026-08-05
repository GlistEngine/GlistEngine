/*
 * gVKFrame.cpp
 *
 * The Vulkan frame loop: acquire an image, record the render pass, submit and
 * present.
 */

#include "gVKFrame.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKSwapchain.h"
#include "gVKBuffer.h"
#include "gUtils.h"
#include <GLFW/glfw3.h>
#include <cstring>

bool gvkBeginFrame(gVKContext& ctx, GLFWwindow* window) {
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchain == VK_NULL_HANDLE || window == nullptr) {
		return false;
	}
	if(ctx.frameactive) {
		gLoge("gVKFrame") << "gvkBeginFrame was called while a frame was already active.";
		return false;
	}

	// A resized window makes the current swapchain the wrong size. Rebuilding it
	// here keeps the check in one place instead of relying on a resize callback.
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	if(width <= 0 || height <= 0) {
		// Minimised: there is nothing to render into.
		return false;
	}
	if(static_cast<uint32_t>(width) != ctx.swapchainextent.width ||
			static_cast<uint32_t>(height) != ctx.swapchainextent.height) {
		gvkRecreateSwapchain(ctx, window);
		return false;
	}

	// Wait until the GPU is done with the previous use of this frame slot, so its
	// command buffer and semaphores are free to be reused.
	vkWaitForFences(ctx.device, 1, &ctx.inflightfences[ctx.currentframe], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(ctx.device, ctx.swapchain, UINT64_MAX,
			ctx.imageavailablesemaphores[ctx.currentframe], VK_NULL_HANDLE, &ctx.currentimageindex);
	if(result == VK_ERROR_OUT_OF_DATE_KHR) {
		gvkRecreateSwapchain(ctx, window);
		return false;
	}
	// Suboptimal still presents correctly, so the frame is drawn and the swapchain
	// is rebuilt after presenting instead.
	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		gLoge("gVKFrame") << "vkAcquireNextImageKHR failed! VkResult: " << result;
		return false;
	}

	// Only reset the fence once it is certain that work will be submitted, other-
	// wise an early return would leave it unsignalled and the next wait would hang.
	vkResetFences(ctx.device, 1, &ctx.inflightfences[ctx.currentframe]);

	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];
	vkResetCommandBuffer(commandbuffer, 0);

	VkCommandBufferBeginInfo begininfo{};
	begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	result = vkBeginCommandBuffer(commandbuffer, &begininfo);
	if(result != VK_SUCCESS) {
		gLoge("gVKFrame") << "vkBeginCommandBuffer failed! VkResult: " << result;
		return false;
	}

	// Dynamic Rendering is not begun here: the clear colour is only final after the
	// canvas has drawn, so the scope opens lazily on the first draw or, for a
	// frame that draws nothing, in gvkEndFrame. Rewind this frame's vertex ring so
	// the draw path can refill it from the start.
	ctx.resetDynamicVertices();
	ctx.renderingactive = false;
	ctx.frameactive = true;
	return true;
}

bool gvkEnsureRendering(gVKContext& ctx) {
	if(!ctx.frameactive) return false;
	if(ctx.renderingactive) return true;

	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];

	VkImageMemoryBarrier barriers[2]{};
	barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barriers[0].srcAccessMask = 0;
	barriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barriers[0].oldLayout = ctx.swapchainimagelayouts[ctx.currentimageindex];
	barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].image = ctx.swapchainimages[ctx.currentimageindex];
	barriers[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barriers[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barriers[1].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[1].image = ctx.depthimages[ctx.currentimageindex];
	barriers[1].subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
	vkCmdPipelineBarrier(commandbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, 0, nullptr, 0, nullptr, 2, barriers);

	VkRenderingAttachmentInfo colorattachment{};
	colorattachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorattachment.imageView = ctx.swapchainimageviews[ctx.currentimageindex];
	colorattachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorattachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorattachment.clearValue = ctx.clearvalue;
	VkRenderingAttachmentInfo depthattachment{};
	depthattachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthattachment.imageView = ctx.depthimageviews[ctx.currentimageindex];
	depthattachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthattachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthattachment.clearValue.depthStencil = {1.0f, 0};
	VkRenderingInfo renderinginfo{};
	renderinginfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderinginfo.renderArea = {{0, 0}, ctx.swapchainextent};
	renderinginfo.layerCount = 1;
	renderinginfo.colorAttachmentCount = 1;
	renderinginfo.pColorAttachments = &colorattachment;
	renderinginfo.pDepthAttachment = &depthattachment;
	vkCmdBeginRendering(commandbuffer, &renderinginfo);

	// A negative-height viewport flips Y, so the orthographic projection the engine
	// builds for OpenGL's top-left origin lands the same way under Vulkan (needs
	// maintenance1 / Vulkan 1.1+, which the selected device already targets).
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = static_cast<float>(ctx.swapchainextent.height);
	viewport.width = static_cast<float>(ctx.swapchainextent.width);
	viewport.height = -static_cast<float>(ctx.swapchainextent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandbuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = ctx.swapchainextent;
	vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

	ctx.renderingactive = true;
	return true;
}

bool gvkEndFrame(gVKContext& ctx, GLFWwindow* window) {
	if(!ctx.frameactive) return false;
	// Open the pass if the frame drew nothing, so the clear still reaches the screen.
	gvkEnsureRendering(ctx);
	ctx.frameactive = false;

	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];
	vkCmdEndRendering(commandbuffer);
	VkBuffer screenshotbuffer = VK_NULL_HANDLE;
	VkDeviceMemory screenshotmemory = VK_NULL_HANDLE;
	const VkDeviceSize screenshotsize = static_cast<VkDeviceSize>(ctx.swapchainextent.width)
			* ctx.swapchainextent.height * 4;
	const bool capturescreenshot = ctx.screenshotrequested
			&& (ctx.surfacecapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0
			&& gvkCreateBuffer(ctx, screenshotsize, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					screenshotbuffer, screenshotmemory);
	if(capturescreenshot) {
		VkImageMemoryBarrier transferbarrier{};
		transferbarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		transferbarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		transferbarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		transferbarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		transferbarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		transferbarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		transferbarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		transferbarrier.image = ctx.swapchainimages[ctx.currentimageindex];
		transferbarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		vkCmdPipelineBarrier(commandbuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &transferbarrier);

		VkBufferImageCopy copyregion{};
		copyregion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		copyregion.imageExtent = {ctx.swapchainextent.width, ctx.swapchainextent.height, 1};
		vkCmdCopyImageToBuffer(commandbuffer, ctx.swapchainimages[ctx.currentimageindex],
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, screenshotbuffer, 1, &copyregion);
	} else if(ctx.screenshotrequested) {
		ctx.screenshotrequested = false;
		gLoge("gVKFrame") << "Could not allocate the screenshot readback buffer.";
	}
	VkImageMemoryBarrier presentbarrier{};
	presentbarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	presentbarrier.srcAccessMask = capturescreenshot ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	presentbarrier.oldLayout = capturescreenshot ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	presentbarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	presentbarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	presentbarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	presentbarrier.image = ctx.swapchainimages[ctx.currentimageindex];
	presentbarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	vkCmdPipelineBarrier(commandbuffer, capturescreenshot ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentbarrier);
	ctx.swapchainimagelayouts[ctx.currentimageindex] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	ctx.renderingactive = false;
	VkResult result = vkEndCommandBuffer(commandbuffer);
	if(result != VK_SUCCESS) {
		gLoge("gVKFrame") << "vkEndCommandBuffer failed! VkResult: " << result;
		return false;
	}

	// The image may still be owned by the presentation engine, so the submission
	// waits for the acquire semaphore before it writes any colour.
	const VkSemaphore waitsemaphores[] = {ctx.imageavailablesemaphores[ctx.currentframe]};
	const VkPipelineStageFlags waitstages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	// Signalled per image rather than per frame, because presentation picks the
	// semaphore by the acquired image index.
	const VkSemaphore signalsemaphores[] = {ctx.renderfinishedsemaphores[ctx.currentimageindex]};

	VkSubmitInfo submitinfo{};
	submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitinfo.waitSemaphoreCount = 1;
	submitinfo.pWaitSemaphores = waitsemaphores;
	submitinfo.pWaitDstStageMask = waitstages;
	submitinfo.commandBufferCount = 1;
	submitinfo.pCommandBuffers = &commandbuffer;
	submitinfo.signalSemaphoreCount = 1;
	submitinfo.pSignalSemaphores = signalsemaphores;

	result = vkQueueSubmit(ctx.graphicsqueue, 1, &submitinfo, ctx.inflightfences[ctx.currentframe]);
	if(result != VK_SUCCESS) {
		gLoge("gVKFrame") << "vkQueueSubmit failed! VkResult: " << result;
		return false;
	}
	if(capturescreenshot) {
		vkWaitForFences(ctx.device, 1, &ctx.inflightfences[ctx.currentframe], VK_TRUE, UINT64_MAX);
		void* mapped = nullptr;
		if(vkMapMemory(ctx.device, screenshotmemory, 0, screenshotsize, 0, &mapped) == VK_SUCCESS) {
			ctx.screenshotpixels.resize(static_cast<size_t>(screenshotsize));
			std::memcpy(ctx.screenshotpixels.data(), mapped, static_cast<size_t>(screenshotsize));
			vkUnmapMemory(ctx.device, screenshotmemory);
			ctx.screenshotwidth = ctx.swapchainextent.width;
			ctx.screenshotheight = ctx.swapchainextent.height;
			ctx.screenshotformat = ctx.swapchainformat;
			ctx.screenshotready = true;
		} else {
			gLoge("gVKFrame") << "Could not map the screenshot readback buffer.";
		}
		ctx.screenshotrequested = false;
		vkDestroyBuffer(ctx.device, screenshotbuffer, nullptr);
		vkFreeMemory(ctx.device, screenshotmemory, nullptr);
	}

	VkPresentInfoKHR presentinfo{};
	presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pWaitSemaphores = signalsemaphores;
	presentinfo.swapchainCount = 1;
	presentinfo.pSwapchains = &ctx.swapchain;
	presentinfo.pImageIndices = &ctx.currentimageindex;

	result = vkQueuePresentKHR(ctx.presentqueue, &presentinfo);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		gvkRecreateSwapchain(ctx, window);
	} else if(result != VK_SUCCESS) {
		gLoge("gVKFrame") << "vkQueuePresentKHR failed! VkResult: " << result;
	}

	static bool loggedfirstframe = false;
	if(!loggedfirstframe) {
		loggedfirstframe = true;
		gLogi("gVKFrame") << "First frame presented";
	}

	ctx.currentframe = (ctx.currentframe + 1) % GVK_MAX_FRAMES_IN_FLIGHT;
	return true;
}

#endif /* GVK_DESKTOP_GLFW */
