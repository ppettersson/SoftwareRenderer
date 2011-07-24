#include "SoftwareRenderer.h"
#include <map>

typedef std::map<void *, void *>  MemoryMap;

static MemoryMap allocations;

void *AllocAlign32(dword size)
{
  void *pad = malloc(size + 31);
  byte *data = (byte *)((((dword)pad) + 31) & ~31);

  allocations[data] = pad;
  return data;
}

void FreeAlign32(void *data)
{
  MemoryMap::iterator itr = allocations.find(data);
  if (itr != allocations.end())
  {
    void *pad = itr->second;
    free(pad);

    allocations.erase(itr);
  }
};