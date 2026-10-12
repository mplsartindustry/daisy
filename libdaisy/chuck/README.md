# chuck

[ChucK](https://chuck.stanford.edu) core, shared by the libDaisy projects
[`../patch_init_chuck`](../patch_init_chuck) and [`../seed_chuck`](../seed_chuck).

These sources are copied from [`../../arduino/patch_init_chuck`](../../arduino/patch_init_chuck);
see the README there for what was changed from upstream `chuck/src/core`. Projects compile them
with `-D__CHUCK_EMBEDDED__` by setting `CHUCK_DIR` in their Makefile.

`sdram_heap.cpp`/`.h` are not part of ChucK. They replace `_sbrk` so that the heap
moves to SDRAM once `EnableSdramHeap()` is called after hardware `Init()`, since
`ChucK::init()` allocates several MB, more than internal SRAM.
