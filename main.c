#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <sys/stat.h>

#include <common/logging.h>
#include <common/file.h>
#include <common/image.h>
#include <common/path.h>
#include "dds.h"

typedef enum {
	HGHT,
	DDS,
	INVALID,
}input_type;

void dds_to_hght(const char* dds_path, const char* hght_path) {
	// Heightmaps are always 256x256 16-bit numbers
	uint16_t hght_data[256*256] = {0};
	const u32 hght_size = sizeof(hght_data);

	// Sanity check that our file is the right size before we do anything
	const u32 expected_size = sizeof(dds_header) + hght_size;
	if (file_size(dds_path) != expected_size) {
		LOG_MSG(error, "Invalid DDS file (wrong size)\n");
		return;
	}

	FILE* hght = fopen(hght_path, "wb");
	FILE* dds = fopen(dds_path, "rb");
	if (!hght || !dds) {
		fclose(hght);
		fclose(dds);
		return;
	}

	// Skip over the DDS header, then read heightmap into memory
	fseek(dds, sizeof(dds_header), SEEK_SET);
	fread(&hght_data, hght_size, 1, dds);
	fclose(dds);

	fwrite(&hght_data, hght_size, 1, hght);
	fclose(hght);
}

void hght_to_dds(const char* hght_path, const char* dds_path) {
	// Heightmaps are always 256x256 16-bit numbers
	u16 hght_data[256*256] = {0};
	const u32 hght_size = sizeof(hght_data);

	// Sanity check that our file is the right size before we do anything
	if (file_size(hght_path) != hght_size) {
		LOG_MSG(error, "Invalid HGHT file (wrong size)\n");
		return;
	}

	FILE* hght = fopen(hght_path, "rb");
	if (!hght) {
		fclose(hght);
		return;
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
}

int main(int argc, char** argv) {
	enable_win_ansi(); // Enable printing in color on Windows.
	const char* input_path = argv[1];

	if (argc == 1) {
		LOG_MSG(error, "Not enough arguments.\n");
		LOG_MSG(info, "Usage: hght [path to HGHT/DDS file]\n");
        return EXIT_FAILURE;
	}

	input_type filetype = INVALID;
    if (path_has_extension(input_path, ".dds")) {
        filetype = DDS;
		LOG_MSG(info, "Converting DDS file to HGHT.\n");
    }
    else if (path_has_extension(input_path, ".hght")) {
        filetype = HGHT;
		LOG_MSG(info, "Converting HGHT file to DDS.\n");
    } else {
		LOG_MSG(error, "Provided file is not HGHT or DDS.\n");
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

        const char* extension = (filetype == HGHT) ? "hght" : "dds";
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
