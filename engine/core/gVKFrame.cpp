/*
 * gVKFrame.cpp
 *
 * The Vulkan frame loop: acquire an image, record the render pass, submit and
 * present.
 */

#include "gVKFrame.h"
#include "gVKDraw.h"
#include "gVKMeshBuffer.h"

#ifdef GVK_VULKAN

#include "gVKSwapchain.h"
#include "gVKBuffer.h"
#include "gUtils.h"
#include "gBaseWindow.h"
#include <cstring>

bool gvkBeginFrame(gVKContext& ctx, gBaseWindow* window) {
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
	width = window->getWidth();
	height = window->getHeight();
	if(width <= 0 || height <= 0) {
		// Minimised: there is nothing to render into.
		return false;
	}
	if(ctx.swapchainrecreaterequested) {
		ctx.swapchainrecreaterequested = false;
		gvkRecreateSwapchain(ctx, window);
		return false;
	}
	// Only compare the window size when the window system lets the application
	// choose the swapchain extent. Android commonly reports a fixed currentExtent
	// in physical pixels while gBaseWindow exposes logical game coordinates; those
	// values are both correct and must not cause a rebuild loop. Fixed-extent
	// surfaces report real changes through acquire/present as OUT_OF_DATE.
	const bool applicationChoosesExtent =
			ctx.surfacecapabilities.currentExtent.width == UINT32_MAX;
	if(applicationChoosesExtent &&
			(static_cast<uint32_t>(width) != ctx.swapchainextent.width ||
			 static_cast<uint32_t>(height) != ctx.swapchainextent.height)) {
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

	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];
	vkResetCommandBuffer(commandbuffer, 0);
	ctx.resetRecordedDrawState();

	VkCommandBufferBeginInfo begininfo{};
	begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	result = vkBeginCommandBuffer(commandbuffer, &begininfo);
	if(result != VK_SUCCESS) {
		gLoge("gVKFrame") << "vkBeginCommandBuffer failed! VkResult: " << result;
		return false;
	}

	// The render pass is not begun here on purpose. Geometry has to be recorded
	// inside it, yet the clear colour is only final after the canvas has drawn, so
	// the pass is opened lazily on the first draw (gvkEnsureRenderPass) or, for a
	// frame that draws nothing, in gvkEndFrame. Rewind this frame's vertex ring so
	// the draw path can refill it from the start.
	ctx.resetDynamicVertices();
	// Rewind the mesh arena too, and grow it first if the last frames wanted more
	// than it holds. Growing here means draining the device, which is why it is
	// done at a frame boundary and only when the high-water mark says it is
	// needed - in practice a handful of times while a level loads, then never.
	if(ctx.mesharenahighwater > ctx.mesharenacapacity) {
		vkDeviceWaitIdle(ctx.device);
		gvkEnsureMeshArena(ctx, ctx.mesharenahighwater * 2);
	}
	ctx.resetMeshArena();
	// The generation has just moved on, so this is the point where the oldest
	// retired buffers become safe to free.
	gvkCollectRetiredMeshBuffers(ctx);
	// Staging of uploads the GPU has already finished. Checked rather than waited
	// on, so a batch still running simply gets collected a frame later.
	gvkCollectUploads(ctx);
	// The vertex ring has just been rewound, so any batch left over from a frame
	// that ended badly now points at bytes about to be overwritten.
	gvkReset2DBatch();
	ctx.renderpassactive = false;
	ctx.frameactive = true;
	return true;
}

