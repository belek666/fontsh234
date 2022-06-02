#include <iostream>
#include <fstream>
#include <cstring>
#include <windows.h>

#include "font.hpp"
#include "tga.hpp"
#include "offset.hpp"

using namespace std;

std::uint8_t sh_pallete[7] =
{
	0x00, 0x5F, 0x7F, 0x9F, 0xBF, 0xDF, 0xFF,
};


static uint8_t *fontFile;
static streamoff fontSize;
static int dataOffset = 0;
static uint32_t fontOffset = 0;
static sh_font_data* fontdata;
static int fontType;
static SHGAME game = UNK;

static sh2_font_file_header *sh2head;
static sh3_font_file_header *sh3head;
static sh4_font_file_header *sh4head;

#define PS2GAMES 6

static ps2sh234_type ps2games[PS2GAMES] = {
	{ "SLES_503.82", PS2_SH2, 0x001D9960, 3074200, 0x000FF800 },
	{ "SLES_511.56", PS2_SH2, 0x001E57A0, 3150616, 0x000FF800 },
	{ "SLUS_202.28", PS2_SH2, 0x001D7950, 3045672, 0x000FF900 },
	{ "SLUS_202.28", PS2_SH2, 0x001E4DA0, 3147928, 0x000FF800 },
	{ "SLPM_123.45", PS2_SH2, 0x002DEAB8, 10065884, 0x000FFF80 },
	{ "SLES_514.34", PS2_SH3, 0x00259FE8, 3395784, 0x000FFA80 },
};

SHGAME loadFontFile(const char *filename) {
	dataOffset = 0;
	fontOffset = 0;
	game = UNK;

	ifstream file(filename, ios::in|ios::binary|ios::ate);
	
	if (file.is_open()) {
		file.seekg(0, file.end);
		fontSize = file.tellg();
		fontFile = new uint8_t [(int)fontSize];
		file.seekg (0, ios::beg);
		file.read ((char *)fontFile, fontSize);
		file.close();
		
		cout << "Opening font file " << filename << ", size " << fontSize << endl;
		
		if (strncmp(filename + strlen(filename) - 3, "bin", 3) == 0) {	
			if (fontFile[0] == 0x10) {
				sh3head = (sh3_font_file_header *)fontFile;
				game = SH3;
			}
			else {
				sh4head = (sh4_font_file_header *)fontFile;
				dataOffset = 0x70;
				game = SH4;
			}
		} 
		else if (fontFile[0] == 0x7F && fontFile[1] == 'E' && fontFile[2] == 'L' && fontFile[3] == 'F' ) {
			for (int i = 0; i < PS2GAMES; i++) {
				if (strcmp(strrchr(filename, '.') - 8, ps2games[i].elfName) == 0 && ps2games[i].size == fontSize) {
					sh2head = (sh2_font_file_header *)(fontFile + ps2games[i].fontStructOffset);
					fontOffset = ps2games[i].offset;
					game = ps2games[i].game;
					break;
				}
			}
		} 
		else {
			for (int i = 0; i < (fontSize - 7); i++) {
				if (fontFile[i] == sh_pallete[0] && fontFile[i + 1] == sh_pallete[1] && 
					fontFile[i + 2] == sh_pallete[2] && fontFile[i + 3] == sh_pallete[3] &&
					fontFile[i + 4] == sh_pallete[4] && fontFile[i + 5] == sh_pallete[5] &&
					fontFile[i + 6] == sh_pallete[6] ) {
			
					sh2head = (sh2_font_file_header *)(fontFile + i - 16);
					
					if (sh2head->unk == 0x1F) {
						fontOffset = 0x00400000;
						game = SH2;
						break;
					}
				}
			}
		}
	} 
	else {
		cout << "Unable to open font file!";
	}
	
	return game;
}

void unloadFontFile() {
	if (fontFile != NULL) {
		delete[] fontFile;
	}
}

void updateFontFile(const char* filename) {
	ofstream file(filename, ofstream::binary);
	if (file.is_open()) {
		file.write((char*)fontFile, fontSize);
		file.close();
	}
}

bool openFont(FONT_SIZE type) {
	if (fontFile != NULL && game != UNK) {
		if (game == SH2 || game == PS2_SH2 || game == PS2_SH3) {
			if (type == NORMAL) {
				fontdata = (sh_font_data*)(fontFile + sh2head->normalFontOffset - fontOffset);
			}
			else if (type == SMALL) {
				fontdata = (sh_font_data*)(fontFile + sh2head->smallFontOffset - fontOffset);
			}
		} else if (game == SH3) {
			if (type == NORMAL) {
				fontdata = (sh_font_data*)(fontFile + sh3head->normalFontOffset);
			}
			else if (type == SMALL) {
				fontdata = (sh_font_data*)(fontFile + sh3head->smallFontOffset);
			}
		} else if (game == SH4) {
			if (type == NORMAL) {
				fontdata = (sh_font_data*)(fontFile + sh4head->fontOffsets[0]);
			}
			else if (type == SMALL) {
				fontdata = (sh_font_data*)(fontFile + sh4head->fontOffsets[1]);
			}
		}
		fontType = type;
		return true;
	}
	return false;
}

