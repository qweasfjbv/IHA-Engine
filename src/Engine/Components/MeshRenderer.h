#pragma once

#include "Core/ECS.h"
#include "Common/UploadBuffer.h"
#include "Render/Mesh.h"
#include "Render/Material.h"


namespace IHA::Engine
{
	struct alignas(256) CBPerObject
	{
		DirectX::XMFLOAT4X4 gWorld;
		DirectX::XMFLOAT4X4 gView;
		DirectX::XMFLOAT4X4 gProj;
	};

	struct MeshRenderer : public Component {
		Mesh* m_mesh = nullptr;
		Material* m_material = nullptr;
		std::unique_ptr<UploadBuffer<CBPerObject>> m_cbPerObject;
	};

	class MeshRendererSystem : public SystemBase<MeshRenderer> {
		void Update(float deltaTime) override;
		
	private:
		ID3D12GraphicsCommandList* m_commandList = nullptr;
	};
}