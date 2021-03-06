#version 450

layout(push_constant) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 0) uniform Model {
  mat4 pos;
} model;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout (location = 0) out vec3 localPos;

void main()
{
    localPos = inPosition;  
    gl_Position =  (ubo.proj * model.pos * vec4(localPos, 1.0)).xyww;
}