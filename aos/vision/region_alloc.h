#ifndef _AOS_VISION_REGION_ALLOC_H_
#define _AOS_VISION_REGION_ALLOC_H_

#include <vector>
#include <stdint.h>
#include <new>
#include <utility>
#include <memory>
#include <stdlib.h>
#include <stdio.h>

class AnalysisAllocator {
 private:
  std::vector<std::unique_ptr<uint8_t[]>> memory;
  size_t next_free = 0;
  size_t block_size = 1024 * 4;
  size_t used_size = 0;
 public:
  template<typename T, typename... Args>
  T* cons_obj(Args&&... args) {
    uint8_t* ptr = NULL;
    if (sizeof(T) + alignof(T) > block_size) {
		  fprintf(stderr, "allocating %d too much\n", (int)sizeof(T));
		  exit(-1);
	  }
    while (ptr == NULL) {
      if (next_free >= memory.size()) {
        if (next_free >= 1024) {
			    fprintf(stderr, "too much alloc\n");
			    exit(-1);
		    }
        memory.emplace_back(new uint8_t[block_size]);
      } else if ((used_size % alignof(T)) != 0) {
        used_size += alignof(T) - (used_size % alignof(T));
      } else if ((used_size + sizeof(T)) <= block_size) {
        ptr = &memory[next_free][used_size];
        used_size += sizeof(T);
      } else {
        used_size = 0;
        next_free++;
      }
    }
    return new (ptr)T(std::forward<Args>(args)...);
  }
  void reset() {
    //printf("memory_used %d %d\n", next_free, (int)memory.size());
    next_free = 0;
    used_size = 0;
  }
};


#endif  // _AOS_VISION_IMAGE_REGION_ALLOC_H_
