#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <common/logging.h>
#include <common/path.h>

#include "util.h"
#include "terrain.h"

typedef enum {
	HGHT,
	DDS,
    WATER_EXTM,
	INVALID,
}file_type;

const char* input_extensions[] = {
    ".hght", ".dds", ".water.extm", "",
};

void usage() {
    LOG_MSG(info, "Usage: hght [path to HGHT/DDS/water.extm file]\n");
    LOG_MSG(info, "       hght file.hght -> file.hght.dds\n");
    LOG_MSG(info, "       hght file.hght.dds -> file.hght\n");
    LOG_MSG(info, "       hght file.water.extm -> file.water.extm.dds\n");
    LOG_MSG(info, "       hght file.water.extm.dds -> file.water.extm\n");
    LOG_MSG(info, "   OR: hght [detail level] [decimal X coordinate] [decimal Y coordinate]\n");
    LOG_MSG(info, "If you're seeing this message but can't figure out how to make the console\n");
    LOG_MSG(info, "stay open, you need to open a full console window. In File Explorer (in the\n");
    LOG_MSG(info, "folder with your files), press Ctrl + L, then type 'cmd' and hit Enter.\n");
    console_pause();
}

int main(int argc, char** argv) {
	enable_win_ansi(); // Enable printing in color on Windows.
	const char* input_path = argv[1];

	if (argc == 1) {
		LOG_MSG(error, "Not enough arguments.\n");
        usage();
        return EXIT_FAILURE;
	}

	file_type in_type = INVALID;
    file_type out_type = INVALID;
    for (u32 i = 0; i < ARRAY_SIZE(input_extensions); i++) {
        if (path_has_extension(input_path, input_extensions[i])) {
            in_type = i;
            break;
        }
    }
    if (in_type == DDS) {
        const char* old_ext = strchr(input_path, '.');
        for (u32 i = 0; i < ARRAY_SIZE(input_extensions); i++) {
            if (strncmp(old_ext, input_extensions[i], strlen(input_extensions[i])) == 0) {
                out_type = i;
                break;
            }
        }
    }

    if (in_type == INVALID && argc == 4) {
        u8 detail_lvl = 0;
        float x = 0.0f;
        float y = 0.0f;
        sscanf(argv[1], "%hhu", &detail_lvl);
        sscanf(argv[2], "%f", &x);
        sscanf(argv[3], "%f", &y);
        const s32 idx = pos_to_zorder_idx(detail_lvl, x, y);
        if (idx > 0) {
            LOG_MSG(info, "X/Y coordinates: %f / %f\n", x, y);
            LOG_MSG(info, "Detail level: %d\n", detail_lvl);
            LOG_MSG(info, "Z-order curve index: %d\n", idx);
            LOG_MSG(info, "HGHT filename: 5%d%08X.hght\n", detail_lvl, idx);
        }
        console_pause();
        return EXIT_SUCCESS;
    }

	// Get paths fom user input. If no dds output is given, use the HGHT path
	// and append ".dds". ("test.hght" outputs "test.hght.dds")
	char* out_path = NULL;
	if (argc >= 3) {
		out_path = argv[2];
	} else {
		// Allocate for output path
        const u32 out_path_size = strlen(input_path) + sizeof(".hght");
		out_path = malloc(out_path_size);
		if (!out_path) {
			LOG_MSG(error, "Failed to allocate for dds path.\n");
			return EXIT_FAILURE;
		}

        if (in_type == DDS) {
            // Cut off the extension to get the new path
            strcpy(out_path, input_path);
            char* dot = strchr(out_path, '.');
            assert(dot != NULL && "Got DDS extension, then couldn't find a '.' ??");
            const char* new_ext = input_extensions[out_type];
            memcpy(dot, new_ext, strlen(new_ext) + 1);
        } else {
            snprintf(out_path, out_path_size, "%s.%s", input_path, "dds");
        }
	}

    switch (in_type) {
    case HGHT:
        hght_to_dds(input_path, out_path);
        break;
    case WATER_EXTM:
        water_to_dds(input_path, out_path);
        break;
    case DDS:
        switch (out_type) {
        case HGHT:
            dds_to_hght(input_path, out_path);
            break;
        case WATER_EXTM:
            dds_to_water(input_path, out_path);
            break;
        default:
            LOG_MSG(error, "Unknown / invalid conversion from .dds -> '%s'\n", input_extensions[out_type]);
            break;
        }
        break;
    default:
        LOG_MSG(error, "Unknown file type!\n");
        break;
    }

	free(out_path);
    return EXIT_SUCCESS;
}
