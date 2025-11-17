#include "MeshRenderer.h"

namespace IHA::Engine {

	void MeshRendererSystem::Update(float deltaTime)
	{
		for (const auto& [_, meshRenderer] : m_data) {
			// TODO - shader, texture 등 한번에 바인드

			// TODO - 행렬계산 필요
			CBPerObject obj = {};
			obj.gWorld = meshRenderer.m_transform->GetWorldMatrix();
			// obj.gView = 
			// obj.gProj = 
			meshRenderer.m_cbPerObject->CopyData(0, obj);

			meshRenderer.m_material->Bind(m_commandList);
			meshRenderer.m_mesh->Bind(m_commandList);

			meshRenderer.m_mesh->Draw(m_commandList);
		}
	}
}