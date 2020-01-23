// -*- coding: utf-8 -*-

typedef struct {
  union {
    uint32_t bytes;
    struct {
      uint8_t R;
      uint8_t G;
      uint8_t B;
      uint8_t X;
    };
  };
  uint8_t blackness;
} RgbPixel;

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
  Point lower_left;
  Point upper_right;
} Rectangle;

typedef struct {
  wchar_t character;
  int* contours;
  int n_contours;
  Point* points;
  int total_point_count;
} Shape;

typedef struct Edge {
  float x_intercept;
  float m, b; // y = m*x + b
  union {
    struct {
      float x0;
      float y0;
      float z0;
    };
    Point start_point;
  };
  union {
    struct {
      float x1;
      float y1;
      float z1;
    };
    Point end_point;
  };
  union {
    struct {
      float delta_x;
      float delta_y;
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
  Point** contours;
  int* contour_vertex_count;
  int total_vertex_count;
  int n_contours;
  EdgeList* edge_list;
} Polygon;

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
  int x_pixel_count;
  int y_pixel_count;
  float pixel_width;
  float pixel_height;
  float x_min;
  float x_max;
  float y_min;
  float y_max;
  float width;
  float height;
} DrawingSurface;

typedef struct {
  Point lower_left;
  Point upper_right;
  Point center;
  float width;
  float height;
} ViewWindow;

typedef struct {
  uint32_t* pixel_buffer;
  uint8_t* blackness_buffer;
  int width;
  int height;
  int bytes_per_pixel;
  int bits_per_pixel;
  int depth;
} DeviceWindow;

