#version 450

layout (location = 0) out float outFragColor;
//layout(location = 0) out vec4 outFragColor;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inLightPos;

void main() 
{
    vec3 lightVec = inPos - inLightPos;
    outFragColor = length(lightVec);
}