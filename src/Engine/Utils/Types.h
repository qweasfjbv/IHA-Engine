#pragma once

namespace IHA::Engine {

	using Entity = uint32_t;
	inline constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1);


	typename<typename T>
	constexpr bool IsValid(T value) {
		return value != static_cast<T>(-1);
	}
}