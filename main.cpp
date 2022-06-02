#include <iostream>
#include <fstream>
#include <cstring>
#include <windows.h>

#include "tga.hpp"
#include "font.hpp"
#include "offset.hpp"

using namespace std;


int main(int argc, char* argv[])
{
	if (argc == 7 || argc == 6 || argc == 5) {
		if (loadFontFile(argv[1]) != UNK) {
			if (argv[2][0] == 'n') {
				openFont(NORMAL);
			}
			else if (argv[2][0] == 's') {
				openFont(SMALL);
			}
			else {
				cout << "Unknown option!" << endl;
				goto exit;
			}

			int numChar = atoi(argv[4]);

			if (argv[3][0] == 'e') {
				decodeToTga(numChar, argv[5]);
			}
			else if (argv[3][0] == 'i') {
				if (encodeFromTga(numChar, argv[5])) {
					updateFontFile(argv[1]);
				}
			}
			else if (argv[3][0] == 'd') {
				if (insertData(getFontData(), numChar, NULL, 0, getDataOffset(), getMaxId())) {
					updateFontFile(argv[1]);
				}
			}
			else if (argv[3][0] == 'b') {
				createBitmap(argv[6], numChar, atoi(argv[5]));
			}
			else if (argv[3][0] == 'w') {
				if (writeBitmap(argv[1], argv[6], numChar, atoi(argv[5]))) {
					updateFontFile(argv[1]);
				}
			}
			else {
				cout << "Unknown option!" << endl;
				goto exit;
			}
		}
	}
	else {
		cout << endl;
		cout << "Silent Hill 2/3/4 PC/PS2 Font Extractor v0.4a" << endl;
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
		cout << "\tCreate bitmap from font file:" << endl;
		cout << "\t<FontFile> <FontSize> b <NumCharInColumn> <NumCharInRow> <tgaFile>" << endl;
		cout << endl;
		cout << "\tWrite bitmap to font file:" << endl;
		cout << "\t<FontFile> <FontSize> w <NumCharInColumn> <NumCharInRow> <tgaFile>" << endl;
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
	unloadFontFile();

	return 0;
}