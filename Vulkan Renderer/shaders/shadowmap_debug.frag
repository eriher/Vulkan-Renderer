#version 450

layout(set=2, binding = 0) uniform sampler2D texSampler;

layout(set=0, binding = 0) uniform Light { 
	vec4 position;
	vec4 color;
	float intensity;
} light;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 viewNormal;
layout(location = 3) in vec3 viewPosition;
layout(location = 4) in mat4 viewMatrix;

layout(location = 0) out vec4 outColor;

float LinearizeDepth(float depth)
{
  float n = 1.0; // camera z near
  float f = 96.0f; // camera z far
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main() {
	float depth = texture(texSampler, fragTexCoord).r;
	outColor = vec4(vec3(1.0-LinearizeDepth(depth)), 1.0);
}

