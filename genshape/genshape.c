// -*- coding: utf-8 -*-

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library  library;

int main()
{
  int rc = FT_Init_FreeType(&library);
  printf("rc=%d\n", rc);
  return 0;
}




