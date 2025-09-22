#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <sys/stat.h>

#include <common/logging.h>
#include <common/image.h>
#include "dds.h"

typedef enum {
	HGHT = 0,
	DDS = 1
}input_type;

// Get size of a file using POSIX stat() function.
uint32_t filesize(const char* path) {
	struct stat st = {0};
	stat(path, &st);
	return st.st_size;
}


void dds_to_hght(const char* dds_path, const char* hght_path) {
	// Heightmaps are always 256x256 16-bit numbers
	uint16_t hght_data[256*256] = {0};
	uint32_t hght_size = sizeof(hght_data);

	// Sanity check that our file is the right size before we do anything
	uint32_t expected_size = sizeof(dds_header) + hght_size;
	if (filesize(dds_path) != expected_size) {
		LOG_MSG(error, "Invalid DDS file (wrong size)\n");
		return;
	}

	// Try to open our HGHT input & DDS output files
	FILE* hght = fopen(hght_path, "wb");
	FILE* dds = fopen(dds_path, "rb");

	// Bail if we can't open the files
	if (hght == NULL || dds == NULL) {
		fclose(hght);
		fclose(dds);
		return;
	}

	// Skip over the DDS header, then read heightmap into memory
	fseek(dds, sizeof(dds_header), SEEK_SET);
	fread(&hght_data, hght_size, 1, dds);
	fclose(dds); // Close input file

	// Write the heightmap data to our output.
	fwrite(&hght_data, hght_size, 1, hght);
	fclose(hght);
}

void hght_to_dds(const char* hght_path, const char* dds_path) {
	// Heightmaps are always 256x256 16-bit numbers
	uint16_t hght_data[256*256] = {0};
	uint32_t hght_size = sizeof(hght_data);

	// Sanity check that our file is the right size before we do anything
	if (filesize(hght_path) != hght_size) {
		LOG_MSG(error, "Invalid HGHT file (wrong size)\n");
		return;
	}

	// Try to open our HGHT input & DDS output files
	FILE* hght = fopen(hght_path, "rb");

	// Bail if we can't open the files
	if (hght == NULL) {
		fclose(hght);
		return;
	}

	// Read heightmap into memory
	fread(hght_data, hght_size, 1, hght);
	fclose(hght); // Close input file

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
		return 1;
	}

	char last_char = input_path[strlen(input_path) - 1];
	input_type filetype = (last_char == 's');
	if (filetype == DDS) {
		LOG_MSG(info, "Converting DDS file to HGHT.\n");
	}
	else if (last_char != 't') {
		LOG_MSG(error, "Provided file is not HGHT or DDS.\n");
	}
	else {
		LOG_MSG(info, "Converting HGHT file to DDS.\n");
	}

	// Get paths fom user input. If no dds output is given, use the HGHT path
	// and append ".dds". ("test.hght" outputs "test.hght.dds")
	char* out_path = NULL;
	if (argc >= 3) {
		out_path = argv[2];
	}
	else {
		// Allocate for output path
		out_path = malloc(strlen(input_path) + sizeof(".dds"));
		if (out_path == NULL) {
			LOG_MSG(error, "Failed to allocate memory for dds path.\n");
			return 1;
		}
		// Copy the input path, then add file extension of the output format to the end
		strcpy(out_path, input_path);
		if (filetype == HGHT) {
			strcat(out_path, ".dds");
		}
		else {
			strcat(out_path, ".hght");
		}
	}

	if (filetype == HGHT) {
		hght_to_dds(input_path, out_path);
	}
	else {
		dds_to_hght(input_path, out_path);
	}

	free(out_path);
}
