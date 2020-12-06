#version 450

const float PI = 3.14159265359;

layout (binding = 1) uniform samplerCube environmentMap;

layout (location = 0) in vec3 localPos;
layout (location = 0) out vec4 outFragColor;

void main()
{		
    // the sample direction equals the hemisphere's orientation 
    vec3 normal = normalize(localPos);
    normal.y *= -1;
    vec3 irradiance = vec3(0.0);  

    vec3 up     = vec3(0.0, 1.0, 0.0);
    vec3 right  = normalize(cross(up, normal));
    up = cross(normal, right);

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    //float nrSamples = (2.0*PI / sampleDelta) * (0.5*PI/sampleDelta);
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    //const float gamma = 2.2;
  
    // reinhard tone mapping
    //vec3 mapped = irradiance / (irradiance + vec3(1.0));
    // gamma correction 
    //mapped = pow(mapped, vec3(1.0 / gamma));
    //outFragColor = vec4(mapped, 1.0);
    outFragColor = vec4(irradiance, 1.0);
}