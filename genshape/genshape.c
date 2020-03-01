// -*- coding: utf-8 -*-

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define local static
#define global static
#define internal static
#define persistent static
#define true 1u
#define false 0u
#define bool uint32_t
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE
#define sizeof_array(array) (sizeof(array)/sizeof(array[0]))

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
  wchar_t s_contour_counts[1024];
  wchar_t s_contour_points[1024];
  for (wchar_t* at = char_list; *at != 0; ++at) {
    wchar_t character = *at;
    fwprintf(c_source_file, L"//  '%s'\n", character);

    if (FT_Load_Glyph(face, character, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP)) {
      error = "ERROR\n";
      break;
    }
    FT_Outline outline = face->glyph->outline;

    wcscpy(s_contour_counts, L"");
    wcscpy(s_contour_points, L"");
    int start_point_i = 0;
    int end_point_i = 0;
    int total_point_count = 0;
    for (int c = 0; c < outline.n_contours; ++c) {
      int contour_point_count = 0;
      end_point_i = outline.contours[c];
      wcscat(s_contour_points, L"\t");
      for (int p = start_point_i; c < end_point_i; ++c) {
        FT_Vector point = outline.points[p];
        wchar_t str_buffer[256];
        swprintf(str_buffer, sizeof_array(str_buffer), L"{%0.1ff, %0.1ff, 1.f}, ", point.x/64.f, point.y/64.f);
        ++contour_point_count;
      }
    }
  }
  if (error) {
    printf(error);
    return 1;
  }
  return 0;
}




