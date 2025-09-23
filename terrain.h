#pragma once
// Functions & structures for several BOTW terrain-related formats
// (HGHT, .grass.extm, .water.extm, etc.)
//
// Most information comes from ZeldaMods:
// https://zeldamods.org/wiki/Category:File_extensions_(STERA)
// https://zeldamods.org/wiki/Water.extm
// https://zeldamods.org/wiki/HGHT

#include <stdbool.h>
#include <assert.h>
#include <common/int.h>

enum {
    HGHT_WIDTH = 256,
    WATER_EXTM_WIDTH = 64,
    WATER_NO_FLOW = 128,
};

typedef struct {
    u16 height[HGHT_WIDTH][HGHT_WIDTH];
}hght_t;
static_assert(sizeof(hght_t) == 2 * 256 * 256, "HGHT struct size is wrong!");

#define MAP_SIZE (6000.0f)

bool hght_to_dds(const char* inpath, const char* outpath);
bool dds_to_hght(const char* inpath, const char* outpath);

s32 pos_to_zorder_idx(u8 detail_lvl, float x, float y);

// https://zeldamods.org/wiki/Water.extm
typedef struct {
    u16 height;
    u16 flow_rate_x;
    u16 flow_rate_z;
    u8 mat_idx3; // Documented on ZeldaMods as being "materialIndex + 3"
    u8 mat_idx;
}water_vert;
static_assert(sizeof(water_vert) == 8, "water.extm vertex size is wrong!");

typedef struct {
    water_vert vertices[WATER_EXTM_WIDTH][WATER_EXTM_WIDTH];
}water_extm;
static_assert(sizeof(water_extm) == 8 * 64 * 64, "water.extm size is wrong!");
