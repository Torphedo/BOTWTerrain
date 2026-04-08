#pragma once
// Functions & structures for several BOTW terrain-related formats
// (HGHT, .grass.extm, .water.extm, etc.)
//
// Most information comes from ZeldaMods:
// https://zeldamods.org/wiki/Category:File_extensions_(STERA)
// https://zeldamods.org/wiki/Water.extm
// https://zeldamods.org/wiki/HGHT
// https://zeldamods.org/wiki/MATE

#include <stdbool.h>
#include <assert.h>
#include <common/int.h>

enum {
    HGHT_WIDTH = 256,
    MATE_WIDTH = HGHT_WIDTH,
    WATER_EXTM_WIDTH = 64,
    WATER_NO_FLOW = 128,
};

// Used to convert world coords to Z order curve index for HGHT
#define MAP_SIZE (6000.0f)

typedef struct {
    u16 height[HGHT_WIDTH][HGHT_WIDTH];
}hght_t;
static_assert(sizeof(hght_t) == 2 * HGHT_WIDTH * HGHT_WIDTH, "HGHT struct size is wrong!");

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

// https://zeldamods.org/wiki/MATE
typedef struct {
    u8 material0;
    u8 material1;
    u8 blend_weight;
    u8 unknown;
}mate_vert;
static_assert(sizeof(mate_vert) == 4, "Material vertex size is wrong!");

typedef struct {
    mate_vert vertices[MATE_WIDTH][MATE_WIDTH];
}mate_t;
static_assert(sizeof(mate_t) == 256 * 256 * 4, "Material map size is wrong!");

bool hght_to_dds(const char* inpath, const char* outpath);
bool dds_to_hght(const char* inpath, const char* outpath);

bool mate_to_dds(const char* mate_path, const char* dds_path);
bool dds_to_mate(const char* dds_path, const char* mate_path);

bool water_to_dds(const char* inpath, const char* outpath);
bool dds_to_water(const char* inpath, const char* outpath);

s32 pos_to_zorder_idx(u8 detail_lvl, float x, float y);

