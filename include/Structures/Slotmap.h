#pragma once

#include <array>

/*
Might be best to not use templated handles in the future.

Leads to verbosity and although I like it more when known types are used, means that other classes
need to know struct stored in slotmap to access it.

For example, Engine needs to know Handle<ImageHandle> for a texture.
1. Very verbose, could be slimmed down to ShaderTextureHandle on Engine side(or something similar)
	- Maybe split into TextureHandle and ShaderTextureHandle
	- Find way for both to be the same?
2. Engine has to know what an Image is to use the Handle(means Image struct has to be exposed which exposes Vulkan)
*/
template<typename T>
class Slotmap
{
public:
	uint32_t add(const T& object);
	T& get(uint32_t handle);

	// TODO: make private
	std::array<T, 1024> array;
private:
	bool first{ true };
	uint32_t lastHandle{ 0U };
	[[nodiscard]] uint32_t getNewHandle()
	{
		return ++lastHandle;
	}
};

template<typename T>
inline uint32_t Slotmap<T>::add(const T& object)
{
	uint32_t handle = getNewHandle();
	array[handle] = object;
	return handle;
}

template<typename T>
inline T& Slotmap<T>::get(uint32_t handle)
{
	return array[handle];
}
