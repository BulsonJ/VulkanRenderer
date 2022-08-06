#pragma once

#include <array>


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
