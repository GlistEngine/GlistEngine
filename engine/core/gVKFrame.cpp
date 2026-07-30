/*
 * gVKFrame.cpp
 *
 * The Vulkan frame loop: acquire an image, record the render pass, submit and
 * present.
 */

#include "gVKFrame.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKSwapchain.h"
#include "gUtils.h"
#include <GLFW/glfw3.h>

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

	VkRenderPassBeginInfo renderpassinfo{};
	renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassinfo.renderPass = ctx.renderpass;
	renderpassinfo.framebuffer = ctx.framebuffers[ctx.currentimageindex];
	renderpassinfo.renderArea.offset = {0, 0};
	renderpassinfo.renderArea.extent = ctx.swapchainextent;
	// The attachment uses a CLEAR load operation, so this is the value that ends up
	// covering the screen.
	renderpassinfo.clearValueCount = 1;
	renderpassinfo.pClearValues = &ctx.clearvalue;
	vkCmdBeginRenderPass(commandbuffer, &renderpassinfo, VK_SUBPASS_CONTENTS_INLINE);

	ctx.frameactive = true;
	return true;
}

bool gvkEndFrame(gVKContext& ctx, GLFWwindow* window) {
	if(!ctx.frameactive) return false;
	ctx.frameactive = false;

	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];
	vkCmdEndRenderPass(commandbuffer);
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
