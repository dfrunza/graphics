// -*- coding: utf-8 -*-
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

#define local static
#define global static
#define internal static
#define true 1u
#define false 0u
#define bool uint32_t
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE
#define PI 3.141592f

#define sizeof_array(array) (sizeof(array)/sizeof(array[0]))

#define assert(EXPR) do { if(!(EXPR)) assert_(#EXPR, __FILE__, __LINE__); } while(0)
void assert_(char* message, char* file, int line)
{
  printf("%s:%d: ", file, line);
  if(!message || message[0] == '\0')
  {
    message = "";
  }
  printf("assert(%s)\n", message);
  *(int*)0 = 0;
}

typedef struct {
  uint32_t R;
  uint32_t G;
  uint32_t B;
} Color;

typedef struct {
  float x;
  float y;
  float z;
} Vector3, Point;

typedef struct {
  int x;
  int y;
  int z;
} iPoint;

typedef struct {
  Point lower_left;
  Point upper_right;
} Rectangle;

typedef struct {
  wchar_t character;
  int* contours;
  int n_contours;
  Point* points;
} Shape;

typedef struct {
  Point** contours;
  int* contour_vertex_count;
  int total_vertex_count;
  int n_contours;
} Polygon;

typedef struct Edge {
  int x_intercept;
  int x_accumulator;
  union {
    struct {
      int x0;
      int y0;
      int z0;
    };
    iPoint start_point;
  };
  union {
    struct {
      int x1;
      int y1;
      int z1;
    };
    iPoint end_point;
  };
  union {
    struct {
      int delta_x;
      int delta_y;
    };
    struct {
      struct Edge* prev_edge;
      struct Edge* next_edge;
    };
  };
} Edge;

typedef struct {
  Edge* entries;
  int count;
} EdgeList;

typedef struct {
  uint8_t* memory;
  uint8_t* avail;
} Arena;

typedef struct {
  union {
    struct {
      float x1;
      float y1;
      float z1;
    };
    Vector3 col1;
  };
  union {
    struct {
      float x2;
      float y2;
      float z2;
    };
    Vector3 col2;
  };
  union {
    struct {
      float x3;
      float y3;
      float z3;
    };
    Vector3 col3;
  };
} Matrix3;

typedef enum {
  ClipEdge_Left,
  ClipEdge_Right,
  ClipEdge_Bottom,
  ClipEdge_Top,

  ClipEdge_COUNT,
} ClippingEdge;

typedef struct {
  Point** buffer_half_A;
  Point** one_past_last_A;
  Point** at_A;
  Point** buffer_half_B;
  Point** one_past_last_B;
  Point** buffer_end;
} CandidateClipVertices;

#define abs(i) \
  ((i) > 0 ? (i) : -(i))

uint8_t*
push_object(Arena* arena, size_t block_size) {
  void* object = arena->avail;
  arena->avail += block_size + 1*KILOBYTE;
  return object;
}

#define push_struct(type) \
  (type*) push_object(&arena, sizeof(type))

#define push_array(type, count) \
  (type*) push_object(&arena, sizeof(type)*(count))

global Arena arena;
global xcb_image_t* image;

global Color COLOR_RED = {.R=255, .G=0, .B=0};
global Color COLOR_GREEN = {.R=255, .G=0, .B=0};
global Color COLOR_BLUE = {.R=0, .G=0, .B=255};
global Color WHITE = {.R=255, .G=255, .B=255};
global Color BLACK = {.R=0, .G=0, .B=0};

iPoint
convert_point_ftoi(Point* f_pt) {
  iPoint i_pt = {};
  i_pt.x = f_pt->x;
  i_pt.y = f_pt->y;
  i_pt.z = f_pt->z;
  return i_pt;
}

Vector3
matrix3_row1(Matrix3* M) {
  Vector3 row = {};
  row.x = M->x1;
  row.y = M->x2;
  row.z = M->x3;
  return row;
}

Vector3
matrix3_row2(Matrix3* M) {
  Vector3 row = {};
  row.x = M->y1;
  row.y = M->y2;
  row.z = M->y3;
  return row;
}

Vector3
matrix3_row3(Matrix3* M) {
  Vector3 row = {};
  row.x = M->z1;
  row.y = M->z2;
  row.z = M->z3;
  return row;
}

float
vector3_dot_product(Vector3* A, Vector3* B) {
  return A->x*B->x + A->y*B->y + A->z*B->z;
}

Vector3
vector3_mul_matrix3(Vector3* V, Matrix3* M) {
  Vector3 result = {};

  Vector3 row_M = matrix3_row1(M);
  result.x = vector3_dot_product(V, &row_M);
  row_M = matrix3_row2(M);
  result.y = vector3_dot_product(V, &row_M);
  row_M = matrix3_row3(M);
  result.z = vector3_dot_product(V, &row_M);

  return result;
}

