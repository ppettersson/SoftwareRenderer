#include "SoftwareRenderer.h"
#include <map>

typedef std::map<void *, void *>  MemoryMap;

static MemoryMap allocations;

void *AllocAlign(dword size, dword align)
{
	// Allocate the unaligned memory block, and add enough space to allow aligning.
	void *pad = malloc(size + align);
	if (!pad)
		return NULL;

	// Calculate the aligned address.
	byte *data = (byte *)((((dword)pad) + align) & ~align);

	// Store the allocated pointer to allow proper free later.
	allocations[data] = pad;
	return data;
}

void FreeAlign(void *data)
{
	// Allow NULL pointers to be passed in.
	if (data)
	{
		// Look up the real allocation.
		MemoryMap::iterator itr = allocations.find(data);
		if (itr != allocations.end())
		{
			void *pad = itr->second;

			// Free the whole data buffer.
			free(pad);

			// Remove the allocation from the memory map.
			allocations.erase(itr);
		}
	}
};
