#version 450

layout(push_constant) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 0, binding = 0) uniform Model {
  mat4 pos;
} model;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outUVW;

void main() 
{

	outUVW = inPosition;
  outUVW.xy *= -1;
  vec4 pos = ubo.proj *  model.pos  * vec4(inPosition, 1.0);
  gl_Position = pos.xyww;
}
