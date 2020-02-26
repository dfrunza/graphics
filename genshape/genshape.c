// -*- coding: utf-8 -*-

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

int main()
{
  char* font_name = "Px437_IBM_ISO8";
  int font_size_px = 16;
  int dpi = 72;

  FT_Library  library;
  if (FT_Init_FreeType(&library)) {
    printf("ERROR\n");
    return 1;
  }
  FT_Face face;
  char ttf_path[128];
  sprintf(ttf_path, "genshape/ttf/%s.ttf", font_name);
  if (FT_New_Face(library, ttf_path, 0, &face)) {
    printf("ERROR\n");
    return 1;
  }
  if (FT_Set_Char_Size(face, 0, font_size_px*64, dpi, dpi)) {
    printf("ERROR\n");
    return 1;
  }
  if (FT_Load_Glyph(face, 'A', FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP)) {
    printf("ERROR\n");
    return 1;
  }
  return 0;
}




