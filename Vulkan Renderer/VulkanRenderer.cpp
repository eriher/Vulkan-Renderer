#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <memory>
#include "Model.h"
#include "Pipeline.h"
#include "SkyboxTexture.h"
#include "HdrTexture.h"
#include "ShadowMap.h"
#include "ShadowCubeMap.h"
#include "RenderToTexture.h"
#include "PBRMaps.h"
#include "BrdfCompute.h"
#include "Camera.h"
#include "RenderingPipeline.h"
#include "PbrPipeline.h"
#include "PbrAltPipeline.h"

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

const std::array<std::string, 1> MODEL_PATH = { "models/skull/craneo.obj" };
//const std::array<std::string, 2> MODEL_PATH = {
//  "models/redBricks/sphere.obj",
//  "models/bricks/quad.obj"
//};
//const std::array<std::string, 1> MATERIAL_MODEL_PATH = { "models/materialtest.obj" };
//const std::array<std::string, 1> MATERIAL_MODEL_PATH = { "models/NewShip.obj" };

//const std::array<std::string, 2> MATERIAL_MODEL_PATH = {  /*"models/rustedSphere/sphere.obj", */"models/scifi/cube.obj", "models/bricks/cube.obj" };
const std::array<std::string, 1> MATERIAL_MODEL_PATH = {  /*"models/rustedSphere/sphere.obj", "models/scifi/cube.obj" */ /*"models/skull/craneo.obj",*/ "models/scifi/cube.obj"/*, "models/rustedSphere2/sphere.obj","models/bricks/quad.obj"*/ };
const std::array<std::string, 0> MATERIAL_ALT_MODEL_PATH = { /*"models/scifi/cube.obj", "models/rustedSphere2/sphere.obj","models/bricks/quad.obj"*/ };
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
  glm::vec3 camPos;
};

struct LightSource {
  glm::vec4 position;
  glm::vec4 color;
  float intensity;
};

class VulkanRenderer {
public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  GLFWwindow* window;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;

  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_2_BIT;

  Camera camera;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkColorSpaceKHR swapChainColorSpace;

  VkRenderPass renderPass;

  std::vector<Pipeline*> pipelines;

  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;

  bool framebufferResized = false;

  QueueFamilyIndices qfi;
  Device device;

  float lastFrame = 0.0f; // Time of last frame
  float deltaFrame = 0.0f;

  uint32_t keyFlags = 0;

  std::vector<VkCommandBuffer> imGuiCommandBuffers;
  VkRenderPass imGuiRenderPass;
  std::vector<VkFramebuffer> imGuiFramebuffers;

  UniformBufferObject pushConstant;
  std::vector<VkCommandBuffer> pushConstantCommandBuffers;

  bool cameraView = true;

  int currentModel = 0;
  int selectedModel = -1;
  int selectedMesh = 0;


  LightSource light;
  VkBuffer lightBuffer;
  VkDeviceMemory lightMemory;
  VkDescriptorSetLayout lightDescriptorLayout;
  VkDescriptorSet lightDescriptor;

  SkyboxTexture skybox;
  //struct {
  //  Model model;
  //  SkyboxTexture texture;
  //  Pipeline pipeline;
  //} skybox;

  VkDescriptorSetLayout modelDescriptorSetLayout;
  VkDescriptorSetLayout materialDescriptorSetLayout;

  RenderingPipeline renderingPipeline;
  PbrPipeline pbrPipeline;
  PbrAltPipeline pbrAltPipeline;

  PBRMaps pbrMaps;

  //ShadowMap shadowMap;

  ShadowCubeMap shadowCubeMap;

  //RenderToTexture renderedTexture;

  BrdfCompute brdfTexture;

  std::vector<std::shared_ptr<Model>> models;

  bool lightChanged = false;

