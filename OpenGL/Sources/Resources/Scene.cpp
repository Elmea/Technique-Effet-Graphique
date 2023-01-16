#include "Scene.h"
#include "Debug.h"
#include "App.h"

using namespace Resource;

void Scene::CreateShadowMaps(Resource::ResourceManager& resourcesManager, const float SCR_WIDTH, const float SCR_HEIGHT, lowRenderer::Light* light)
{
	if (!light->isActive())
		return;

	//to depth map
	glViewport(0, 0, light->shadowParameters.SHADOW_WIDTH, light->shadowParameters.SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, light->shadowParameters.depthMapFBO);

	//ConfigureShaderAndMatrices();
	myMaths::Mat4 lightProjection;
	
	switch (light->getType())
	{
	case lowRenderer::lightType::LT_DIRECTIONAL:
		lightProjection = lightProjection.ToOrtho(-100, 100, -100, 100, 0.1, 100);
		break;
	case lowRenderer::lightType::LT_POINT:
		lightProjection = myMaths::Mat4::getProjection(80, SCR_WIDTH / SCR_HEIGHT, 0.1, 25);
		break;
	case lowRenderer::lightType::LT_SPOT:
		lightProjection = myMaths::Mat4::getProjection(80, SCR_WIDTH / SCR_HEIGHT, 0.1, 25);
		break;

	default:
		break;
	}

	//lightProjection = lightProjection.ToOrtho(-100, 100, -100, 100, 0.1, 100);

	myMaths::Float3 rotation = myMaths::Float3::dirToEuler(light->getDirection());
	myMaths::Mat4 lightView = myMaths::Mat4::getTranslation(light->getPosition()) * myMaths::Mat4::getRotationY(rotation.y) * myMaths::Mat4::getRotationX(rotation.x) * myMaths::Mat4::getRotationZ(rotation.z) * myMaths::Mat4::getScale({ 1,1,1 });

	light->lightSpaceMatrix = (lightProjection * lightView.getInverseMatrix());

	Resource::Shader* shadowMap = (Resource::Shader*)resourcesManager.Get<Resource::Shader>("ShadowMap");

	glClear(GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < GetObjectCount(); i++)
		objects[i]->DrawDiffShader(light->lightSpaceMatrix, shadowMap);

	//set back parameters to render the scene
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void Scene::Render(Resource::ResourceManager& resourcesManager, lowRenderer::Camera* camera, float SCR_WIDTH, float SCR_HEIGHT)
{

	int objectsCount = objects.size();
	int lightsCount = lights.size();

	myMaths::Mat4 VPMat = camera->getVP();

	for (int lightId = 0; lightId < lightsCount; lightId++)
		CreateShadowMaps(resourcesManager, SCR_WIDTH, SCR_HEIGHT, lights[lightId]);

	for (int i = 0; i < objectsCount; i++)
	{
		if (objects[i]->GotMesh())
		{
			for (int lightId = 0; lightId < lightsCount; lightId++)
			{
				if (lights[lightId]->followCamera)
				{
					lights[lightId]->SetPos(camera->position);
					lights[lightId]->SetDir(myMaths::Float3(-sinf(camera->rotation.y * DEG2RAD) * cosf(camera->rotation.x * DEG2RAD), sinf(camera->rotation.x * DEG2RAD), -cosf(camera->rotation.y * DEG2RAD) * cosf(camera->rotation.x * DEG2RAD)));
				}
				lights[lightId]->BindShadowMap(objects[i]->getShader());
				lights[lightId]->Generate(objects[i]->getShader(), camera->position);
			}

			objects[i]->Draw(VPMat);
		}
	}
	
	ImGui();
}

void Scene::AddObject(lowRenderer::GameObject* obj)
{
	objects.push_back(obj);
}

void Scene::AddLight(lowRenderer::Light* obj)
{
	lights.push_back(obj);
}

void Scene::Clear()
{
	for (int i = 0; i < meshes.size(); i++)
		delete meshes[i];

	for (int i = 0; i < lights.size(); i++)
		delete lights[i];

	for (int i = 0; i < objects.size(); i++)
		delete objects[i];

	meshes.clear();
	lights.clear();
	objects.clear();
	isLoaded = false;
}

Scene::~Scene()
{
	Clear();
}

void Scene::TurnOffAllLights()
{
	for(int i = 0; i < lights.size(); i++)
	{
		lights[i]->setActive(false);
	}
}

void Scene::Reload(Resource::ResourceManager& resourcesManager)
{
	if (isLoaded)
	{
		Clear();
		resourcesManager.Reset();
		Core::Debug::log.Print("Reloading scene...");
		Init(resourcesManager);
	}
}

bool Scene::CheckLoadedModel()
{
	int LoadedObjects = 0;

	for (lowRenderer::Mesh* mesh : meshes)
	{
		if (mesh->Model()->IsLoaded() && !mesh->Model()->IsInBuffer())
		{
			mesh->Model()->fillBuffers();
		}
		if (mesh->Texture()->IsLoad() && !mesh->Texture()->IsBuffer())
		{
			mesh->Texture()->ImgBuffer();
		}

		if (mesh->Model()->IsLoaded() && mesh->Model()->IsInBuffer() && mesh->Texture()->IsLoad() && mesh->Texture()->IsBuffer())
			LoadedObjects++;
	}

	if (LoadedObjects == meshes.size())
	{
		if (!isLoaded)
		{
			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsedSeconds = end - reloadStart;
			Core::Debug::log.Print("Scene Loaded in : %fs", elapsedSeconds.count());
		}
		isLoaded = true;
		return true;
	}

	return false;
}

void Scene::ImGui()
{
	int objectsCount = objects.size();
	int lightsCount = lights.size();

	if (ImGui::Begin("Object"))
	{
		for (int i = 0; i < objectsCount; i++)
		{
			if (objects[i]->IsLoaded() || !objects[i]->GotMesh())
			{
				std::string name = "Info " + objects[i]->name;
				if (ImGui::CollapsingHeader(name.c_str()))
				{
					name = "Position " + objects[i]->name;
					ImGui::SliderFloat3(name.c_str(), &objects[i]->position.x, -10.0f, 10.0f);
					name = "Rotation " + objects[i]->name;
					ImGui::SliderFloat3(name.c_str(), &objects[i]->rotation.x, 0.f, 360.f);
					name = "Scale " + objects[i]->name;
					ImGui::SliderFloat3(name.c_str(), &objects[i]->scale.x, 0.f, 2.f);

					if (ImGui::Button("Reset"))
					{
						objects[i]->position.x = 0.f;
						objects[i]->position.y = 0.f;
						objects[i]->position.z = 0.f;

						objects[i]->rotation.x = 0.f;
						objects[i]->rotation.y = 0.f;
						objects[i]->rotation.z = 0.f;

						objects[i]->scale.x = 1.f;
						objects[i]->scale.y = 1.f;
						objects[i]->scale.z = 1.f;
					}
				}
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("Lights"))
	{
		for (int i = 0; i < lightsCount; i++)
		{
			std::string name = "Info " + lights[i]->name;
			if (ImGui::CollapsingHeader(name.c_str()))
			{
				name = "Ambiant " + lights[i]->name;
				ImGui::ColorEdit3(name.c_str(), &lights[i]->getAmbiantPtr()->x);

				name = "Diffuse " + lights[i]->name;
				ImGui::ColorEdit3(name.c_str(), &lights[i]->getDiffusePtr()->x);

				name = "Specular " + lights[i]->name;
				ImGui::ColorEdit3(name.c_str(), &lights[i]->getSpecularPtr()->x);

				name = "Intensity " + lights[i]->name;
				ImGui::SliderFloat(name.c_str(), lights[i]->getIntensityPtr(), 0.f, 20.f);

				switch (lights[i]->getType())
				{
				case lowRenderer::lightType::LT_DIRECTIONAL:
					name = "Direction " + lights[i]->name;
					ImGui::SliderFloat3(name.c_str(), &lights[i]->getDirectionPtr()->x, -10.0f, 10.0f);
					break;
				case lowRenderer::lightType::LT_POINT:
					name = "Position " + lights[i]->name;
					ImGui::SliderFloat3(name.c_str(), &lights[i]->getPositionPtr()->x, -10.0f, 10.0f);
					break;
				case lowRenderer::lightType::LT_SPOT:
					name = "Direction " + lights[i]->name;
					ImGui::SliderFloat3(name.c_str(), &lights[i]->getDirectionPtr()->x, -10.0f, 10.0f);
					name = "Position " + lights[i]->name;
					ImGui::SliderFloat3(name.c_str(), &lights[i]->getPositionPtr()->x, -10.0f, 10.0f);
					name = "Cut Off " + lights[i]->name;
					ImGui::SliderFloat(name.c_str(), lights[i]->getCutOffPtr(), 0.0f, 1.0f);
					name = "Outter Cut Off " + lights[i]->name;
					ImGui::SliderFloat(name.c_str(), lights[i]->getOuterCutOffPtr(), 0.0f, 1.0f);
					break;
				default:
					break;
				}

				if (lights[i]->isActive())
					name = "turn off " + lights[i]->name;
				else
					name = "turn on " + lights[i]->name;

				if (ImGui::Button(name.c_str()))
					lights[i]->setActive(!lights[i]->isActive());

				if (!lights[i]->followCamera)
					name = "follow Camera";
				else
					name = "Stop following";

				if (ImGui::Button(name.c_str()))
					lights[i]->followCamera = !lights[i]->followCamera;
			}
		}
	}
	ImGui::End();
}