#version 450
#extension GL_EXT_multiview : enable
layout(binding = 0) uniform UBO {
  mat4 proj;
  mat4 view[6];
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 localPos;

void main()
{
    localPos = inPosition;  
    gl_Position =  (ubo.proj * ubo.view[gl_ViewIndex] * vec4(localPos, 1.0)).xyww;
}