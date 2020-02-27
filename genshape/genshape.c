// -*- coding: utf-8 -*-

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H

int main()
{
  char* font_name = "Px437_IBM_ISO8";
  int font_size_px = 16;
  int dpi = 72;
  wchar_t char_list[] = L"ABC";

  FILE* c_source_file = fopen("shape_data.c", "w");
  if (!c_source_file) {
    printf("ERROR\n");
    return 1;
  }
  fwprintf(c_source_file, L"// -*- coding: utf-8 -*-\n\n");
  fwprintf(c_source_file, L"#define FONT_NAME L\"%s\"\n\n", font_name);

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
  printf("%s.units_per_EM=%d\n", font_name, face->units_per_EM);

  char* error = 0;
  for (wchar_t* at = char_list; at != 0; ++at) {
    wchar_t ch = *at;
    if (FT_Load_Glyph(face, ch, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP)) {
      error = "ERROR\n";
      break;
    }
    FT_Outline outline = face->glyph->outline;
  }
  if (error) {
    printf(error);
    return 1;
  }
  return 0;
}




