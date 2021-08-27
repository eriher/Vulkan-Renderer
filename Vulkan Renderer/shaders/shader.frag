#version 450
#define ambient 0.2
#define EPSILON 0.015
#define SHADOW_OPACITY 0.5

layout(set=1, binding = 0) uniform Light { 
	vec4 position;
	vec4 color;
	float intensity;
} light;

layout(set=3, binding = 0) uniform Material {
	vec4	color;
	float		reflectivity;
	float		shininess;
	float		metalness;
	float		fresnel;
	float		emission;
	float		transparency;
} m;

layout(set = 3, binding = 1) uniform sampler2D colorMap;
layout(set = 3, binding = 2) uniform sampler2D metalMap;
layout(set = 3, binding = 3) uniform sampler2D roughnessMap;
layout(set = 3, binding = 4) uniform sampler2D normalMap;
layout(set = 3, binding = 5) uniform sampler2D aoMap;
layout(set = 3, binding = 6) uniform sampler2D emissionMap;
layout(set = 3, binding = 7) uniform sampler2D depthMap;

//layout(set=3, binding = 1) uniform sampler2D shadowMap;
layout(set=4, binding = 0) uniform samplerCube shadowCubeMap;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 tangentLightPos;
layout(location = 2) in vec3 tangentViewPos;
layout(location = 3) in vec3 tangentFragPos;
layout(location = 4) in vec3 tangentNormPos;
layout(location = 5) in mat3 TBN;

layout(location = 0) out vec4 outColor;


vec2 occlusionParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
		// number of depth layers
		const float minLayers = 8.0;
		const float maxLayers = 32.0;
		float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * 0.1; 
    vec2 deltaTexCoords = P / numLayers;

		// get initial values
		vec2  currentTexCoords     = texCoords;
		float currentDepthMapValue = 1.0f - texture(depthMap, currentTexCoords).r;
  
		while(currentLayerDepth < currentDepthMapValue)
		{
				// shift texture coordinates along direction of P
				currentTexCoords -= deltaTexCoords;
				// get depthmap value at current texture coordinates
				currentDepthMapValue = 1.0f - texture(depthMap, currentTexCoords).r;  
				// get depth of next layer
				currentLayerDepth += layerDepth;  
		}

		// get texture coordinates before collision (reverse operations)
		vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

		// get depth after and before collision for linear interpolation
		float afterDepth  = currentDepthMapValue - currentLayerDepth;
		float beforeDepth = 1.0f - texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
		// interpolation of texture coordinates
		float weight = afterDepth / (afterDepth - beforeDepth);
		vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

		return finalTexCoords;  

} 


float shadowPCF(vec3 sc)
{
	float currentDepth = length(sc);

  vec3 sampleOffsetDirections[20] = vec3[]
	(
		 vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
		 vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
		 vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
		 vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
		 vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);   
	
	float shadow = 0.0;
  float closestDepth = texture(shadowCubeMap, sc).r;
	if(currentDepth > closestDepth + EPSILON)
			shadow += 1.0;
	int samples  = 20;
	float viewDistance = length(sc);
	float diskRadius = 0.01 + (viewDistance/100.0)/10;  
	//diskRadius = 0.01;
	for(int i = 0; i < samples; ++i)
	{
			float closestDepth = texture(shadowCubeMap, sc + sampleOffsetDirections[i%20] * diskRadius).r;
			if(currentDepth > closestDepth + EPSILON)
					shadow += 1.0;
	}
	shadow /= 1.0f+float(samples);  
	return shadow;
}


void main() {

		vec3 V = normalize(tangentViewPos - tangentFragPos);

    vec2 texCoords = fragTexCoord;

    if(textureSize(depthMap,0).x > 1)
	    texCoords = occlusionParallaxMapping(texCoords, V);

		if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
			discard;

		vec3 N = normalize(tangentNormPos);
    if(textureSize(normalMap,0).x > 1)
		{
			N = normalize(texture(normalMap, texCoords).xyz * 2.0 - 1.0);
		}

    vec3 albedo;
    if(textureSize(colorMap,0).x > 1){
	    albedo = pow(texture(colorMap, texCoords).rgb,vec3(2.2));
			}
    else
	    albedo = m.color.rgb;

		vec3 emission_term;
		if(textureSize(emissionMap,0).x > 1)
			emission_term = texture(emissionMap, texCoords).r*10.0 * albedo;
		else
			emission_term = m.emission * albedo;

		vec3 amb = ambient  * light.color.rgb;

    vec3 lightVec = tangentLightPos - tangentFragPos;
    vec3 L = normalize(lightVec);

		float lightDistance = length(lightVec);
		vec3 diffuse = vec3(0.0);
    float attenuation = 1.0 / (lightDistance * lightDistance);
    vec3 radiance = light.intensity * light.color.rgb * attenuation;
		//float shadow = 0.0;
    //float closestDepth = texture(shadowCubeMap, TBN * -L).x;
    //if(lightDistance > closestDepth)
    float shadow = shadowPCF(TBN * -lightVec);
		float diff = max(dot(L, N), 0.0);
		diffuse = diff * radiance * (1.0 - shadow);
		vec3 H = normalize(V + L);

		//blinn
		float spec = pow(max(dot(N, H), 0.0), 16.0);
		//phong
		//float spec = pow(max(dot(V, R), 0.0), 8.0);

		vec3 color = emission_term + (amb + diffuse)* albedo + spec;

		float exposure = 3.0f;
		color = vec3(1.0) - exp(-color * exposure);

		outColor = vec4(color, 1.0);
}

