#version 450

layout (location = 0) out float outFragColor;
//layout(location = 0) out vec4 outFragColor;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inLightPos;

void main() 
{
	// Store distance to light as 32 bit float value
    vec3 lightVec = inPos - inLightPos;
    //outFragColor = min(length(lightVec),100.0f);
    outFragColor = length(lightVec);
}