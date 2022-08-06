#pragma once

#include <array>

/*
Might be best to not use templated handles in the future.

Leads to verbosity and although I like it more when known types are used, means that other classes
need to know struct stored in slotmap to access it.

For example, Engine needs to know Handle<Handle<Image>> for a texture.
1. Very verbose, could be slimmed down to ShaderTextureHandle on Engine side(or something similar)
	- Maybe split into TextureHandle and ShaderTextureHandle
	- Find way for both to be the same?
2. Engine has to know what an Image is to use the Handle(means Image struct has to be exposed which exposes Vulkan)
*/
template <typename T>
struct Handle
{
	uint32_t slot;
};

template<typename T>
class Slotmap
{
public:
	Handle<T> add(const T& object);
	T& get(Handle<T> handle);

	// TODO: make private
	std::array<T, 1024> array;
private:
	bool first{ true };
	uint32_t lastHandle{ 0U };
	[[nodiscard]] Handle<T> getNewHandle()
	{
		if (first) { 
			first = false;
			return Handle<T>{lastHandle};
		}
		return Handle<T>{++lastHandle};
	}
};

template<typename T>
inline Handle<T> Slotmap<T>::add(const T& object)
{
	Handle<T> handle = getNewHandle();
	array[handle.slot] = object;
	return handle;
}

template<typename T>
inline T& Slotmap<T>::get(Handle<T> handle)
{
	return array[handle.slot];
}
