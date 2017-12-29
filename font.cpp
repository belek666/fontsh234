#include <iostream>
#include <fstream>
#include <cstring>
#include <windows.h>

#include "font.hpp"
#include "tga.h"

using namespace std;

#define NORMAL 0
#define SMALL 1

sh_font::SHGAME game = sh_font::UNK;

sh_font::SHGAME LoadFontFile(const char *filename);
void UnloadFontFile();
bool OpenFont(int type);

int GetCharSize(int charId);
uint8_t *DecodeChar(int charId);
int EncodeChar(uint8_t *charImg, int size);

int GetCharWidth(int charId);
int GetCharHeight();

bool decode2tga(int charId, const char *filename);
int encodeFromTga(int charId, const char *filename);
void insert2file(int charId, int size, const char *filename);

uint8_t *charEncoded;

uint8_t *fontFile;
int fontSize;

sh_font::sh2_font_file_header *sh2head;
sh_font::sh3_font_file_header *sh3head;
sh_font::sh4_font_file_header *sh4head;

int main(int argc, char *argv[])
{
	int i;
	int numChar;
	int size;

	if (argc == 6 || argc == 5) {
		
		game = LoadFontFile(argv[1]);
		
		if (game != sh_font::UNK) {
			if (argv[2][0] == 'n') {
				OpenFont(NORMAL);
			} else if (argv[2][0] == 's') {
				OpenFont(SMALL);
			} else {
				cout << "Unknown option!" << endl;
				goto exit;
			}
			
			numChar = atoi(argv[4]);
			
			if (argv[3][0] == 'e') {
				decode2tga(numChar, argv[5]);
			} else if (argv[3][0] == 'i') {
				size = encodeFromTga(numChar, argv[5]);
				if (size > 0)
					insert2file(numChar, size, argv[1]);
			} else if (argv[3][0] == 'd') {
				insert2file(numChar, 0, argv[1]);
			} else {
				cout << "Unknown option!" << endl;
				goto exit;
			}
		}
	} else {
		cout << endl;
		cout << "Silent Hill 2/3/4 PC Font Extractor v0.1a" << endl;
		cout << "Created by belek666, e-mail: belek666@onet.eu" << endl;
		cout << "Unpacking code provided by Dencraft" << endl;
		cout << endl;
		cout << "Usage:" << endl;
		cout << "\tExtract single character from font file to tga:" << endl;
		cout << "\t<FontFile> <FontSize> e <CharacterNumber> <tgaFile>" << endl;
		cout << endl;
		cout << "\tInsert single character from tga to font file:" << endl;
		cout << "\t<FontFile> <FontSize> i <CharacterNumber> <tgaFile>" << endl;
		cout << endl;
		cout << "\tDelete single character from font file:" << endl;
		cout << "\t<FontFile> <FontSize> d <CharacterNumber>" << endl;
		cout << endl;
		cout << "\tWhere:" << endl;
		cout << "\t<FontFile> - name of main file with fonts (*.exe, *.bin files)" << endl;
		cout << "\t<FontSize> - size of game's font, options: n - normal, s - small " << endl;
		cout << "\t<CharacterNumber> - encoding number of character in decimal" << endl;
		cout << "\t<tgaFile> - name of tga file that will be created/used" << endl;
		cout << endl;
		cout << "\tDon't forget to make backup of the main file - it will be overwritten" << endl;
	}

exit:
	UnloadFontFile();
	
	return 0;
}

