// -*- coding: utf-8 -*-

#define abs(i) \
  ((i) > 0 ? (i) : -(i))

#define fabs(f) \
  ((f) > 0.f ? (f) : -(f))

void* push_object(Arena* arena, size_t block_size) {
  void* object = arena->avail;
  arena->avail += block_size + 1*KILOBYTE;
  return object;
}

#define push_struct(type) \
  (type*) push_object(&arena, sizeof(type))

#define push_array(type, count) \
  (type*) push_object(&arena, sizeof(type)*(count))

fVector3 matrix3_row1(fMatrix3* M)
{
  fVector3 row = {};
  row.x = M->x1;
  row.y = M->x2;
  row.z = M->x3;
  return row;
}

fVector3 matrix3_row2(fMatrix3* M)
{
  fVector3 row = {};
  row.x = M->y1;
  row.y = M->y2;
  row.z = M->y3;
  return row;
}

fVector3 matrix3_row3(fMatrix3* M)
{
  fVector3 row = {};
  row.x = M->z1;
  row.y = M->z2;
  row.z = M->z3;
  return row;
}

float vector3_dot_product(fVector3* A, fVector3* B)
{
  return A->x*B->x + A->y*B->y + A->z*B->z;
}

fVector3 vector3_mul_matrix3(fVector3* V, fMatrix3* M)
{
  fVector3 result = {};

  fVector3 row_M = matrix3_row1(M);
  result.x = vector3_dot_product(V, &row_M);
  row_M = matrix3_row2(M);
  result.y = vector3_dot_product(V, &row_M);
  row_M = matrix3_row3(M);
  result.z = vector3_dot_product(V, &row_M);

  return result;
}