Matrix3
matrix3_mul(Matrix3* A, Matrix3* B) {
  Matrix3 result = {};
  
  Vector3 row_A = matrix3_row1(A);
  result.x1 = vector3_dot_product(&row_A, &B->col1);
  result.x2 = vector3_dot_product(&row_A, &B->col2);
  result.x3 = vector3_dot_product(&row_A, &B->col3);

  row_A = matrix3_row2(A);
  result.y1 = vector3_dot_product(&row_A, &B->col1);
  result.y2 = vector3_dot_product(&row_A, &B->col2);
  result.y3 = vector3_dot_product(&row_A, &B->col3);

  row_A = matrix3_row3(A);
  result.z1 = vector3_dot_product(&row_A, &B->col1);
  result.z2 = vector3_dot_product(&row_A, &B->col2);
  result.z3 = vector3_dot_product(&row_A, &B->col3);

  return result;
}

void
apply_xform(Shape* shape, Matrix3* T) {
  Point* pt = shape->points;
  for (int i = 0; i < shape->n_contours; ++i) {
    for (int j = 0; j < shape->contours[i]; ++j) {
      *pt = vector3_mul_matrix3(pt, T);
      ++pt;
    }
  }
}

void
translate(Matrix3* T, float t_x, float t_y) {
  Vector3* col = &T->col1;
  col->x = 1.f;
  col->y = 0.f;
  col->z = 0.f;

  col = &T->col2;
  col->x = 0.f;
  col->y = 1.f;
  col->z = 0.f;

  col = &T->col3;
  col->x = t_x;
  col->y = t_y;
  col->z = 1.f;
}

void
rotate(Matrix3* T, float rotation_angle) {
  float sin_phi = sinf(rotation_angle);
  float cos_phi = cosf(rotation_angle);

  Vector3* col = &T->col1;
  col->x = cos_phi;
  col->y = sin_phi;
  col->z = 0.f;

  col = &T->col2;
  col->x = -sin_phi;
  col->y = cos_phi;
  col->z = 0.f;

  col = &T->col3;
  col->x = 0.f;
  col->y = 0.f;
  col->z = 1.f;
}

void
rotate_pivot(Matrix3* T, float rotation_angle, Point* pivot_pt) {
  float sin_phi = sinf(rotation_angle);
  float cos_phi = cosf(rotation_angle);

  Vector3* col = &T->col1;
  col->x = cos_phi;
  col->y = sin_phi;
  col->z = 0.f;

  col = &T->col2;
  col->x = -sin_phi;
  col->y = cos_phi;
  col->z = 0.f;

  col = &T->col3;
  col->x = pivot_pt->x*(1.f - cos_phi) + pivot_pt->y*sin_phi;
  col->y = pivot_pt->y*(1.f - cos_phi) + pivot_pt->x*sin_phi;
  col->z = 1.f;
}

void
scale(Matrix3* T, float s_x, float s_y) {
  Vector3* col = &T->col1;
  col->x = s_x;
  col->y = 0.f;
  col->z = 0.f;

  col = &T->col2;
  col->x = 0.f;
  col->y = s_y;
  col->z = 0.f;

  col = &T->col3;
  col->x = 0.f;
  col->y = 0.f;
  col->z = 1.f;
}

void
scale_pivot(Matrix3* T, float s_x, float s_y, Point* pivot_pt) {
  Vector3* col = &T->col1;
  col->x = s_x;
  col->y = 0.f;
  col->z = 0.f;

  col = &T->col2;
  col->x = 0.f;
  col->y = s_y;
  col->z = 0.f;

  col = &T->col3;
  col->x = pivot_pt->x*(1.f - s_x);
  col->y = pivot_pt->y*(1.f - s_y);
  col->z = 1.f;
}

uint32_t
make_grayscale_rgb32(uint8_t intensity) {
  uint32_t value = intensity;
  return value | value << 8 | value << 16;
}

void
draw_pixel_black(int x, int y) {
  uint32_t* p = (uint32_t*)(image->data + (image->width*4)*y + 4*x);
  *p = make_grayscale_rgb32(0);
}

void
draw_pixel_rgb(int x, int y, uint8_t R, uint8_t G, uint8_t B) {
  uint32_t* p = (uint32_t*)(image->data + (image->width*4)*y + 4*x);
  *p = ((uint32_t)B | (uint32_t)G << 8 | (uint32_t)R << 16);
}

void
draw_pixel_red(int x, int y) {
  draw_pixel_rgb(x, y, COLOR_RED.R, COLOR_RED.G, COLOR_RED.B);
}

void
draw_pixel_green(int x, int y) {
  draw_pixel_rgb(x, y, COLOR_GREEN.R, COLOR_GREEN.G, COLOR_GREEN.B);
}

void
draw_pixel_blue(int x, int y) {
  draw_pixel_rgb(x, y, COLOR_BLUE.R, COLOR_BLUE.G, COLOR_BLUE.B);
}

bool
compare_edge_is_less(Edge* edge_A, Edge* edge_B) {
  return edge_A->y0 < edge_B->y0;
}