int getCharWidth(int charId) {
	if (charId >= 0xE0) {
		if (fontType == NORMAL) {
			if (game == SH2 || game == PS2_SH2 || game == PS2_SH3) {
				return sh2head->normalFontWidth;
			}
			else if (game == SH4) {
				return 25;
			}
			else {
				return 20;
			}
		} 
		else if (fontType == SMALL) {
			if (game == SH2 || game == PS2_SH2 || game == PS2_SH3) {
				return sh2head->smallFontWidth;
			}
			else if (game == SH4) {
				return 20;
			}
			else {
				return 16;
			}
		}
	} 
	else {
		return fontdata->widthData[charId];
	}
	return 0;
}

int getCharHeight() {
	if (fontType == NORMAL) {
		if (game == SH2 || game == PS2_SH2 || game == PS2_SH3) {
			return sh2head->normalFontHeight;
		}
		else if (game == SH4) {
			return 32;
		}
		else {
			return 30;
		}
	}
	else if (fontType == SMALL) {
		if (game == SH2 || game == PS2_SH2 || game == PS2_SH3) {
			return sh2head->smallFontHeight;
		}
		else if (game == SH4) {
			return 26;
		}
		else {
			return 24;
		}
	}
	
	return 0;
}

void setCharWidth(int charId, uint8_t width) {
	if (charId < 0xE0) {
		fontdata->widthData[charId] = width;
	}
}

int getDataOffset() {
	return dataOffset;
}

int getMaxId() {
	return 7580; //fix me?
}

sh_font_data* getFontData() {
	return fontdata;
}

static void printProgress(char* string, int current, int max) {
	int progress = current * 100 / max;
	if (progress > 100) {
		progress = 100;
	}
	printf("\r%s : [%3d%%]", string, progress);
}

static int charData = 0;
static int dataReadThree = 0;
static unsigned int readOffset = 0;

static uint8_t GetBits(int offset) { //reads 3 bits from packed data
	uint8_t charBits = 0;
	
	if (readOffset == 24) {
		dataReadThree += 3;
		readOffset = 0;
	}
	
	charData = *(int *)((uint8_t *)fontdata + offset + dataReadThree);
	charData = charData >> readOffset;
	
	readOffset += 3;
	
	charBits = (uint8_t)charData & 0x07;
	
	return charBits;
}

uint8_t *decodeChar(int charId) {
	int charSize = getCharWidth(charId) * getCharHeight() * 4;
	int charOffset = getCharOffset(fontdata, charId, dataOffset);
	
	if (fontdata->offsetData[charId + dataOffset] == 0) {
		return NULL;
	}

	uint8_t *charDecoded = new uint8_t[charSize + 2048];
	
	charData = 0;
	dataReadThree = 0;
	readOffset = 0;
	
	uint8_t Bits = 0;
	uint8_t Bits2 = 0;
	uint8_t Bits3 = 0;
	int dataOffset = 0;
	int zeroSpace = 0;
	int num = 0;

	char log[100];
	sprintf_s(log, 100, "Decoding, id: %d offset: 0x%08X size: %d", charId, charOffset, charSize);
	printProgress(log, 0, charSize);

	while (dataOffset < charSize) {
		Bits = GetBits(charOffset);
		
		if (Bits == 0x07) {
			zeroSpace = 0;
			Bits = GetBits(charOffset);
			
			if (Bits == 0) {
				Bits = GetBits(charOffset);
				
				if (Bits == 0) {
					Bits = GetBits(charOffset);
					
					if (Bits == 0) {
						Bits2 = GetBits(charOffset);
						Bits = GetBits(charOffset);
						
						if (Bits == 0 && Bits2 == 0) {
							Bits3= GetBits(charOffset);
							Bits2 = GetBits(charOffset);
							Bits = GetBits(charOffset);

							num = 0;
							num = (int)((Bits << 6) | (Bits2 << 3) | (Bits3));
							zeroSpace = zeroSpace + num + 84;
						} 
						else {
							num = 0;
							num = (int)((Bits << 3) | (Bits2));
							zeroSpace = zeroSpace + num + 21;
						}
					} 
					else {
						zeroSpace = zeroSpace + Bits + 14;
					}
				} 
				else {
					zeroSpace = zeroSpace + Bits + 7;
				}
			} 
			else {
				zeroSpace = Bits;
			}
			
			zeroSpace++;
			
			while (zeroSpace) {
				charDecoded[dataOffset + 0] = 0;
				charDecoded[dataOffset + 1] = 0;
				charDecoded[dataOffset + 2] = 0;
				charDecoded[dataOffset + 3] = 0;

				dataOffset += 4;
				zeroSpace--;
			}
		}
		else {
			charDecoded[dataOffset + 0] = sh_pallete[Bits];
			charDecoded[dataOffset + 1] = sh_pallete[Bits];
			charDecoded[dataOffset + 2] = sh_pallete[Bits];
			charDecoded[dataOffset + 3] = 0xFF;
			
			if (charDecoded[dataOffset] == 0) {
				charDecoded[dataOffset + 3] = 0x00;
			}

			dataOffset += 4;
		}
		
		printProgress(log, dataOffset, charSize);
	}
	
	cout << endl;
	
	return charDecoded;
}

