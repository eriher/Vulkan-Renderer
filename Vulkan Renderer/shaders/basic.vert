#version 450

layout(set=0, binding = 0) uniform UBO {
  mat4 transform;
} ubo;

layout(set = 1, binding = 0) uniform Model {
  mat4 pos;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;


void main() {
    gl_Position = ubo.transform * model.pos * vec4(inPosition, 1.0);
}

