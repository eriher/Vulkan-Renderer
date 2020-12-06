#version 450
const float PI = 3.14159265359;
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;

//  float theta = acos(max(-1.0f, min(1.0f, v.y)));
//	float phi = atan(v.z, v.x);
//	if (phi < 0.0f) phi = phi + 2.0f * PI;
//	// Use these to lookup the color in the environment map
//	return vec2(phi / (2.0 * PI), theta / PI);
}

layout (binding = 1) uniform sampler2D equirectangularMap;


layout (location = 0) in vec3 localPos;

layout (location = 0) out vec4 outFragColor;

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(localPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    vec3 mapped = color / (color + vec3(1.0));
    outFragColor = vec4(color, 1.0);
}