Edge
pop_polygon_edge(EdgeList* heap) {
  assert(heap->count > 0);
  Edge next_edge = heap->entries[1];
  heap->entries[1] = heap->entries[heap->count--];
  int pos = 1;
  while (true) {
    int left_child = 2*pos;
    int right_child = 2*pos+1;
    int min_child = left_child;
    if (left_child > heap->count) {
      break;
    }
    if (right_child <= heap->count && compare_edge_is_less(&heap->entries[right_child], &heap->entries[min_child])) {
      min_child = right_child;
    }
    if (compare_edge_is_less(&heap->entries[min_child], &heap->entries[pos])) {
      Edge v = heap->entries[pos];
      heap->entries[pos] = heap->entries[min_child];
      heap->entries[min_child] = v;
      pos = min_child;
    } else {
      break;
    }
  }
  return next_edge;
}

void
add_polygon_edge(EdgeList* heap, Edge* edge) {
  heap->entries[++heap->count] = *edge;
  int pos = heap->count;
  while (pos > 1) {
    int parent = pos/2;
    if (compare_edge_is_less(&heap->entries[pos], &heap->entries[parent])) {
      Edge v = heap->entries[parent];
      heap->entries[parent] = heap->entries[pos];
      heap->entries[pos] = v;
      pos = parent;
    } else {
      break;
    }
  }
}

void
insert_active_edge(EdgeList* list, Edge* edge) {
  int i = 0;
  while (edge->x_intercept >= list->entries[i].x_intercept) {
    ++i;
  }
  Edge swap_edge = list->entries[i];
  list->entries[i] = *edge;
  ++list->count;
  for (int j = i+1; j <= list->count; ++j) {
    Edge t = list->entries[j];
    list->entries[j] = swap_edge;
    swap_edge = t;
  }
}

void
remove_active_edge(EdgeList* list, Edge* edge, int i) {
  assert(i < list->count);
  for (int j = i; j < list->count; ++j) {
    list->entries[j] = list->entries[j+1];
  }
  --list->count;
}

void
sort_active_edge_list(EdgeList* list) {
  for (int i = 0; i < list->count; ++i) {
    for (int j = i; list->entries[j].x_intercept < list->entries[j-1].x_intercept; --j) {
      Edge t = list->entries[j];
      list->entries[j] = list->entries[j-1];
      list->entries[j-1] = t;
    }
  }
}

#include "font_shapes.c"

Shape* find_shape(wchar_t character) {
  Shape* result = 0;
  for (int i = 0; i < sizeof_array(font_shapes); ++i) {
    if (font_shapes[i].character == character) {
      result = &font_shapes[i];
      break;
    }
  }
  return result;
}

void
make_polygon(Polygon* polygon, Shape* shape) {
  static int MAX_VERTEX_COUNT = 100;

  polygon->contour_vertex_count = push_array(int, shape->n_contours);
  polygon->n_contours = shape->n_contours;
  polygon->contours = push_array(Point*, shape->n_contours);
  for (int i = 0; i < shape->n_contours; ++i) {
    polygon->contour_vertex_count[i] = 0;
    polygon->contours[i] = push_array(Point, shape->contours[i]+3);
    polygon->contours[i] += 1;
    Point* p;

    int j = 0; 
    for (; j < shape->contours[i]; ++j) {
      polygon->contours[i][j] = shape->points[j];
    }

    polygon->contour_vertex_count[i] = j;
    polygon->total_vertex_count += polygon->contour_vertex_count[i];
    polygon->contours[i][-1] = polygon->contours[i][polygon->contour_vertex_count[i]-1];
    polygon->contours[i][polygon->contour_vertex_count[i]] = polygon->contours[i][0];
    polygon->contours[i][polygon->contour_vertex_count[i]+1] = polygon->contours[i][1];

    shape->points += shape->contours[i];
  }
}

void
update_x_intercept(Edge* edge) {
  edge->x_accumulator += edge->delta_x;
  int x_increment = edge->x_accumulator / edge->delta_y;
  if (abs(edge->x_accumulator) >= abs(edge->delta_y)) {
    edge->x_intercept += x_increment;
    edge->x_accumulator -= x_increment * edge->delta_y;
  }
}

void
print_edge_list(EdgeList* edge_list) {
  for (int i = 0; i < edge_list->count; ++i) {
    Edge* edge = &edge_list->entries[i];
    printf("((x0=%d, y0=%d), (x1=%d,y1=%d), x_intercept=%d)\n",
           edge->x0, edge->y0, edge->x1, edge->y1, edge->x_intercept);
  }
}

EdgeList
new_empty_edge_list() {
  EdgeList object = {};
  return object;
}

Edge
new_empty_edge() {
  Edge object = {};
  return object;
}

Polygon
new_empty_polygon() {
  Polygon object = {};
  return object;
}

