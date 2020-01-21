// -*- coding: utf-8 -*-

#define abs(i) \
  ((i) > 0 ? (i) : -(i))

#define fabs(f) \
  ((f) > 0.f ? (f) : -(f))

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
  Matrix3 result = {0};

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
mk_translate_matrix(Matrix3* T, float t_x, float t_y) {
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
mk_rotate_matrix(Matrix3* T, float rotation_angle) {
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
mk_pivot_rotate_matrix(Matrix3* T, float rotation_angle, Point* pivot_pt) {
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
mk_scale_matrix(Matrix3* T, float s_x, float s_y) {
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
mk_pivot_scale_matrix(Matrix3* T, float s_x, float s_y, Point* pivot_pt) {
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

void
mk_flip_vertical_matrix(Matrix3* T) {
  Vector3* col = &T->col1;
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

uint32_t
make_grayscale_rgb32(uint8_t intensity) {
  uint32_t value = intensity;
  return value | value << 8 | value << 16;
}

void
draw_pixel_black(DeviceWindow* device_window, int x, int y) {
  uint32_t* p = device_window->pixel_buffer + device_window->width*y + x;
  *p = make_grayscale_rgb32(0);
}

void
draw_pixel_gray(DeviceWindow* device_window, int x, int y, uint8_t intensity) {
  uint32_t* p = device_window->pixel_buffer + device_window->width*y + x;
  *p = make_grayscale_rgb32(intensity);
}

void
print_shape_points(Shape* shape) {
  for (int i = 0; i < shape->total_point_count; ++i) {
    Point* p = &shape->points[i];
    printf("(%.4f, %.4f) ", p->x, p->y);
  }
  printf("\n");
}

void
print_edge_list(EdgeList* edge_list) {
  for (int i = 0; i < edge_list->count; ++i) {
    Edge* edge = &edge_list->entries[i];
    printf("((x0=%.4f, y0=%.4f), (x1=%.4f,y1=%.4f), x_intercept=%.4f)\n",
           edge->x0, edge->y0, edge->x1, edge->y1, edge->x_intercept);
  }
  printf("\n");
}

int
truncate_float(float f) {
  int result = (int)f;
  return result;
}

bool
float_is_zero(float a) {
  bool result = fabs(a) <= FLOAT_EPSILON;
  return result;
}

bool
float_is_equal(float a, float b) {
  bool result = fabs(a - b) <= FLOAT_EPSILON;
  return result;
}

bool
float_is_greater(float a, float b) {
  bool result = (a > b) && !float_is_equal(a, b);
  return result;
}

bool
float_is_greater_or_equal(float a, float b) {
  bool result = float_is_greater(a, b) || float_is_equal(a, b);
  return result;
}

bool
float_is_less(float a, float b) {
  bool result = (a < b) && !float_is_equal(a, b);
  return result;
}

bool
float_is_less_or_equal(float a, float b) {
  bool result = float_is_less(a, b) || float_is_equal(a, b);
  return result;
}

float
drawing_surface_to_device_window_y_value(DrawingSurface* drawing_surface, float y) {
  float result = (y - drawing_surface->y_min) / drawing_surface->pixel_height;
  return result;
}

float
drawing_surface_to_device_window_x_value(DrawingSurface* drawing_surface, float x) {
  float result = (x - drawing_surface->x_min) / drawing_surface->pixel_width;
  return result;
}

void
set_pixel_on_device_window(DrawingSurface* drawing_surface, DeviceWindow* device_window, float x, float y) {
  int pixel_y = round((y - drawing_surface->y_min)/drawing_surface->pixel_height);
  assert(pixel_y >= 0 && pixel_y < drawing_surface->y_pixel_count);
  int pixel_x = round((x - drawing_surface->x_min)/drawing_surface->pixel_width);
  assert(pixel_x >= 0 && pixel_x < drawing_surface->x_pixel_count);

  draw_pixel_black(device_window, pixel_x, pixel_y);
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
  while (float_is_greater_or_equal(edge->x_intercept, list->entries[i].x_intercept)) {
    ++i;
    assert (i <= list->count);
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

EdgeList
new_empty_edge_list() {
  EdgeList result = {0};
  return result;
}

Edge
new_empty_edge() {
  Edge result = {0};
  return result;
}

Polygon
new_empty_polygon() {
  Polygon result = {0};
  return result;
}

void
make_polygon(Polygon* polygon, Shape* shape, DrawingSurface* drawing_surface) {
  polygon->contour_vertex_count = push_array(int, shape->n_contours);
  polygon->n_contours = shape->n_contours;
  polygon->contours = push_array(Point*, shape->n_contours);
  Point* shape_points = shape->points;
  for (int i = 0; i < shape->n_contours; ++i) {
    polygon->contour_vertex_count[i] = 0;
    polygon->contours[i] = push_array(Point, shape->contours[i]+3);
    polygon->contours[i] += 1;
    Point* p;

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
      assert (edge->start_point.x >= drawing_surface->x_min && edge->start_point.x <= drawing_surface->x_max);
      assert (edge->start_point.y >= drawing_surface->y_min && edge->start_point.y <= drawing_surface->y_max);
      assert (edge->end_point.x >= drawing_surface->x_min && edge->end_point.x <= drawing_surface->x_max);
      assert (edge->end_point.y >= drawing_surface->y_min && edge->end_point.y <= drawing_surface->y_max);
      //if (fabs(edge->y1 - edge->y0) < drawing_surface->pixel_height) {
      //  float min_y = edge->y0;
      //  if (edge->y1 < min_y) {
      //    min_y = edge->y1;
      //  }
      //  float lower_scanline = floor(drawing_surface_to_device_window_y_value(drawing_surface, min_y));
      //  if (drawing_surface_to_device_window_y_value(drawing_surface, edge->y0) >= lower_scanline &&
      //      drawing_surface_to_device_window_y_value(drawing_surface, edge->y1) >= lower_scanline) {
      //    accumulated_y_distance += fabs(edge->y1 - edge->y0);
      //    if (accumulated_y_distance >= drawing_surface->pixel_height) {
      //      edge->start_point = prev_edge->end_point;
      //      accumulated_y_distance = 0.f;
      //    }
      //    else {
      //      edge->y0 = edge->y1 = min_y;
      //      continue;
      //    }
      //  }
      //}
      if (float_is_equal(edge->y1, edge->y0)) {
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

    printf("Contour #%d\n", i);
    print_edge_list(&edge_list[i]);
  }

  for (int i = 0; i < polygon->n_contours; ++i) {
    int edge_count = edge_list[i].count;
    for (int j = 0; j < edge_list[i].count; ++j) {
      Edge* edge = &edge_list[i].entries[j];
      Edge* prev_edge = edge->prev_edge;
      Edge* next_edge = edge->next_edge;
      assert (fabs(prev_edge->y1 - edge->y0) < drawing_surface->pixel_height);
      assert (fabs(edge->y1 - next_edge->y0) < drawing_surface->pixel_height);
      if (edge->y1 > edge->y0) {
        edge->x_intercept = edge->x0;
        //if (float_is_greater(next_edge->y1, edge->y1)) {
        //  edge->y1 -= drawing_surface->pixel_height; //FLOAT_EPSILON;
        //}
      }
      else if (edge->y1 < edge->y0) {
        edge->x_intercept = edge->x1;
        //if (float_is_greater(prev_edge->y0, edge->y0)) {
        //  prev_edge->y1 += drawing_surface->pixel_height; //FLOAT_EPSILON;
        //}
      }
      else assert(false);
    }
    edge_list[i].count = edge_count;
//    printf("Contour #%d\n", i);
//    print_edge_list(&edge_list[i]);
  }

  polygon->edge_list = edge_list;
}

void
update_x_intercept(Edge* edge, float y) {
  if (edge->m != INFINITY) {
    edge->x_intercept = (y - edge->b)/edge->m;
  }
}

Matrix3
new_empty_matrix3() {
  Matrix3 result = {0};
  return result;
}

float
y_intercept_at(DrawingSurface* drawing_surface, int scanline_nr) {
  float result = drawing_surface->y_min + (drawing_surface->pixel_height * scanline_nr);
  return result;
}

void
draw_polygon(Polygon* polygon, DrawingSurface* drawing_surface, DeviceWindow* device_window) {
  EdgeList edge_heap = {0};
  edge_heap.entries = push_array(Edge, polygon->total_vertex_count);
  edge_heap.count = 0;
  edge_heap.entries[0] = new_empty_edge();
  edge_heap.entries[0].y0 = drawing_surface->y_min;
  for (int i = 0; i < polygon->n_contours; ++i) {
    EdgeList* edge_list = &polygon->edge_list[i];
    for (int j = 0; j < edge_list->count; ++j) {
      Edge* edge = &edge_list->entries[j];
      assert (!float_is_equal(edge->y1, edge->y0));
      if (edge->y1 < edge->y0) {
        Point p = edge->start_point;
        edge->start_point = edge->end_point;
        edge->end_point = p;
      }
      edge->delta_x = edge->x1 - edge->x0;
      edge->delta_y = edge->y1 - edge->y0;
      assert (!float_is_zero(edge->delta_y));
      edge->x_intercept = edge->x0;
      edge->m = edge->delta_y / edge->delta_x;  // y = m*x + b
      edge->b = edge->y0 - edge->m*edge->x0;
      add_polygon_edge(&edge_heap, edge);
    }
  }

  EdgeList active_edge_list = {};
  active_edge_list.entries = push_array(Edge, edge_heap.count + 2);
  active_edge_list.entries += 1;
  active_edge_list.count = 0;
  active_edge_list.entries[-1] = new_empty_edge();
  active_edge_list.entries[-1].x_intercept = drawing_surface->x_min;
  active_edge_list.entries[0] = new_empty_edge();
  active_edge_list.entries[0].x_intercept = drawing_surface->x_max;

  assert(edge_heap.count >= 2);
  int at_y = 0;
  float y = y_intercept_at(drawing_surface, at_y);
  while (edge_heap.count > 0 &&
         (edge_heap.entries[1].y0 >= y) &&
         (edge_heap.entries[1].y0 - y) < drawing_surface->pixel_height) {
    Edge edge = pop_polygon_edge(&edge_heap);
    insert_active_edge(&active_edge_list, &edge);
  }
  do {
    printf("--------------- %d -----------------\n", at_y);
    printf("y = %.4f\n", y);
    for (int i = 0; i < active_edge_list.count; ++i) {
      Edge* edge = &active_edge_list.entries[i];
      update_x_intercept(edge, y);
    }

    sort_active_edge_list(&active_edge_list);
    print_edge_list(&active_edge_list);

    assert((active_edge_list.count % 2) == 0);
    for (int i = 0; i < active_edge_list.count; i += 2) {
      Edge* left_edge = &active_edge_list.entries[i];
      Edge* right_edge = &active_edge_list.entries[i+1];
      printf("left_edge=(%.6f,%.6f)\n", left_edge->x_intercept, y);
      printf("right_edge=(%.6f,%.6f)\n", right_edge->x_intercept, y);

      set_pixel_on_device_window(drawing_surface, device_window, left_edge->x_intercept, y);
      for (float x = left_edge->x_intercept;
           x < right_edge->x_intercept;
           x += drawing_surface->pixel_width) {
        set_pixel_on_device_window(drawing_surface, device_window, x, y);
      }
    }
    ++at_y;
    y = y_intercept_at(drawing_surface, at_y);
    for (int i = 0; i < active_edge_list.count;) {
      Edge* edge = &active_edge_list.entries[i];
      if (fabs(edge->y1 - y) < drawing_surface->pixel_height) {
        remove_active_edge(&active_edge_list, edge, i);
        continue;
      }
      ++i;
    }
    while (edge_heap.count > 0 &&
           fabs(edge_heap.entries[1].y0 - y) < drawing_surface->pixel_height) {
      Edge edge = pop_polygon_edge(&edge_heap);
      insert_active_edge(&active_edge_list, &edge);
    }
  } while (at_y < drawing_surface->y_pixel_count);
}


void
clear_device_window(DeviceWindow* device_window, uint8_t intensity) {
  int i, j;
  uint32_t* p = device_window->pixel_buffer;
  for (int j = 0; j < device_window->height; ++j) {
    for (int i = 0; i < device_window->width; ++i) {
      *p++ = make_grayscale_rgb32(intensity);
    }
  }
}

#if 0/*>--*/
void
line(int x0, int y0, int x1, int y1) {
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
      draw_pixel_black(x, y);
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
      draw_pixel_black(*p_x, *p_y);
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
  line(x0, y0, x1, y1);
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
#endif/*--<*/

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
  assert(!float_is_equal(p1->x, p0->x));
  float m = (p1->y - p0->y)/(float)(p1->x - p0->x);
  result->y = p1->y + m*(x - p1->x);
  result->x = x;
}

void
horizontal_line_intersection(Point* p0, Point* p1, Point* result, int y) {
  assert(!float_is_equal(p1->y, p0->y));
  float m = (p1->y - p0->y)/(float)(p1->x - p0->x);
  result->x = p1->x + (y - p1->y)/m;
  result->y = y;
}

bool
does_intersect_clipping_edge(Point* p0, Point* p1, ClippingEdge clipping_edge, float clipping_boundary[static ClipEdge_COUNT]) {
  bool result = false;
  if (clipping_edge == ClipEdge_Left || clipping_edge == ClipEdge_Right) {
    int x = clipping_boundary[clipping_edge];
    result = !float_is_equal(p0->x, p1->x) &&
      ((float_is_greater(p0->x, x) && float_is_less(p1->x, x)) ||
       (float_is_less(p0->x, x) && (p1->x > x)));
  }
  else if (clipping_edge == ClipEdge_Bottom || clipping_edge == ClipEdge_Top) {
    int y = clipping_boundary[clipping_edge];
    result = !float_is_equal(p0->y, p1->y) &&
      ((float_is_greater(p0->y, y) && float_is_less(p1->y, y)) ||
       (float_is_less(p0->y, y) && float_is_greater(p1->y, y)));
  }
  else {
    assert(false);
  }
  return result;
}

void
get_clip_edge_intersection(Point* r, Point* v, Point* result,
                           ClippingEdge clipping_edge, float clipping_boundary[static ClipEdge_COUNT]) {
  if (clipping_edge == ClipEdge_Left || clipping_edge == ClipEdge_Right) {
    vertical_line_intersection(r, v, result, clipping_boundary[clipping_edge]);
  }
  else if (clipping_edge == ClipEdge_Bottom || clipping_edge == ClipEdge_Top) {
    horizontal_line_intersection(r, v, result, clipping_boundary[clipping_edge]);
  }
}

bool
is_point_inside_clip_boundary(Point* v, ClippingEdge clipping_edge, float clipping_boundary[static ClipEdge_COUNT]) {
  bool result = false;
  if (clipping_edge == ClipEdge_Left) {
    if (float_is_greater_or_equal(v->x, clipping_boundary[ClipEdge_Left])) {
      result = true;
    }
  }
  else if (clipping_edge == ClipEdge_Right) {
    if (float_is_less_or_equal(v->x, clipping_boundary[ClipEdge_Right])) {
      result = true;
    }
  }
  else if (clipping_edge == ClipEdge_Bottom) {
    if (float_is_greater_or_equal(v->y, clipping_boundary[ClipEdge_Bottom])) {
      result = true;
    }
  }
  else if (clipping_edge == ClipEdge_Top) {
    if (float_is_less_or_equal(v->y, clipping_boundary[ClipEdge_Top])) {
      result = true;
    }
  }
  else {
    assert(false);
  }
  return result;
}

void
do_clip_point(Point* v, ClippingEdge clipping_edge, Point* first_clipped[static ClipEdge_COUNT],
              Point* recently_clipped[static ClipEdge_COUNT], float clipping_boundary[static ClipEdge_COUNT],
              Point* clipped_contour, int* clipped_vertex_count) {
  if (!first_clipped[clipping_edge]) {
    first_clipped[clipping_edge] = v;
  }
  else {
    if (recently_clipped[clipping_edge]) {
      if (does_intersect_clipping_edge(recently_clipped[clipping_edge], v, clipping_edge, clipping_boundary)) {
        Point* intersection_point = push_struct(Point);
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

Shape
clip_shape(Shape* shape, float clipping_boundary[static ClipEdge_COUNT]) {
  Shape clipped_shape = {0};
  clipped_shape.n_contours = shape->n_contours;
  clipped_shape.contours = push_array(int, shape->n_contours);
  clipped_shape.points = push_array(Point, shape->total_point_count*2);
  Point* clipped_contour = clipped_shape.points;
  Point* shape_points = shape->points;
  for (int i = 0; i < shape->n_contours; ++i) {
    int contour_vertex_count = shape->contours[i];
    int clipped_vertex_count = 0;
    Point* recently_clipped[ClipEdge_COUNT] = {0};
    Point* first_clipped[ClipEdge_COUNT] = {0};
    for (int j = 0; j < contour_vertex_count; ++j) {
      Point* v = shape_points++;
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
        Point* intersection_point = push_struct(Point);
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
    clipped_shape.contours[i] = clipped_vertex_count;
    clipped_contour += clipped_vertex_count;
    clipped_shape.total_point_count += clipped_vertex_count;
  }
  return clipped_shape;
}

void
draw(DeviceWindow* device_window) {
  DrawingSurface drawing_surface = {0};
  drawing_surface.x_pixel_count = device_window->width;
  drawing_surface.y_pixel_count = device_window->height;
  drawing_surface.x_min = -1.f;
  drawing_surface.x_max = 1.f;
  drawing_surface.y_min = -1.f;
  drawing_surface.y_max = 1.f;
  drawing_surface.width = drawing_surface.x_max - drawing_surface.x_min;
  drawing_surface.height = drawing_surface.y_max - drawing_surface.y_min;
  drawing_surface.pixel_width = drawing_surface.width / drawing_surface.x_pixel_count;
  drawing_surface.pixel_height = drawing_surface.height / drawing_surface.y_pixel_count;

  Shape* shape = find_shape(L'A');
  assert (shape);
  Rectangle shape_bb = get_bounding_box(shape);
  printf("Bounding box: (%0.4f, %0.4f), (%0.4f, %0.4f)\n",
        shape_bb.lower_left.x, shape_bb.lower_left.y, shape_bb.upper_right.x, shape_bb.upper_right.y);

  ViewWindow view_window = {0};
  view_window.lower_left = shape_bb.lower_left;
  view_window.width = 1000.f;
  view_window.height = 1000.f;
  view_window.upper_right.x = view_window.lower_left.x + view_window.width;
  view_window.upper_right.y = view_window.lower_left.y + view_window.height;
  view_window.center.x = view_window.lower_left.x + view_window.width/2.f;
  view_window.center.y = view_window.lower_left.y + view_window.height/2.f;

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

  printf("---- BEFORE CLIPPING ----\n");
  print_shape_points(shape);
  Shape clipped_shape = clip_shape(shape, clipping_boundary);
  shape = &clipped_shape;
  printf("---- AFTER CLIPPING ----\n");
  print_shape_points(shape);

  Matrix3 translate_window = {0};
  mk_translate_matrix(&translate_window, -view_window.center.x, -view_window.center.y);
  apply_xform(shape, &translate_window);

  Matrix3 scale_window = {0};
  mk_scale_matrix(&scale_window, 1.f/view_window.width, 1.f/view_window.height);
  apply_xform(shape, &scale_window);

  clear_device_window(device_window, 255);
  if (shape->total_point_count > 0) {
    Polygon polygon = {0};
    make_polygon(&polygon, shape, &drawing_surface);
    draw_polygon(&polygon, &drawing_surface, device_window);
  }
}

