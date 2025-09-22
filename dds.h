#pragma once

#include <stdint.h>

// DDS structures & constants taken from MSDN (Microsoft documentation)
typedef enum dds_flags {
	DDSD_CAPS        = 0x00000001,
	DDSD_HEIGHT      = 0x00000002,
	DDSD_WIDTH       = 0x00000004,
	DDSD_PITCH       = 0x00000008,
	DDSD_PIXELFORMAT = 0x00001000,
	DDSD_MIPMAPCOUNT = 0x00020000,
	DDSD_LINEARSIZE  = 0x00080000,
	DDSD_DEPTH       = 0x00800000,

	// This constant has all the flags required in every DDS file.
	REQUIRED_BASE_FLAGS = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
}dds_flags;

// Pixel format constants for the pixel format structure
typedef enum dds_format_flags {
	DDPF_ALPHAPIXELS = 0x00000001,
	DDPF_ALPHA       = 0x00000002,
	DDPF_FOURCC      = 0x00000004,
	DDPF_RGB         = 0x00000040,
	DDPF_YUV         = 0x00000200,
	DDPF_LUMINANCE   = 0x00020000
}dds_format_flags;

typedef enum dds_caps_flags {
	DDSCAPS_COMPLEX = 0x00000008,
	DDSCAPS_MIPMAP  = 0x00400000,
	DDSCAPS_TEXTURE = 0x00001000
}dds_caps_flags;

// This structure describes how each pixel should be read
typedef struct dds_pixel_format {
	uint32_t size; // Must be 32 (0x20)
	uint32_t flags;
	uint32_t format_char_code; // See dwFourCC at https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat.
	uint32_t bits_per_pixel;
	uint32_t red_bitmask;
	uint32_t green_bitmask;
	uint32_t blue_bitmask;
	uint32_t alpha_bitmask;
}dds_pixel_format;

// "DDS " in little endian, because using the string constant would include the null terminator.
#define DDS_BEGIN 0x20534444

// DDS file header.
typedef struct dds_header {
	uint32_t identifier;   // "DDS ", or DDS_BEGIN defined above. Also known as the "file magic" or "magic number".
	uint32_t size;         // Must be 124 (0x7C)
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitch_or_linear_size;
	uint32_t depth;
	uint32_t mipmap_count;
	uint32_t reserved[11]; // Unused
	dds_pixel_format pixel_format;
	uint32_t caps;         // Flags for complexity of the surface
	uint32_t caps2;        // Will always be 0 because we're not dealing with cubemaps or volumes
	uint32_t caps3;        // Unused
	uint32_t caps4;        // Unused
	uint32_t reserved2;    // Unused
}dds_header;
uint64_t test = (uint64_t)1 << 32;