void
fill_polygon(Polygon* polygon) {
  assert(polygon->total_vertex_count >= 3);
  EdgeList* edge_list = push_array(EdgeList, polygon->n_contours);
  for (int i = 0; i < polygon->n_contours; ++i) {
    edge_list[i] = new_empty_edge_list();
    edge_list[i].count = 0;
    edge_list[i].entries = push_array(Edge, polygon->contour_vertex_count[i]+1);
    edge_list[i].entries += 1;
    edge_list[i].entries[-1] = new_empty_edge();
    edge_list[i].entries[-1].next_edge = &edge_list[i].entries[0];
    edge_list[i].entries[-1].prev_edge = &edge_list[i].entries[0];

    Edge* next_edge = &edge_list[i].entries[-1];
    Edge* prev_edge = next_edge;
    for (int j = 0; j < polygon->contour_vertex_count[i]; ++j) {
      Edge* edge = &edge_list[i].entries[edge_list[i].count];
      edge->start_point = convert_point_ftoi(&polygon->contours[i][j]);
      edge->end_point = convert_point_ftoi(&polygon->contours[i][j+1]);
      if (edge->y1 == edge->y0) {
        continue;
      }
      edge->prev_edge = prev_edge;
      edge->next_edge = next_edge;
      next_edge->prev_edge = edge;
      prev_edge->next_edge = edge;
      prev_edge = edge;
      ++edge_list[i].count;
    }
    edge_list[i].entries[0].prev_edge = &edge_list[i].entries[edge_list[i].count-1];
    edge_list[i].entries[edge_list[i].count-1].next_edge = &edge_list[i].entries[0];

    //printf("Contour #%d\n", i);
    //print_edge_list(&edge_list[i]);
  }

  for (int i = 0; i < polygon->n_contours; ++i) {
    for (int j = 0; j < edge_list[i].count; ++j) {
      Edge* edge = &edge_list[i].entries[j];
      Edge* prev_edge = edge->prev_edge;
      Edge* next_edge = edge->next_edge;
      if (edge->y1 > edge->y0) {
        edge->x_intercept = edge->x0;
        if (next_edge->y1 > edge->y1) {
          edge->y1 -= 1.f;
        }
      }
      else if (edge->y1 < edge->y0) {
        edge->x_intercept = edge->x1;
        if (prev_edge->y0 > edge->y0) {
          prev_edge->y1 += 1.f;
        }
      }
      else assert(false);
    }
    //printf("Contour #%d\n", i);
    //print_edge_list(&edge_list[i]);
  }

  EdgeList edge_heap = new_empty_edge_list();
  edge_heap.entries = push_array(Edge, polygon->total_vertex_count);
  edge_heap.count = 0;
  edge_heap.entries[0] = new_empty_edge();
  edge_heap.entries[0].y0 = INT_MIN;
  for (int i = 0; i < polygon->n_contours; ++i) {
    for (int j = 0; j < edge_list[i].count; ++j) {
      Edge* edge = &edge_list[i].entries[j];
      assert (edge->y1 != edge->y0);
      if (edge->y1 < edge->y0) {
        iPoint p = edge->start_point;
        edge->start_point = edge->end_point;
        edge->end_point = p;
      }
      edge->delta_x = edge->x1 - edge->x0;
      edge->delta_y = edge->y1 - edge->y0;
      assert(edge->delta_y != 0);
      edge->x_intercept = edge->x0;
      add_polygon_edge(&edge_heap, edge);
    }
  }

  EdgeList active_edge_list = {};
  active_edge_list.entries = push_array(Edge, edge_heap.count + 2);
  active_edge_list.entries += 1;
  active_edge_list.count = 0;
  active_edge_list.entries[-1] = new_empty_edge();
  active_edge_list.entries[-1].x_intercept = INT_MIN;
  active_edge_list.entries[0] = new_empty_edge();
  active_edge_list.entries[0].x_intercept = INT_MAX;

  assert(edge_heap.count >= 2);
  int y = edge_heap.entries[1].y0;
  do {
    //printf("--------------- %d -----------------\n", y);
    for (int i = 0; i < active_edge_list.count; ++i) {
      Edge* edge = &active_edge_list.entries[i];
      update_x_intercept(edge);
    }

    sort_active_edge_list(&active_edge_list);
    print_edge_list(&active_edge_list);

    assert((active_edge_list.count % 2) == 0);
    for (int i = 0; i < active_edge_list.count; i += 2) {
      Edge* left_edge = &active_edge_list.entries[i];
      Edge* right_edge = &active_edge_list.entries[i+1];
      //printf("left_edge=(%d,%d)\n", left_edge->x_intercept, y);
      //printf("right_edge=(%d,%d)\n", right_edge->x_intercept, y);
      draw_pixel_black(left_edge->x_intercept, y);
      for (int x = left_edge->x_intercept; x < right_edge->x_intercept; ++x) {
        draw_pixel_black(x, y);
      }
    }
    for (int i = 0; i < active_edge_list.count;) {
      Edge* edge = &active_edge_list.entries[i];
      if (edge->y1 <= y) {
        remove_active_edge(&active_edge_list, edge, i);
        continue;
      }
      ++i;
    }
    ++y;
    while (edge_heap.count > 0 && edge_heap.entries[1].y0 <= y) {
      Edge edge = pop_polygon_edge(&edge_heap);
      edge.x_accumulator = 0;
      insert_active_edge(&active_edge_list, &edge);
    }
  } while (active_edge_list.count > 0);
}


