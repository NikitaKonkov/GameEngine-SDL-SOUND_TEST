#pragma once

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <vector>

class Renderer {
public:
    // Constructor takes a SDL window for rendering
    Renderer(SDL_Window* window);
    
    // Destructor handles cleanup
    ~Renderer();
    
    // Draw a frame
    void drawFrame();
    
    // Handle window resize events
    void handleWindowResize(int width, int height);

private:
    // Window handle
    SDL_Window* window;
    
    // Vulkan instance and device handles
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t queueFamilyIndex;
    VkQueue graphicsQueue;
    
    // Swapchain
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkSurfaceFormatKHR surfaceFormat;
    VkSurfaceCapabilitiesKHR capabilities;
    
    // Render pass and framebuffers
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    // Pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    
    // Command submission
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    
    // Synchronization objects
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    
    // Initialization methods
    void initVulkan();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createShaderModules();
    void createPipeline();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
    
    // Cleanup resources
    void cleanup();
};