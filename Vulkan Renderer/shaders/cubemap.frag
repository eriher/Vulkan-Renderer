#version 450
const float PI = 3.14159265359;
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
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