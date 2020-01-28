#!/usr/bin/python3
# -*- coding: utf-8 -*-
import math
from shapely.geometry import Polygon
from freetype import Face, \
       FT_LOAD_DEFAULT, FT_LOAD_NO_BITMAP

# sudo -H pip3 install shapely freetype-py fonttools numpy

FONT_NAME = "Px437_CompaqThin_8x16"
FONT_SIZE = 14
CHAR_LIST = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "
CHAR_LIST += "~!@#$%^&*()_+-={}|:\"<>?`[]\\;',./"
CHAR_LIST += "▲♠♣★"

def escape_character(character):
  escapable_chars = ("\\", "\'", )
  if character in escapable_chars:
    return "\\" + character
  return character

c_source_file = open("font_shapes.c", "w")
c_source_file.write("#define FONT_NAME L\"%s\"\n\n" % FONT_NAME)

face = Face("fonts/%s.ttf" % FONT_NAME)
face.set_char_size(FONT_SIZE*FONT_SIZE)

shape_id = 0
for character in CHAR_LIST:
  c_source_file.write("//  '%s'\n" % character)
  face.load_char(character, FT_LOAD_DEFAULT|FT_LOAD_NO_BITMAP)
  outline = face.glyph.outline

  s_contour_counts = ""
  s_contour_points = ""
  start_point_i = 0
  end_point_i = 0
  total_point_count = 0
  for c in outline.contours:
    contour_point_count = 0
    end_point_i = c
    s_contour_points += "\t"
    for p in range(start_point_i, end_point_i+1):
      x, y = outline.points[p]
      s_contour_points += "{%0.1ff, %0.1ff, 1.f}, " % (x, y)
      contour_point_count += 1
    total_point_count += contour_point_count
    s_contour_points += "\n\n"
    s_contour_counts += "%d, " % contour_point_count
    start_point_i = end_point_i+1
  c_source_file.write("#define shape_%s_n_contours %d\n" % (shape_id, len(outline.contours)))
  c_source_file.write("int shape_%s_contours[] = {%s};\n" % (shape_id, s_contour_counts))
  c_source_file.write("Point shape_%s_points[] = {\n%s};\n" % (shape_id, s_contour_points))
  c_source_file.write("#define shape_%s_total_point_count %d\n" % (shape_id, total_point_count))
  c_source_file.write("\n")
  shape_id += 1

s_font_data = ""
for shape_id in range(0, len(CHAR_LIST)):
  s_font_data += "\t{\n"
  s_font_data += "\t.character=L'%s',\n" % escape_character(CHAR_LIST[shape_id])
  s_font_data += "\t.contours=shape_%d_contours,\n" % shape_id
  s_font_data += "\t.n_contours=shape_%d_n_contours,\n" % shape_id
  s_font_data += "\t.points=shape_%d_points,\n" % shape_id
  s_font_data += "\t.total_point_count=shape_%d_total_point_count,\n" % shape_id
  s_font_data += "\t}, \n"
c_source_file.write("Shape font_shapes[] = {\n%s};\n" % s_font_data)