sh_font::SHGAME LoadFontFile(const char *filename)
{
	int i;
	
	ifstream file(filename, ios::in|ios::binary|ios::ate);
	
	if (file.is_open()) {
		file.seekg(0, file.end);
		fontSize = file.tellg();
		fontFile = new uint8_t [fontSize];
		file.seekg (0, ios::beg);
		file.read ((char *)fontFile, fontSize);
		file.close();
		
		cout << "Opening font file " << filename << ", size " << fontSize << endl;
		
		if (strncmp(filename + strlen(filename) - 3, "bin", 3) == 0) {	
			if (fontFile[0] == 0x10) {
				sh3head = (sh_font::sh3_font_file_header *)fontFile;
				return sh_font::SH3;
			} else {
				sh4head = (sh_font::sh4_font_file_header *)fontFile;
				return sh_font::SH4;
			}
		} else {
			for (i = 0; i < (fontSize - 7); i++) {
				if (fontFile[i] == sh_font::sh_pallete[0] && fontFile[i + 1] == sh_font::sh_pallete[1] && 
					fontFile[i + 2] == sh_font::sh_pallete[2] && fontFile[i + 3] == sh_font::sh_pallete[3] &&
					fontFile[i + 4] == sh_font::sh_pallete[4] && fontFile[i + 5] == sh_font::sh_pallete[5] &&
					fontFile[i + 6] == sh_font::sh_pallete[6] ) {
			
					sh2head = (sh_font::sh2_font_file_header *)(fontFile + i - 16);
					
					if (sh2head->unk == 0x1F)
					 	return sh_font::SH2;
				}
			}
		}
	} else {
		cout << "Unable to open font file!";
	}
	
	return sh_font::UNK;
}

void UnloadFontFile()
{
	if(fontFile != NULL)
		delete[] fontFile;
	
	if(charEncoded != NULL)
		delete[] charEncoded;	
}

sh_font::sh_font_data *fontdata;

int fontType;

bool OpenFont(int type)
{
	if (fontFile != NULL && game != sh_font::UNK) {
		if (game == sh_font::SH2) {
			if (type == NORMAL)
				fontdata = (sh_font::sh_font_data *)(fontFile + sh2head->normalFontOffset - 0x00400000);
			else if (type == SMALL)
				fontdata = (sh_font::sh_font_data *)(fontFile + sh2head->smallFontOffset - 0x00400000);
		} else if (game == sh_font::SH3) {
			if (type == NORMAL)
				fontdata = (sh_font::sh_font_data *)(fontFile + sh3head->normalFontOffset);
			else if (type == SMALL)
				fontdata = (sh_font::sh_font_data *)(fontFile + sh3head->smallFontOffset);
		} else if (game == sh_font::SH4) {
			if (type == NORMAL)
				fontdata = (sh_font::sh_font_data *)(fontFile + sh4head->fontOffsets[0]);
			else if (type == SMALL)
				fontdata = (sh_font::sh_font_data *)(fontFile + sh4head->fontOffsets[1]);
		}

		fontType = type;
		
		return true;
	}
	
	return false;
}

int GetCharWidth(int charId)
{
	if (charId >= 0xE0) {
		if (fontType == NORMAL) {
			if (game == sh_font::SH4)
				return 25;
			else
				return 20;
		} else if (fontType == SMALL) {
			if (game == sh_font::SH4)
				return 20;
			else
				return 16;
		}
	}
	
	return fontdata->widthData[charId];
}

int GetCharHeight()
{
	if (fontType == NORMAL)
		return 30;
	else if (fontType == SMALL)
		return 24;
	
	return 0;
}

int charData = 0;
int	dataReadThree = 0;
int readOffset = 0;

uint8_t GetBits(int offset) //reads 3 bits from packed data
{
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

uint8_t *DecodeChar(int charId)
{
	int charSize = GetCharWidth(charId) * GetCharHeight() * 4;
	int charOffset = fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] * 4;
	
	if (fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] == 0)
		return NULL;
		
	if (fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] < fontdata->offsetData[1 + ((game == sh_font::SH4) * 0x70)])
		charOffset += 0x10000 * 4;
	
	uint8_t *charDecoded = new uint8_t[charSize];
	
	charData = 0;
	dataReadThree = 0;
	readOffset = 0;
	
	uint8_t Bits = 0;
   	uint8_t Bits2 = 0;
   	uint8_t Bits3 = 0;
	int dataOffset = 0;
	int zeroSpace = 0;
	int num = 0;
	
	cout << "Decoding" << endl; 
	
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
					 	} else {
							num = 0;
							num = (int)((Bits << 3) | (Bits2));
							zeroSpace = zeroSpace + num + 21;
					 	}
					} else {
						zeroSpace = zeroSpace + Bits + 14;
					}
				} else {
					zeroSpace = zeroSpace + Bits + 7;
				}
			} else {
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

			   cout << "*";
			}
		} else {
			charDecoded[dataOffset + 0] = sh_font::sh_pallete[Bits];
			charDecoded[dataOffset + 1] = sh_font::sh_pallete[Bits];
			charDecoded[dataOffset + 2] = sh_font::sh_pallete[Bits];
			charDecoded[dataOffset + 3] = 0xFF;
			
			if (charDecoded[dataOffset] == 0)
				charDecoded[dataOffset + 3] = 0x00;

			dataOffset += 4;
		}
		
		cout << "*";
	}
	
	cout << endl;
	
	return charDecoded;
}

