#pragma once
#include "Scene.h"

namespace Resource
{
	class Scene3 : public Scene
	{
	public:
		void Init(Resource::ResourceManager& resourcesManager) override;
	};
}