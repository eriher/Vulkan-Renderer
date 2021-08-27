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

layout(set=6, binding = 0) uniform sampler2D brdfLUT;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldSpaceNormal; 
layout(location = 2) in vec3 worldSpacePosition; 
layout(location = 3) in vec3 worldSpaceCam;
layout(location = 4) in mat3 TBN;
layout(location = 8) in mat3 TTBN;

layout(location = 0) out vec4 outColor;
float heightScale = 0.1;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * 0.1; 
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


void main()
{		
	  vec3 V = normalize(worldSpaceCam - worldSpacePosition);

    vec2 texCoords = fragTexCoord;

    if(textureSize(depthMap,0).x > 1){
	    texCoords = parallaxMapping(texCoords, TTBN * V);
     }

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
	    emission_term = texture(emissionMap, texCoords).r*m.emission * albedo;
    else
	    emission_term = m.emission * albedo;

    float roughness;
    if(textureSize(roughnessMap,0).x > 1)
	    roughness = 1.0f-texture(roughnessMap, texCoords).r;
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

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metal);

    // reflectance equation
    vec3 direct_illumination_term = vec3(0.0);
    for(int i = 0; i < 1; ++i) 
    {
        // calculate per-light radiance
        vec3 lightVec = light.position.xyz - worldSpacePosition;
        vec3 L = normalize(lightVec);
        float lightDirection = dot(L, N);
        if(lightDirection > 0){
          float shadow = shadowPCF(-lightVec);
          if(shadow < 0.99){
            vec3 H = normalize(V + L);
            vec3 li =  light.intensity * light.color.xyz * (1/pow(length(lightVec),2.0));

            // Cook-Torrance BRDF
            float NDF = DistributionGGX(N, H, roughness);   
            float G   = GeometrySmith(N, V, L, roughness);    
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
          
            vec3 nominator    = NDF * G * F;
            float denominator = 4 * max(dot(N, V), 0.0) * lightDirection + 0.001; // 0.001 to prevent divide by zero.
            vec3 specular = nominator / denominator;
          
            // kS is equal to Fresnel
            vec3 kS = F;
            // for energy conservation, the diffuse and specular light can't
            // be above 1.0 (unless the surface emits light); to preserve this
            // relationship the diffuse component (kD) should equal 1.0 - kS.
            vec3 kD = vec3(1.0) - kS;
            // multiply kD by the inverse metalness such that only non-metals 
            // have diffuse lighting, or a linear blend if partly metal (pure metals
            // have no diffuse light).
            kD *= 1.0 - metal;

            // add to outgoing radiance
            direct_illumination_term += (kD * albedo / PI + specular) * li * lightDirection * (1.0 - shadow);
          }
        }
    }   
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metal;

    vec3 irradiance = texture(irradianceMap,  N).rgb;
    vec3 diffuse      = irradiance * albedo;
    //vec3 diffuse = albedo * (1.0 / PI) * irradiance;

    vec3 R = reflect(-V, N);

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 8.0;

    vec3 prefilteredColor = textureLod(reflectionMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 indirect_illumination = (kD * diffuse + specular) * ao;
    
    vec3 color = emission_term +  direct_illumination_term + indirect_illumination;
    
    float exposure = 3.0f;
		color = vec3(1.0) - exp(-color * exposure);
    outColor = vec4(specular, 1.0);
}