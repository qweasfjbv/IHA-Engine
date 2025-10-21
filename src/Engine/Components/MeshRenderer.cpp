#include "MeshRenderer.h"

namespace IHA::Engine {

	void MeshRendererSystem::Update(float deltaTime)
	{
		for (const auto& [_, meshRenderer] : m_data) {


			// TODO - shader, texture �� �ѹ��� ���ε�
			meshRenderer.material->Bind(m_commandList);
			meshRenderer.mesh->Bind(m_commandList);

			meshRenderer.mesh->Draw(m_commandList);
		}
	}

}