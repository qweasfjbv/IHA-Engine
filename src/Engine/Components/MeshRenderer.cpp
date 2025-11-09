#include "MeshRenderer.h"

namespace IHA::Engine {

	void MeshRendererSystem::Update(float deltaTime)
	{
		for (const auto& [_, meshRenderer] : m_data) {
			// TODO - shader, texture 등 한번에 바인드
			meshRenderer.material->Bind(m_commandList);
			meshRenderer.mesh->Bind(m_commandList);

			meshRenderer.mesh->Draw(m_commandList);
		}
	}
}