#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <common/logging.h>
#include <common/platform.h>
#include <common/file.h>
#include <common/path.h>

#include "util.h"
#include "terrain.h"

typedef enum {
	HGHT,
	DDS,
	INVALID,
}input_type;

const char* input_extensions[] = {
    ".hght", ".dds", "",
};

void usage() {
    LOG_MSG(info, "Usage: hght [path to HGHT/DDS file]\n");
    LOG_MSG(info, "   OR: hght [detail level] [decimal X coordinate] [decimal Y coordinate]\n");
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

	input_type filetype = INVALID;
    for (u32 i = 0; i < ARRAY_SIZE(input_extensions); i++) {
        if (path_has_extension(input_path, input_extensions[i])) {
            filetype = i;
        }
    }

    if (filetype == INVALID && argc == 4) {
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
    } else {
        return EXIT_FAILURE;
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

        const char* extension = (filetype == HGHT) ? "dds" : "hght";
        snprintf(out_path, out_path_size, "%s.%s", input_path, extension);
	}

	if (filetype == HGHT) {
		hght_to_dds(input_path, out_path);
	} else {
		dds_to_hght(input_path, out_path);
	}

	free(out_path);
    return EXIT_SUCCESS;
}
