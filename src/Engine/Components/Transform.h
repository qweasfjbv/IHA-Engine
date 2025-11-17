#pragma once

#include <vector>
#include "Core/ECS.h"
#include "Common/MathHelper.h"


namespace IHA::Engine
{
	struct Transform : public Component
	{
		XMFLOAT4X4			m_worldMatrix = MakeIdentity();
		XMFLOAT3			m_position = { 0.0f, 0.0f, 0.0f };
		XMFLOAT4			m_rotation = { 0.0f, 0.0f, 0.0f, 0.0f };
		XMFLOAT3			m_scale = { 1.0f, 1.0f, 1.0f };

		Entity				m_parent = INVALID_ENTITY;
		std::vector<Entity> m_children;

		XMFLOAT4X4 GetWorldMatrix() 
		{
			XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
			XMMATRIX R = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
			XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

			XMStoreFloat4x4(&m_worldMatrix, S * R * T);
			return m_worldMatrix;
		}
	};

	class TrasnformSystem : public SystemBase<Transform>
	{

	public:
		~TrasnformSystem() = default;

		void Update(float deltaTime) override;
		void SetParent(Entity child, Entity parent);

	private:
		std::vector<int> m_roots;
	};
}