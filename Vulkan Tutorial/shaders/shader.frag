#version 450

layout(set=0, binding = 0) uniform Light { 
	vec4 position;
	vec4 color;
	float intensity;
} light;

layout(set=2, binding = 0) uniform sampler2D texSampler;

layout(set=3, binding = 1) uniform sampler2D shadowMap;
layout(set=4, binding = 0) uniform samplerCube shadowCubeMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 viewSpaceNormal;
layout(location = 3) in vec3 worldSpacePosition;
layout(location = 4) in mat4 viewMatrix;
layout(location = 8) in vec4 lightSpacePos;

layout(location = 0) out vec4 outColor;

#define ambient 0.1
#define EPSILON 0.015
#define SHADOW_OPACITY 0.5

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = ambient;
		}
	}
	return shadow;
}

/*
float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5; 
		float closestDepth = texture(shadowMap, projCoords.xy).r;   
		float currentDepth = projCoords.z;  
		
		float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;  
		return shadow;
}
*/

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

float filterPCF2(vec3 sc)
{
	float currentDepth = length(sc);
  
	
  //float closestDepth = texture(shadowCubeMap, sc).r;
  //float shadow = currentDepth  <= closestDepth + EPSILON ? 1.0 : 0.0;
	//return shadow;
	

	vec3 sampleOffsetDirections[20] = vec3[]
	(
		 vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		 vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		 vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		 vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		 vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);   

	float shadow = 0.0;
	float bias   = 0.015;
	int samples  = 20;
	float viewDistance = length((viewMatrix * vec4(worldSpacePosition,1.0)).xyz);
	//float diskRadius = 0.005;
	float diskRadius = (1.0 + (viewDistance / 100.0)) / 25.0;  
	for(int i = 0; i < samples; ++i)
	{
			float closestDepth = texture(shadowCubeMap, sc + sampleOffsetDirections[i] * diskRadius).r;
			if(currentDepth  <= closestDepth + EPSILON)
					shadow += 1.0;
	}
	shadow /= float(samples);  
	return shadow;
/*

	float shadow  = 0.0;
	float samples = 4.0;
	float offset  = 0.1;

	for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	{
			for(float y = -offset; y < offset; y += offset / (samples * 0.5))
			{
					for(float z = -offset; z < offset; z += offset / (samples * 0.5))
					{
							float closestDepth = texture(shadowCubeMap, sc + vec3(x, y, 0)).r;
							//float closestDepth = texture(shadowCubeMap, fragToLight + vec3(x, y, z)).r; 
							if(currentDepth  <= closestDepth + EPSILON)
									shadow += 1.0;
					}
			}
	}
	shadow /= (samples * samples * samples);
	return shadow;
	*/
}


void main() {
	
		vec3 color = texture(texSampler, fragTexCoord).rgb;
		vec3 eyePos = vec3(0.0);
		vec3 viewSpacePosition = (viewMatrix * vec4(worldSpacePosition, 1.0)).xyz;
    //ambient
    vec3 amb = 0.15 * color;

		vec3 diffuse = vec3(0.0);
		vec3 specular = vec3(0.0);

		//Shadow
		vec3 worldLightVec = worldSpacePosition - light.position.xyz;
		float sampledDist = texture(shadowCubeMap, worldLightVec).r;
		float dist = length(worldLightVec);

		if(dist <= sampledDist + EPSILON)
		{
			// diffuse
			vec3 wo = normalize(-viewSpacePosition);
			vec3 n = normalize(viewSpaceNormal);
		
			vec3 lightPos = (viewMatrix * light.position).xyz;
			vec3 lightVec = lightPos - viewSpacePosition;
			vec3 wi = normalize(lightVec);
			vec3 li = light.intensity * light.color.rgb * (1/pow(length(lightVec),2.0));
			float diff = max(dot(wi, n), 0.0);
			diffuse = diff * li;	
    
			// specular
			float spec = 0.0;
			vec3 wh = normalize(wi + wo);
			float val = dot(n, wh);
			if(val > 0)
				spec = pow(val, 64.0);
			else
				spec = 0.0;
			//spec = pow(max(dot(n, wh), 0.0), 64.0);
			specular = spec * li;    
    }

		color = (amb+diffuse+specular) * color;
		
		//Tonemapping??
		//float exposure = 1.0f;
		
		//vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
		
		outColor = vec4(color, 1.0);
}

