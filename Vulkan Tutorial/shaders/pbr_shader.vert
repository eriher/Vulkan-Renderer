#version 450
//#extension GL_EXT_debug_printf : enable

layout(push_constant) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set=1, binding = 0) uniform Model {
  mat4 pos;
} model;

layout(set=3, binding = 0) uniform LightView {
  mat4 transform;
} lightView;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3	viewSpaceNormal; 
layout(location = 1) out vec3	viewSpacePosition; 
layout(location = 2) out mat4 viewMatrix;
layout(location = 6) out vec2 fragTexCoord;

void main() {
    //debugPrintfEXT("Here are two float values");
    gl_Position = ubo.proj * ubo.view * model.pos * vec4(inPosition, 1.0);
    viewSpaceNormal = (inverse(transpose(ubo.view*model.pos)) * vec4(inNormal, 1.0)).xyz;
    viewSpacePosition = ((ubo.view*model.pos) * vec4(inPosition, 1.0)).xyz;
    viewMatrix = ubo.view;
    fragTexCoord = inTexCoord;
    //lightSpacePos = (lightView.transform * model.pos) * vec4(inPosition, 1.0);
}

