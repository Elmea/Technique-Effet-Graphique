#include "Light.h"
#include "Debug.h"

using namespace lowRenderer;

Light::Light(const char* _name, lightType _type, int _id) 
{
	if (NextFreeTextureSlot == 0)
		NextFreeTextureSlot = GL_TEXTURE15;

	textureSlot = NextFreeTextureSlot;
	NextFreeTextureSlot--;

	name = _name;
	type = _type; setId(_id);

	glGenFramebuffers(1, &shadowParameters.depthMapFBO);

	glGenTextures(1, &shadowParameters.depthMap);
	glBindTexture(GL_TEXTURE_2D, shadowParameters.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		shadowParameters.SHADOW_WIDTH, shadowParameters.SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowParameters.depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowParameters.depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Light::SetLight(colorType type, myMaths::Float3 color)
{
	switch (type)
	{
	case lowRenderer::colorType::CT_AMBIANT:
		ambiantColor = color;
		break;

	case lowRenderer::colorType::CT_DIFFUSE:
		diffuseColor = color;
		break;

	case lowRenderer::colorType::CT_SPECULAR:
		specularColor = color;
		break;

	default:
		break;
	}
}

void Light::SetLight(myMaths::Float3 _AmbiantColor, myMaths::Float3 _DiffuseColor, myMaths::Float3 _SpecularColor)
{
	ambiantColor = _AmbiantColor;
	diffuseColor = _DiffuseColor;
	specularColor = _SpecularColor;
}

void Light::BindShadowMap(Resource::Shader* shader)
{
	if (!active)
		return; 

	glUseProgram(shader->GetShader());
	glActiveTexture(textureSlot);
	shader->setInt("Lights[" + std::to_string(id) + "].shadowMap", textureSlot - GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowParameters.depthMap);
}

void Light::Generate(Resource::Shader* shader, myMaths::Float3 viewPos)
{
	myMaths::Mat4 matrix =
	{
		position.x, position.y, position.z, diffuseColor.x,
		direction.x, direction.y, direction.z, diffuseColor.y,
		ambiantColor.x, ambiantColor.y, ambiantColor.z, diffuseColor.z,
		specularColor.x, specularColor.y, specularColor.z, 0
	};

	//  pos x   pos y   pos z   dif x
	//  dir x   dir y   dir z   dif y
	//  amd x   amd y   amd z   dif z
	//  spc x   spc y   spc z   active

	// active : 0 = inactive, 1 = directional, 2 = point, 3 = spot

	std::string i = std::to_string(id);

	if (active)
	{
		switch (type)
		{
		case lowRenderer::lightType::LT_DIRECTIONAL:
			matrix.mat[3][3] = 1;
			break;

		case lowRenderer::lightType::LT_POINT:
			shader->setFloat("Lights[" + i + "].constant", constant);
			shader->setFloat("Lights[" + i + "].quadratic", quadratic);
			shader->setFloat("Lights[" + i + "].alinear", linear);
			matrix.mat[3][3] = 2;
			break;

		case lowRenderer::lightType::LT_SPOT:
			shader->setFloat("Lights[" + i + "].constant", constant);
			shader->setFloat("Lights[" + i + "].quadratic", quadratic);
			shader->setFloat("Lights[" + i + "].alinear", linear);
			shader->setFloat("Lights[" + i + "].cutOff", cutOff);
			shader->setFloat("Lights[" + i + "].outerCutOff", outerCutOff);
			matrix.mat[3][3] = 3;
			break;

		default:
			break;
		}
	}
	else
	{
		matrix.mat[3][3] = 0;
	}

	shader->setFloat(("Lights[" + i + "].intensity"), intensity);
	shader->setMat4(("Lights[" + i + "].Matrix"), matrix, false);
}

void Light::setId(int _id)
{
	if (_id < 1)
		ASSERT(0, "Trying to bind an id too low, please choose one in range ( 1 ; 8 )");

	if (_id > 8)
		ASSERT(0, "Trying to bind an id oversized, please choose one in range ( 1 ; 8 )");

	id = _id - 1;
}

void Light::setAtenuation(float _constant, float _linear, float _quadratic)
{
	constant = _constant;
	linear = _linear;
	quadratic = _quadratic;
}

void Light::SetDir(myMaths::Float3 vector)
{
	direction = vector;
}

void Light::SetIntensity(float _intensity)
{
	intensity = _intensity;
}

void Light::SetCutOff(float _cutOff)
{
	cutOff = _cutOff;
}

void Light::SetOuterCutOff(float _outerCutOff)
{
	outerCutOff = _outerCutOff;
}