#include "terrain.h"
#include <stdio.h>
#include <stdlib.h>

#include <common/logging.h>
#include <common/file.h>
#include <common/vfile.h>
#include <common/image.h>

#include "dds.h"
#include "util.h"

bool dds_to_hght(const char* dds_path, const char* hght_path) {
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
    texture tex = image_buf_load(dds_path, data, sizeof(dds_size));

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
	// Heightmaps are always 256x256 16-bit numbers
	u16 hght_data[256*256] = {0};
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
        .height = 256,
        .width = 256,
        .data = (u8*)hght_data,
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

    // How many HGHT files wide the map is
    const u16 grid_res = 1 << detail_lvl;
    // How wide each grid cell is in world coordinates
    const float grid_size = MAP_SIZE / ((float)grid_res);

    LOG_MSG(debug, "HGHT size in world: %f\n", grid_size);

    // Grid coordinates of this location
    const u32 i = (u32)(x / grid_size);
    const u32 j = (u32)(y / grid_size);

    // Interleave bits to get Z-order curve index. Look at the Wikipedia page
    // about it if you want to know why this works
    return interleave16(i, j);
}

