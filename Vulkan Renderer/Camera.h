#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
class Camera
{
public:
  Camera():cameraPos(glm::vec3(0.0f, 0.0f, 10.0f)) {};
  Camera(glm::vec3 pos) :cameraPos(pos) {};
  Camera(float x, float y, float z) :cameraPos(x,y,z) {};
  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  float yaw = -90.0f;
  float pitch = 0.0f;
  
  glm::vec3 tpPos = glm::vec3(0.0f, 0.0f, 0.0f);
  float tpX = 0.0f;
  float tpY = 0.0f;
  float cameraRadius = 0.0f;
  double mouseStartX = 0.0f;
  double mouseStartY = 0.0f;
  uint32_t camFlags = 0;

  
  void updateCam(float, float, float, float);
  void updateCam(float, double, double);
  glm::vec3 updateObjectCam(double, double);
  glm::mat4 getViewMat();
};

