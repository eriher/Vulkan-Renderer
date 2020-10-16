#version 450
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_EXT_debug_printf : enable

layout(push_constant) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 0) uniform Model {
  mat4 pos;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3	viewSpaceNormal; 
layout(location = 3) out vec3	viewSpacePosition; 
layout(location = 4) out mat4 viewMatrix;

void main() {
    //debugPrintfEXT("Here are two float values");
    gl_Position = ubo.proj * ubo.view * model.pos * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    viewSpaceNormal = (inverse(transpose(ubo.view*model.pos)) * vec4(inNormal, 1.0)).xyz;
    viewSpacePosition = ((ubo.view * model.pos) * vec4(inPosition, 1.0)).xyz;
    viewMatrix = ubo.view;
}
