#include <iostream>
#include <fstream>
#include <cstring>
#include <windows.h>

#include "offset.hpp"

using namespace std;

static int getPage(sh_font_data* data, int charId);
static int getFirstCharId(sh_font_data* data, int startId, int max, int offset);
static int getLastCharId(sh_font_data* data, int max, int offset);
static int getCharSize(sh_font_data* data, int charId, int offset, int max);
static bool updateCharOffset(sh_font_data* data, int charId, int newOffset, int offset, int max);

static uint16_t tempPage[8];

int getCharOffset(sh_font_data* data, int charId, int offset) {
	int charOffset = -1;
	if (data != NULL && charId >= 0) {
		charOffset = data->offsetData[charId + offset] * 4;
		int page = getPage(data, charId);
		if (charOffset > 0) {
			charOffset += 0x40000 * page;
		}
		else if (page > 0 && data->boffset[page - 1] == charId) {
			charOffset += 0x40000 * page;
		}
	}
	return charOffset;
}

bool insertData(sh_font_data* data, int charId, uint8_t* charData, int size, int offset, int max) {
	int currentCharSize = getCharSize(data, charId, offset, max);
	int currentCharOffset = getCharOffset(data, charId, offset);
	int startOffset = getCharOffset(data, getFirstCharId(data, 0x00, max, offset), offset);

	if (currentCharOffset == 0) {
		//update offset to next id
		int nextId = getFirstCharId(data, charId + 1, max, offset);
		currentCharOffset = getCharOffset(data, nextId, offset);
	}

	if (currentCharOffset > 0 && currentCharSize > -1 && startOffset > -1) {
		memcpy(tempPage, data->boffset, 16);
		if (currentCharSize == size) {
			memcpy((uint8_t*)data + currentCharOffset, charData, size);
		}
		else if (currentCharSize < size) {
			int sizeDifference = size - currentCharSize;
			uint8_t* tempData = new uint8_t[currentCharOffset - startOffset];
			memcpy(tempData, (uint8_t*)data + startOffset, currentCharOffset - startOffset);
			memcpy((uint8_t*)data + startOffset - sizeDifference, tempData, currentCharOffset - startOffset);
			memcpy((uint8_t*)data + currentCharOffset - sizeDifference, charData, size);
			delete[] tempData;
			for (int i = 0; i < charId; i++) {
				int currentOffset = getCharOffset(data, i, offset);
				if (currentOffset > 0) {
					currentOffset -= sizeDifference;
					if (!updateCharOffset(data, i, currentOffset, offset, max)) {
						cout << "\t Failed to update char offset" << endl;
						return false;
					}
				}
			}
			currentCharOffset -= sizeDifference;
			if (!updateCharOffset(data, charId, currentCharOffset, offset, max)) {
				cout << "\t Failed to update char offset" << endl;
				return false;
			}
		}
		else {
			int sizeDifference = currentCharSize - size;
			uint8_t* tempData = new uint8_t[currentCharOffset - startOffset];
			memcpy(tempData, (uint8_t*)data + startOffset, currentCharOffset - startOffset); //save data from between first and new char
			memcpy((uint8_t*)data + startOffset + sizeDifference, tempData, currentCharOffset - startOffset); //move it to make space
			memset((uint8_t*)data + startOffset, 0x00, sizeDifference); //clear space
			memcpy((uint8_t*)data + currentCharOffset + sizeDifference, charData, size); //insert new data
			delete[] tempData;
			for (int i = 0; i < charId; i++) {
				int currentOffset = getCharOffset(data, i, offset);
				if (currentOffset > 0) {
					currentOffset += sizeDifference;
					if (!updateCharOffset(data, i, currentOffset, offset, max)) {
						cout << "\t Failed to update char offset" << endl;
						return false;
					}
				}
			}
			if (size == 0) {
				if (charId < 0xE0) {
					data->widthData[charId] = 0;
				}
				currentCharOffset = 0;
			}
			else {
				currentCharOffset += sizeDifference;
			}
			if (!updateCharOffset(data, charId, currentCharOffset, offset, max)) {
				cout << "\t Failed to update char offset" << endl;
				return false;
			}
		}
		memcpy(data->boffset, tempPage, 16);
		return true;
	}
	if (startOffset < 0) {
		cout << "\t Failed to find first char offset!" << endl;
	}
	else {
		cout << "\t Wrong char parameters: offset: " << currentCharOffset << " size: " << currentCharSize << " page " << data->boffset[0] << endl;
	}
	return false;
}

static int getPage(sh_font_data* data, int charId) {
	int page = 0;
	if (data->boffset[0] != 0) {
		for (page = 0; page < 8; page++) {
			if (data->boffset[page] != 0 && data->boffset[page] <= charId) {
				break;
			}
		}
		if (page > 7) {
			page = 0;
		}
		else {
			page++;
		}
	}
	return page;
}

static int getFirstCharId(sh_font_data* data, int startId, int max, int offset) {
	if (data != NULL && startId > -1) {
		for (int i = startId; i < max; i++) {
			if (data->offsetData[i + offset] != 0x00) {
				return i;
			}
		}
	}
	return -1;
}

static int getLastCharId(sh_font_data* data, int max, int offset) {
	if (data != NULL) {
		int firstId = getFirstCharId(data, 0x00, max, offset);
		if (firstId > -1) {
			int dataOffset = getCharOffset(data, firstId, offset);
			if (dataOffset > -1) {
				int maxIds = ((int)data + dataOffset - (int)data->offsetData) / 2;
				for (int i = maxIds - 1; i >= 0; i--) {
					if (data->offsetData[i + offset] != 0x00) {
						return i;
					}
				}
			}
		}
	}
	return -1;
}

static int getCharSize(sh_font_data* data, int charId, int offset, int max) {
	int startOffset = getCharOffset(data, charId, offset);
	if (startOffset == 0) {
		return 0;
	}
	else if (startOffset < 0) {
		cout << "\t Failed to get char offset: " << charId << endl;
		return -1;
	}

	int lastChar = getLastCharId(data, max, offset);
	int nextId = getFirstCharId(data, charId + 1, max, offset);
	int nextOffset = getCharOffset(data, nextId, offset);

	if (charId > lastChar) {
		cout << "\t Char id " << charId << " is higher than last char: " << lastChar << endl;
		return -1;
	}
	else if (charId == lastChar) {
		//find a way to get last char data size
		cout << "\t Failed to get last char data: lastChar " << lastChar << endl;
		return -1; //return minimal size?
	}

	if (startOffset > 0 && nextOffset > 0) {
		return nextOffset - startOffset;
	}
	return -1;
}

static bool updateCharOffset(sh_font_data* data, int charId, int newOffset, int offset, int max) {
	int lastChar = getLastCharId(data, max, offset);
	if (charId > lastChar || newOffset < 0) {
		return false;
	}
	if (newOffset >= 0x40000) {
		int page = (newOffset / 0x40000) - 1;
		if (tempPage[page] > charId) {
			//cout << "\tCurrent page " << page << " = " << tempPage[page] << " update to " << charId << " new offset " << newOffset << endl;
			tempPage[page] = charId;
		}
		data->offsetData[charId + offset] = (newOffset - 0x40000 * (page + 1)) / 4;
	}
	else {
		int page = getPage(data, charId) - 1;
		if (page > -1 && tempPage[page] <= charId) {
			//cout << "\tCurrent page " << page << " = " << tempPage[page] << " remove " << charId << " new offset " << newOffset << endl;
			if (newOffset > 0) {
				tempPage[page] = charId + 1;
			}
		}
		data->offsetData[charId + offset] = newOffset / 4;
	}
	return true;
}