int writeOffset = 0;
int encOffset = 0;
int encSize = 0;

void GiveBits(uint8_t Bits)
{
   int dataBits = 0;

   dataBits = Bits & 0x07;

   charData |= dataBits << writeOffset;
   
   charEncoded[encOffset	] = charData;
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

int EncodeChar(uint8_t *charImg, int size)
{
	int i;
	
	charEncoded = new uint8_t[size];
	
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
	
	cout << "Encoding" << endl; 
	
	while (dataOffset < size) {
		if (charImg[dataOffset	] == 0 &&
			charImg[dataOffset + 1] == 0 &&
			charImg[dataOffset + 2] == 0 &&
			charImg[dataOffset + 3] == 0 &&
			charImg[dataOffset + 4] == 0) {
			
			Bits = 0x07;
			GiveBits(Bits);
			zeroSpace = 0;					  
			while (charImg[dataOffset	] == 0 &&
				   charImg[dataOffset + 1] == 0 &&
				   charImg[dataOffset + 2] == 0 &&
				   charImg[dataOffset + 3] == 0) {
			   dataOffset += 4;				
			   zeroSpace++;
			   
			   if (dataOffset >= size) 
			   		break;
			   		
			   	cout << "*";
			}
			
			zeroSpace--;
			
			if ((zeroSpace - 84) > 0) {
				Bits = 0; //x5
			   	for(i = 0; i < 5; i++)
			   		GiveBits(Bits);
			   
			   	num = zeroSpace - 84;
  
			   	Bits3 = (uint8_t)num;
			   	GiveBits(Bits3);
			   	Bits2 = num >> 3;
			   	GiveBits(Bits2);
			   	Bits = num >> 6;
			   	GiveBits(Bits);
			} else if ((zeroSpace - 21) > 0) {
			   	Bits = 0; //x3
			   	for(i=0; i<3; i++)
			   		GiveBits(Bits);
			   
			   	num = zeroSpace - 21;

			   	Bits2 = (uint8_t)num;
			   	GiveBits(Bits2);
			   	Bits = num >> 3;
			   	GiveBits(Bits);
			} else if ((zeroSpace - 14) > 0) {
			   	Bits = 0; //x2
			   	for(i=0; i<2; i++)
			   		GiveBits(Bits);
			   
			   	Bits = zeroSpace - 14;
			   	GiveBits(Bits);	  
			} else if ((zeroSpace - 7) > 0) {
			   	Bits = 0;
			   	GiveBits(Bits);	 
			   	Bits = zeroSpace - 7;
			   	GiveBits(Bits);
			} else {
			   	Bits = zeroSpace;
			   	GiveBits(Bits);
			}
		} else {   
			for (i = 0; i < 7; i++) {
				if (charImg[dataOffset	] == sh_font::sh_pallete[i] && 
					charImg[dataOffset + 1] == sh_font::sh_pallete[i] &&
					charImg[dataOffset + 2] == sh_font::sh_pallete[i]) {
					Bits = i;
					break;
				}
			}
			dataOffset += 4;
			GiveBits(Bits);
		}
		cout << "*";
	}
	
	cout << endl;
	
	return (encSize * 3 + 31) / 32 * 4;
}

uint8_t tga_pallete[] =
{
	0x00, 0x00, 0x00, 0x00, 0x5F, 0x5F, 0x5F, 0xFF, 0x7F, 0x7F, 0x7F, 0xFF, 0x9F, 0x9F, 0x9F, 0xFF, 
	0xBF, 0xBF, 0xBF, 0xFF, 0xDF, 0xDF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
};

bool decode2tga(int charId, const char *filename)
{
	int i, j, k;
	TGA_FILEHEADER tga;
	
	memset(&tga, 0, sizeof(TGA_FILEHEADER));
	
	tga.color_map_type = 1;
	tga.image_type = 1;
	tga.cm_length = sizeof(tga_pallete) / 4;
	tga.map_entry_size = 32;
	tga.pixel_depth = 8;
	tga.width = GetCharWidth(charId);
	tga.height = GetCharHeight();
	
	ofstream file(filename, ofstream::binary);
	file.write ((char *)&tga, sizeof(TGA_FILEHEADER));
	file.write ((char *)&tga_pallete, sizeof(tga_pallete));
	
	uint8_t *data = DecodeChar(charId);
	
	if (data == NULL) {
		cout << "No data" << endl;
		return false;
	}
	
	for (i = GetCharHeight() - 1; i > -1; i--) {
		for (j = 0; j < GetCharWidth(charId); j++) {
			for (k = 0; k < 7; k++) {
				if (data[i * GetCharWidth(charId) * 4 + j * 4] == sh_font::sh_pallete[k]) {
					file.write ((char *)&k, 1);
				}
			}
		}
	}
	
	file.close();

	return true;
}

int encodeFromTga(int charId, const char *filename)
{
	int i, j, k;
	uint8_t c;
	TGA_FILEHEADER tga;

	ifstream file(filename, ios::in|ios::binary|ios::ate);
	
	if (file.is_open()) {
		file.seekg(0, ios::beg);
		file.read((char *)&tga, sizeof(TGA_FILEHEADER));
		
		if (tga.color_map_type != 1 || tga.image_type != 1 ||
			tga.cm_length != sizeof(tga_pallete) / 4 || tga.map_entry_size != 32 ||
			tga.pixel_depth != 8) {
			cout << "Wrong tga format!" << endl;
			return 0;  	
		}
		
		if (tga.height != GetCharHeight()) {
			cout << "Wrong character height!" << endl;
			return 0;
		}
		
		if (charId >= 0xE0 && tga.width != GetCharWidth(charId)) {
			cout << "Wrong character width!" << endl;
			return 0;
		}
		
		file.seekg(sizeof(TGA_FILEHEADER) + sizeof(tga_pallete), ios::beg);
		
		int size = tga.width * tga.height * 4;
		uint8_t *charData = new uint8_t[size];
		
		for (i = tga.height - 1; i > -1; i--) {
			for (j = 0; j < tga.width; j++) {
				file.read((char *)&c, 1);
				
				if (c == 0) {
					charData[i * tga.width * 4 + j * 4 + 0] = 0;
					charData[i * tga.width * 4 + j * 4 + 1] = 0;
					charData[i * tga.width * 4 + j * 4 + 2] = 0;
					charData[i * tga.width * 4 + j * 4 + 3] = 0;
				} else {
					charData[i * tga.width * 4 + j * 4 + 0] = sh_font::sh_pallete[c];
					charData[i * tga.width * 4 + j * 4 + 1] = sh_font::sh_pallete[c];
					charData[i * tga.width * 4 + j * 4 + 2] = sh_font::sh_pallete[c];
					charData[i * tga.width * 4 + j * 4 + 3] = 0xFF;
				}
			}
		}
		
		file.close();
		
		if (charId < 0xE0)
			fontdata->widthData[charId] = tga.width;
		
		return EncodeChar(charData, size);
	}
	
	return 0;
}

void insert2file(int charId, int size, const char *filename)
{
	int i;
	int charStartOffset, charEndOffset;
	int oldSize;
	uint8_t *tempData;
	
	charStartOffset = fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] * 4;
	
	if (charStartOffset == 0) {
		i = 0;
		while (charStartOffset == 0) {
			charStartOffset = fontdata->offsetData[charId + i + ((game == sh_font::SH4) * 0x70)] * 4;
			if (fontdata->offsetData[charId + i + ((game == sh_font::SH4) * 0x70)] > 0 && fontdata->offsetData[charId + i + ((game == sh_font::SH4) * 0x70)] < fontdata->offsetData[1 + ((game == sh_font::SH4) * 0x70)])
				charStartOffset += 0x10000 * 4;

			i++;
		}
		oldSize = 0;
	} else {
		if (fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] < fontdata->offsetData[1 + ((game == sh_font::SH4) * 0x70)])
			charStartOffset += 0x10000 * 4;
		
		charEndOffset = fontdata->offsetData[charId + 1 + ((game == sh_font::SH4) * 0x70)] * 4;
			
		if (charEndOffset == 0) {
			i = 0;
			while (charEndOffset == 0) {
				charEndOffset = fontdata->offsetData[charId + 1 + i + ((game == sh_font::SH4) * 0x70)] * 4;
				if (fontdata->offsetData[charId + 1 + i + ((game == sh_font::SH4) * 0x70)] > 0 && fontdata->offsetData[charId + 1 + i + ((game == sh_font::SH4) * 0x70)] < fontdata->offsetData[1 + ((game == sh_font::SH4) * 0x70)])
					charEndOffset += 0x10000 * 4;

				i++;
			}
		} else {
			if (fontdata->offsetData[charId + 1 + ((game == sh_font::SH4) * 0x70)] < fontdata->offsetData[1 + ((game == sh_font::SH4) * 0x70)])
				charEndOffset += 0x10000 * 4;
		}
		
		oldSize = charEndOffset - charStartOffset;
	}

	if (oldSize == size) {
		memcpy((uint8_t *)fontdata + charStartOffset, charEncoded, size);
	} else if (oldSize < size) {
		i = 0;
		charEndOffset = 0;
		while (charEndOffset == 0) {
			charEndOffset = fontdata->offsetData[i + ((game == sh_font::SH4) * 0x70)] * 4;
			i++;
		}
		tempData = new uint8_t[charStartOffset - charEndOffset];
		memcpy(tempData, (uint8_t *)fontdata + charEndOffset, charStartOffset - charEndOffset);
		memcpy((uint8_t *)fontdata + charEndOffset - (size - oldSize), tempData, charStartOffset - charEndOffset);
		memcpy((uint8_t *)fontdata + charStartOffset - (size - oldSize), charEncoded, size);
		for (i = 0; i < charId; i++) {
			if (fontdata->offsetData[i + ((game == sh_font::SH4) * 0x70)] != 0)
				fontdata->offsetData[i + ((game == sh_font::SH4) * 0x70)] -= (size - oldSize) / 4;
		}
		if ((charStartOffset - (size - oldSize)) > 0x40000)
			charStartOffset -= 0x40000;
	
		fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] = (charStartOffset - (size - oldSize)) / 4;
		delete[] tempData;
	} else if (oldSize > size) {
		i = 0;
		charEndOffset = 0;
		while (charEndOffset == 0) {
			charEndOffset = fontdata->offsetData[i + ((game == sh_font::SH4) * 0x70)] * 4;
			i++;
		}
		tempData = new uint8_t[charStartOffset - charEndOffset];
		memcpy(tempData, (uint8_t *)fontdata + charEndOffset, charStartOffset - charEndOffset);
		memcpy((uint8_t *)fontdata + charEndOffset + (oldSize - size), tempData, charStartOffset - charEndOffset);
		memcpy((uint8_t *)fontdata + charStartOffset + (oldSize - size), charEncoded, size);
		for (i = 0; i < charId; i++) {
			if (fontdata->offsetData[i + ((game == sh_font::SH4) * 0x70)] != 0)
				fontdata->offsetData[i + ((game == sh_font::SH4) * 0x70)] += (oldSize - size) / 4;
		}
		if ((charStartOffset + (oldSize - size)) > 0x40000)
			charStartOffset -= 0x40000;
	
		fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] = (charStartOffset + (oldSize - size)) / 4;
		
		if (size == 0) {
			fontdata->offsetData[charId + ((game == sh_font::SH4) * 0x70)] = 0;
			if (charId < 0xE0)
				fontdata->widthData[charId] = 0;
		}
		
		delete[] tempData;
	}

	ofstream efile(filename, ofstream::binary);
	efile.write ((char *)fontFile, fontSize);
	efile.close();
}