static xcb_format_t*
find_format(xcb_connection_t* conn, uint8_t depth, uint8_t bpp) {
  const xcb_setup_t* setup = xcb_get_setup(conn);
  xcb_format_t* fmt = xcb_setup_pixmap_formats(setup);
  xcb_format_t* fmtend = fmt + xcb_setup_pixmap_formats_length(setup);
  for(; fmt != fmtend; ++fmt) {
    if((fmt->depth == depth) && (fmt->bits_per_pixel == bpp)) {
      //printf("fmt %p has pad %d depth %d, bpp %d\n", fmt, fmt->scanline_pad, depth, bpp);
      return fmt;
    }
  }
  return 0;
}

void
fill_image(uint8_t intensity) {
  int i, j;
  uint32_t* p = (uint32_t*)image->data;
  for (int j = 0; j < image->height; ++j) {
    for (int i = 0; i < image->width; ++i) {
      *p++ = make_grayscale_rgb32(intensity);
    }
  }
}

xcb_image_t*
create_image(xcb_connection_t* conn, int width, int height) {
  const xcb_setup_t* setup = xcb_get_setup(conn);
  uint8_t* image_data = push_array(uint8_t, width*height*4);
  xcb_format_t* fmt = find_format(conn, 24, 32);
  if (fmt == NULL)
    return NULL;

  return xcb_image_create(width, height,
                          XCB_IMAGE_FORMAT_Z_PIXMAP,
                          fmt->scanline_pad,
                          fmt->depth,
                          fmt->bits_per_pixel,
                          0, setup->image_byte_order,
                          XCB_IMAGE_ORDER_LSB_FIRST,
                          image_data, width*height*4, image_data);
}

void
line(int x0, int y0, int x1, int y1, Color* color) {
  int abs_dx = abs(x1 - x0);
  int abs_dy = abs(y1 - y0);

  if (abs_dy == 0) {
    int y = y0;
    int x = x0;
    int x_end = x1;
    if (x0 > x1) {
      x = x1;
      x_end = x0;
    }
    for (; x <= x_end; ++x) {
      draw_pixel_rgb(x, y, color->R, color->G, color->B);
    }
  }
  else 
  {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int p = 2*abs_dy - abs_dx;
    int y = y0;
    int x = x0;
    int x_end = x1;
    int y_incr = (dy < 0) ? -1 : 1;
    if (dx < 0) {
      y = y1;
      x = x1;
      x_end = x0;
      y_incr = (dy > 0) ? -1 : 1;
    }
    int *p_x = &x;
    int *p_y = &y;

    if (abs_dy > abs_dx) {
      dx = y1 - y0;
      dy = x1 - x0;
      abs_dx = abs(dx);
      abs_dy = abs(dy);
      p = 2*abs_dy - abs_dx;
      y = x0;
      x = y0;
      x_end = y1;
      y_incr = (dy < 0) ? -1 : 1;
      if (dx < 0) {
        y = x1;
        x = y1;
        x_end = y0;
        y_incr = (dy > 0) ? -1 : 1;
      }
      p_x = &y;
      p_y = &x;
    }

    for (; x <= x_end; ++x) {
      draw_pixel_rgb(*p_x, *p_y, color->R, color->G, color->B);
      if (p < 0) {
        p += 2*abs_dy;
      } else {
        y += y_incr;
        p += 2*abs_dy - 2*abs_dx;
      }
    }
  }
}

void
line_black(int x0, int y0, int x1, int y1) {
  Color black = {.R=0, .G=0, .B=0};
  line(x0, y0, x1, y1, &black);
}

void
circle(int center_x, int center_y, int radius) {
  int x = 0;
  int y = radius;
  int p = 1 - radius;
  while (x <= y) {
    draw_pixel_black(center_x+x, center_y+y);
    draw_pixel_black(center_x+x, center_y-y);
    draw_pixel_black(center_x-x, center_y+y);
    draw_pixel_black(center_x-x, center_y-y);
    draw_pixel_black(center_y+y, center_x+x);
    draw_pixel_black(center_y+y, center_x-x);
    draw_pixel_black(center_y-y, center_x+x);
    draw_pixel_black(center_y-y, center_x-x);

    ++x;
    if (p < 0) {
      p = p + 2*x + 1;
    } else {
      --y;
      p = p + 2*x + 1 - 2*y;
    }
  }
}

