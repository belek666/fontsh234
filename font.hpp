#pragma once

#include <cstdint>

enum SHGAME {
	UNK = 0,
	SH2,
	SH3,
	SH4,
	PS2_SH2,
	PS2_SH3,
	XBOX_SH2,
};

enum FONT_SIZE {
	NORMAL = 0,
	SMALL
};

#pragma pack(push, 1)

typedef struct {
	float x;
	float y;
	float z;
	std::uint32_t color;
} fontPrimVertex;

struct sh2_font_file_header
{
	std::uint16_t normalFontWidth;		// 20
	std::uint16_t normalFontHeight;		// 30
	std::uint16_t smallFontWidth;		// 16
	std::uint16_t smallFontHeight;		// 24
	std::uint32_t normalFontOffset;		// character height = 30
	std::uint32_t smallFontOffset;		// character height = 24
	std::uint8_t  pallete[7];
	std::uint8_t  unk;
	fontPrimVertex a;					// vertices for D3DPT_TRIANGLELIST
	fontPrimVertex b;
	fontPrimVertex c;
	fontPrimVertex d;
	fontPrimVertex e;
	fontPrimVertex f;
};

struct sh3_font_file_header
{
	std::uint32_t normalFontOffset;		// character height = 30
	std::uint32_t smallFontOffset;		// character height = 24
	std::uint32_t unknownDataOffset;	// what is this?
	std::uint32_t padding;
};

struct sh4_font_file_header
{
	std::uint32_t fontNumber;
	std::uint32_t fontOffsets[7];
};

struct sh_font_data
{
	std::uint16_t boffset[8];			// char ids which offsets >= 0x40000
	std::uint8_t widthData[0xE0];		// character width table
	std::uint16_t offsetData[];			// character data offset table
};

struct sh234_type
{
	const char* fileName;
	SHGAME game;
	std::uint32_t fontStructOffset;
	std::uint32_t size;
	std::uint32_t offset;
};
#pragma pack(pop)

extern std::uint8_t sh_pallete[7];

SHGAME loadFontFile(const char* filename);
void unloadFontFile();
bool openFont(FONT_SIZE type);
uint8_t* decodeChar(int charId);
int encodeChar(uint8_t* charImg, int charId, int size, uint8_t* charEncoded);
int getCharWidth(int charId);
int getCharHeight();
void setCharWidth(int charId, uint8_t width);
void updateFontFile(const char* filename);
int getDataOffset();
int getMaxId();
sh_font_data* getFontData();
