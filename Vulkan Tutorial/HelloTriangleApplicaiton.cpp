#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
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
//#include "Model.h"
#include "Pipeline.h"
#include "SkyboxTexture.h"
#include "HdrTexture.h"
#include "cubeMap.h"

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

const std::array<std::pair<std::string, std::string>, 2> MODEL_PATH = { std::make_pair("models/craneo.obj","textures/difuso_flip_oscuro.jpg"),
std::make_pair("models/cube.obj","textures/texture.jpg") };
const std::array<std::string, 1> MATERIAL_MODEL_PATH = { "models/sphere.obj" };

//const std::array<std::string, 1> MODEL_PATH = { "models/craneo.obj"};
//const std::string TEXTURE_PATH = "textures/difuso_flip_oscuro.jpg";

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
  //alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct LightSource {
  glm::vec4 position;
  glm::vec4 color;
  float intensity;
};

class HelloTriangleApplication {
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

  //VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_2_BIT;
  //VkDevice device;


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

  //std::vector<VkBuffer> uniformBuffers;
  //std::vector<VkDeviceMemory> uniformBuffersMemory;

  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;

  bool framebufferResized = false;

  QueueFamilyIndices qfi;
  Device device;

  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 8.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  float cameraY = 0.0f;
  float cameraZ = -1.0f;
  float cameraX = 0.0f;
  float cameraRadius = 4.0f;
  float xoffset = 0;
  float yoffset = 0;

  //float deltaTime = 0.0f;	// Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame
  float deltaFrame = 0.0f;

  int keyFlags = 0;
  int mouseFlags = 0;
  double mouseStartX, mouseStartY, mouseScroll = 0.0;

  float yaw = -90;
  float pitch = 0;

  std::vector<VkCommandBuffer> imGuiCommandBuffers;
  VkRenderPass imGuiRenderPass;
  std::vector<VkFramebuffer> imGuiFramebuffers;

  std::vector<UniformBufferObject> pushConstants;
  std::vector<VkCommandBuffer> pushConstantCommandBuffers;

  bool cameraView = true;

  int currentModel = 0;
  std::vector<Model*> models;

  LightSource light;
  std::vector<VkBuffer> lightBuffer;
  std::vector<VkDeviceMemory> lightMemory;

  //Model skyboxModel;
  //Model* skyboxModel;
  //SkyboxTexture sTex;

  struct {
    Model* model;
    SkyboxTexture texture;
    Pipeline pipeline;
  } skybox;


  int zcounter = 1;