void
ellipse(int center_x, int center_y, int radius_x, int radius_y) {
  int x = 0;
  int y = radius_y;
  int radius_y_squared = radius_y*radius_y;
  int radius_x_squared = radius_x*radius_x;
  
  int p1 = radius_y_squared - radius_x_squared*radius_y + 0.25*radius_x_squared;
  while (2*radius_y_squared*x < 2*radius_x_squared*y) {
    draw_pixel_black(center_x+x, center_y+y);
    draw_pixel_black(center_x+x, center_y-y);
    draw_pixel_black(center_x-x, center_y+y);
    draw_pixel_black(center_x-x, center_y-y);

    ++x;
    if (p1 < 0) {
      p1 += 2*radius_y_squared*x + radius_y_squared;
    } else {
      --y;
      p1 += 2*radius_y_squared*x - 2*radius_x_squared*y + radius_y_squared;
    }
  }

  int p2 = radius_y_squared*(x+0.5)*(x+0.5) + radius_x_squared*(y-1)*(y-1) - radius_x_squared*radius_y_squared;
  while (y >= 0) {
    draw_pixel_black(center_x+x, center_y+y);
    draw_pixel_black(center_x+x, center_y-y);
    draw_pixel_black(center_x-x, center_y+y);
    draw_pixel_black(center_x-x, center_y-y);

    --y;
    if (p2 > 0) {
      p2 += -2*radius_x_squared*y + radius_x_squared;
    } else {
      ++x;
      p2 += 2*radius_y_squared*x - 2*radius_x_squared*y + radius_x_squared;
    }
  }
}

Rectangle
get_bounding_box(Shape* shape) {
  Rectangle bb = {};
  Point* pt = shape->points;
  bb.lower_left = *pt;
  bb.upper_right = *pt;
  for (int i = 0; i < shape->n_contours; ++i) {
    for (int j = 0; j < shape->contours[i]; ++j) {
      if (bb.lower_left.x > pt->x) {
        bb.lower_left.x = pt->x;
      }
      if (bb.lower_left.y > pt->y) {
        bb.lower_left.y = pt->y;
      }
      if (bb.upper_right.x < pt->x) {
        bb.upper_right.x = pt->x;
      }
      if (bb.upper_right.y < pt->y) {
        bb.upper_right.y = pt->y;
      }
      bb.lower_left.z = bb.upper_right.z = 1.f;
      ++pt;
    }
  }
  return bb;
}

void
vertical_line_intersection(Point* p0, Point* p1, Point* result, int x) {
  assert(p1->x != p0->x);
  float m = (p1->y - p0->y)/(float)(p1->x - p0->x);
  result->y = p1->y + m*(x - p1->x);
  result->x = x;
}

void
horizontal_line_intersection(Point* p0, Point* p1, Point* result, int y) {
  assert(p1->x != p0->x);
  float m = (p1->y - p0->y)/(float)(p1->x - p0->x);
  result->x = p1->x + (y - p1->y)/m;
  result->y = y;
}

void
init_candidate_clip_vertces(CandidateClipVertices* ccv, int max_vertex_count) {
  ccv->buffer_half_A = push_array(Point*, max_vertex_count);
  ccv->buffer_half_B = push_array(Point*, max_vertex_count);
  ccv->buffer_end = ccv->buffer_half_B + max_vertex_count;
  ccv->at_A = ccv->buffer_end;
  ccv->one_past_last_A = ccv->buffer_half_A;
  ccv->one_past_last_B = ccv->buffer_half_B;
}

Point*
next_candidate_clip_vertex(CandidateClipVertices* ccv) {
  Point* result = 0;
  if (ccv->at_A < ccv->one_past_last_A) {
    result = *ccv->at_A;
    ++ccv->at_A;
  }
  return result;
}

void
add_candidate_clip_vertex(CandidateClipVertices* ccv, Point* v) {
  assert(ccv->one_past_last_B < ccv->buffer_end);
  *ccv->one_past_last_B = v;
  ++ccv->one_past_last_B;
}

void
candidate_clip_swap_buffers(CandidateClipVertices* ccv) {
  Point** t = ccv->buffer_half_B;
  ccv->buffer_half_B = ccv->buffer_half_A;
  ccv->buffer_half_A = t;

  ccv->one_past_last_A = ccv->one_past_last_B;
  ccv->at_A = ccv->buffer_half_A;
  ccv->one_past_last_B = ccv->buffer_half_B;
}

