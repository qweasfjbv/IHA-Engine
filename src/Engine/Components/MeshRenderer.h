#pragma once

#include "Core/ECS.h"
#include "Render/Mesh.h"
#include "Render/Material.h"

namespace IHA::Engine
{
	struct MeshRenderer : public Component {
		Mesh* mesh = nullptr;
		Material* material = nullptr;
	};

	class MeshRendererSystem : public SystemBase<MeshRenderer> {

	};
}