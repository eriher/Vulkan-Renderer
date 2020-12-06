#include "Camera.h"


void Camera::updateCam(float longitudinal, float lateral, float y, float p)
{
    cameraPos += cameraFront * longitudinal;
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * lateral;
    
    yaw += y;
    pitch += p;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront.y = sin(glm::radians(pitch));
    cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(cameraFront);
    cameraFront = glm::normalize(cameraFront);
}

void Camera::updateCam(float deltFrame, double x, double y)
{
  float speed = 5.0f * deltFrame;
  if (((camFlags & 16 && camFlags & 32) || camFlags & 1) && !(camFlags & 2))
    cameraPos += cameraFront * speed;
  else if (camFlags & 2 && !(camFlags & 1))
    cameraPos -= cameraFront * speed;
  if (camFlags & 8 && !(camFlags & 4))
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
  else if (camFlags & 4 && !(camFlags & 8))
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
  

  if (camFlags & 16) {
    const float sensitivity = 0.1f;
    yaw += (x - mouseStartX) * sensitivity;
    pitch += (mouseStartY - y) * sensitivity;
    mouseStartX = x;
    mouseStartY = y;
  }

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront.y = sin(glm::radians(pitch));
  cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(cameraFront);
  cameraFront = glm::normalize(cameraFront);
}

glm::vec3 Camera::updateObjectCam(double x, double y)
{
  if (camFlags > 8) {
    const float sensitivity = 0.1f;

    tpX += (x - mouseStartX) * sensitivity;
    tpY += (mouseStartY - y) * sensitivity;
    mouseStartX = x;
    mouseStartY = y;

    if (tpY > 89.0f)
      tpY = 89.0f;
    else if (tpY < -89.0f)
      tpY = -89.0f;
  }
  if (cameraRadius < 1.0f) {
    tpX = 0.0f;
    tpY = tpX;
    tpPos = glm::vec3(tpX);
  }
  else {
    float newX = 0.0f;
    float newY = -std::sin(glm::radians(tpY));
    float newZ = -std::cos(glm::radians(tpY));
    if (!(camFlags & 32)) {
      newX = std::sin(glm::radians(tpX))* std::cos(glm::radians(tpY));
      newZ *= std::cos(glm::radians(tpX));
    }
    
    tpPos = glm::vec3(cameraRadius * newX, cameraRadius * newY, cameraRadius * newZ);
    
  }
  return tpPos;
}

glm::mat4 Camera::getViewMat()
{
  return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