bool gvkEnsureRenderPass(gVKContext& ctx) {
	if(!ctx.frameactive) return false;
	// The shadow pass is open, which means this is the depth pass and whatever is
	// asking to draw belongs to the screen pass - 2D overlays, mostly, since the
	// canvas draws itself whole in both passes. Refusing here is what keeps them
	// out of the shadow map; the mesh path has its own branch and never gets this
	// far.
	if(ctx.shadowpassactive) return false;
	if(ctx.renderpassactive) return true;

	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];

	VkRenderPassBeginInfo renderpassinfo{};
	renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassinfo.renderPass = ctx.renderpass;
	const size_t framebufferindex = static_cast<size_t>(ctx.currentframe) * ctx.swapchainimageviews.size()
			+ ctx.currentimageindex;
	if(framebufferindex >= ctx.framebuffers.size()) return false;
	renderpassinfo.framebuffer = ctx.framebuffers[framebufferindex];
	renderpassinfo.renderArea.offset = {0, 0};
	renderpassinfo.renderArea.extent = ctx.swapchainextent;
	// The colour and depth attachments use a CLEAR load operation, and the array is
	// indexed by attachment number, so entry 0 is the colour the screen ends up
	// showing wherever nothing is drawn and entry 1 is the depth the buffer starts
	// at. 1.0 is the far plane, so any fragment passes the default
	// VK_COMPARE_OP_LESS. With MSAA on those two are the multisampled pair and the
	// swapchain image follows as attachment 2, but it loads DONT_CARE - every texel
	// of it is written by the resolve - so it needs no clear value and this array
	// stays two entries either way.
	VkClearValue clearvalues[2];
	clearvalues[0] = ctx.clearvalue;
	clearvalues[1].depthStencil = {1.0f, 0};
	renderpassinfo.clearValueCount = 2;
	renderpassinfo.pClearValues = clearvalues;
	vkCmdBeginRenderPass(commandbuffer, &renderpassinfo, VK_SUBPASS_CONTENTS_INLINE);
	ctx.resetRecordedDrawState();
	// From here until this pass closes, every draw has to use pipelines built for
	// this pass's sample count. Set after the begin rather than at the top of the
	// function, because the early return above is also taken while an *offscreen*
	// pass is open, and that one is single-sampled.
	ctx.useScreenPipelines();

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

	ctx.renderpassactive = true;
	return true;
}

bool gvkEndFrame(gVKContext& ctx, gBaseWindow* window) {
	if(!ctx.frameactive) return false;
	// Open the pass if the frame drew nothing, so the clear still reaches the screen.
	gvkEnsureRenderPass(ctx);
	// Whatever 2D is still batched has to be recorded while the pass is open. This
	// is the last chance: the batch holds a range of the frame's vertex buffer, and
	// that buffer is rewound at the start of the next frame.
	gvkFlush2DBatch(ctx);
	ctx.frameactive = false;

	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];
	vkCmdEndRenderPass(commandbuffer);
	ctx.renderpassactive = false;

	// Screenshot readback, ported from Mehmet's branch and adapted: his frame loop
	// uses Dynamic Rendering and has to raise the present barrier itself, while the
	// render pass here already leaves the image in PRESENT_SRC_KHR through its
	// finalLayout. So the copy takes the image out of that layout and puts it back,
	// rather than sitting between the attachment write and the present transition.
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
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = ctx.swapchainimages[ctx.currentimageindex];
		barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vkCmdPipelineBarrier(commandbuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBufferImageCopy copyregion{};
		copyregion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		copyregion.imageExtent = {ctx.swapchainextent.width, ctx.swapchainextent.height, 1};
		vkCmdCopyImageToBuffer(commandbuffer, ctx.swapchainimages[ctx.currentimageindex],
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, screenshotbuffer, 1, &copyregion);

		// Back to what the render pass left, so presenting stays valid.
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkCmdPipelineBarrier(commandbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	} else if(ctx.screenshotrequested) {
		ctx.screenshotrequested = false;
		gLoge("gVKFrame") << "Could not allocate the screenshot readback buffer.";
	}
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

	// Anything uploaded while this frame was recorded has to reach the queue before
	// the frame does. Submission order is what orders them: the barriers ending each
	// transfer apply to every command submitted after it on this queue, so the frame
	// reads finished textures and vertices without the CPU waiting for either.
	gvkFlushUploads(ctx);


	// Reset only immediately before submission. Any earlier recording error must
	// leave the already-signalled fence intact so the next frame cannot deadlock.
	result = vkResetFences(ctx.device, 1, &ctx.inflightfences[ctx.currentframe]);
	if(result != VK_SUCCESS) {
		gLoge("gVKFrame") << "vkResetFences failed! VkResult: " << result;
		return false;
	}
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
	if(result == VK_ERROR_OUT_OF_DATE_KHR) {
		gvkRecreateSwapchain(ctx, window);
	// SUBOPTIMAL is still a successful presentation. Some Android emulator
	// surfaces report it persistently even when their extent is stable; rebuilding
	// here would therefore destroy and recreate the complete frame path every frame.
	} else if(result == VK_SUBOPTIMAL_KHR) {
		// Keep rendering until acquire/present reports OUT_OF_DATE or an explicit
		// resize/vsync request asks for a rebuild.
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

#endif /* GVK_VULKAN */
