#include "Scene3.h"

using namespace Resource;

void Scene3::Init(Resource::ResourceManager& resourcesManager)
{
	Clear();
	reloadStart = std::chrono::high_resolution_clock::now();

	name = "Instancing";

	// Create shader
	Resource::Shader* shader = (Resource::Shader*)resourcesManager.Get<Resource::Texture>("InstancingShader");
	if (shader == nullptr)
	{
		shader = (Resource::Shader*)resourcesManager.Create<Resource::Shader>("InstancingShader", "InstancingShader");
		shader->createShader("Resources/Shaders/instancingVertexShader.glsl", "Resources/Shaders/instancingFragmentShader.glsl");
	}

	// Create Textures
	Resource::Texture* wallTexture = (Resource::Texture*)resourcesManager.Get<Resource::Texture>("wallTexture");
	if (wallTexture == nullptr)
		wallTexture = (Resource::Texture*)resourcesManager.Create<Resource::Texture>("Resources/Textures/wall.jpg", "wallTexture");

	// Create models
	Resource::Model* cubeModel = (Resource::Model*)resourcesManager.Get<Resource::Texture>("cubeModel");
	if (cubeModel == nullptr)
		cubeModel = (Resource::Model*)resourcesManager.Create<Resource::Model>("Resources/Obj/cube.obj", "cubeModel");

	// Create and add light
	lowRenderer::Light* dirlight = new lowRenderer::Light("Directional", lowRenderer::lightType::LT_DIRECTIONAL, 1);
	dirlight->SetDir(myMaths::Float3(0.f, -1.f, 0.f));
	dirlight->SetLight(myMaths::Float3(0.5f, 0.5f, 0.5f), myMaths::Float3(1.f, 1.f, 1.f), myMaths::Float3(0.25f, 0.25f, 0.25f));
	dirlight->SetIntensity(0.5f);
	dirlight->setActive(true);
	AddLight(dirlight);



	// Create Meshes
	// ------------------------------------------------
	lowRenderer::Mesh* cubeMesh = new lowRenderer::Mesh(cubeModel, wallTexture, shader);
	meshes.push_back(cubeMesh);

	// Create and add object
	// ------------------------------------------------
	lowRenderer::GameObject* cube = new lowRenderer::GameObject(cubeMesh);
	cube->name = "Cube";
	cube->position = myMaths::Float3(0.f, 0.f, 0.f);
	cube->scale = myMaths::Float3(1.f, 1.f, 1.f);
	AddObject(cube);


	lowRenderer::GameObject* everything = new lowRenderer::GameObject(nullptr);
	everything->position = myMaths::Float3(0.f, -2.f, 0.f);
	everything->AddChild(*cube);
	everything->name = "Everything";
	AddObject(everything);
}