  void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    //window2 = glfwCreateWindow(offscreenDim, offscreenDim, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
  }

  static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }

  void initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();

    //createSurface2();

    createDevice();

    createSwapChain();
    createImageViews();

    createRenderPass();


    //loadModels();

    createUniformBuffers();
    setupLights();


    createSkybox();

    createGraphicsPipelines();

    //generateCubeMap();
    setupImGUI();

    createColorResources();
    createDepthResources();
    createFramebuffers();

    
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

    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_W) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 1;
      else if (action == GLFW_RELEASE)
        app->keyFlags &= ~1;
      //app->cameraPos += app->cameraFront * speed;
    }
    if (key == GLFW_KEY_S) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 2;
      else if (action == GLFW_RELEASE)
        app->keyFlags &= ~2;
      //app->cameraPos -= app->cameraFront * speed;
    }
    if (key == GLFW_KEY_A) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 4;
      else if (action == GLFW_RELEASE)
        app->keyFlags &= ~4;
      //app->cameraPos -= glm::normalize(glm::cross(app->cameraFront, app->cameraUp)) * speed;
    }
    if (key == GLFW_KEY_D) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 8;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~8;
    }
    if (key == GLFW_KEY_UP) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 16;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~16;
    }
    if (key == GLFW_KEY_DOWN) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 32;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~32;
    }
    if (key == GLFW_KEY_LEFT) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 64;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~64;
    }
    if (key == GLFW_KEY_RIGHT) {
      if (action == GLFW_PRESS)
        app->keyFlags |= 128;
      if (action == GLFW_RELEASE)
        app->keyFlags &= ~128;
    }
    if (key == GLFW_KEY_Z) {
      if (action == GLFW_PRESS)
      {
        //1.0f*((app->zcounter+1)%2)
        //* (app->zcounter % 2)
        //1.0f * (app->zcounter%2)
        //app->offscreenUbo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f , 1.0f , 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //auto dir = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //dir[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        //app->offscreenModel.modelPos = dir;
        //app->offscreenModel.updateDescriptors(0);
        //app->offscreenUbo.view = glm::rotate(app->offscreenUbo.view, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //app->renderOffscreen();
        //app->updateSkybox();
        
        
        
      }
      //if (action == GLFW_RELEASE)
        
    }
  }
  
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse)
      return;
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    if (yoffset > 0)
    {
      if (app->cameraRadius > 2.0)
        app->cameraRadius--;
      else {
        app->cameraY = 0.0f;
        app->cameraZ = -1.0f;
        app->cameraX = 0.0f;
        app->yoffset = 0.0f;
        app->cameraRadius = 0.0f;
      }
        

      /*if (app->cameraY > 1.0)
        app->cameraY--;
      else if (app->cameraY < -1.0)
        app->cameraY++;
      else
        app->cameraY = 0.0f;*/
      
    }
    else
    {
      if(app->cameraRadius < 2.0f){
        app->cameraRadius = 2.0f;
      }
      else
        app->cameraRadius++;
     // app->cameraY++;
    }
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse)
      return;

    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      if (action == GLFW_PRESS) {
        //std::cout << "LEFT Mouse Button Pressed" << std::endl;
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //glfwGetCursorPos(window, &app->mouseStartX, &app->mouseStartY);
        //std::cout << "start x: " << app->mouseStartX << "start y: " << app->mouseStartY << std::endl;
        glfwGetCursorPos(window, &app->mouseStartX, &app->mouseStartY);
        app->mouseFlags |= 2;

      }
      else if (action == GLFW_RELEASE) {
        //std::cout << "LEFT Mouse Button Released" << std::endl;
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        app->mouseFlags &= ~2;
      }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (action == GLFW_PRESS) {
        //std::cout << "LEFT Mouse Button Pressed" << std::endl;
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &app->mouseStartX, &app->mouseStartY);
        //std::cout << "start x: " << app->mouseStartX << "start y: " << app->mouseStartY << std::endl;
        app->mouseFlags |=  1;

      }
      else if (action == GLFW_RELEASE) {
        //std::cout << "LEFT Mouse Button Released" << std::endl;
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        app->mouseFlags &= ~1;
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
    for (auto& model : models)
      std::cout << model->name << std::endl;
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

    for (auto& pipeline : pipelines) {
      pipeline->cleanup();
    }

    skybox.texture.cleanup();
    skybox.model->cleanup();
    delete skybox.model;
    skybox.pipeline.cleanup();

    for (auto i = 0; i < lightBuffer.size(); i++) {
      vkDestroyBuffer(device.device, lightBuffer[i], nullptr);
      vkFreeMemory(device.device, lightMemory[i], nullptr);
    }

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
    //createGraphicsPipelines();

    //createImGuiRenderPass();

    createColorResources();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createCommandBuffers();
  }

  void createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
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

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
      throw std::runtime_error("failed to create render pass!");
    }
  }


  void createSkybox() {
    skybox.model = new Model;
    skybox.model->device = &device;
    skybox.model->swapChainSize = swapChainImages.size();
    skybox.model->loadModel("skybox/cube.obj");
    //SkyboxTexture sTex;
    skybox.texture.device = &device;
    skybox.texture.loadHdr("envmaps/001.hdr",2048, skybox.model);
    //sTex.load("envmaps/001.hdr");


    VkDescriptorSetLayout descriptorSetLayout;
    {
    VkDescriptorSetLayoutBinding vertLayoutBinding{};
    vertLayoutBinding.binding = 0;
    vertLayoutBinding.descriptorCount = 1;
    vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vertLayoutBinding.pImmutableSamplers = nullptr;
    vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding fragLayoutBinding{};
    fragLayoutBinding.binding = 1;
    fragLayoutBinding.descriptorCount = 1;
    fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    fragLayoutBinding.pImmutableSamplers = nullptr;
    fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { vertLayoutBinding, fragLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor set layout!");
    }


    skybox.model->descriptorSetLayout = descriptorSetLayout;
    skybox.model->createDescriptorBuffers();
    }

    //auto pipeline = new Pipeline;
    skybox.pipeline.device = &device;
    skybox.pipeline.descriptorSetLayout = descriptorSetLayout;
    skybox.pipeline.swapChainExtent = swapChainExtent;
    skybox.pipeline.renderPass = renderPass;
    skybox.pipeline.pushConstantSize = sizeof(UniformBufferObject);
    skybox.pipeline.msaaSamples = msaaSamples;
    skybox.pipeline.createGraphicsPipeline("shaders/skybox_vert.spv", "shaders/skybox_frag.spv");
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = device.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    skybox.model->descriptorSets.resize(swapChainImages.size());
    if (vkAllocateDescriptorSets(device.device, &allocInfo, skybox.model->descriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapChainImages.size(); i++) {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = skybox.model->descriptorBuffer[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(glm::mat4);

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = skybox.texture.view;
      imageInfo.sampler = skybox.texture.sampler;

      std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = skybox.model->descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = skybox.model->descriptorSets[i];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pImageInfo = &imageInfo;

      vkUpdateDescriptorSets(device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
  }

  void setupLights() {
    light.color = glm::vec4(1.0f);
    light.position = glm::vec4(0.0f, 20.0f, 0.0f, 1.0f);
    light.intensity = 500.0f;

    VkDeviceSize lightBufferSize = sizeof(LightSource);
    lightBuffer.resize(swapChainImages.size());
    lightMemory.resize(swapChainImages.size());
    for (auto i = 0; i < swapChainImages.size(); i++) {
      device.createBuffer(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, lightBuffer[i], lightMemory[i]);
      void* data;
      vkMapMemory(device.device, lightMemory[i], 0, lightBufferSize, 0, &data);
      memcpy(data, &light, lightBufferSize);
      vkUnmapMemory(device.device, lightMemory[i]);
    }

  }

  void createGraphicsPipelines() {
    //textured models
    {
      VkDescriptorSetLayout descriptorSetLayout;
      VkDescriptorSetLayoutBinding uboLayoutBinding{};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.pImmutableSamplers = nullptr;
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding samplerLayoutBinding{};
      samplerLayoutBinding.binding = 1;
      samplerLayoutBinding.descriptorCount = 1;
      samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      samplerLayoutBinding.pImmutableSamplers = nullptr;
      samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
      VkDescriptorSetLayoutCreateInfo layoutInfo{};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
      layoutInfo.pBindings = bindings.data();

      if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
      }
      //create the pipeline
      auto pipeline = new Pipeline;
      pipeline->device = &device;
      pipeline->descriptorSetLayout = descriptorSetLayout;
      pipeline->swapChainExtent = swapChainExtent;
      pipeline->renderPass = renderPass;
      pipeline->pushConstantSize = sizeof(UniformBufferObject);
      pipeline->msaaSamples = msaaSamples;
      pipeline->createGraphicsPipeline("shaders/vert.spv", "shaders/frag.spv");
      //create the models
      int tmpMult = 0;
      for (auto& pair : MODEL_PATH) {
        std::cout << pair.first << std::endl;
        auto model = new Model();
        model->device = &device;
        model->swapChainSize = swapChainImages.size();
        if(tmpMult == 0)
          model->loadModel(pair.first, pair.second);
        else{
          //model->textures.push_back(offscreenTexture);
          //model->loadModel("models/quad.obj");
          model->loadModel("models/quad.obj", "textures/texture.jpg");
        }
        model->modelPos[3][0] = tmpMult*3.0f;
        //model->createDescriptorSetLayout();
        model->descriptorSetLayout = descriptorSetLayout;
        model->createDescriptorBuffers();
        model->createDescriptorSets();
        pipeline->models.push_back(model);
        models.push_back(model);
        tmpMult++;
      }

      pipelines.push_back(pipeline);
    }
    ////MATERIAL MODELS
    {
      VkDescriptorSetLayout descriptorSetLayout;
      VkDescriptorSetLayoutBinding vertLayoutBinding{};
      vertLayoutBinding.binding = 0;
      vertLayoutBinding.descriptorCount = 1;
      vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      vertLayoutBinding.pImmutableSamplers = nullptr;
      vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding fragLayoutBinding{};
      fragLayoutBinding.binding = 1;
      fragLayoutBinding.descriptorCount = 1;
      fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      fragLayoutBinding.pImmutableSamplers = nullptr;
      fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding lightLayoutBinding{};
      lightLayoutBinding.binding = 2;
      lightLayoutBinding.descriptorCount = 1;
      lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      lightLayoutBinding.pImmutableSamplers = nullptr;
      lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::array<VkDescriptorSetLayoutBinding, 3> bindings = { vertLayoutBinding, fragLayoutBinding, lightLayoutBinding };
      VkDescriptorSetLayoutCreateInfo layoutInfo{};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
      layoutInfo.pBindings = bindings.data();

      if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
      }

      auto pipeline = new Pipeline;
      pipeline->device = &device;
      pipeline->descriptorSetLayout = descriptorSetLayout;
      pipeline->swapChainExtent = swapChainExtent;
      pipeline->renderPass = renderPass;
      pipeline->pushConstantSize = sizeof(UniformBufferObject);
      pipeline->msaaSamples = msaaSamples;
      pipeline->createGraphicsPipeline("shaders/pbr_vert.spv", "shaders/pbr_frag.spv");
      //create the models
      for (auto& mpath : MATERIAL_MODEL_PATH) {
        std::cout << mpath << std::endl;
        auto model = new Model();
        model->device = &device;
        model->swapChainSize = swapChainImages.size();
        model->loadModel(mpath);
        model->modelPos[3][0] = -3.0f;
        //model->createDescriptorSetLayout();
        model->descriptorSetLayout = descriptorSetLayout;
        model->createDescriptorBuffers();
        model->createMaterialBuffers();
        //model->createDescriptorSets();
        //#######TEMP STUFF################
        std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = device.descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
        allocInfo.pSetLayouts = layouts.data();

        model->descriptorSets.resize(swapChainImages.size());
        if (vkAllocateDescriptorSets(device.device, &allocInfo, model->descriptorSets.data()) != VK_SUCCESS) {
          throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < swapChainImages.size(); i++) {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = model->descriptorBuffer[i];
          bufferInfo.offset = 0;
          bufferInfo.range = sizeof(glm::mat4);

          VkDescriptorBufferInfo bufferInfo2{};
          bufferInfo2.buffer = model->materialBuffer[i];
          bufferInfo2.offset = 0;
          bufferInfo2.range = sizeof(MaterialProperties);

          VkDescriptorBufferInfo bufferInfo3{};
          bufferInfo3.buffer = lightBuffer[i];
          bufferInfo3.offset = 0;
          bufferInfo3.range = sizeof(LightSource);

          std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

          descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          descriptorWrites[0].dstSet = model->descriptorSets[i];
          descriptorWrites[0].dstBinding = 0;
          descriptorWrites[0].dstArrayElement = 0;
          descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          descriptorWrites[0].descriptorCount = 1;
          descriptorWrites[0].pBufferInfo = &bufferInfo;

          descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          descriptorWrites[1].dstSet = model->descriptorSets[i];
          descriptorWrites[1].dstBinding = 1;
          descriptorWrites[1].dstArrayElement = 0;
          descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          descriptorWrites[1].descriptorCount = 1;
          descriptorWrites[1].pBufferInfo = &bufferInfo2;

          descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
          descriptorWrites[2].dstSet = model->descriptorSets[i];
          descriptorWrites[2].dstBinding = 2;
          descriptorWrites[2].dstArrayElement = 0;
          descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          descriptorWrites[2].descriptorCount = 1;
          descriptorWrites[2].pBufferInfo = &bufferInfo3;

          vkUpdateDescriptorSets(device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
        pipeline->models.push_back(model);
        models.push_back(model);
      }

      pipelines.push_back(pipeline);
    }
    //pbr models
    //create descriptorsetlayout
    //create the pipeline
    //create the models
    //"shaders/pbr_vert.spv"
    //"shaders/pbr_frag.spv"


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

  bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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

  void createUniformBuffers() {

    pushConstants.resize(swapChainImages.size());
    for (auto i = 0; i < swapChainImages.size(); i++) {
      pushConstants[i].proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
      pushConstants[i].proj[1][1] *= -1;

      /*pushConstants[i].proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
      pushConstants[i].proj[1][1] *= -1;
      pushConstants[i].view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));*/
    }
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

      for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
          throw std::runtime_error("failed to begin recording command buffer!");
        }

        //vkCmdWaitEvents(commandBuffers[i], 1, &offscreenEvent, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
        
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

        //std::cout << pipelines.size() << std::endl;

        for(auto pipeline:pipelines){

          vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphicsPipeline);
          
          for(auto model: pipeline->models){

            VkBuffer vertexBuffers[] = { model->vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffers[i], model->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            //vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0, 1, &model->descriptorSets[i], 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[i], model->indices, 1, 0, 0, 0);

          }
        }
        
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, skybox.pipeline.graphicsPipeline);
        
        VkBuffer vertexBuffers[] = { skybox.model->vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[i], skybox.model->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, skybox.pipeline.pipelineLayout, 0, 1, &skybox.model->descriptorSets[i], 0, nullptr);

        vkCmdDrawIndexed(commandBuffers[i], skybox.model->indices, 1, 0, 0, 0);

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

  void updatePushConstant(uint32_t imageIndex) {
    if(cameraView){
      float speed = 5.0f * deltaFrame;

      if (((mouseFlags & 1 && mouseFlags & 2) || keyFlags & 1) && !(keyFlags & 2))
        cameraPos += cameraFront * speed;
      if (keyFlags & 2 && !(keyFlags & 1))
        cameraPos -= cameraFront * speed;
      if (keyFlags & 4 && !(keyFlags & 8))
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
      if (keyFlags & 8 && !(keyFlags & 4))
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;

      if (mouseFlags & 1) {

        //mouse button is pressed
        double currentX, currentY;
        glfwGetCursorPos(window, &currentX, &currentY);

        float xoffset = currentX - mouseStartX;
        float yoffset = mouseStartY - currentY;
        mouseStartX = currentX;
        mouseStartY = currentY;



        const float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
          pitch = 89.0f;
        if (pitch < -89.0f)
          pitch = -89.0f;


        //std::cout << "yaw: " << yaw << std::endl;
        //std::cout << "cam x: " << cameraFront.x << std::endl;
        //std::cout << "cam y: " << cameraFront.y << std::endl;
        //std::cout << "cam z: " << cameraFront.z << std::endl;

        //glm::vec3 direction;
        //direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        //direction.y = sin(glm::radians(pitch));
        //direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        //yaw = 0;
        cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront.y = sin(glm::radians(pitch));
        cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(cameraFront);

      }

      pushConstants[imageIndex].view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }
    else {
      //glm::vec3 pos{models[currentModel]->modelPos[3][0], models[currentModel]->modelPos[3][1]+3.0f,models[currentModel]->modelPos[3][2] - 3.0f };
      //glm::mat4 test{ models[currentModel]->modelPos };
      //test[3][0] = 0;
      //test[3][1] = 0;
      //test[3][2] = 0;
      //glm::vec4 front{ models[currentModel]->modelPos[3][0], models[currentModel]->modelPos[3][1],models[currentModel]->modelPos[3][2]+1.0, 1.0 };
      //front = test * front;
      if (mouseFlags) {
        double currentX, currentY;
        glfwGetCursorPos(window, &currentX, &currentY);

        const float sensitivity = 0.1f;

        float local_xOffset = (currentX - mouseStartX) * sensitivity;
        float local_yOffset = (mouseStartY - currentY) * sensitivity;
        mouseStartX = currentX;
        mouseStartY = currentY;
        //std::cout << "x: " << local_xOffset << glm::radians(local_xOffset) << std::endl;
        //std::cout << "y: " << local_yOffset << glm::radians(local_yOffset) << std::endl;
        xoffset += local_xOffset;
        yoffset += local_yOffset;




        if (yoffset > 85.0f)
          yoffset = 85.0f;
        else if (yoffset < -85.0f)
          yoffset = -85.0f;


        float newX = std::sin(glm::radians(xoffset)) * std::cos(glm::radians(yoffset));
        float newY = -std::sin(glm::radians(yoffset));
        float newZ = -std::cos(glm::radians(yoffset));
        if (mouseFlags & 2) {
          models[currentModel]->modelPos = glm::rotate(models[currentModel]->modelPos, glm::radians(xoffset), glm::vec3(0.0f, -1.0f, 0.0f));
          if (mouseFlags & 1)
            models[currentModel]->modelPos = glm::translate(models[currentModel]->modelPos, glm::vec3(0.0f, 0.0f, 2.0f * 5.0f * deltaFrame));
          cameraX = 0.0f;
          xoffset = local_xOffset;

        }
        else if (cameraRadius > 1.0f) {
          newZ *= std::cos(glm::radians(xoffset));
          cameraX = newX;
        }
        else{
          xoffset = 0.0f;
          yoffset = 0.0f;
        }
        if (cameraRadius > 1.0f) {
          cameraY = newY;
          cameraZ = newZ;
        }

      }


      glm::vec4 pos(cameraRadius*cameraX, cameraRadius*cameraY, cameraRadius*cameraZ, 1.0f);

      pos = models[currentModel]->modelPos * pos;
      glm::vec4 front(0.0f,0.0f,0.1f,1.0f);
      front = models[currentModel]->modelPos * front;
      pushConstants[imageIndex].view = glm::lookAt(glm::vec3(pos), glm::vec3(front), cameraUp);

    }
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

    //updateUniformBuffer(imageIndex);
    
    
    models[currentModel]->updateModelpos(keyFlags, deltaFrame);
    models[currentModel]->updateDescriptors(imageIndex);
    updatePushConstant(imageIndex);
    if(models[currentModel]->materials.size() > 0)
      models[currentModel]->updateMaterialBuffer(imageIndex);
    
    skybox.model->modelPos = pushConstants[imageIndex].view;
    skybox.model->modelPos[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    skybox.model->updateDescriptors(imageIndex);


    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];


    {
      VkCommandBufferBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      if (vkBeginCommandBuffer(pushConstantCommandBuffers[imageIndex], &info) != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui command buffer!");
      }
    }

    //VkCommandBuffer pcBuffer = device.beginSingleTimeCommands();
    for(auto pipeline:pipelines)
      vkCmdPushConstants(pushConstantCommandBuffers[imageIndex], pipeline->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UniformBufferObject), (void*)&pushConstants[imageIndex]);
    //device.endSingleTimeCommands(pcBuffer);

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
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
          counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

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

        ImGui::Text("Camera Control");

        if (cameraView && ImGui::Button("Switch to Model view")) {
          cameraView = false;
        }
        else if (!cameraView && ImGui::Button("Switch to Camera view")) {
          cameraView = true;
        }
        if (!cameraView) {
          ImGui::SliderFloat("X-axis:", &cameraX, -10.0f, 10.0f);
          ImGui::SliderFloat("Y-axis:", &cameraY, -10.0f, 10.0f);
          ImGui::SliderFloat("Z-axis:", &cameraZ, -10.0f, 10.0f);
          ImGui::SliderFloat("Camera radius:", &cameraRadius, 0.0f, 20.0f);
          float r = std::sqrt(std::pow(cameraX, 2) + std::pow(cameraY, 2) + std::pow(cameraZ, 2));
          ImGui::Text("real radius: %.3f", r);
        }

        ImGui::Text("Model Select");
        if (ImGui::BeginCombo("##combo", models[currentModel]->name.c_str())) // The second parameter is the label previewed before opening the combo.
        {
          for (int n = 0; n < models.size(); n++)
          {
            bool is_selected = (n == currentModel); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(models[n]->name.c_str(), is_selected))
              currentModel = n;
            if (is_selected)
              ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
          }
          ImGui::EndCombo();
        }


        if (models[currentModel]->materials.size() > 0) {
          ImGui::Text("Material Name %s", models[currentModel]->materials[0].m_name.c_str());
          ImGui::ColorEdit3("Color", &models[currentModel]->materials[0].properties.m_color.x);
          ImGui::SliderFloat("Reflectivity", &models[currentModel]->materials[0].properties.m_reflectivity, 0.0f, 1.0f);
          ImGui::SliderFloat("Metalness", &models[currentModel]->materials[0].properties.m_metalness, 0.0f, 1.0f);
          ImGui::SliderFloat("Fresnel", &models[currentModel]->materials[0].properties.m_fresnel, 0.0f, 1.0f);
          ImGui::SliderFloat("shininess", &models[currentModel]->materials[0].properties.m_shininess, 0.0f, 25000.0f);
          ImGui::SliderFloat("Emission", &models[currentModel]->materials[0].properties.m_emission, 0.0f, 1.0f);
        }

        //if (ImGui::Button("Reload Shaders")) {
        //  models[currentModel]->updateMaterialBuffer(imageIndex);
        //}

        ImGui::End();


      }
      ImGui::Render();

      //if (vkResetCommandPool(device, imGuiCommandPool, 0) != VK_SUCCESS) {
      //	throw std::runtime_error("failed to reset command pool!");
      //}

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

    //std::array<VkCommandBuffer, 3> submitCommandBuffers =
    //{ commandBuffers[imageIndex], pushConstantCommandBuffers[imageIndex], imGuiCommandBuffers[imageIndex] };

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

    //submitInfo.commandBufferCount = 1;
    //submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

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

  VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    return availableFormats[0];
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

  static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }
};

int main() {
  HelloTriangleApplication app;

  try {
    app.run();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

