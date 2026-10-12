/*

    mplsartindustry/daisy
    Copyright (c) 2024-2026 held jointly by the individual authors.

    This file is part of mplsartindustry/daisy.

    mplsartindustry/daisy is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    mplsartindustry/daisy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with mplsartindustry/daisy.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "dev/sdram.h"
#include "sdram_heap.h"

//
// Heap for malloc/new, replacing _sbrk from libnosys.
//
// SDRAM is not usable until hardware Init() runs (e.g. DaisySeed, DaisyPatchSM), but static constructors
// may allocate before main(). Allocations are served from the remainder of
// internal AXI SRAM until EnableSdramHeap() is called, and from SDRAM after.
//

// end of .heap section in AXI SRAM, from linker script
extern "C" char end;

// end of AXI SRAM (0x24000000 + 512K)
static char * const SRAM_LIMIT = reinterpret_cast<char *>(0x24080000);

// SDRAM heap, leave some SDRAM for DSY_SDRAM_BSS buffers
static constexpr size_t SDRAM_HEAP_SIZE = 56 * 1024 * 1024;
static char DSY_SDRAM_BSS __attribute__((aligned(8))) sdram_heap[SDRAM_HEAP_SIZE];

static char * sram_break = &end;
static char * sdram_break = NULL;

void EnableSdramHeap() {
  sdram_break = sdram_heap;
}

size_t SdramHeapUsed() {
  return sdram_break == NULL ? 0 : static_cast<size_t>(sdram_break - sdram_heap);
}

extern "C" void * _sbrk(ptrdiff_t increment) {
  char ** brk = sdram_break == NULL ? &sram_break : &sdram_break;
  char * limit = sdram_break == NULL ? SRAM_LIMIT : sdram_heap + SDRAM_HEAP_SIZE;

  if (*brk + increment > limit) {
    errno = ENOMEM;
    return reinterpret_cast<void *>(-1);
  }
  char * previous = *brk;
  *brk += increment;
  return previous;
}
