#include "terrain.h"
#include <stdio.h>
#include <stdlib.h>

#include <common/logging.h>
#include <common/file.h>
#include <common/vfile.h>
#include <common/image.h>
#include <common/path.h>

#include "dds.h"
#include "util.h"

bool dds_to_hght(const char* dds_path, const char* hght_path) {
    assert(path_has_extension(hght_path, ".hght"));
    assert(path_has_extension(dds_path, ".dds"));

	const u32 minimum_size = sizeof(dds_header) + sizeof(hght_t);
    const u32 dds_size = file_size(dds_path);
	if (dds_size < minimum_size) {
		LOG_MSG(error, "Invalid DDS file (too small, should be at least %d bytes)\n", minimum_size);
		return false;
	}

    u8* data = malloc(dds_size);
    if (!data) {
        LOG_MSG(error, "Failed to allocate %d bytes to load DDS file\n", dds_size);
        return false;
    }
    texture tex = image_buf_load(dds_path, data, dds_size);

	FILE* hght = fopen(hght_path, "wb");
	if (!hght) {
        LOG_MSG(error, "Failed to open HGHT file '%s'\n", hght_path);
		return false;
	}

    bool result = true;
    if (tex.channels == 1 && tex.unit_size == 2) {
        // Format matches HGHT file, just copy the data
        fwrite(data, sizeof(hght_t), 1, hght);
    }
    else if (tex.compressed && tex.fmt == DDS_FORMAT_FLOAT) {
        // Exported as 32-bit, convert to 16-bit and copy. We allow this
        // because paint.net and GIMP can both import R16 but won't export it.
        // R32 is the only lossless way to export.
        vfile vf = vfile_open(tex.data, dds_size);
        for (u32 i = 0; i < HGHT_WIDTH; i++) {
            for (u32 j = 0; j < HGHT_WIDTH; j++) {
                const float value = VFILE_READ(float, &vf) * (float)UINT16_MAX;
                const u16 output = (u16)value;
                fwrite(&output, sizeof(output), 1, hght);
            }
        }
    } else {
        LOG_MSG(error, "Can't convert DDS - must be R16 or floating-point R32\n");
        result = false;
    }

    if (result) {
        LOG_MSG(info, "Saved HGHT to '%s'\n", hght_path);
    }
    free(data);
	fclose(hght);
    console_pause();
    return true;
}

bool hght_to_dds(const char* hght_path, const char* dds_path) {
    assert(path_has_extension(hght_path, ".hght"));
    assert(path_has_extension(dds_path, ".dds"));

	// Heightmaps are always 256x256 16-bit numbers
	u16 hght_data[HGHT_WIDTH*HGHT_WIDTH] = {0};
	const u32 hght_size = sizeof(hght_data);

	// Sanity check that our file is the right size before we do anything
	if (file_size(hght_path) != hght_size) {
		LOG_MSG(error, "Invalid HGHT file (wrong size)\n");
		return false;
	}

	FILE* hght = fopen(hght_path, "rb");
	if (!hght) {
		fclose(hght);
		return false;
	}

	// Read heightmap into memory
	fread(hght_data, hght_size, 1, hght);
	fclose(hght);

	// Write the image header to our output, then the heightmap data
	// (which will be interpreted as pixels).
    texture tex = {
        .channels = 1,
        .unit_size = 2,
        .height = HGHT_WIDTH,
        .width = HGHT_WIDTH,
        .data = (u8*)hght_data,
    };
    img_write(tex, dds_path);

    LOG_MSG(info, "Saved DDS to '%s'\n", dds_path);
    console_pause();

    return true;
}

bool water_to_dds(const char* inpath, const char* outpath) {
    assert(path_has_extension(inpath, ".water.extm"));
    assert(path_has_extension(outpath, ".dds"));

    const u32 insize = file_size(inpath);
    if (insize != sizeof(water_extm)) {
        LOG_MSG(error, ".water.extm files should be %d bytes, yours is %d", sizeof(water_extm), insize);
        return false;
    }

    u16 heightmap_size = sizeof(u16) * WATER_EXTM_WIDTH * WATER_EXTM_WIDTH;
    u16* heightmap = malloc(heightmap_size);
    vfile vf_out = vfile_open(heightmap, heightmap_size);

    FILE* f = fopen(inpath, "rb");
    if (!f) {
        return false;
    }

    for (u32 i = 0; i < WATER_EXTM_WIDTH; i++) {
        for (u32 j = 0; j < WATER_EXTM_WIDTH; j++) {
            water_vert vert = {0};
            fread(&vert, sizeof(vert), 1, f);
            VFILE_WRITE(u16, &vf_out, vert.height);
        }
    }
    fclose(f);

    texture tex = {
        .data = (u8*)heightmap,
        .channels = 1,
        .unit_size = 2,
        .height = WATER_EXTM_WIDTH,
        .width = WATER_EXTM_WIDTH,
    };
    img_write(tex, outpath);
    free(heightmap);
    LOG_MSG(info, "Saved DDS to '%s'\n", outpath);
    console_pause();

    return true;
}

