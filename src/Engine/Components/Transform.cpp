#include "Transform.h"
#include "algorithm"

namespace IHA::Engine
{
	void TrasnformSystem::Update(float deltaTime)
	{
	}
	void TrasnformSystem::SetParent(Entity child, Entity parent)
	{
		if (!IsValid(child)) {
			LOG_ERROR("Child Entity is Invalid");
			return;
		}
		
		Transform& childTransform = Get(child);
		if (childTransform.m_parent == parent) return;

		// Erase child from parent
		if (!IsValid(childTransform.m_parent)) {
			auto it = std::find(m_roots.begin(), m_roots.end(), child);
			if (it != m_roots.end()) m_roots.erase(it);
		}
		else {
			Transform& panentTransform = Get(childTransform.m_parent);
			auto it = std::find(panentTransform.m_children.begin(), panentTransform.m_children.end(), child);
			if (it != panentTransform.m_children.end()) panentTransform.m_children.erase(it);
			childTransform.m_parent = INVALID_ENTITY;
		}
		
		// Set parent/child
		if (!IsValid(parent)) {
			m_roots.push_back(child);	
		}
		else {
			Transform& parentTransform = Get(parent);
			parentTransform.m_children.push_back(child);
			childTransform.m_parent = parent;
		}
	}
}