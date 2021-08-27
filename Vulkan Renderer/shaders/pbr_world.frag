#version 450
#define PI 3.14159265359
#define EPSILON 0.015

layout(set=1, binding = 0) uniform Light { 
	vec4 position;
	vec4 color;
	float intensity;
} light;

layout(set=3, binding = 0) uniform Material {
	vec4	  color;
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

layout(set=5, binding = 0) uniform samplerCube irradianceMap;
layout(set=5, binding = 1) uniform samplerCube reflectionMap;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldSpaceNormal; 
layout(location = 2) in vec3 worldSpacePosition; 
layout(location = 3) in vec3 worldSpaceCam;
layout(location = 4) in mat3 TBN;
layout(location = 8) in mat3 TTBN;

layout(location = 0) out vec4 outColor;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	// number of depth layers
	const float minLayers = 8.0;
	const float maxLayers = 64.0;
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
	float bias   = 0.015;
	int samples  = 20;
	float viewDistance = length(sc);
	float diskRadius = 0.01 + (viewDistance/100.0)/10;  
	//diskRadius = 0.1;
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(shadowCubeMap, sc + sampleOffsetDirections[i] * diskRadius).r;
		if(currentDepth > closestDepth + EPSILON)
				shadow += 1.0;
	}
	shadow /= float(samples);  
	return shadow;
}

void main() {

	vec3 V = normalize(worldSpaceCam - worldSpacePosition);

    vec2 texCoords = fragTexCoord;

  	if(textureSize(depthMap,0).x > 1)
		texCoords = ParallaxMapping(texCoords,V);

	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
		discard;

    vec3 N = normalize(worldSpaceNormal);
    if(textureSize(normalMap,0).x > 1){
      N = (texture(normalMap, texCoords).xyz * 2.0 - 1.0);
      N = TBN * N ;
    }

	vec3 albedo;
	if(textureSize(colorMap,0).x > 1)
		albedo = pow(texture(colorMap, texCoords).rgb,vec3(2.2));
	else
		albedo = m.color.rgb;

	vec3 emission_term;
	if(textureSize(emissionMap,0).x > 1)
		emission_term = texture(emissionMap, texCoords).r*10.0 * albedo;
	else
		emission_term = m.emission * albedo;

	float roughness;
	if(textureSize(roughnessMap,0).x > 1)
		roughness = 1.0f - texture(roughnessMap, texCoords).r;
	else
		roughness = sqrt(sqrt(2/(m.shininess+2)));

	float ao;
	if(textureSize(aoMap,0).x > 1)
		ao = texture(aoMap, texCoords).r;
	else
		ao = 1.0f;

	float metal;
	if(textureSize(metalMap,0).x > 1)
		metal = texture(metalMap, texCoords).r;
	else
		metal = m.metalness;
	
	vec3 direct_illumination_term = vec3(0.0);
	
	{
		vec3 lightVec = light.position.xyz - worldSpacePosition;
		vec3 L = normalize(lightVec);
		float lightDirection = dot(L, N);
		if(lightDirection > 0){
			float shadow = shadowPCF(-lightVec);
			if(shadow < 0.99){
				vec3 li =  light.intensity * light.color.xyz * (1/pow(length(lightVec),2.0));
				vec3 diffuse_term = albedo * (1/PI) * lightDirection * li;

				vec3 wh = normalize(L + V);
				float s = m.shininess;
				float f = m.fresnel + (1.0 - m.fresnel)*pow(1 - dot(wh,L),5.0);
				float d = (s+2.0)/(2.0*PI) * pow(dot(N,wh),s);
				float NDwo = dot(N,V);
				float g = min(1.0, 2.0*dot(N,wh)/dot(V,wh) * min(NDwo,lightDirection));
				float brdf = (f*d*g)/(4.0*NDwo*lightDirection);

				vec3 dielectric_term = brdf * dot(N, L) * li + (1 - f) * diffuse_term;
				vec3 metal_term = brdf * albedo * dot(N, L) * li;
				vec3 microfacet_term = metal * metal_term + (1 - metal) * dielectric_term;
				direct_illumination_term = m.reflectivity * microfacet_term + (1 - m.reflectivity) * diffuse_term * (1.0 - shadow);
			}
		}
	}

	vec3 indirect_illumination = vec3(0.0);
	
	float environment_multiplier = 1.0;
	
	vec3 irradiance = environment_multiplier * texture(irradianceMap, N).rgb;
	vec3 diffuse_term = albedo * (1.0 / PI) * irradiance;

	vec3 wi = reflect(-V, N);
	vec3 wh = normalize(wi + V);
	vec3 li = /*environment_multiplier **/ textureLod(reflectionMap, wi, roughness * 8.0).rgb;

	float f = m.fresnel + (1 - m.fresnel)*pow((1 - dot(wi,wh)),5);
	vec3 dielectric_term = f * li + (1 - f) * diffuse_term;
	vec3 metal_term = f * albedo * li;
	vec3 microfacet_term = metal * metal_term + (1 -metal) * dielectric_term;
	indirect_illumination = m.reflectivity * microfacet_term + (1 - m.reflectivity) * diffuse_term;
	indirect_illumination *= ao;

	vec3 color = emission_term +  direct_illumination_term + indirect_illumination;

  	float exposure = 3.0f;
	color = vec3(1.0) - exp(-color * exposure);
	outColor = vec4(color,1.0);

}

