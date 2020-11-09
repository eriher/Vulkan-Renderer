#version 450
#define PI 3.14159265359

layout(set=1, binding = 0) uniform Material {
	vec4	color;
	float		reflectivity;
	float		shininess;
	float		metalness;
	float		fresnel;
	float		emission;
	float		transparency;
} m;

layout(set = 1, binding = 1) uniform sampler2D colorTexture;

layout(set=0, binding = 1) uniform Light { 
	vec4 position;
	vec4 color;
	float intensity;
} light;

layout(set=0, binding = 2) uniform samplerCube irradianceMap;
layout(set=0, binding = 3) uniform samplerCube reflectionMap;

layout(location = 0) in vec3 viewSpaceNormal; 
layout(location = 1) in vec3 viewSpacePosition; 
layout(location = 2) in mat4 viewMatrix; 
layout(location = 6) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 calculateDirectIllumiunation(vec3 wo, vec3 n)
{
	vec3 lightPos = (viewMatrix * light.position).xyz;
	vec3 lightVec = lightPos - viewSpacePosition;
	vec3 wi = normalize(lightVec);
	///////////////////////////////////////////////////////////////////////////
	// Task 1.2 - Calculate the radiance Li from the light, and the direction
	//            to the light. If the light is backfacing the triangle, 
	//            return vec3(0); 
	///////////////////////////////////////////////////////////////////////////
	float direction = dot(wi, n);
	if(direction <= 0)
		return vec3(0.0);
	//return vec3(0.5);
	vec3 li =  light.intensity * light.color.xyz * (1/pow(length(lightVec),2.0));
	//return li;
	///////////////////////////////////////////////////////////////////////////
	// Task 1.3 - Calculate the diffuse term and return that as the result
	///////////////////////////////////////////////////////////////////////////
	vec3 diffuse_term = texture(colorTexture,fragTexCoord).rgb * (1/PI) * direction * li;
	//return(diffuse_term);
	
	///////////////////////////////////////////////////////////////////////////
	// Task 2 - Calculate the Torrance Sparrow BRDF and return the light 
	//          reflected from that instead
	///////////////////////////////////////////////////////////////////////////
	
	vec3 wh = normalize(wi + wo);
	float s = m.shininess;
	float f = m.fresnel + (1.0 - m.fresnel)*pow(1 - dot(wh,wi),5.0);
	float d = (s+2.0)/(2.0*PI) * pow(dot(n,wh),s);
	//float g = min(1.0, min(2.0*((dot(n,wh)*dot(n,wo))/dot(wo,wh)),2.0*((dot(n,wh)*direction)/dot(wo,wh))));
	float g = min(1.0, 2.0*dot(n,wh)/dot(wo,wh) * min(dot(n,wo),direction));
	float brdf = (f*d*g)/(4.0*dot(n,wo)*direction);
	//return brdf * direction * li; 
	///////////////////////////////////////////////////////////////////////////
	// Task 3 - Make your shader respect the parameters of our material model.
	///////////////////////////////////////////////////////////////////////////
	vec3 dielectric_term = brdf * dot(n, wi) * li + (1 - f) * diffuse_term;
	vec3 metal_term = brdf * texture(colorTexture,fragTexCoord).rgb * dot(n, wi) * li;
	vec3 microfacet_term = m.metalness * metal_term + (1 - m.metalness) * dielectric_term;
	return m.reflectivity * microfacet_term + (1 - m.reflectivity) * diffuse_term;
	//return vec3(material_color);
}

vec3 calculateIndirectIllumination(vec3 wo, vec3 n)
{
	///////////////////////////////////////////////////////////////////////////
	// Task 5 - Lookup the irradiance from the irradiance map and calculate
	//          the diffuse reflection
	///////////////////////////////////////////////////////////////////////////
	
	mat4 viewInverse = inverse(viewMatrix);
	vec3 dir = normalize(viewInverse * vec4(n, 0.0)).xyz;
	float environment_multiplier = 1.0;
	
	// Calculate the spherical coordinates of the direction
	//float theta = acos(max(-1.0f, min(1.0f, dir.y)));
	//float phi = atan(dir.z, dir.x);
	//if (phi < 0.0f) phi = phi + 2.0f * PI;
	// Use these to lookup the color in the environment map
	//vec2 lookup = vec2(phi / (2.0 * PI), theta / PI);
	vec3 irradiance = environment_multiplier * texture(irradianceMap, dir).xyz;
	vec3 diffuse_term = texture(colorTexture,fragTexCoord).rgb * (1.0 / PI) * irradiance;
	//return diffuse_term;
	///////////////////////////////////////////////////////////////////////////
	// Task 6 - Look up in the reflection map from the perfect specular 
	//          direction and calculate the dielectric and metal terms. 
	///////////////////////////////////////////////////////////////////////////
	//vec3 wi = -reflect(wo, n);
	vec3 wi = reflect(-wo, n);
	vec3 dir2 = normalize(viewInverse * vec4(wi, 0.0)).xyz;
	vec3 wh = normalize(wi + wo);

	//float theta2 = acos(max(-1.0f, min(1.0f, dir2.y)));
	//float phi2 = atan(dir2.z, dir2.x);
	//if (phi < 0.0f) phi = phi2 + 2.0f * PI;
	// Use these to lookup the color in the environment map
	//vec2 lookup2 = vec2(phi2 / (2.0 * PI), theta2 / PI);

	float f = m.fresnel + (1 - m.fresnel)*pow((1 - dot(wi,wh)),5);
	float roughness = sqrt(sqrt(2/(m.shininess+2)));
	vec3 li = environment_multiplier * textureLod(reflectionMap, dir2, roughness * 8.0).xyz;
	vec3 dielectric_term = f * li + (1 - f) * diffuse_term;
	vec3 metal_term = f * texture(colorTexture,fragTexCoord).rgb * li;
	vec3 microfacet_term = m.metalness * metal_term + (1 - m.metalness) * dielectric_term;
	return m.reflectivity * microfacet_term + (1 - m.reflectivity) * diffuse_term;
}



void main() {
    //outColor = texture(texSampler, fragTexCoord);
	vec3 emission_term = m.emission * texture(colorTexture,fragTexCoord).rgb;

	//emission_term = emission_term / (emission_term + vec3(1.0));
	//emission_term = pow(emission_term, vec3(1.0/2.2)); 

	//outColor = vec4(emission_term, 1.0);

	//vec3 wo = normalize(vec3(0.0) - viewSpacePosition);
	vec3 wo = normalize(-viewSpacePosition);
	vec3 n = normalize(viewSpaceNormal);
	
	vec3 direct_illumination_term = vec3(0.0);
	{ // Direct illumination
		direct_illumination_term = calculateDirectIllumiunation(wo, n);
	}
	direct_illumination_term = direct_illumination_term / (direct_illumination_term + vec3(1.0));
  //color = pow(color, vec3(1.0/2.2));  
	vec3 indirect_illumination = vec3(0.0);
	indirect_illumination = calculateIndirectIllumination(wo,n);
	

	vec3 color = emission_term + direct_illumination_term + indirect_illumination;

  //color = color / (color + vec3(1.0));
  //color = pow(color, vec3(1.0/2.2));  

	outColor = vec4(color,1.0);

}

