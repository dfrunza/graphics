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

char* codepoint_to_utf8(wchar_t c)
{
  persistent unsigned char utf8_buffer[8];
  char* b = utf8_buffer;
  if (c < 0x80) {
    *b++=c;
  }
  else if (c < 0x800) {
    *b++=192+c/64;
    *b++=128+c%64;
  }
  else if (c-0xd800u < 0x800) {
    goto error;
  }
  else if (c < 0x10000) {
    *b++=224+c/4096;
    *b++=128+c/64%64;
    *b++=128+c%64;
  }
  else if (c < 0x110000) {
    *b++=240+c/262144;
    *b++=128+c/4096%64;
    *b++=128+c/64%64;
    *b++=128+c%64;
  }
  else {
    goto error;
  }
error:
  *b = 0;
  return utf8_buffer;
}

int main()
{
  char* font_name = "Px437_IBM_ISO8";
  int font_size_px = 16;
  int dpi = 72;
  wchar_t char_list[] = L"AB▲■";
  int char_list_len = wcslen(char_list);

  FILE* c_source_file = fopen("shape_data.c", "w");
  if (!c_source_file) {
    printf("ERROR\n");
    return 1;
  }
  fprintf(c_source_file, "// -*- coding: utf-8 -*-\n\n");
  fprintf(c_source_file, "#define FONT_NAME \"%s\"\n\n", font_name);

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
  if (FT_Set_Char_Size(face, font_size_px*64, font_size_px*64, dpi, dpi)) {
    printf("ERROR\n");
    return 1;
  }
  printf("%s.units_per_EM=%d\n", font_name, face->units_per_EM);

  char* error = 0;
  char s_contour_counts[1*KILOBYTE];
  char s_contour_points[1*KILOBYTE];
  int shape_id = 0;
  for (wchar_t* at = char_list; *at != 0; ++at) {
    wchar_t character = *at;
    unsigned char* utf8_character = codepoint_to_utf8(character);
    fprintf(c_source_file, "//  '%s'\n", utf8_character);

    if (FT_Load_Char(face, character, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP)) {
      error = "ERROR\n";
      break;
    }
    FT_Outline outline = face->glyph->outline;

    strcpy(s_contour_counts, "");
    strcpy(s_contour_points, "");
    int start_point_i = 0;
    int end_point_i = 0;
    int total_point_count = 0;
    char temp_str_buffer[256];
    for (int c = 0; c < outline.n_contours; ++c) {
      int contour_point_count = 0;
      end_point_i = outline.contours[c];
      strcat(s_contour_points, "\t");
      for (int p = start_point_i; p <= end_point_i; ++p) {
        FT_Vector point = outline.points[p];
        sprintf(temp_str_buffer, "{%0.1ff, %0.1ff, 1.0f}, ", point.x/64.0f, point.y/64.0f);
        strcat(s_contour_points, temp_str_buffer);
        ++contour_point_count;
      }
      total_point_count += contour_point_count;
      strcat(s_contour_points, "\n\n");
      sprintf(temp_str_buffer, "%d, ", contour_point_count);
      strcat(s_contour_counts, temp_str_buffer);
      start_point_i = end_point_i+1;
    }
    fprintf(c_source_file, "#define shape_%d_n_contours %d\n", shape_id, outline.n_contours);
    fprintf(c_source_file, "int shape_%d_contours[] = {%s};\n", shape_id, s_contour_counts);
    fprintf(c_source_file, "Point shape_%d_points[] = {\n%s};\n", shape_id, s_contour_points);
    fprintf(c_source_file, "#define shape_%d_total_point_count %d\n", shape_id, total_point_count);
    fprintf(c_source_file, "\n");
    ++shape_id;
  }
  if (error) {
    printf(error);
    return 1;
  }
  char s_shape_data[1*KILOBYTE] = {0};
  char temp_str_buffer[1*KILOBYTE] = {0};
  for (int s = 0; s < char_list_len; ++s) {
    sprintf(temp_str_buffer, "\t{\n");
    strcat(s_shape_data, temp_str_buffer);

    unsigned char* utf8_character = codepoint_to_utf8(char_list[s]);
    if ((char_list[s] == L'\\') || (char_list[s] == '\'')) {
      sprintf(temp_str_buffer, "\t\t.character=L'\\%s',\n", utf8_character);
    } else {
      sprintf(temp_str_buffer, "\t\t.character=L'%s',\n", utf8_character);
    }
    strcat(s_shape_data, temp_str_buffer);

    sprintf(temp_str_buffer, "\t\t.contours=shape_%d_contours,\n", s);
    strcat(s_shape_data, temp_str_buffer);

    sprintf(temp_str_buffer, "\t\t.n_contours=shape_%d_n_contours,\n", s);
    strcat(s_shape_data, temp_str_buffer);

    sprintf(temp_str_buffer, "\t\t.points=shape_%d_points,\n", s);
    strcat(s_shape_data, temp_str_buffer);

    sprintf(temp_str_buffer, "\t\t.total_point_count=shape_%d_total_point_count,\n", s);
    strcat(s_shape_data, temp_str_buffer);

    sprintf(temp_str_buffer, "\t}, \n");
    strcat(s_shape_data, temp_str_buffer);
  }
  fprintf(c_source_file, "Shape shape_data[] = {\n%s};\n", s_shape_data);
  return 0;
}