void
draw_figure() {
#if 0
  line_black(245, 128, 371, 128);
  line_black(371, 128, 371, 0);
  line_black(371, 0, 245, 0);
  line_black(245, 0, 245, 128);

  // Line drawing tests
//  line_black(20, 10, 30, 18);
//  line_black(20, 10, 30, 28);
//  line_black(20, 18, 30, 10);
//  line_black(20, 28, 30, 10);
//  line_black(30, 10, 20, 10);
//  line_black(20, 10, 20, 18);

  // Intersection with a vertical line
//  int x0 = 10;
//  int y0 = 10;
//  int x1 = 50;
//  int y1 = 30;
//  int x = 27;
//  float m = (y1 - y0)/(float)(x1 - x0);
//  line_black(x0, y0, x1, y1);
//  line(x, 10, x, 30, &COLOR_BLUE);
//  int y = y1 + m*(x - x1);
//  draw_pixel_red(x, y);

  // Intersection with a horizontal line
//  int x0 = 10;
//  int y0 = 10;
//  int x1 = 50;
//  int y1 = 30;
//  int y = 27;
//  float m = (y1 - y0)/(float)(x1 - x0);
//  line_black(x0, y0, x1, y1);
//  line(10, y, 50, y, &COLOR_BLUE);
//  int x = x1 + (y - y1)/m;
//  draw_pixel_red(x, y);

#elif 1
  Shape* shape = find_shape(L'▲');
  Rectangle shape_bb = get_bounding_box(shape);
  printf("Bounding box: (%0.1f, %0.1f), (%0.1f, %0.1f)\n",
         shape_bb.lower_left.x, shape_bb.lower_left.y, shape_bb.upper_right.x, shape_bb.upper_right.y);
  Matrix3 w2vp_translate = {};
  translate(&w2vp_translate, -shape_bb.lower_left.x + 100, -shape_bb.lower_left.y + 100);
  Matrix3 w2vp_scale = {};
  scale_pivot(&w2vp_scale, 1.0f, 1.0f, &shape_bb.lower_left);
  Matrix3 w2vp_xform = {};
  w2vp_xform = matrix3_mul(&w2vp_translate, &w2vp_scale);
  apply_xform(shape, &w2vp_xform);

  Polygon polygon = new_empty_polygon();
  make_polygon(&polygon, shape);

  int clipping_boundary[ClipEdge_COUNT] = {0};
  clipping_boundary[ClipEdge_Left] = 200;
  clipping_boundary[ClipEdge_Right] = 600;
  clipping_boundary[ClipEdge_Top] = 500;
  clipping_boundary[ClipEdge_Bottom] = 50;

  for (int i = 0; i < polygon.n_contours; ++i) {
    Point* contour = polygon.contours[i];
    int contour_vertex_count = polygon.contour_vertex_count[i];
    Point* clipped_contour = push_array(Point, contour_vertex_count*2);
    int clipped_vertex_count = 0;
    Point* recently_clipped[ClipEdge_COUNT] = {0};
    CandidateClipVertices candidate_vertices = {0};
    init_candidate_clip_vertces(&candidate_vertices, contour_vertex_count*2);
    for (int j = 0; j < contour_vertex_count; ++j) {
      add_candidate_clip_vertex(&candidate_vertices, &contour[j]);
      candidate_clip_swap_buffers(&candidate_vertices);

      Point* v = 0;
      while (v = next_candidate_clip_vertex(&candidate_vertices)) {
        if (v->x < clipping_boundary[ClipEdge_Left]) {
          recently_clipped[ClipEdge_Left] = v;
        }
        else {
          add_candidate_clip_vertex(&candidate_vertices, v);
          Point* recently_clipped_point = recently_clipped[ClipEdge_Left];
          if (recently_clipped_point) {
            Point* intersection_point = push_struct(Point);
            vertical_line_intersection(recently_clipped_point, v, intersection_point, clipping_boundary[ClipEdge_Left]);
            add_candidate_clip_vertex(&candidate_vertices, intersection_point);
          }
        }
      }
      candidate_clip_swap_buffers(&candidate_vertices);

      while (v = next_candidate_clip_vertex(&candidate_vertices)) {
        if (v->x > clipping_boundary[ClipEdge_Right]) {
          recently_clipped[ClipEdge_Right] = v;
        }
        else {
          add_candidate_clip_vertex(&candidate_vertices, v);
          Point* recently_clipped_point = recently_clipped[ClipEdge_Right];
          if (recently_clipped_point) {
            Point* intersection_point = push_struct(Point);
            vertical_line_intersection(recently_clipped_point, v, intersection_point, clipping_boundary[ClipEdge_Right]);
            add_candidate_clip_vertex(&candidate_vertices, intersection_point);
          }
        }
      }
      candidate_clip_swap_buffers(&candidate_vertices);

      while (v = next_candidate_clip_vertex(&candidate_vertices)) {
        if (v->y < clipping_boundary[ClipEdge_Bottom]) {
          recently_clipped[ClipEdge_Bottom] = v;
        }
        else {
          add_candidate_clip_vertex(&candidate_vertices, v);
          Point* recently_clipped_point = recently_clipped[ClipEdge_Bottom];
          if (recently_clipped_point) {
            Point* intersection_point = push_struct(Point);
            horizontal_line_intersection(recently_clipped_point, v, intersection_point, clipping_boundary[ClipEdge_Bottom]);
            add_candidate_clip_vertex(&candidate_vertices, intersection_point);
          }
        }
      }
      candidate_clip_swap_buffers(&candidate_vertices);

      while (v = next_candidate_clip_vertex(&candidate_vertices)) {
        if (v->y > clipping_boundary[ClipEdge_Top]) {
          recently_clipped[ClipEdge_Top] = v;
        }
        else {
          add_candidate_clip_vertex(&candidate_vertices, v);
          Point* recently_clipped_point = recently_clipped[ClipEdge_Top];
          if (recently_clipped_point) {
            Point* intersection_point = push_struct(Point);
            horizontal_line_intersection(recently_clipped_point, v, intersection_point, clipping_boundary[ClipEdge_Top]);
            add_candidate_clip_vertex(&candidate_vertices, intersection_point);
          }
        }
      }
      candidate_clip_swap_buffers(&candidate_vertices);

      while (v = next_candidate_clip_vertex(&candidate_vertices)) {
        clipped_contour[clipped_vertex_count++] = *v;
      }
    }

//    polygon.contours[i] = clipped_contour;
//    polygon.contour_vertex_count[i] = clipped_vertex_count;
//    assert(clipped_vertex_count > 1);
//    Point* p0 = &clipped_contour[0];
//    for (int i = 1; i < clipped_vertex_count; ++i) {
//      Point* p1 = &clipped_contour[i];
//      line(p0->x, p0->y, p1->x, p1->y, &COLOR_RED);
//      p0 = p1;
//    }
  }

  //fill_polygon(&polygon);
//  line(clipping_boundary[ClipEdge_Left], clipping_boundary[ClipEdge_Bottom],
//       clipping_boundary[ClipEdge_Left], clipping_boundary[ClipEdge_Top], &COLOR_BLUE);
//  line(clipping_boundary[ClipEdge_Right], clipping_boundary[ClipEdge_Top],
//       clipping_boundary[ClipEdge_Right], clipping_boundary[ClipEdge_Bottom], &COLOR_BLUE);
//  line(clipping_boundary[ClipEdge_Left], clipping_boundary[ClipEdge_Bottom],
//       clipping_boundary[ClipEdge_Right], clipping_boundary[ClipEdge_Bottom], &COLOR_BLUE);
//  line(clipping_boundary[ClipEdge_Left], clipping_boundary[ClipEdge_Top],
//       clipping_boundary[ClipEdge_Right], clipping_boundary[ClipEdge_Top], &COLOR_BLUE);
#endif
}

