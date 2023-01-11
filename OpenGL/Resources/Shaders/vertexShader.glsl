#version 330 core

#define MAX_LIGHTS 8

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 normal;
out vec4 fragPos;
out vec4 FragPosLightSpace[MAX_LIGHTS];

uniform mat4 model;
uniform mat4 VP;
uniform mat4 lightSpaceMatrix[MAX_LIGHTS];

void main()
{
	gl_Position = VP * model * vec4(aPos, 1.0);
	fragPos = model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	normal = vec3(mat3(model) * aNormal);

	for (int i = 0; i < MAX_LIGHTS; i++)
    {
		FragPosLightSpace[i] = lightSpaceMatrix[i] * fragPos;
	}
}