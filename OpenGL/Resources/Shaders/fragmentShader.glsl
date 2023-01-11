#version 330 core

#define MAX_LIGHTS 8

out vec4 FragColor;

in vec2 TexCoord;
in vec3 normal;
in vec4 fragPos;
in vec4 FragPosLightSpace[MAX_LIGHTS];

uniform sampler2D texture1;

uniform vec3 lightPos;
uniform vec3 viewPos;

struct Light
{
    float constant;
    float alinear;
    float quadratic;

    float intensity;
    float cutOff;
    float outerCutOff;

    sampler2D shadowMap;

    mat4 Matrix;

    //  pos x   pos y   pos z   dif x
    //  dir x   dir y   dir z   dif y
    //  amd x   amd y   amd z   dif z
    //  spc x   spc y   spc z   active

    // active : 0 inactive, 1 directional, 2 point, 3 spot
};

float ShadowCalculation(vec4 fragPosLightSpace, Light light)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(light.shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}  

uniform Light Lights[MAX_LIGHTS];

vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, vec4 fragPosLightSpace)
{
    vec3 lightDir = normalize(-vec3(light.Matrix[0][1], light.Matrix[1][1], light.Matrix[2][1]));
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // combine results
    vec3 ambient = vec3(light.Matrix[0][2], light.Matrix[1][2], light.Matrix[2][2]);
    vec3 diffuse = vec3(light.Matrix[3][0], light.Matrix[3][1], light.Matrix[3][2]) * diff;
    vec3 specular = vec3(light.Matrix[0][3], light.Matrix[1][3], light.Matrix[2][3]) * spec;

    float shadow = ShadowCalculation(fragPosLightSpace, light); 

    return (ambient + (1 - shadow) * (diffuse + specular)) * light.intensity;
}

vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 fragPosLightSpace)
{
    vec3 lightDir = normalize(vec3(light.Matrix[0][0], light.Matrix[1][0], light.Matrix[2][0]) - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // attenuation
    float distance = length(vec3(light.Matrix[0][0], light.Matrix[1][0], light.Matrix[2][0]) - fragPos);
    float attenuation = 1.0 / (light.constant + light.alinear * distance +
        light.quadratic * (distance * distance));

    // combine results
    vec3 ambient = vec3(light.Matrix[0][2], light.Matrix[1][2], light.Matrix[2][2]);
    vec3 diffuse = vec3(light.Matrix[3][0], light.Matrix[3][1], light.Matrix[3][2]) * diff;
    vec3 specular = vec3(light.Matrix[0][3], light.Matrix[1][3], light.Matrix[2][3]) * spec;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = ShadowCalculation(fragPosLightSpace, light); 

    return (ambient + (1 - shadow) * (diffuse + specular)) * light.intensity;
}

vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 fragPosLightSpace)
{
    vec3 lightDir = normalize(vec3(light.Matrix[0][0], light.Matrix[1][0], light.Matrix[2][0]) - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // attenuation
    float distance = length(vec3(light.Matrix[0][0], light.Matrix[1][0], light.Matrix[2][0]) - fragPos);
    float attenuation = 1.0 / (light.constant + light.alinear * distance + light.quadratic * (distance * distance));

    // spotlight intensity
    float theta = dot(lightDir, normalize(-vec3(light.Matrix[0][1], light.Matrix[1][1], light.Matrix[2][1])));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient = vec3(light.Matrix[0][2], light.Matrix[1][2], light.Matrix[2][2]);
    vec3 diffuse = vec3(light.Matrix[3][0], light.Matrix[3][1], light.Matrix[3][2]) * diff;
    vec3 specular = vec3(light.Matrix[0][3], light.Matrix[1][3], light.Matrix[2][3]) * spec;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    float shadow = ShadowCalculation(fragPosLightSpace, light); 

    return (ambient + (1 - shadow) * (diffuse + specular)) * light.intensity;
}

void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - vec3(fragPos));
    vec3 result = vec3(0.f, 0.f, 0.f);
 
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (Lights[i].Matrix[3][3] == 0)
            continue;

        if (Lights[i].Matrix[3][3] == 1)
            result += CalcDirLight(Lights[i], norm, viewDir, FragPosLightSpace[i]);

        if (Lights[i].Matrix[3][3] == 2)
            result += CalcPointLight(Lights[i], norm, vec3(fragPos), viewDir, FragPosLightSpace[i]);

        if (Lights[i].Matrix[3][3] == 3)
            result += CalcSpotLight(Lights[i], norm, vec3(fragPos), viewDir, FragPosLightSpace[i]);
    }


	FragColor = texture(texture1, TexCoord) * vec4(result, 1.0);
}