#if 1
int
main(int argc, char** argv) {
  uint32_t values[2];

  arena.memory = malloc(10*MEGABYTE);
  if (!arena.memory) {
    printf("ERROR\n");
    exit(1);
  }
  arena.avail = arena.memory;

  xcb_connection_t* conn = xcb_connect(NULL, NULL);
  if (!conn) {
    printf("ERROR\n");
    exit(1);
  }

  const xcb_setup_t* setup = xcb_get_setup(conn);
  xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;
  printf("root depth %d\n",screen->root_depth);

  image = create_image(conn, 1000, 900);
  if (image == NULL) {
    printf("ERROR\n");
    xcb_disconnect(conn);
    return 1;
  }

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_BUTTON_PRESS;

  xcb_window_t window = xcb_generate_id(conn);
  xcb_create_window(conn, 24/*XCB_COPY_FROM_PARENT*/, window, screen->root,
                    10, 10, image->width, image->height, 1,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual,
                    mask, values);

  char* title = "Drawing";
  xcb_icccm_set_wm_name(conn, window, XCB_ATOM_STRING, 8, strlen(title), title);

  xcb_pixmap_t pixmap = xcb_generate_id(conn);
  xcb_create_pixmap(conn, 24, pixmap, window, image->width, image->height);

  /* Create pixmap plot gc */
  mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
  values[0] = screen->black_pixel;
  values[1] = 0x00ffffff;

  xcb_gcontext_t gc = xcb_generate_id (conn);
  xcb_create_gc(conn, gc, pixmap, mask, values);

  /* Put the image into the pixmap */
  xcb_image_put(conn, pixmap, gc, image, 0, 0, 0);

  /* Show the window */
  xcb_map_window(conn, window);
  xcb_flush(conn);

  /* Set background */
  fill_image(255);
  draw_figure();

  xcb_image_put(conn, pixmap, gc, image, 0, 0, 0);
  xcb_copy_area(conn, pixmap, window, gc, 0, 0, 0, 0, image->width, image->height);
  xcb_flush(conn);

  xcb_generic_event_t* e;
  int done = false;
  while (!done && (e = xcb_wait_for_event(conn))) {
    switch (e->response_type) {
      case XCB_EXPOSE: {
        xcb_expose_event_t* ee = (xcb_expose_event_t*)e;
        //printf("expose %d,%d - %d,%d\n", ee->x, ee->y, ee->width, ee->height);
        xcb_copy_area(conn, pixmap, window, gc,
                      ee->x, ee->y,
                      ee->x, ee->y,
                      ee->width, ee->height);
        xcb_flush(conn);
        break;
      }

      case XCB_KEY_PRESS:
        done = true;
        break;

      case XCB_BUTTON_PRESS:
        break;
    }
    free(e);
  }

  xcb_free_pixmap(conn, pixmap);
  xcb_disconnect (conn);

  return 0;
}
#endif

