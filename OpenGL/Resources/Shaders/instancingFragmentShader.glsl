#version 330 core


out vec4 FragColor;

in vec2 TexCoord;
in vec3 normal;
in vec4 fragPos;
in vec4 FragPosLightSpace;

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
    float bias = 0.005;
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(light.shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(light.shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}  

uniform Light Lights;

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
    result += CalcDirLight(Lights, norm, viewDir, FragPosLightSpace);

	FragColor = texture(texture1, TexCoord);
}