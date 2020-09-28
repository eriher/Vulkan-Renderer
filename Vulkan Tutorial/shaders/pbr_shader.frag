#version 450
#extension GL_ARB_separate_shader_objects : enable
#define PI 3.14159265359

layout(binding = 1) uniform Material {
	vec4	color;
	float		reflectivity;
	float		shininess;
	float		metalness;
	float		fresnel;
	float		emission;
	float		transparency;
} m;

layout(binding = 2) uniform Light { 
	vec4 position;
	vec4 color;
	float intensity;
} light;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 viewSpaceNormal; 
layout(location = 3) in vec3 viewSpacePosition; 
layout(location = 4) in mat4 viewMatrix; 

layout(location = 0) out vec4 outColor;

vec3 calculateDirectIllumiunation(vec3 wo, vec3 n)
{
	vec3 lightPos = (viewMatrix*light.position).xyz;
	vec3 wi = normalize(lightPos - viewSpacePosition);
	///////////////////////////////////////////////////////////////////////////
	// Task 1.2 - Calculate the radiance Li from the light, and the direction
	//            to the light. If the light is backfacing the triangle, 
	//            return vec3(0); 
	///////////////////////////////////////////////////////////////////////////
	float direction = dot(n, wi);
	if(direction <= 0)
		return vec3(0.0);
	//return vec3(0.5);
	vec3 li =  light.intensity * light.color.xyz * (1/pow(length(lightPos - viewSpacePosition),2));
	//return li;
	///////////////////////////////////////////////////////////////////////////
	// Task 1.3 - Calculate the diffuse term and return that as the result
	///////////////////////////////////////////////////////////////////////////
	vec3 diffuse_term = m.color.xyz * (1/PI) * abs(direction) * li;
	//return(diffuse_term);
	
	///////////////////////////////////////////////////////////////////////////
	// Task 2 - Calculate the Torrance Sparrow BRDF and return the light 
	//          reflected from that instead
	///////////////////////////////////////////////////////////////////////////
	
	vec3 wh = normalize(wi + wo);
	float s = m.shininess;
	float f = m.fresnel + (1 - m.fresnel)*pow((1 - dot(wi,wh)),5);
	float d = (s+2)/(2*PI) * pow(dot(n,wh),s);
	float g = min(1, min(2*((dot(n,wh)*dot(n,wo))/dot(wo,wh)),2*((dot(n,wh)*dot(n,wi))/dot(wo,wh))));
	float brdf = (f*d*g)/(4*dot(n,wo)*dot(n,wi));
	//return brdf * dot(n, wi) * li; 
	///////////////////////////////////////////////////////////////////////////
	// Task 3 - Make your shader respect the parameters of our material model.
	///////////////////////////////////////////////////////////////////////////
	vec3 dielectric_term = brdf * dot(n, wi) * li + (1 - f) * diffuse_term;
	vec3 metal_term = brdf * m.color.xyz * dot(n, wi) * li;
	vec3 microfacet_term = m.metalness * metal_term + (1 - m.metalness) * dielectric_term;
	return m.reflectivity * microfacet_term + (1 - m.reflectivity) * diffuse_term;
	//return vec3(material_color);
}

void main() {
    //outColor = texture(texSampler, fragTexCoord);

	vec3 wo = normalize(vec3(0.0) - viewSpacePosition);
	vec3 n = normalize(viewSpaceNormal);
	
	vec3 direct_illumination_term = vec3(1.0);
	{ // Direct illumination
		direct_illumination_term = calculateDirectIllumiunation(wo, n);
	}
	
	vec3 emission_term = m.emission * m.color.xyz;

	outColor = vec4(direct_illumination_term+emission_term,1.0);

}

