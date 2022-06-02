#pragma once

#include "font.hpp"

int getCharOffset(sh_font_data* data, int charId, int offset);
bool insertData(sh_font_data* data, int charId, uint8_t* charData, int size, int offset, int max);