bool dds_to_water(const char* inpath, const char* outpath) {
    assert(path_has_extension(inpath, ".dds"));
    assert(path_has_extension(outpath, ".water.extm"));
    assert(file_exists(outpath) && "For .dds -> .water.extm, the water file must already exist.");
    const u32 insize = file_size(inpath);
    const u32 outsize = file_size(outpath);
    if (outsize != sizeof(water_extm)) {
        LOG_MSG(error, "Your output water file is the wrong size (%d bytes, should be %d bytes)\n", outsize, sizeof(water_extm));
        return false;
    }

    FILE* outfile = fopen(outpath, "r+"); // This is a read/write handle
    if (!outfile) {
        LOG_MSG(error, "Failed to open '%s'\n", outfile);
        return false;
    }

    u8* data = malloc(insize);
    if (!data) {
        LOG_MSG(error, "Failed to allocate %d bytes to load DDS file\n", insize);
        fclose(outfile);
        return false;
    }
    texture tex = image_buf_load(inpath, data, insize);
    vfile vf_out = vfile_open(data, insize);

    for (u32 i = 0; i < WATER_EXTM_WIDTH; i++) {
        for (u32 j = 0; j < WATER_EXTM_WIDTH; j++) {
            water_vert vert = {0};
            fread(&vert, sizeof(vert), 1, outfile);

            // Get height value from texture
            if (tex.channels == 1 && tex.unit_size == 2) {
                // Format matches the file, just copy
                vert.height = VFILE_READ(u16, &vf_out);
            }
            else if (tex.compressed && tex.fmt == DDS_FORMAT_FLOAT) {
                vert.height = (u16)(VFILE_READ(float, &vf_out) * UINT16_MAX);
            } else {
                LOG_MSG(error, "Can't convert DDS - must be R16 or floating-point R32\n");
            }

            // Write back the modified vertex
            fseek(outfile, -1 * (s64)sizeof(vert), SEEK_CUR);
            fwrite(&vert, sizeof(vert), 1, outfile);
        }
    }

    fclose(outfile);
    free(data);
    LOG_MSG(info, "Overwrote '%s'\n", outpath);
    console_pause();
    return true;
}

bool dds_to_mate(const char* dds_path, const char* mate_path) {
    assert(path_has_extension(mate_path, ".mate"));
    assert(path_has_extension(dds_path, ".dds"));

	const u32 minimum_size = sizeof(dds_header) + sizeof(mate_t);
    const u32 dds_size = file_size(dds_path);
	if (dds_size < minimum_size) {
		LOG_MSG(error, "Invalid DDS file (too small, should be at least %d bytes)\n", minimum_size);
		return false;
	}

    u8* data = malloc(dds_size);
    if (!data) {
        LOG_MSG(error, "Failed to allocate %d bytes to load DDS file\n", dds_size);
        return false;
    }
    texture tex = image_buf_load(dds_path, data, dds_size);

	FILE* f = fopen(mate_path, "wb");
	if (!f) {
        LOG_MSG(error, "Failed to open MATE file '%s'\n", mate_path);
		return false;
	}

    assert(tex.channels == 4 && tex.unit_size == 1);
    // Format matches MATE file, just copy the data
    fwrite(data, sizeof(mate_t), 1, f);
    LOG_MSG(info, "Saved MATE to '%s'\n", mate_path);

    free(data);
	fclose(f);
    console_pause();
    return true;
}

bool mate_to_dds(const char* mate_path, const char* dds_path) {
    assert(path_has_extension(mate_path, ".mate"));
    assert(path_has_extension(dds_path, ".dds"));

	mate_t mate_data = {0};
	const u32 mate_size = sizeof(mate_data);

	// Sanity check that our file is the right size before we do anything
	if (file_size(mate_path) != mate_size) {
		LOG_MSG(error, "Invalid MATE file (wrong size)\n");
		return false;
	}

	FILE* f = fopen(mate_path, "rb");
	if (!f) {
		fclose(f);
		return false;
	}

	// Read heightmap into memory
	fread(&mate_data, mate_size, 1, f);
	fclose(f);

	// Write the image header to our output, then the heightmap data
	// (which will be interpreted as pixels).
    texture tex = {
        .channels = 4,
        .unit_size = 1,
        .height = HGHT_WIDTH,
        .width = HGHT_WIDTH,
        .data = (u8*)&mate_data,
    };
    img_write(tex, dds_path);

    LOG_MSG(info, "Saved DDS to '%s'\n", dds_path);
    console_pause();

    return true;
}


s32 pos_to_zorder_idx(u8 detail_lvl, float x, float y) {
    if (detail_lvl > 8) {
        LOG_MSG(error, "The highest detail level is 8, but you asked for %d\n", detail_lvl);
        return -1;
    }
    // Make the top left corner (-6000, -6000) become (0, 0).
    x += MAP_SIZE;
    y += MAP_SIZE;

    // How many HGHT files wide the map is
    const u16 grid_res = 1 << detail_lvl;
    // How wide each grid cell is in world coordinates
    const float grid_size = (2 * MAP_SIZE) / ((float)grid_res);

    LOG_MSG(debug, "HGHT size in world: %f\n", grid_size);

    // Grid coordinates of this location
    const u32 i = (u32)(x / grid_size);
    const u32 j = (u32)(y / grid_size);

    // Interleave bits to get Z-order curve index. Look at the Wikipedia page
    // about it if you want to know why this works
    const s32 result = interleave16(i, j);
    assert(result <= grid_res * grid_res && "Programmer error: Z order curve index out of bounds");

    return result;
}

