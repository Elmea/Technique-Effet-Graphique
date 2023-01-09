#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 normal;
out vec4 fragPos;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 VP;
uniform mat4 lightSpaceMatrix;
uniform vec3 offsets[1000];

void main()
{
	vec3 offset = offsets[gl_InstanceID];

	gl_Position = VP * model * vec4(aPos+offset, 1.0);
	fragPos = model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	normal = vec3(mat3(model) * aNormal);
	FragPosLightSpace = lightSpaceMatrix * fragPos;
}