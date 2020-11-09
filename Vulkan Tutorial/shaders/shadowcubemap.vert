#version 450
#extension GL_EXT_multiview : enable

layout(binding = 0) uniform UBO {
  mat4 proj;
  mat4 view[6];
  vec4 lightPos;
} ubo;

layout(set = 1, binding = 0) uniform Model {
  mat4 pos;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outLightPos;

void main() {
  gl_Position = ubo.proj * ubo.view[gl_ViewIndex] * model.pos * vec4(inPosition, 1.0);
	//outPos = (ubo.view[gl_ViewIndex] * model.pos) * vec4(inPosition, 1.0);	
	//outLightPos = (ubo.view[gl_ViewIndex] * ubo.lightPos).xyz; 
  outPos = (model.pos * vec4(inPosition, 1.0)).xyz;	
  outPos.x *= 1;
	outLightPos = ubo.lightPos.xyz; 
}