  void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
  }

  static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }

  void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();

    createDevice();
    setupEmptyImage();
    createSwapChain();
    createImageViews();

    createRenderPass();

    setupLights();

    createDescriptorSetLayouts();
    loadModels();

    createBrdf();
    //createShadowMap();
    createShadowCubeMap();

    createSkybox();
    createPBRMaps();
    createGraphicsPipelines();

    setupImGUI();

    createColorResources();
    createDepthResources();
    createFramebuffers();

    setupPushConstant();

    createCommandBuffers();
    createSyncObjects();

  }

  void createDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto& d : devices) {
      if (isDeviceSuitable(d)) {

        device.initDevice(d, qfi, enableValidationLayers);

        break;
      }
    }
  }

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    if (action == GLFW_REPEAT)
      return;

    auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_W) {
      if (action == GLFW_PRESS)
        app->camera.camFlags |= 1;
      else if (action == GLFW_RELEASE)
        app->camera.camFlags &= ~1;
    }
    if (key == GLFW_KEY_S) {
      if (action == GLFW_PRESS)
        app->camera.camFlags |= 2;
      else if (action == GLFW_RELEASE)
        app->camera.camFlags &= ~2;
    }
    if (key == GLFW_KEY_A) {
      if (action == GLFW_PRESS)
        app->camera.camFlags |= 4;
      else if (action == GLFW_RELEASE)
        app->camera.camFlags &= ~4;
    }
    if (key == GLFW_KEY_D) {
      if (action == GLFW_PRESS)
        app->camera.camFlags |= 8;
      if (action == GLFW_RELEASE)
        app->camera.camFlags &= ~8;
    }
    
    if (app->selectedModel < 0)
      return;

    if (key == GLFW_KEY_UP) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 1;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~1;
    }
    if (key == GLFW_KEY_DOWN) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 2;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~2;
    }
    if (key == GLFW_KEY_LEFT) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 4;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~4;
    }
    if (key == GLFW_KEY_RIGHT) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 8;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~8;
    }
  }
  
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse)
      return;
    auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
    if (yoffset > 0)
    {
      if (app->camera.cameraRadius > 2.0)
        app->camera.cameraRadius--;
      else {
        app->camera.cameraRadius = 0.0f;
      }
      
    }
    else
    {
      if(app->camera.cameraRadius < 2.0f){
        app->camera.cameraRadius = 2.0f;
      }
      else
        app->camera.cameraRadius++;
    }
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse)
      return;

    auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      if (action == GLFW_PRESS) {
        glfwGetCursorPos(window, &app->camera.mouseStartX, &app->camera.mouseStartY);
        app->camera.camFlags |= 32;
      }
      else if (action == GLFW_RELEASE) {
        app->camera.camFlags &= ~32;
      }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (action == GLFW_PRESS) {
        glfwGetCursorPos(window, &app->camera.mouseStartX, &app->camera.mouseStartY);
        app->camera.camFlags |=  16;
      }
      else if (action == GLFW_RELEASE) {
        app->camera.camFlags &= ~16;
      }

    }

  }

  void setupImGUI() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    createImGuiRenderPass();
  }

  void createImGuiRenderPass() {
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = device.physicalDevice;
    init_info.Device = device.device;
    init_info.QueueFamily = device.queueFamilies.graphicsFamily.value();
    init_info.Queue = device.graphicsQueue;
    //init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = device.descriptorPool;
    //init_info.Allocator = g_Allocator;
    init_info.MinImageCount = 2;
    init_info.ImageCount = static_cast<uint32_t>(swapChainImages.size());
    //init_info.CheckVkResultFn = check_vk_result;
    init_info.PipelineCache = device.pipelineCache;
    VkAttachmentDescription attachment = {};
    attachment.format = swapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //color_attachment.layout = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    if (vkCreateRenderPass(device.device, &info, nullptr, &imGuiRenderPass) != VK_SUCCESS) {
      throw std::runtime_error("Could not create Dear ImGui's render pass");
    }

    ImGui_ImplVulkan_Init(&init_info, imGuiRenderPass);

    VkCommandBuffer command_buffer = device.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    device.endSingleTimeCommands(command_buffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }

  void mainLoop() {
    lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawFrame();
      //renderOffscreen();
      float currentFrame = glfwGetTime();
      deltaFrame = currentFrame - lastFrame;
      lastFrame = currentFrame;
    }

    vkDeviceWaitIdle(device.device);
  }

  void cleanupSwapChain() {
    vkDestroyImageView(device.device, depthImageView, nullptr);
    vkDestroyImage(device.device, depthImage, nullptr);
    vkFreeMemory(device.device, depthImageMemory, nullptr);

    vkDestroyImageView(device.device, colorImageView, nullptr);
    vkDestroyImage(device.device, colorImage, nullptr);
    vkFreeMemory(device.device, colorImageMemory, nullptr);

    for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(device.device, framebuffer, nullptr);
    }

    for (auto framebuffer : imGuiFramebuffers) {
      vkDestroyFramebuffer(device.device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(device.device, device.graphicsCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkFreeCommandBuffers(device.device, device.resetCommandPool, static_cast<uint32_t>(imGuiCommandBuffers.size()), imGuiCommandBuffers.data());

    vkFreeCommandBuffers(device.device, device.resetCommandPool, static_cast<uint32_t>(pushConstantCommandBuffers.size()), pushConstantCommandBuffers.data());


    vkDestroyRenderPass(device.device, renderPass, nullptr);
    

    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(device.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device.device, swapChain, nullptr);

    //for (size_t i = 0; i < swapChainImages.size(); i++) {
    //  vkDestroyBuffer(device.device, uniformBuffers[i], nullptr);
    //  vkFreeMemory(device.device, uniformBuffersMemory[i], nullptr);
    //}

    //vkDestroyDescriptorPool(device.device, descriptorPool, nullptr);
  }

  void cleanup() {
    cleanupSwapChain();

    //for (auto& pipeline : pipelines) {
    //  pipeline->cleanup();
    //}

    skybox.cleanup();
    if(renderingPipeline.valid())
      renderingPipeline.cleanup();
    if (pbrPipeline.valid())
      pbrPipeline.cleanup();
    if (pbrAltPipeline.valid())
      pbrAltPipeline.cleanup();

    //if (pbr.models.size() > 0) {
    //  for (auto& m : pbr.models)
    //    m->cleanup();
    //  pbr.pipeline.cleanup();
    //}

    //if(pbr_alt.models.size() > 0){
    //  for (auto& m : pbr_alt.models)
    //    m->cleanup();
    //  pbr_alt.pipeline.cleanup();
    //}

    //if (trad.models.size() > 0) {
    //  for (auto& m : trad.models)
    //    m->cleanup();
    //  trad.pipeline.cleanup();
    //}

    vkDestroyDescriptorSetLayout(device.device, materialDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.device, modelDescriptorSetLayout, nullptr);

    brdfTexture.cleanup();

    pbrMaps.cleanup();

    shadowCubeMap.cleanup();

    //shadowMap.cleanup();

    //for (auto i = 0; i < lightBuffer.size(); i++) {
    //  vkDestroyBuffer(device.device, lightBuffer[i], nullptr);
    //  vkFreeMemory(device.device, lightMemory[i], nullptr);
    //}

    //Cleanup lights
    vkDestroyBuffer(device.device, lightBuffer, nullptr);
    vkFreeMemory(device.device, lightMemory, nullptr);
    vkFreeDescriptorSets(device.device, device.descriptorPool, 1, &lightDescriptor);
    vkDestroyDescriptorSetLayout(device.device, lightDescriptorLayout, nullptr);
    vkDestroyRenderPass(device.device, imGuiRenderPass, nullptr);


    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device.device, renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device.device, imageAvailableSemaphores[i], nullptr);
      vkDestroyFence(device.device, inFlightFences[i], nullptr);
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //vkDestroyCommandPool(device.device, commandPool, nullptr);

    //vkDestroyDevice(device.device, nullptr);
    //model.~Model();
    //device.~Device();

    device.cleanup();
    
    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }

  void recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(window, &width, &height);
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();

    createColorResources();
    createDepthResources();
    createFramebuffers();

    createCommandBuffers();
  }

  void createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
  }

  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
  }

  void setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }

  void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface!");
    }
  }

  void createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device.physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { qfi.graphicsFamily.value(), qfi.presentFamily.value() };

    if (qfi.graphicsFamily != qfi.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device.device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
      throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device.device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    swapChainColorSpace = surfaceFormat.colorSpace;
    
  }


  /**
  * Creates renderpass used for the main rendering
  */
  void createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dstAccessMask = 0;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
  }


  /**
  * Generates skybox from hdr skybox
  */
  void createSkybox() {
    Model m{};
    m.device = &device;
    m.swapChainSize = 1;
    m.loadModel("skybox/cube.obj");
    m.descriptorSetLayout = modelDescriptorSetLayout;
    m.createDescriptorBuffers();
    m.createDescriptorSets();
    skybox.model = m;

    skybox.device = &device;
    skybox.loadHdr("skybox/Newport_Loft_Ref.hdr", 1024);

    Pipeline p{};
    p.device = &device;
    p.descriptorSetLayouts.push_back(modelDescriptorSetLayout);
    p.descriptorSetLayouts.push_back(skybox.skyboxDescriptorSetLayout);
    p.swapChainExtent = swapChainExtent;
    p.renderPass = renderPass;
    p.pushConstantSize = sizeof(UniformBufferObject);
    p.msaaSamples = msaaSamples;
    p.createGraphicsPipeline("compiledshaders/skybox.vert.spv", "compiledshaders/skybox.frag.spv");
    skybox.pipeline = p;

  }

  /**
  * Generates irradiance and reflectance maps
  */
  void createPBRMaps() {
    pbrMaps.device = &device;
    pbrMaps.generateMaps(&skybox);
  }

  /**
  * Setup lights, currently only point light
  */
  void setupLights() {
    light.color = glm::vec4(1.0f);
    light.position = glm::vec4(0.0f, 50.0f, 10.0f, 1.0f);
    light.intensity = 2000.0f;

    VkDeviceSize lightBufferSize = sizeof(LightSource);

    device.createBuffer(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, lightBuffer, lightMemory);
    void* data;
    vkMapMemory(device.device, lightMemory, 0, lightBufferSize, 0, &data);
    memcpy(data, &light, lightBufferSize);
    vkUnmapMemory(device.device, lightMemory);

    VkDescriptorSetLayoutBinding lightLayoutBinding{};
    lightLayoutBinding.binding = 0;
    lightLayoutBinding.descriptorCount = 1;
    lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightLayoutBinding.pImmutableSamplers = nullptr;
    lightLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(1);
    layoutInfo.pBindings = &lightLayoutBinding;

    vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &lightDescriptorLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = device.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
    allocInfo.pSetLayouts = &lightDescriptorLayout;

    if (vkAllocateDescriptorSets(device.device, &allocInfo, &lightDescriptor) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets!");
    }
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = lightBuffer;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(LightSource);

      std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = lightDescriptor;
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = static_cast<uint32_t>(1);
      descriptorWrites[0].pBufferInfo = &bufferInfo;


      vkUpdateDescriptorSets(device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
  
  /*
  * Empty Image used when material textures are not present
  */
  void setupEmptyImage() {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = 1;
    imageInfo.extent.height = 1;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    //imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device.device, &imageInfo, nullptr, &device.emptyImage.image) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.device, device.emptyImage.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device.device, &allocInfo, nullptr, &device.emptyImage.memory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device.device, device.emptyImage.image, device.emptyImage.memory, 0);

    auto commands = device.beginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = device.emptyImage.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
      commands,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );

    device.endSingleTimeCommands(commands);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = device.emptyImage.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = 1;
    //viewInfo.subresourceRange.baseMipLevel = 0;
    //viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;
    //viewInfo.subresourceRange.levelCount = mipLevels;


    if (vkCreateImageView(device.device, &viewInfo, nullptr, &device.emptyImage.view) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f; // Optional
    samplerInfo.maxLod = 0.0f;
    samplerInfo.mipLodBias = 0.0f; // Optional

    if (vkCreateSampler(device.device, &samplerInfo, nullptr, &device.emptyImage.sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }

  }

  void createDescriptorSetLayouts() {
    {
      VkDescriptorSetLayoutBinding materialLayoutBinding{};
      materialLayoutBinding.binding = 0;
      materialLayoutBinding.descriptorCount = 1;
      materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      materialLayoutBinding.pImmutableSamplers = nullptr;
      materialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding colTextureBinding{};
      colTextureBinding.binding = 1;
      colTextureBinding.descriptorCount = 1;
      colTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      colTextureBinding.pImmutableSamplers = nullptr;
      colTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding metalTextureBinding{};
      metalTextureBinding.binding = 2;
      metalTextureBinding.descriptorCount = 1;
      metalTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      metalTextureBinding.pImmutableSamplers = nullptr;
      metalTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding roughTextureBinding{};
      roughTextureBinding.binding = 3;
      roughTextureBinding.descriptorCount = 1;
      roughTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      roughTextureBinding.pImmutableSamplers = nullptr;
      roughTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding normalTextureBinding{};
      normalTextureBinding.binding = 4;
      normalTextureBinding.descriptorCount = 1;
      normalTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      normalTextureBinding.pImmutableSamplers = nullptr;
      normalTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding aoTextureBinding{};
      aoTextureBinding.binding = 5;
      aoTextureBinding.descriptorCount = 1;
      aoTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      aoTextureBinding.pImmutableSamplers = nullptr;
      aoTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding emissionTextureBinding{};
      emissionTextureBinding.binding = 6;
      emissionTextureBinding.descriptorCount = 1;
      emissionTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      emissionTextureBinding.pImmutableSamplers = nullptr;
      emissionTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding depthTextureBinding{};
      depthTextureBinding.binding = 7;
      depthTextureBinding.descriptorCount = 1;
      depthTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      depthTextureBinding.pImmutableSamplers = nullptr;
      depthTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::array<VkDescriptorSetLayoutBinding, 8> bindings = { materialLayoutBinding, colTextureBinding, metalTextureBinding, roughTextureBinding, normalTextureBinding, aoTextureBinding, emissionTextureBinding, depthTextureBinding };

      VkDescriptorSetLayoutCreateInfo layoutInfo{};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
      layoutInfo.pBindings = bindings.data();


      if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &materialDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
      }
    }
    {
      VkDescriptorSetLayoutBinding uboLayoutBinding{};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.pImmutableSamplers = nullptr;
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
      VkDescriptorSetLayoutCreateInfo layoutInfo{};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
      layoutInfo.pBindings = bindings.data();

      if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &modelDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
      }
    }
  }

  void loadModels() {
    std::array<glm::vec4, 10> positions = {
      glm::vec4(0.0f,0.0f,0.0f,1.0f),
      glm::vec4(-2.0f,2.0f, 0.0f,1.0f),
      glm::vec4(-2.0f,4.0f, 0.0f,1.0f),
      glm::vec4(-2.0f, 6.0f, 0.0f,1.0f),
      glm::vec4(2.0f,2.0f,0.0f,1.0f),
      glm::vec4(2.0f,4.0f,0.0f,1.0f),
      glm::vec4(2.0f,6.0f, 0.0f,1.0f),
      glm::vec4(0.0f, 5.0f,0.0f,1.0f),
      glm::vec4(0.0f,0.0f,0.0f,0.1f),
      glm::vec4(5.0f,5.0f,1.0f,0.1f),
    };
    int posIndx = 0;
    //non-material models
    //for (auto& pair : MODEL_PATH) {
    for (auto& path : MODEL_PATH) {
      std::cout << path << std::endl;
      auto m = std::make_shared<Model>();
      //Model m;
      m->device = &device;
      m->swapChainSize = swapChainImages.size();
      //m->loadModel(pair.first, pair.second);
      m->loadModel(path);
      m->modelPos[3] = positions[posIndx];
      m->createDescriptorBuffers();
      //m->createDescriptorSetLayout();
      m->descriptorSetLayout = modelDescriptorSetLayout;
      m->createDescriptorSets();
      m->materialDescriptorSetLayout = materialDescriptorSetLayout;
      m->createMaterialBuffers();
      m->name += std::to_string(posIndx);
      renderingPipeline.models.push_back(m);
      models.push_back(m);
      posIndx++;
    }


    //material models
    for (auto& path : MATERIAL_MODEL_PATH) {
      std::cout << path << std::endl;
      auto m = std::make_shared<Model>();
      //Model m;
      m->device = &device;
      m->swapChainSize = swapChainImages.size();
      m->loadModel(path);
      m->modelPos[3] = positions[posIndx];
      
      //m->createDescriptorSetLayout();
      m->descriptorSetLayout = modelDescriptorSetLayout;
      m->materialDescriptorSetLayout = materialDescriptorSetLayout;

      m->createDescriptorBuffers();
      m->createDescriptorSets();
      
      m->createMaterialBuffers();
      m->name += "_world_"+std::to_string(posIndx);
      pbrPipeline.models.push_back(m);
      models.push_back(m);
      posIndx++;
      //models.push_back(&pbr.models.back());
    }
    {
      auto path = "models/platform/platform2.obj";
      std::cout << path << std::endl;
      auto m = std::make_shared<Model>();
      //Model m;
      m->device = &device;
      m->swapChainSize = swapChainImages.size();
      m->loadModel(path);
      m->modelPos = glm::scale(m->modelPos, glm::vec3(50.0f, 50.0f, 50.0f));
      m->modelPos = glm::translate(m->modelPos, glm::vec3(0.0f, -0.3f, 0.0f));
      //m->modelPos[3] = glm::vec4(0.0f, -5.0f, 0.0f, 1.0f);
      //m->modelPos = glm::scale(m->modelPos, glm::vec3(1.0f, 1.0f, 1.0f));
      //m->modelPos = glm::rotate(m->modelPos, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
      //m->createDescriptorSetLayout();
      m->descriptorSetLayout = modelDescriptorSetLayout;
      m->materialDescriptorSetLayout = materialDescriptorSetLayout;

      m->createDescriptorBuffers();
      m->createDescriptorSets();

      m->createMaterialBuffers();
      m->name += "_pbr_" + std::to_string(posIndx);
      pbrPipeline.models.push_back(m);
      models.push_back(m);
      //posIndx++;
    }


    for (auto& path : MATERIAL_ALT_MODEL_PATH) {
      std::cout << path << std::endl;
      auto m = std::make_shared<Model>();
      //Model m;
      m->device = &device;
      m->swapChainSize = swapChainImages.size();
      m->loadModel(path);
      m->modelPos[3] = positions[posIndx];

      //m->createDescriptorSetLayout();
      m->descriptorSetLayout = modelDescriptorSetLayout;
      m->materialDescriptorSetLayout = materialDescriptorSetLayout;

      m->createDescriptorBuffers();
      m->createDescriptorSets();

      m->createMaterialBuffers();
      m->name += "_pbrAlt_" + std::to_string(posIndx);
      pbrAltPipeline.models.push_back(m);
      models.push_back(m);
      posIndx++;
      //models.push_back(&pbr.models.back());
    }


  }

  void setupPushConstant() {
    pushConstant.proj = Tools::projection(glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f));
  }

  void createGraphicsPipelines() {
    //textured models
    if(renderingPipeline.valid())
    {
      renderingPipeline.device = &device;
      renderingPipeline.swapChainExtent = swapChainExtent;
      renderingPipeline.renderPass = renderPass;
      renderingPipeline.pushConstantSize = sizeof(UniformBufferObject);
      renderingPipeline.msaaSamples = msaaSamples;
      renderingPipeline.descriptorSetLayouts.push_back(lightDescriptorLayout);
     
      renderingPipeline.descriptorSetLayouts.push_back(modelDescriptorSetLayout);
      renderingPipeline.descriptorSetLayouts.push_back(materialDescriptorSetLayout);
      
      //renderingPipeline.descriptorSetLayouts.push_back(shadowMap.descriptorLayout);
      renderingPipeline.descriptorSetLayouts.push_back(shadowCubeMap.shadowDescriptorSetLayout);
      renderingPipeline.createGraphicsPipeline("compiledshaders/shader.tangent.vert.spv", "compiledshaders/shader.frag.spv");
    }

    //MATERIAL MODELS
    if(pbrPipeline.valid())
    {
      pbrPipeline.device = &device;
      pbrPipeline.swapChainExtent = swapChainExtent;
      pbrPipeline.renderPass = renderPass;
      pbrPipeline.pushConstantSize = sizeof(UniformBufferObject);
      pbrPipeline.msaaSamples = msaaSamples;
      pbrPipeline.descriptorSetLayouts.push_back(lightDescriptorLayout);

      pbrPipeline.descriptorSetLayouts.push_back(modelDescriptorSetLayout);
      pbrPipeline.descriptorSetLayouts.push_back(materialDescriptorSetLayout);

      pbrPipeline.descriptorSetLayouts.push_back(shadowCubeMap.shadowDescriptorSetLayout);
      pbrPipeline.descriptorSetLayouts.push_back(pbrMaps.descriptorSetLayout);

      pbrPipeline.createGraphicsPipeline("compiledshaders/shader.world.vert.spv", "compiledshaders/pbr_world.frag.spv");
    }
    if (pbrAltPipeline.valid())
    {
      pbrAltPipeline.device = &device;
      pbrAltPipeline.swapChainExtent = swapChainExtent;
      pbrAltPipeline.renderPass = renderPass;
      pbrAltPipeline.pushConstantSize = sizeof(UniformBufferObject);
      pbrAltPipeline.msaaSamples = msaaSamples;
      pbrAltPipeline.descriptorSetLayouts.push_back(lightDescriptorLayout);

      pbrAltPipeline.descriptorSetLayouts.push_back(modelDescriptorSetLayout);
      pbrAltPipeline.descriptorSetLayouts.push_back(materialDescriptorSetLayout);

      pbrAltPipeline.descriptorSetLayouts.push_back(shadowCubeMap.shadowDescriptorSetLayout);
      pbrAltPipeline.descriptorSetLayouts.push_back(pbrMaps.descriptorSetLayout);
      pbrAltPipeline.descriptorSetLayouts.push_back(brdfTexture.descriptorSetLayout);

      pbrAltPipeline.createGraphicsPipeline("compiledshaders/shader.tangent.vert.spv", "compiledshaders/pbr_alt_tangent.frag.spv");
    }

  }

  /*void createShadowMap() {
    shadowMap.device = &device;
    shadowMap.createShadowMap(light.position, models);
  }*/

  void createShadowCubeMap() {
    shadowCubeMap.device = &device;
    shadowCubeMap.setupShadowMap(&light.position, models, swapChainImageViews.size());
  }

  //void createRenderedTexture() {
  //  renderedTexture.device = &device;
  //  std::vector<std::shared_ptr<Model>> test{ models[1] };
  //  renderedTexture.createShadowMap(light.position, test);
  //}

  void createBrdf() {
    brdfTexture.device = &device;
    brdfTexture.gen();
  }

  void createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      std::array<VkImageView, 3> attachments = {
          colorImageView,
          depthImageView,
          swapChainImageViews[i]
      };

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device.device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }

    imGuiFramebuffers.resize(swapChainImageViews.size());
    {
	    VkImageView attachment[1];
	    VkFramebufferCreateInfo info = {};
	    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    info.renderPass = imGuiRenderPass;
	    info.attachmentCount = 1;
	    info.pAttachments = attachment;
	    info.width = swapChainExtent.width;
	    info.height = swapChainExtent.height;
	    info.layers = 1;
	    for (uint32_t i = 0; i < imGuiFramebuffers.size(); i++)
	    {
		    attachment[0] = swapChainImageViews[i];
		    if (vkCreateFramebuffer(device.device, &info, nullptr, &imGuiFramebuffers[i]) != VK_SUCCESS) {
			    throw std::runtime_error("failed to create framebuffer!");
		    }
	    }
    }

  }

  void createColorResources() {
    VkFormat colorFormat = swapChainImageFormat;

    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }

  void createDepthResources() {
    VkFormat depthFormat = findDepthFormat();

    createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
  }

  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(device.physicalDevice, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
        return format;
      }
      else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }

    throw std::runtime_error("failed to find supported format!");
  }

  VkFormat findDepthFormat() {
    return findSupportedFormat(
      { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
  }

  void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
      swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
  }

  VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    //viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.levelCount = mipLevels;


    VkImageView imageView;
    if (vkCreateImageView(device.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
  }

  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //imageInfo.mipLevels = 1;
    imageInfo.mipLevels = mipLevels;
    //imageInfo.flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (vkCreateImage(device.device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device.device, image, imageMemory, 0);
  }

  void createCommandBuffers() {
    {
      pushConstantCommandBuffers.resize(swapChainFramebuffers.size());
      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = device.resetCommandPool;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = (uint32_t)pushConstantCommandBuffers.size();

      if (vkAllocateCommandBuffers(device.device, &allocInfo, pushConstantCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
      }
    }

    {
      imGuiCommandBuffers.resize(swapChainFramebuffers.size());
      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = device.resetCommandPool;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = (uint32_t)imGuiCommandBuffers.size();

      if (vkAllocateCommandBuffers(device.device, &allocInfo, imGuiCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
      }
    }

    {
      commandBuffers.resize(swapChainFramebuffers.size());

      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = device.graphicsCommandPool;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

      if (vkAllocateCommandBuffers(device.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
      }
      //vkCmdWaitEvents(commandBuffers[i], 1, &offscreenEvent, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 0, nullptr);


      for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
          throw std::runtime_error("failed to begin recording command buffer!");
        }

        shadowCubeMap.draw(commandBuffers[i], i);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };


        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //Non-pbr models
        std::vector<VkDescriptorSet*> descriptors = {&lightDescriptor,&shadowCubeMap.shadowDescriptorSet[i]};
        if(renderingPipeline.valid())
        {
          renderingPipeline.draw(commandBuffers[i], descriptors, i);
        }
        descriptors.push_back(&pbrMaps.descriptorSet);
        //PBR models
        if (pbrPipeline.valid())
        {
          pbrPipeline.draw(commandBuffers[i], descriptors, i);
        }
        descriptors.push_back(&brdfTexture.descriptorSet);
        if (pbrAltPipeline.valid()) {
          pbrAltPipeline.draw(commandBuffers[i], descriptors, i);
        }

        skybox.draw(commandBuffers[i]);

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
          throw std::runtime_error("failed to record command buffer!");
        }
      }
    }
  }

  void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
      }
    }
  }

  void updatePositions() {
    double currentX, currentY;
    glfwGetCursorPos(window, &currentX, &currentY);
    bool objectMoved = false;
    //Should the selected model be moved?
    if (keyFlags) {
      models[selectedModel]->updateModelpos(keyFlags, deltaFrame);
      objectMoved = true;
    }

    //If we have selected model update materials..
    if (selectedModel >= 0 && models[selectedModel]->materials.size() > 0)
      models[selectedModel]->updateMaterial(models[selectedModel]->materials[models[selectedModel]->meshes[selectedMesh].material]);

    //Update camera positions
    if(cameraView){
      //Seperate camera
      camera.updateCam(deltaFrame, currentX, currentY);
      pushConstant.view = camera.getViewMat();
      pushConstant.camPos = camera.cameraPos;
    }
    else {
      //Camera attached to object
      //move object based on mouse input if rightclick pressed
      if (camera.camFlags & 32) {
        models[selectedModel]->modelPos = glm::rotate(models[selectedModel]->modelPos, glm::radians(float(currentX - camera.mouseStartX)*0.2f), glm::vec3(0.0f, -1.0f, 0.0f));
        camera.tpX = 0.0f;
        if (camera.camFlags & 16)
          models[selectedModel]->modelPos = glm::translate(models[selectedModel]->modelPos, glm::vec3(0.0f, 0.0f, 2.0f * 5.0f * deltaFrame));
        objectMoved = true;
      }
      pushConstant.camPos = models[selectedModel]->modelPos * glm::vec4(camera.updateObjectCam(currentX, currentY),1.0f);
      glm::vec4 front(0.0f, 0.0f, 0.1f, 1.0f);
      front = models[selectedModel]->modelPos * front;
      pushConstant.view = glm::lookAt(pushConstant.camPos, glm::vec3(front),glm::vec3(0.0f,1.0f,0.0f));
    }

    //if object has moved, update descriptors
    if (objectMoved) {
      models[selectedModel]->updateDescriptors();
    }

    //update the positional information for skybox
    skybox.model.modelPos = pushConstant.view;
    skybox.model.modelPos[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    skybox.model.updateDescriptors();

    //simple update for single light source...
    void* data;
    vkMapMemory(device.device, lightMemory, 0, sizeof(LightSource), 0, &data);
    memcpy(data, &light, sizeof(LightSource));
    vkUnmapMemory(device.device, lightMemory);

    //Update shadowmap descriptors with potential new light position
    shadowCubeMap.updateRenderDescriptorSets();

  }

  void drawFrame() {
    vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device.device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updatePositions();
    //send the camera info as pushconstant
    {
      VkCommandBufferBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      if (vkBeginCommandBuffer(pushConstantCommandBuffers[imageIndex], &info) != VK_SUCCESS) {
        throw std::runtime_error("failed to create psuch constant command buffer!");
      }
    }
    vkCmdPushConstants(pushConstantCommandBuffers[imageIndex], renderingPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformBufferObject), (void*)&pushConstant);
    if (vkEndCommandBuffer(pushConstantCommandBuffers[imageIndex]) != VK_SUCCESS) {
      throw std::runtime_error("failed to end imgui command buffer!");
    }

    //IMGUI start
    {
      
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      {
        static float f = 0.0f;

        ImGui::Begin("Settings");                          

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Text("MSAA level: %d", msaaSamples);
        ImGui::SameLine();
        if (ImGui::Button("2x"))
          msaaSamples = VK_SAMPLE_COUNT_2_BIT;
        ImGui::SameLine();
        if (ImGui::Button("4x"))
          msaaSamples = VK_SAMPLE_COUNT_4_BIT;
        ImGui::SameLine();
        if (ImGui::Button("8x"))
          msaaSamples = VK_SAMPLE_COUNT_8_BIT;

        if (ImGui::Button("apply"))
          framebufferResized = true;

        {
          ImGui::Text("Adjust Light");
          ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 10000.0f);
          ImGui::ColorEdit3("LColor", &light.color.x);
          ImGui::DragFloat("X", &light.position[0]);
          ImGui::DragFloat("Y", &light.position[1]);
          ImGui::DragFloat("Z", &light.position[2]);
        }

        ImGui::Text("Model Select");
        if (cameraView && selectedModel >= 0 && ImGui::Button("Switch to Model view")) {
          cameraView = false;
        }
        else if (!cameraView && ImGui::Button("Switch to Camera view")) {
          cameraView = true;
        }
        if (ImGui::BeginCombo("##modelSelectCombo", selectedModel >= 0 ? models[selectedModel]->name.c_str(): "select a model")) // The second parameter is the label previewed before opening the combo.
        {
          //ImGui::Text("-Non-Mat. Models-");
          for (int i = 0; i < models.size(); i ++)
          {
            bool is_selected = (i == selectedModel); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(models[i]->name.c_str(), is_selected)){
              selectedModel = i;
              selectedMesh = 0;
            }
            if (is_selected)
              ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
          }
          ImGui::EndCombo();
        }
        
        if (selectedModel >= 0 && models[selectedModel]->meshes.size() > 1) {
          ImGui::Text("Mesh Select");
          if (ImGui::BeginCombo("##meshSelectCombo", models[selectedModel]->meshes[selectedMesh].name.c_str())) // The second parameter is the label previewed before opening the combo.
          {
            for (int i = 0; i < models[selectedModel]->meshes.size(); i++)
            {
              bool is_selected = (i == selectedMesh); // You can store your selection however you want, outside or inside your objects
              if (ImGui::Selectable(models[selectedModel]->meshes[i].name.c_str(), is_selected)){
                selectedMesh = i;
              }
              if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
          }
        }

        if (selectedModel >= 0 && models[selectedModel]->materials.size() > 0) {
          int idx = models[selectedModel]->meshes[selectedMesh].material;
          ImGui::Text("Material Name %s", models[selectedModel]->materials[idx].name.c_str());
          if (!models[selectedModel]->materials[idx].colorTexture.valid)
            ImGui::ColorEdit3("Color", &models[selectedModel]->materials[idx].properties.m_color.x);
          else
            ImGui::Text("Texture: %s", &models[selectedModel]->materials[idx].colorTexture.filename);
          ImGui::SliderFloat("Reflectivity", &models[selectedModel]->materials[idx].properties.m_reflectivity, 0.0f, 1.0f);
          ImGui::SliderFloat("Metalness", &models[selectedModel]->materials[idx].properties.m_metalness, 0.0f, 1.0f);
          ImGui::SliderFloat("Fresnel", &models[selectedModel]->materials[idx].properties.m_fresnel, 0.0f, 1.0f);
          ImGui::SliderFloat("Shininess", &models[selectedModel]->materials[idx].properties.m_shininess, 0.0f, 20000.0f);
          ImGui::SliderFloat("Emission", &models[selectedModel]->materials[idx].properties.m_emission, 0.0f, 10.0f);
        }

        ImGui::End();


      }
      ImGui::Render();

      {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(imGuiCommandBuffers[imageIndex], &info) != VK_SUCCESS) {
          throw std::runtime_error("failed to create imgui command buffer!");
        }
      }
      {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = imGuiRenderPass;
        info.framebuffer = imGuiFramebuffers[imageIndex];
        info.renderArea.extent.width = swapChainExtent.width;
        info.renderArea.extent.height = swapChainExtent.height;
        info.clearValueCount = 0;
        //info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(imGuiCommandBuffers[imageIndex], &info, VK_SUBPASS_CONTENTS_INLINE);
      }

      ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imGuiCommandBuffers[imageIndex]);

      vkCmdEndRenderPass(imGuiCommandBuffers[imageIndex]);
      if (vkEndCommandBuffer(imGuiCommandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("failed to end imgui command buffer!");
      }
    }
    //IMGUI END

    std::array<VkCommandBuffer, 3> submitCommandBuffers =
    { pushConstantCommandBuffers[imageIndex], commandBuffers[imageIndex], imGuiCommandBuffers[imageIndex] };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
    submitInfo.pCommandBuffers = submitCommandBuffers.data();

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device.device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(device.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
      framebufferResized = false;
      recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
      //if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      //  return availableFormat;
      //}
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    //return availableFormats[0];
  }

  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    }
    else {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);

      VkExtent2D actualExtent = {
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height)
      };

      actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
    }
  }

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
  }

  bool isDeviceSuitable(VkPhysicalDevice device) {
    //QueueFamilyIndices indices = findQueueFamilies(device);
    findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return qfi.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
  }

  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }

    throw std::runtime_error("failed to find suitable memory type!");
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  void findQueueFamilies(VkPhysicalDevice device) {
    //QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        //std::cout << "Supports gfx" << '\n';
        qfi.graphicsFamily = i;
      }
      else if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
        //std::cout << "Supports Compute" << '\n';
      }
      else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
        //std::cout << "Supports transfer" << '\n';
        qfi.transferFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      //vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface2, &presentSupport);
      if (presentSupport) {
        qfi.presentFamily = i;
      }

      if (qfi.isComplete()) {
        break;
      }

      i++;
    }

    //return indices;
  }

  std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        return false;
      }
    }

    return true;
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }
};

int main() {
  VulkanRenderer app;

  try {
    app.run();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

