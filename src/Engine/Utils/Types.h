#pragma once

namespace IHA::Engine {

	using Entity = uint32_t;
	inline constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1);

	bool IsValid(Entity entity) { return entity != INVALID_ENTITY; }
}