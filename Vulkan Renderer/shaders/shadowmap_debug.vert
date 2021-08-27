#version 450

layout(set=0, binding = 0) uniform UBO {
  mat4 proj;
  mat4 view[6];
  vec4 lightPos;
} ubo;

layout(set = 1, binding = 0) uniform Model {
  mat4 pos;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout (location = 0) out vec2 outUV;

void main() {
	  outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	  gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}