fMatrix3 matrix3_mul(fMatrix3* A, fMatrix3* B)
{
  fMatrix3 result = {0};

  fVector3 row_A = matrix3_row1(A);
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

void apply_xform(Shape* shape, fMatrix3* T)
{
  fPoint* pt = shape->points;
  for (int i = 0; i < shape->n_contours; ++i) {
    for (int j = 0; j < shape->contours[i]; ++j) {
      *pt = vector3_mul_matrix3(pt, T);
      ++pt;
    }
  }
}

void mk_translate_matrix(fMatrix3* T, float t_x, float t_y)
{
  fVector3* col = &T->col1;
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

void mk_rotate_matrix(fMatrix3* T, float rotation_angle)
{
  float sin_phi = sinf(rotation_angle);
  float cos_phi = cosf(rotation_angle);

  fVector3* col = &T->col1;
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

void mk_pivot_rotate_matrix(fMatrix3* T, float rotation_angle, fPoint* pivot_pt)
{
  float sin_phi = sinf(rotation_angle);
  float cos_phi = cosf(rotation_angle);

  fVector3* col = &T->col1;
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

void mk_scale_matrix(fMatrix3* T, float s_x, float s_y)
{
  fVector3* col = &T->col1;
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

void mk_pivot_scale_matrix(fMatrix3* T, float s_x, float s_y, fPoint* pivot_pt)
{
  fVector3* col = &T->col1;
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

void mk_flip_vertical_matrix(fMatrix3* T)
{
  fVector3* col = &T->col1;
  col->x = 1.f;
  col->y = 0.f;
  col->z = 0.f;

  col = &T->col2;
  col->x = 0.f;
  col->y = -1.f;
  col->z = 0.f;

  col = &T->col3;
  col->x = 0.f;
  col->y = 0.f;
  col->z = 1.f;
}

uint32_t make_grayscale_rgb32(uint8_t blackness)
{
  uint32_t value = blackness;
  return value | value << 8 | value << 16;
}

void device_window_copy_backbuf_to_framebuf(DeviceWindow* device_window)
{
  // flip vertically
  for (int i = 0; i < device_window->height; ++i) {
    uint32_t* src_line = device_window->backbuffer + device_window->width*i;
    uint32_t* dest_line = (uint32_t*)device_window->framebuffer + device_window->width*(device_window->height-1) - device_window->width*i;
    for (int j = 0; j < device_window->width; ++j) {
      uint32_t p = src_line[j];
      dest_line[j] = p;
    }
  }
}

uint32_t* device_window_get_pixel_at(DeviceWindow* device_window, int x, int y)
{
  uint32_t* result = device_window->backbuffer + device_window->width*y + x;
  return result;
}

void increase_pixel_blackness(DeviceWindow* device_window, int x, int y, int blackness)
{
  RgbPixel* pixel = (RgbPixel*)device_window_get_pixel_at(device_window, x, y);

  int new_blackness = pixel->X + blackness;
  if (new_blackness > 255) {
    new_blackness = 255;
  }
  pixel->X = new_blackness;
  pixel->R = 255 - pixel->X;
  pixel->G = 255 - pixel->X;
  pixel->B = 255 - pixel->X;
}

void device_window_set_pixel_blackness(DeviceWindow* device_window, int x, int y, int blackness)
{
  RgbPixel* pixel = (RgbPixel*)device_window_get_pixel_at(device_window, x, y);
  pixel->X = blackness;
  pixel->R = 255 - pixel->X;
  pixel->G = 255 - pixel->X;
  pixel->B = 255 - pixel->X;
}

void draw_pixel_black(DeviceWindow* device_window, int x, int y)
{
  uint32_t* p = device_window_get_pixel_at(device_window, x, y);
  *p = make_grayscale_rgb32(0);
}

void draw_pixel_gray(DeviceWindow* device_window, int x, int y, uint8_t blackness)
{
  uint32_t* p = device_window_get_pixel_at(device_window, x, y);
  *p = make_grayscale_rgb32(blackness);
}

void print_shape_points(Shape* shape)
{
  for (int i = 0; i < shape->total_point_count; ++i) {
    fPoint* p = &shape->points[i];
    printf("(%.4f, %.4f) ", p->x, p->y);
  }
  printf("\n");
}

void edge_list_print(EdgeList* edge_list)
{
  Edge* edge = &edge_list->entries[0];
  for (int i = 0; i < edge_list->count; ++i) {
    printf("((x0=%d, y0=%d), (x1=%d,y1=%d), x_intercept=%d)\n",
           edge->x0, edge->y0, edge->x1, edge->y1, edge->x_intercept);
    edge = edge->next_edge;
  }
  printf("\n");
}

int truncate_float(float f)
{
  int result = (int)f;
  return result;
}

bool float_is_equal(float a, float b)
{
  bool result = fabs(a - b) <= FLOAT_EPSILON;
  return result;
}

float drawing_surface_to_device_window_y_value(DrawingSurface* drawing_surface, float y)
{
  float result = (y - drawing_surface->y_min) / drawing_surface->pixel_height;
  return result;
}

float drawing_surface_to_device_window_x_value(DrawingSurface* drawing_surface, float x)
{
  float result = (x - drawing_surface->x_min) / drawing_surface->pixel_width;
  return result;
}

void set_pixel_on_device_window(DrawingSurface* drawing_surface, DeviceWindow* device_window, float x, float y)
{
  int pixel_y = round((y - drawing_surface->y_min)/drawing_surface->pixel_height);
  assert(pixel_y >= 0 && pixel_y < drawing_surface->y_pixel_count);
  int pixel_x = round((x - drawing_surface->x_min)/drawing_surface->pixel_width);
  assert(pixel_x >= 0 && pixel_x < drawing_surface->x_pixel_count);

// 4x4
  persistent float blackness_level_map[4][4] = {
    {1.f/24.f, 1.f/24.f, 1.f/24.f, 1.f/24.f},
    {1.f/24.f, 1.f/8.f, 1.f/8.f, 1.f/24.f},
    {1.f/24.f, 1.f/8.f, 1.f/8.f, 1.f/24.f},
    {1.f/24.f, 1.f/24.f, 1.f/24.f, 1.f/24.f},

//    {1.f/48.f, 5.f/96.f, 5.f/96.f, 1.f/48.f},
//    {5.f/96.f, 1.f/8.f, 1.f/8.f, 5.f/96.f},
//    {5.f/96.f, 1.f/8.f, 1.f/8.f, 5.f/96.f},
//    {1.f/48.f, 5.f/96.f, 5.f/96.f, 1.f/48.f},
  };
  int blackness_box_x = pixel_x % 4;
  int blackness_box_y = pixel_y % 4;
  int pixel_blackness = round(blackness_level_map[blackness_box_x][blackness_box_y]*255.f);
  int device_pixel_x = pixel_x/4;
  int device_pixel_y = pixel_y/4;

// 3x3
//  persistent float blackness_level_map[3][3] = {
//    {1.f/16.f, 1.f/8.f, 1.f/16.f},
//    {1.f/8.f, 1.f/4.f, 1.f/8.f},
//    {1.f/16.f, 1.f/8.f, 1.f/16.f}
//  };
//  int blackness_box_x = pixel_x % 3;
//  int blackness_box_y = pixel_y % 3;
//  int pixel_blackness = round(blackness_level_map[blackness_box_x][blackness_box_y]*255.f);
//  int device_pixel_x = pixel_x/3;
//  int device_pixel_y = pixel_y/3;

  increase_pixel_blackness(device_window, device_pixel_x, device_pixel_y, pixel_blackness);
}

bool compare_edge_is_less(Edge* edge_A, Edge* edge_B)
{
  return edge_A->y0 < edge_B->y0;
}

Edge pop_polygon_edge(EdgeList* heap)
{
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

void add_polygon_edge(EdgeList* heap, Edge* edge)
{
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

void insert_active_edge(EdgeList* list, Edge* edge)
{
  int i = 0;
  for (; i < list->count; ++i) {
    if (edge->x_intercept < list->entries[i].x_intercept) {
      break;
    }
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

void remove_active_edge(EdgeList* list, Edge* edge, int i)
{
  assert(i < list->count);
  for (int j = i; j < list->count; ++j) {
    list->entries[j] = list->entries[j+1];
  }
  --list->count;
}

void edge_list_sort(EdgeList* list)
{
  for (int i = 0; i < list->count; ++i) {
    for (int j = i;
         list->entries[j].x_intercept < list->entries[j-1].x_intercept;
         --j) {
      Edge t = list->entries[j];
      list->entries[j] = list->entries[j-1];
      list->entries[j-1] = t;
    }
  }
}

#include "genshape/shape_data.c"

Shape* find_shape(wchar_t character)
{
  Shape* result = 0;
  for (int i = 0; i < sizeof_array(shape_data); ++i) {
    if (shape_data[i].character == character) {
      result = &shape_data[i];
      break;
    }
  }
  return result;
}

fPoint new_empty_point()
{
  fPoint result = {0};
  return result;
}

EdgeList new_empty_edge_list()
{
  EdgeList result = {0};
  return result;
}

Edge new_empty_edge()
{
  Edge result = {0};
  return result;
}

iPolygon new_empty_polygon()
{
  iPolygon result = {0};
  return result;
}

Shape new_empty_shape()
{
  Shape result = {0};
  return result;
}

fMatrix3 new_empty_matrix3()
{
  fMatrix3 result = {0};
  return result;
}

// RasterSurface
// .............................................................................

void raster_surface_clear(RasterSurface* surface)
{
  uint8_t* buffer = surface->subpixel_buffer;
  for (int i = 0; i < surface->subpixel_count; ++i) {
    buffer[i] = 0;
  }
}

void raster_surface_set_pixel(RasterSurface* surface, int x, int y)
{
  uint8_t* p = surface->subpixel_buffer + (y * surface->width) + x;
  *p = 1;
}

void raster_subpixel_reader_init(RasterSubpixelReader* reader, RasterSurface* surface, DeviceWindow* window)
{
  reader->surface_subpixel_count = surface->subpixel_count;
  reader->subpixel_count = 0;
  reader->pixel_scanline = surface->subpixel_buffer;
  reader->at_pixel = reader->pixel_scanline;
  reader->pixel_count_y = window->height;
  reader->pixel_count_x = window->width;
  reader->pixel_square_side = surface->subsampling_factor;
  reader->subpixel_buffer = push_array(uint8_t, reader->pixel_square_side * reader->pixel_square_side);
  reader->at_pixel_i = 0;
  reader->at_pixel_j = 0;
}

bool raster_subpixel_reader_next(RasterSubpixelReader* reader)
{
  bool got_pixel = true;
  if (reader->at_pixel_j >= reader->pixel_count_x)
  {
    ++reader->at_pixel_i;
    if (reader->at_pixel_i >= reader->pixel_count_y)
    {
      got_pixel = false;
      assert (reader->subpixel_count == reader->surface_subpixel_count);
      return got_pixel;
    }
    reader->pixel_scanline += (reader->pixel_count_x * reader->pixel_square_side) * reader->pixel_square_side;
    reader->at_pixel = reader->pixel_scanline;
    reader->at_pixel_j = 0;
  }
  uint8_t* subpixel_scanline = reader->at_pixel;
  uint8_t* dest_subpixel = reader->subpixel_buffer;
  for (int s = 0; s < reader->pixel_square_side; ++s)
  {
    uint8_t* src_subpixel = subpixel_scanline;
    for (int t = 0; t < reader->pixel_square_side; ++t)
    {
      uint8_t subpixel_value = *src_subpixel;
      assert (subpixel_value == 0 || subpixel_value == 1);
      //printf("%d", subpixel_value);
      *dest_subpixel = subpixel_value;
      ++dest_subpixel;
      ++src_subpixel;
      ++reader->subpixel_count;
    }
    subpixel_scanline += reader->pixel_count_x * reader->pixel_square_side;
    //printf("\n");
  }
  //printf("\n");
  reader->at_pixel += reader->pixel_square_side;
  ++reader->at_pixel_j;
  return got_pixel;
}

float raster_surface_get_blackness_level_at(RasterSurface* surface, int x, int y)
{
  float* subpixel = surface->blackness_levels + surface->subsampling_factor * x + y;
  float level = *subpixel;
  return level;
}

// .............................................................................

// TODO: We could take the contours of the Shape and make a Polygon out of that,
// instead of the Polygon having the contrours contained in it, like the Shape does.
void shape_to_polygon(iPolygon* polygon, RasterShape* shape)
{
  polygon->contour_vertex_count = push_array(int, shape->n_contours);
  polygon->n_contours = shape->n_contours;
  polygon->contours = push_array(iPoint*, shape->n_contours);
  iPoint* shape_points = shape->points;
  for (int i = 0; i < shape->n_contours; ++i) {
    polygon->contour_vertex_count[i] = 0;
    polygon->contours[i] = push_array(iPoint, shape->contours[i]+3);
    polygon->contours[i] += 1;
    fPoint* p;

    int j = 0;
    for (; j < shape->contours[i]; ++j) {
      polygon->contours[i][j] = shape_points[j];
    }

    polygon->contour_vertex_count[i] = j;
    polygon->total_vertex_count += polygon->contour_vertex_count[i];
    polygon->contours[i][-1] = polygon->contours[i][polygon->contour_vertex_count[i]-1];
    polygon->contours[i][polygon->contour_vertex_count[i]] = polygon->contours[i][0];
    polygon->contours[i][polygon->contour_vertex_count[i]+1] = polygon->contours[i][1];

    shape_points += shape->contours[i];
  }

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
    float accumulated_y_distance = 0.f;
    for (int j = 0; j < polygon->contour_vertex_count[i]; ++j) {
      Edge* edge = &edge_list[i].entries[edge_list[i].count];
      edge->start_point = polygon->contours[i][j];
      edge->end_point = polygon->contours[i][j+1];
      //assert (edge->start_point.x >= drawing_surface->x_min && edge->start_point.x <= drawing_surface->x_max);
      //assert (edge->start_point.y >= drawing_surface->y_min && edge->start_point.y <= drawing_surface->y_max);
      //assert (edge->end_point.x >= drawing_surface->x_min && edge->end_point.x <= drawing_surface->x_max);
      //assert (edge->end_point.y >= drawing_surface->y_min && edge->end_point.y <= drawing_surface->y_max);
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
  }

  for (int i = 0; i < polygon->n_contours; ++i) {
    int edge_count = edge_list[i].count;
    for (int j = 0; j < edge_list[i].count; ++j) {
      Edge* edge = &edge_list[i].entries[j];
      Edge* prev_edge = edge->prev_edge;
      Edge* next_edge = edge->next_edge;
      if (edge->y1 > edge->y0) {
        edge->x_intercept = edge->x0;
      }
      else if (edge->y1 < edge->y0) {
        edge->x_intercept = edge->x1;
      }
      else assert(false);

      assert (edge->y1 != edge->y0);
      //if (edge->y1 == edge->y0) {
      //  prev_edge->y1 = next_edge->y0;

      //  prev_edge->next_edge = next_edge;
      //  next_edge->prev_edge = prev_edge;
      //  edge->next_edge = edge->prev_edge = 0;
      //  --edge_count;
      //}
    }
    edge_list[i].count = edge_count;
    //printf("Contour #%d\n", i);
    //edge_list_print(&edge_list[i]);
  }

  polygon->edge_list = edge_list;
}

int y_intercept_at(RasterSurface* surface, int scanline_nr) {
  int result = surface->min_y + scanline_nr;
  return result;
}

void draw_polygon(iPolygon* polygon, RasterSurface* surface)
{
  EdgeList edge_heap = {0};
  edge_heap.entries = push_array(Edge, polygon->total_vertex_count);
  edge_heap.count = 0;
  edge_heap.entries[0] = new_empty_edge();
  edge_heap.entries[0].y0 = surface->min_y;
  for (int i = 0; i < polygon->n_contours; ++i) {
    EdgeList* edge_list = &polygon->edge_list[i];
    Edge* edge = &edge_list->entries[0];
    for (int j = 0; j < edge_list->count; ++j) {
      if (edge->y1 < edge->y0) {
        iPoint p = edge->start_point;
        edge->start_point = edge->end_point;
        edge->end_point = p;
      }
      edge->delta_x = edge->x1 - edge->x0;
      edge->delta_y = edge->y1 - edge->y0;
      assert (!float_is_equal(edge->delta_y, 0.f));
      edge->x_intercept = edge->x0;
      edge->m = edge->delta_y / edge->delta_x;  // y = m*x + b
      edge->b = edge->y0 - edge->m*edge->x0;
      add_polygon_edge(&edge_heap, edge);
      edge = edge->next_edge;
    }
  }

  EdgeList active_edge_list = {};
  active_edge_list.entries = push_array(Edge, edge_heap.count + 2);
  active_edge_list.entries += 1;
  active_edge_list.count = 0;
  active_edge_list.entries[-1] = new_empty_edge();
  active_edge_list.entries[-1].x_intercept = surface->min_x;
  active_edge_list.entries[0] = new_empty_edge();
  active_edge_list.entries[0].x_intercept = surface->max_x;

  assert(edge_heap.count >= 2);
  int y = surface->min_y;
  while (y <= surface->max_y) {
    for (int i = 0; i < active_edge_list.count; ++i) {
      Edge* edge = &active_edge_list.entries[i];
      if (edge->m != INFINITY) {
        edge->x_intercept = round((y - edge->b)/edge->m);
        if (edge->x_intercept < surface->min_x) {
          edge->x_intercept = surface->min_x;
        }
        else if (edge->x_intercept > surface->max_x) {
          edge->x_intercept = surface->max_x;
        }
      }
    }

    edge_list_sort(&active_edge_list);
    //if (active_edge_list.count > 0) {
    //  printf("--------------- %d -----------------\n", y);
    //  printf("y=%d\n", y);
    //  edge_list_print(&active_edge_list);
    //}

    assert((active_edge_list.count % 2) == 0);
    for (int i = 0; i < active_edge_list.count; i += 2) {
      Edge* left_edge = &active_edge_list.entries[i];
      Edge* right_edge = &active_edge_list.entries[i+1];
      //printf("left_edge=(%d, %d)\n", left_edge->x_intercept, y);
      //printf("right_edge=(%d, %d)\n", right_edge->x_intercept, y);

      for (int x = left_edge->x_intercept; x < right_edge->x_intercept; ++x) {
        raster_surface_set_pixel(surface, x, y);
      }
    }
    ++y;
    for (int i = 0; i < active_edge_list.count;) {
      Edge* edge = &active_edge_list.entries[i];
      if (edge->y1 <= y) {
        remove_active_edge(&active_edge_list, edge, i);
        continue;
      }
      ++i;
    }
    while (edge_heap.count > 0 && edge_heap.entries[1].y0 <= y) {
      Edge edge = pop_polygon_edge(&edge_heap);
      insert_active_edge(&active_edge_list, &edge);
    }
  }
}

void clear_device_window(DeviceWindow* device_window, uint8_t blackness)
{
  int i, j;
  uint32_t* p = device_window->backbuffer;
  for (int j = 0; j < device_window->height; ++j) {
    for (int i = 0; i < device_window->width; ++i) {
      *p++ = make_grayscale_rgb32(blackness);
    }
  }
}

void draw_line(iLine* line, RasterSurface* surface)
{
  int abs_dx = abs(line->x1 - line->x0);
  int abs_dy = abs(line->y1 - line->y0);

  if (abs_dy == 0) {
    int y = line->y0;
    int x = line->x0;
    int x_end = line->x1;
    if (line->x0 > line->x1) {
      x = line->x1;
      x_end = line->x0;
    }
    for (; x <= x_end; ++x) {
      raster_surface_set_pixel(surface, x, y);
    }
  }
  else
  {
    int dx = line->x1 - line->x0;
    int dy = line->y1 - line->y0;
    int p = 2*abs_dy - abs_dx;
    int y = line->y0;
    int x = line->x0;
    int x_end = line->x1;
    int y_incr = (dy < 0) ? -1 : 1;
    if (dx < 0) {
      y = line->y1;
      x = line->x1;
      x_end = line->x0;
      y_incr = (dy > 0) ? -1 : 1;
    }
    int *p_x = &x;
    int *p_y = &y;

    if (abs_dy > abs_dx) {
      dx = line->y1 - line->y0;
      dy = line->x1 - line->x0;
      abs_dx = abs(dx);
      abs_dy = abs(dy);
      p = 2*abs_dy - abs_dx;
      y = line->x0;
      x = line->y0;
      x_end = line->y1;
      y_incr = (dy < 0) ? -1 : 1;
      if (dx < 0) {
        y = line->x1;
        x = line->y1;
        x_end = line->y0;
        y_incr = (dy > 0) ? -1 : 1;
      }
      p_x = &y;
      p_y = &x;
    }

    for (; x <= x_end; ++x) {
      raster_surface_set_pixel(surface, *p_x, *p_y);
      if (p < 0) {
        p += 2*abs_dy;
      } else {
        y += y_incr;
        p += 2*abs_dy - 2*abs_dx;
      }
    }
  }
}

#if 0/*>--*/
void

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
#endif/*--<*/

fRectangle get_bounding_box(Shape* shape)
{
  fRectangle bb = {};
  fPoint* pt = shape->points;
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

void vertical_line_intersection(fPoint* p0, fPoint* p1, fPoint* result, int x)
{
  assert(!float_is_equal(p1->x, p0->x));
  float m = (p1->y - p0->y)/(float)(p1->x - p0->x);
  result->y = p1->y + m*(x - p1->x);
  result->x = x;
}

void horizontal_line_intersection(fPoint* p0, fPoint* p1, fPoint* result, int y)
{
  assert(!float_is_equal(p1->y, p0->y));
  float m = (p1->y - p0->y)/(float)(p1->x - p0->x);
  result->x = p1->x + (y - p1->y)/m;
  result->y = y;
}

bool does_intersect_clipping_edge(fPoint* p0, fPoint* p1, ClippingEdge clipping_edge, float clipping_boundary[static ClipEdge_COUNT])
{
  bool result = false;
  if (clipping_edge == ClipEdge_Left || clipping_edge == ClipEdge_Right) {
    int x = clipping_boundary[clipping_edge];
    result = !float_is_equal(p0->x, p1->x) &&
      ((p0->x > x) && (p1->x < x) ||
       (p0->x < x) && (p1->x > x));
  }
  else if (clipping_edge == ClipEdge_Bottom || clipping_edge == ClipEdge_Top) {
    int y = clipping_boundary[clipping_edge];
    result = !float_is_equal(p0->y, p1->y) &&
      ((p0->y > y) && (p1->y < y) ||
       (p0->y < y) && (p1->y > y));
  }
  else {
    assert(false);
  }
  return result;
}

void get_clip_edge_intersection(fPoint* r, fPoint* v, fPoint* result,
                           ClippingEdge clipping_edge, float clipping_boundary[static ClipEdge_COUNT])
{
  if (clipping_edge == ClipEdge_Left || clipping_edge == ClipEdge_Right) {
    vertical_line_intersection(r, v, result, clipping_boundary[clipping_edge]);
  }
  else if (clipping_edge == ClipEdge_Bottom || clipping_edge == ClipEdge_Top) {
    horizontal_line_intersection(r, v, result, clipping_boundary[clipping_edge]);
  }
}

bool is_point_inside_clip_boundary(fPoint* v, ClippingEdge clipping_edge, float clipping_boundary[static ClipEdge_COUNT])
{
  bool result = false;
  if (clipping_edge == ClipEdge_Left) {
    if (v->x >= clipping_boundary[ClipEdge_Left]) {
      result = true;
    }
  }
  else if (clipping_edge == ClipEdge_Right) {
    if (v->x <= clipping_boundary[ClipEdge_Right]) {
      result = true;
    }
  }
  else if (clipping_edge == ClipEdge_Bottom) {
    if (v->y >= clipping_boundary[ClipEdge_Bottom]) {
      result = true;
    }
  }
  else if (clipping_edge == ClipEdge_Top) {
    if (v->y <= clipping_boundary[ClipEdge_Top]) {
      result = true;
    }
  }
  else {
    assert(false);
  }
  return result;
}

void do_clip_point(fPoint* v, ClippingEdge clipping_edge, fPoint* first_clipped[static ClipEdge_COUNT],
                   fPoint* recently_clipped[static ClipEdge_COUNT], float clipping_boundary[static ClipEdge_COUNT],
                   fPoint* clipped_contour, int* clipped_vertex_count)
{
  if (!first_clipped[clipping_edge]) {
    first_clipped[clipping_edge] = v;
  }
  else {
    if (recently_clipped[clipping_edge]) {
      if (does_intersect_clipping_edge(recently_clipped[clipping_edge], v, clipping_edge, clipping_boundary)) {
        fPoint* intersection_point = push_struct(fPoint);
        intersection_point->z = 1.f;
        get_clip_edge_intersection(recently_clipped[clipping_edge], v, intersection_point, clipping_edge, clipping_boundary);
        if (clipping_edge < ClipEdge_COUNT-1) {
          do_clip_point(intersection_point, clipping_edge+1, first_clipped, recently_clipped, clipping_boundary,
                        clipped_contour, clipped_vertex_count);
        }
        else {
          clipped_contour[*clipped_vertex_count] = *intersection_point;
          ++(*clipped_vertex_count);
        }
      }
    }
  }
  recently_clipped[clipping_edge] = v;
  if (is_point_inside_clip_boundary(v, clipping_edge, clipping_boundary)) {
    if (clipping_edge < ClipEdge_COUNT-1) {
      do_clip_point(v, clipping_edge+1, first_clipped, recently_clipped, clipping_boundary,
                    clipped_contour, clipped_vertex_count);
    }
    else {
      clipped_contour[*clipped_vertex_count] = *v;
      ++(*clipped_vertex_count);
    }
  }
}

Shape clip_shape(Shape* shape, float clipping_boundary[static ClipEdge_COUNT], Shape* clipped_shape)
{
  clipped_shape->n_contours = shape->n_contours;
  clipped_shape->contours = push_array(int, shape->n_contours);
  clipped_shape->points = push_array(fPoint, shape->total_point_count*2);
  fPoint* clipped_contour = clipped_shape->points;
  fPoint* shape_points = shape->points;
  for (int i = 0; i < shape->n_contours; ++i) {
    int contour_vertex_count = shape->contours[i];
    int clipped_vertex_count = 0;
    fPoint* recently_clipped[ClipEdge_COUNT] = {0};
    fPoint* first_clipped[ClipEdge_COUNT] = {0};
    for (int j = 0; j < contour_vertex_count; ++j) {
      fPoint* v = shape_points++;
      do_clip_point(v, ClipEdge_Left, first_clipped, recently_clipped, clipping_boundary,
                    clipped_contour, &clipped_vertex_count);
    }
    for (ClippingEdge clipping_edge = ClipEdge_Left;
         clipping_edge < ClipEdge_COUNT;
         ++clipping_edge) {
      if (!first_clipped[clipping_edge]) {
        continue;
      }
      if (does_intersect_clipping_edge(recently_clipped[clipping_edge], first_clipped[clipping_edge], clipping_edge, clipping_boundary)) {
        fPoint* intersection_point = push_struct(fPoint);
        intersection_point->z = 1.f;
        get_clip_edge_intersection(recently_clipped[clipping_edge], first_clipped[clipping_edge], intersection_point, clipping_edge, clipping_boundary);
        if (clipping_edge < ClipEdge_COUNT-1) {
          do_clip_point(intersection_point, clipping_edge+1, first_clipped, recently_clipped, clipping_boundary,
                        clipped_contour, &clipped_vertex_count);
        }
        else {
          clipped_contour[clipped_vertex_count] = *intersection_point;
          clipped_vertex_count++;
        }
      }
    }
    clipped_shape->contours[i] = clipped_vertex_count;
    clipped_contour += clipped_vertex_count;
    clipped_shape->total_point_count += clipped_vertex_count;
  }
}

#if 0
void draw_test(WinDeviceWindow* device_window) {
  int pitch = device_window->width*device_window->bytes_per_pixel;
  uint8_t* row = (uint8_t*)device_window->framebuffer;
  for (int y = 0; y < device_window->height; ++y) {
    RgbPixel* pixel = (RgbPixel*)row;
    for (int x = 0; x < device_window->width; ++x) {
      pixel->R = 128;
      pixel->G = 128;
      pixel->B = 0;
      pixel->X = 0;
      ++pixel;
    }
    row += pitch;
  }
}
#endif

void draw(DeviceWindow* device_window)
{
// Define the characters that we wish to draw.
// Compute the attributes such as font width and height, character spacing, etc..
// .............................................................................
  //wchar_t* string = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  //wchar_t* string = L" abcdefghijklmnopqrstuvwxyz";
  //wchar_t* string = L"0123456789";
  //wchar_t* string = L"~!@#$%^&*()_+-={}|:\"<>?`[]\\;',./";
  wchar_t* string = L"drawing_surface.x_min = {-0.5f}; printf(\"Bounding box '%lc': \")";
  int string_length = wcslen(string);

  fRectangle max_bbox = {0};
  max_bbox.lower_left.x = INT_MAX;
  max_bbox.lower_left.y = INT_MAX;
  max_bbox.upper_right.x = INT_MIN;
  max_bbox.upper_right.y = INT_MIN;
  for (int i = 0; i < string_length; ++i) {
    Shape* shape = find_shape(string[i]);
    assert (shape);
    fRectangle* shape_bbox = shape->bbox;
    printf("Bounding box '%lc': (x0=%.4f, y0=%.4f), (x1=%.4f, y1=%.4f)\n", string[i],
            shape_bbox->lower_left.x, shape_bbox->lower_left.y, shape_bbox->upper_right.x, shape_bbox->upper_right.y);
    if (shape_bbox->lower_left.x < max_bbox.lower_left.x) {
      max_bbox.lower_left.x = shape_bbox->lower_left.x;
    }
    if (shape_bbox->lower_left.y < max_bbox.lower_left.y) {
      max_bbox.lower_left.y = shape_bbox->lower_left.y;
    }
    if (shape_bbox->upper_right.x > max_bbox.upper_right.x) {
      max_bbox.upper_right.x = shape_bbox->upper_right.x;
    }
    if (shape_bbox->upper_right.y > max_bbox.upper_right.y) {
      max_bbox.upper_right.y = shape_bbox->upper_right.y;
    }
  }
  printf("Max. extent bbox: (x0=%.4f, y0=%.4f), (x1=%.4f, y1=%.4f)\n",
         max_bbox.lower_left.x, max_bbox.lower_left.y, max_bbox.upper_right.x, max_bbox.upper_right.y);
  int font_width = max_bbox.upper_right.x - max_bbox.lower_left.x;
  int font_height = max_bbox.upper_right.y - max_bbox.lower_left.y;
  int character_spacing = 1;  // px
  int line_spacing = 2; // px
// .............................................................................

// Set-up the Drawing Surface, the Viewing Window.
// .............................................................................
  DrawingSurface drawing_surface = {0};
  drawing_surface.x_pixel_count = device_window->width*4;
  drawing_surface.y_pixel_count = device_window->height*4;
  drawing_surface.x_min = -0.5f;
  drawing_surface.x_max = 0.5f;
  drawing_surface.y_min = -0.5f;
  drawing_surface.y_max = 0.5f;
  drawing_surface.width = drawing_surface.x_max - drawing_surface.x_min;
  drawing_surface.height = drawing_surface.y_max - drawing_surface.y_min;
  drawing_surface.pixel_width = drawing_surface.width / drawing_surface.x_pixel_count;
  drawing_surface.pixel_height = drawing_surface.height / drawing_surface.y_pixel_count;

  ViewWindow view_window = {0};
  view_window.width = (float)device_window->width*1.0f;
  view_window.height = (float)device_window->height*1.0f;
  
  RasterSurface raster_surface = {0};
#define SAMPLE_RATE 4
  raster_surface.subsampling_factor = SAMPLE_RATE;
  persistent float blackness_levels[SAMPLE_RATE][SAMPLE_RATE];
  float* level_at = blackness_levels[0];
  for (int i = 0; i < SAMPLE_RATE*SAMPLE_RATE; ++i)
  {
    *level_at = 1.f/(float)(SAMPLE_RATE*SAMPLE_RATE);
    ++level_at;
  }
#undef SAMPLE_RATE

  raster_surface.blackness_levels = blackness_levels[0];
  raster_surface.width = device_window->width * raster_surface.subsampling_factor;
  raster_surface.height = device_window->height * raster_surface.subsampling_factor;
  raster_surface.min_x = 0, raster_surface.min_y = 0;
  raster_surface.max_x = raster_surface.width, raster_surface.max_y = raster_surface.height;
  raster_surface.subpixel_count = raster_surface.width * raster_surface.height;
  raster_surface.subpixel_buffer = push_array(uint8_t, raster_surface.subpixel_count);
// .............................................................................

// Arrange the shapes in the World space.
// .............................................................................
  Shape* shapes = push_array(Shape, string_length);
  for (int i = 0; i < string_length; ++i) {
    Shape* shape_template = find_shape(string[i]);
    shapes[i] = new_empty_shape();
    shapes[i].n_contours = shape_template->n_contours;
    shapes[i].contours = push_array(int, shapes[i].n_contours);
    for (int k = 0; k < shapes[i].n_contours; ++k) {
      shapes[i].contours[k] = shape_template->contours[k];
    }
    shapes[i].total_point_count = shape_template->total_point_count;
    shapes[i].points = push_array(fPoint, shapes[i].total_point_count);
    for (int k = 0; k < shapes[i].total_point_count; ++k) {
      shapes[i].points[k] = shape_template->points[k];
    }
    shapes[i].bbox = push_struct(fRectangle);
    *shapes[i].bbox = *shape_template->bbox;
  }

  for (int i = 0; i < string_length; ++i) {
    Shape* shape = &shapes[i];
    fRectangle* shape_bbox = shape->bbox;
    float lower_left_x = character_spacing + i*(font_width+character_spacing) + 0;
    float lower_left_y = shape_bbox->lower_left.y+line_spacing+font_height + 0;
    fMatrix3 horizontal_align_xform = {0};
    mk_translate_matrix(&horizontal_align_xform, lower_left_x, lower_left_y);
    apply_xform(shape, &horizontal_align_xform);
  }
// .............................................................................

// Place the View Window in the World space and clip the shapes to fit inside it.
// .............................................................................
  view_window.lower_left = max_bbox.lower_left;
  view_window.upper_right.x = view_window.lower_left.x + view_window.width;
  view_window.upper_right.y = view_window.lower_left.y + view_window.height;

  float clipping_boundary[ClipEdge_COUNT] = {0};
  clipping_boundary[ClipEdge_Left] = view_window.lower_left.x;
  clipping_boundary[ClipEdge_Right] = view_window.upper_right.x;
  clipping_boundary[ClipEdge_Bottom] = view_window.lower_left.y;
  clipping_boundary[ClipEdge_Top] = view_window.upper_right.y;
  assert (clipping_boundary[ClipEdge_Left] < clipping_boundary[ClipEdge_Right]);
  assert (clipping_boundary[ClipEdge_Bottom < clipping_boundary[ClipEdge_Top]]);

  printf("Clipping window: (Left=%.4f,Right=%.4f,Bottom=%.4f,Top=%.4f)\n",
         clipping_boundary[ClipEdge_Left], clipping_boundary[ClipEdge_Right],
         clipping_boundary[ClipEdge_Bottom], clipping_boundary[ClipEdge_Top]);

  Shape* clipped_shapes = push_array(Shape, string_length);
  for (int i = 0; i < string_length; ++i) {
    clipped_shapes[i] = new_empty_shape();
    clip_shape(&shapes[i], clipping_boundary, &clipped_shapes[i]);
  }
// .............................................................................

#if 1
// Place the View Window inside the World space.
// .............................................................................
  for (int i = 0; i < string_length; ++i) {
    Shape* shape = &clipped_shapes[i];
    fMatrix3 translate = {0};
    mk_translate_matrix(&translate, -view_window.lower_left.x, -view_window.lower_left.y);
    apply_xform(shape, &translate);
  }
// .............................................................................

// Transform the View Window from World to Normalized coordinates.
// .............................................................................
  for (int i = 0; i < string_length; ++i) {
    Shape* shape = &clipped_shapes[i];
    assert (view_window.height <= view_window.width);
    fMatrix3 scale = {0};
    mk_scale_matrix(&scale, 1.f/view_window.width, 1.f/view_window.width);
    apply_xform(shape, &scale);
  }
// .............................................................................

// Transform the View Window from Normalized to Raster coordinates.
// .............................................................................
  RasterShape* raster_shapes = push_array(RasterShape, string_length);
  for (int i = 0; i < string_length; ++i) {
    Shape* shape = &clipped_shapes[i];
    RasterShape* raster_shape = &raster_shapes[i];
    raster_shape->n_contours = shape->n_contours;
    raster_shape->contours = push_array(int, raster_shape->n_contours);
    raster_shape->total_point_count = shape->total_point_count;
    raster_shape->points = push_array(iPoint, raster_shape->total_point_count);
  }

  for (int i = 0; i < string_length; ++i) {
    Shape* shape = &clipped_shapes[i];
    RasterShape* raster_shape = &raster_shapes[i];
    assert (raster_surface.height <= raster_surface.width);
    fMatrix3 scale = {0};
    mk_scale_matrix(&scale, raster_surface.width, raster_surface.width);
    apply_xform(shape, &scale);

    assert (shape->n_contours == raster_shape->n_contours);
    for (int k = 0; k < shape->n_contours; ++k) {
      raster_shape->contours[k] = shape->contours[k];
    }

    assert (shape->total_point_count == raster_shape->total_point_count);
    for (int k = 0; k < shape->total_point_count; ++k) {
      fPoint* point = &shape->points[k];
      iPoint* raster_point = &raster_shape->points[k];

      raster_point->x = round(point->x);
      if (raster_point->x < raster_surface.min_x) {
        raster_point->x = raster_surface.min_x;
      }
      else if (raster_point->x > raster_surface.width) {
        raster_point->x = raster_surface.width;
      }

      raster_point->y = round(point->y);
      if (raster_point->y < raster_surface.min_y) {
        raster_point->y = raster_surface.min_y;
      }
      else if (raster_point->y > raster_surface.height) {
        raster_point->y = raster_surface.height;
      }
    }
  }
// .............................................................................

// Draw the shapes onto the Raster Surface.
// .............................................................................
  raster_surface_clear(&raster_surface);

  for (int i = 0; i < string_length; ++i) {
    RasterShape* shape = &raster_shapes[i];
    if (shape->total_point_count > 0) {
      iPolygon polygon = {0};
      shape_to_polygon(&polygon, shape);
      draw_polygon(&polygon, &raster_surface);
    }
  }
// .............................................................................

#if 1
  iLine line = {0};
  line.x0 = 5*8;
  line.y0 = 5*8;
  line.x1 = 200;
  line.y1 = 5*8;
  draw_line(&line, &raster_surface);
#endif

// Draw the Raster Surface onto the Device Window.
// .............................................................................

  clear_device_window(device_window, 255);

  RasterSubpixelReader subpixel_reader = {0};
  raster_subpixel_reader_init(&subpixel_reader, &raster_surface, device_window);
  for (int y = 0; y < subpixel_reader.pixel_count_y; ++y)
  {
    for (int x = 0; x < subpixel_reader.pixel_count_x; ++x)
    {
      //printf("(%d, %d)\n", y, x);
      raster_subpixel_reader_next(&subpixel_reader);
      float pixel_value = 0.f;
      uint8_t* subpixel_line = subpixel_reader.subpixel_buffer;
      for (int s = 0; s < subpixel_reader.pixel_square_side; ++s)
      {
        uint8_t* subpixel = subpixel_line;
        for (int t = 0; t < subpixel_reader.pixel_square_side; ++t)
        {
          uint8_t subpixel_value = *subpixel;
          //printf("%d", subpixel_value);
          pixel_value += subpixel_value * raster_surface_get_blackness_level_at(&raster_surface, t, s) * 255.f;
          ++subpixel;
        }
        //printf("= %f\n", pixel_value);
        subpixel_line += subpixel_reader.pixel_square_side;
      }
      assert (pixel_value <= 255.f);
      device_window_set_pixel_blackness(device_window, x, y, pixel_value);
    }
  }

// .............................................................................
#endif
}

