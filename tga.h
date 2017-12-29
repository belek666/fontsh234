typedef unsigned char  TGA_UCHAR8;
typedef unsigned int   TGA_UINT32;
typedef unsigned short TGA_UINT16;
typedef unsigned long long TGA_UINT64;

enum TGA_image_type {
    CMI = 1,	    // colour map image
	RGBA,	        // RGB(A) uncompressed
	GREY,           // greyscale uncompressed
	GREY_RLE = 9,	// greyscale RLE (compressed)
	RGBA_RLE,	    // RGB(A) RLE (compressed)
};

#pragma pack(push, 1)
typedef struct {
    TGA_UCHAR8 id;               // id
    TGA_UCHAR8 color_map_type;   // colour map type
    TGA_UCHAR8 image_type;       // image type
    TGA_UINT16 cm_first_entry;   // colour map first entry
    TGA_UINT16 cm_length;        // colour map length
    TGA_UCHAR8 map_entry_size;   // map entry size, colour map depth (16, 24 , 32)
    TGA_UINT16 h_origin;         // horizontal origin
    TGA_UINT16 v_origin;         // vertical origin
    TGA_UINT16 width;            // width
    TGA_UINT16 height;           // height
    TGA_UCHAR8 pixel_depth;      // pixel depth
    TGA_UCHAR8 image_desc;       // image descriptor
} TGA_FILEHEADER;
#pragma pack(pop)