static unsigned int writeOffset = 0;
static int encOffset = 0;
static int encSize = 0;

static void GiveBits(uint8_t Bits, uint8_t *charEncoded) {
	int dataBits = 0;

	dataBits = Bits & 0x07;

	charData |= dataBits << writeOffset;
	
	charEncoded[encOffset    ] = charData;
	charEncoded[encOffset + 1] = charData >> 8;
	charEncoded[encOffset + 2] = charData >> 16;
	
	if (writeOffset == 21) {
	  encOffset += 3;
	  writeOffset = 0;
	  charData = 0;
	} else {
		writeOffset += 3;
	}
	
	encSize++;
}

int encodeChar(uint8_t *charImg, int size, uint8_t *charEncoded) {
	if (charImg == NULL || charEncoded == NULL) {
		return -1;
	}

	charData = 0;
	writeOffset = 0;
	encOffset = 0;
	encSize = 0;
	
	uint8_t Bits = 0;
	uint8_t Bits2 = 0;
	uint8_t Bits3 = 0;
	int dataOffset = 0;
	int zeroSpace = 0;
	int num = 0;
	
	char log[100];
	sprintf_s(log, 100, "Encoding, size: %d", size);
	printProgress(log, 0, size);

	while (dataOffset < size) {
		if (charImg[dataOffset    ] == 0 &&
			charImg[dataOffset + 1] == 0 &&
			charImg[dataOffset + 2] == 0 &&
			charImg[dataOffset + 3] == 0 &&
			charImg[dataOffset + 4] == 0) {
			
			Bits = 0x07;
			GiveBits(Bits, charEncoded);
			zeroSpace = 0;
			while (charImg[dataOffset    ] == 0 &&
				   charImg[dataOffset + 1] == 0 &&
				   charImg[dataOffset + 2] == 0 &&
				   charImg[dataOffset + 3] == 0) {
				dataOffset += 4;
				zeroSpace++;
				
				printProgress(log, dataOffset, size);

				if (dataOffset >= size) 
					break;
			}
			
			zeroSpace--;
			
			if ((zeroSpace - 84) > 0) {
				Bits = 0; //x5
				for (int i = 0; i < 5; i++) {
					GiveBits(Bits, charEncoded);
				}
				num = zeroSpace - 84;
				Bits3 = (uint8_t)num;
				GiveBits(Bits3, charEncoded);
				Bits2 = num >> 3;
				GiveBits(Bits2, charEncoded);
				Bits = num >> 6;
				GiveBits(Bits, charEncoded);
			} 
			else if ((zeroSpace - 21) > 0) {
				Bits = 0; //x3
				for (int i = 0; i < 3; i++) {
					GiveBits(Bits, charEncoded);
				}
				num = zeroSpace - 21;
				Bits2 = (uint8_t)num;
				GiveBits(Bits2, charEncoded);
				Bits = num >> 3;
				GiveBits(Bits, charEncoded);
			} 
			else if ((zeroSpace - 14) > 0) {
				Bits = 0; //x2
				for (int i = 0; i < 2; i++) {
					GiveBits(Bits, charEncoded);
				}
				Bits = zeroSpace - 14;
				GiveBits(Bits, charEncoded);
			}
			else if ((zeroSpace - 7) > 0) {
				Bits = 0;
				GiveBits(Bits, charEncoded);
				Bits = zeroSpace - 7;
				GiveBits(Bits, charEncoded);
			}
			else {
				Bits = zeroSpace;
				GiveBits(Bits, charEncoded);
			}
		} else {
			for (int i = 0; i < 7; i++) {
				if (charImg[dataOffset    ] == sh_pallete[i] && 
					charImg[dataOffset + 1] == sh_pallete[i] &&
					charImg[dataOffset + 2] == sh_pallete[i]) {
					Bits = i;
					break;
				}
			}
			dataOffset += 4;
			GiveBits(Bits, charEncoded);
		}
		printProgress(log, dataOffset, size);
	}
	
	cout << endl;
	
	return (encSize * 3 + 31) / 32 * 4;
}
