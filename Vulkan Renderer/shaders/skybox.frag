#version 450

layout (set = 2, binding = 0) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 color = texture(samplerCubeMap, inUVW).rgb;
  //color = color / (color + vec3(1.0));
  //color = pow(color, vec3(1.0/2.2));  
	float exposure = 3.0f;
  //color = vec3(1.0) - exp(-color * exposure);
	outFragColor = vec4(color, 1.0);
	//outFragColor = textureLod(samplerCubeMap, inUVW, 5.0);
}