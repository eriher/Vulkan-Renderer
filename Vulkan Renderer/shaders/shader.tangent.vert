#version 450

layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

layout(set=1, binding = 0) uniform Light { 
	vec4 position;
	vec4 color;
	float intensity;
} light;


layout(set=2, binding = 0) uniform Model {
  mat4 pos;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 tangentLightPos;
layout(location = 2) out vec3 tangentViewPos;
layout(location = 3) out vec3 tangentFragPos;
layout(location = 4) out vec3 tangentNormPos;
layout(location = 5) out mat3 TBN;

void main() {

    fragTexCoord = inTexCoord;

    vec3 T = normalize(vec3(model.pos * vec4(inTangent, 0.0)));
    vec3 N = normalize(vec3(model.pos * vec4(inNormal, 0.0)));
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = -cross(N, T);
    TBN = mat3(T, B, N);
    mat3 TTBN = transpose(TBN);

    
    tangentLightPos = TTBN * light.position.xyz;
    tangentViewPos  = TTBN * ubo.camPos;
    tangentFragPos  = TTBN * vec3(model.pos * vec4(inPosition, 1.0));
    tangentNormPos  = TTBN * mat3(inverse(transpose(model.pos))) * inNormal;
    gl_Position = ubo.proj * ubo.view * model.pos * vec4(inPosition, 1.0);

}

