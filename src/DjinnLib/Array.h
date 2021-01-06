#ifndef DJINNLIB_ARRAY_INCLUDE_H
#define DJINNLIB_ARRAY_INCLUDE_H

#include <concepts>
#include <cstring>
#include <cassert>


namespace Djinn
{
	template <typename T, size_t size>
	class Array1D
	{
	public:

		constexpr Array1D() {}

		constexpr Array1D(std::initializer_list<T> const& list)
		{
			std::copy(list.begin(), list.end(), ptr);
		}


		// return num elements
		constexpr size_t NumElem() const { return size; }

		// return size of array in bytes
		constexpr size_t ByteSize() const { return sizeof(ptr); }

		void Memset(const char fill) { memset(ptr, fill, ByteSize()); }

		// array operators
		const T& operator[](const size_t index) const 
		{ 
			assert(index < size, "Index exceeds array bounds"); 
			return ptr[index];
		}

		T& operator[](const size_t index)
		{
			assert(index < size, "Index exceeds array bounds");
			return ptr[index];
		}

		// returns a pointer 
		T const* Ptr() const { return ptr; }
		T* Ptr() { return ptr; }


	private:
		T ptr[size];
	};

}

#endif // DJINNLIB_ARRAY_INCLUDE_H