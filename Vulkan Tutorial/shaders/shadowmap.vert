#version 450

layout(set=0, binding = 0) uniform UBO {
  mat4 lightView;
} ubo;

layout(set = 1, binding = 0) uniform Model {
  mat4 pos;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;


void main() {
    gl_Position = ubo.lightView * model.pos * vec4(inPosition, 1.0);
    //gl_Position = ubo.test * model.pos * vec4(inPosition, 1.0);
}

