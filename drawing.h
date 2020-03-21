// -*- coding: utf-8 -*-

#define local static
#define global static
#define internal static
#define persistent static
#define true 1u
#define false 0u
#define bool uint32_t
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE
#define PI 3.141592f
#define FLOAT_EPSILON 0.000001

#define sizeof_array(array) (sizeof(array)/sizeof(array[0]))

typedef struct {
  union {
    uint32_t bytes;
    struct {
      uint8_t R;
      uint8_t G;
      uint8_t B;
      union {
        uint8_t blackness;
        uint8_t X;
      };
    };
  };
} RgbPixel;

typedef struct {
  uint32_t R;
  uint32_t G;
  uint32_t B;
} Color;

global Color COLOR_RED = {.R=255, .G=0, .B=0};
global Color COLOR_GREEN = {.R=255, .G=0, .B=0};
global Color COLOR_BLUE = {.R=0, .G=0, .B=255};
global Color WHITE = {.R=255, .G=255, .B=255};
global Color BLACK = {.R=0, .G=0, .B=0};

typedef struct {
  float x;
  float y;
  float z;
} fVector3, fPoint;

typedef struct {
  int x;
  int y;
  int z;
} iVector3, iPoint;

typedef struct {
  union {
    fPoint lower_left;
    struct {
      float lower_left_x;
      float lower_left_y;
    };
  };
  union {
    fPoint upper_right;
    struct {
      float upper_right_x;
      float upper_right_y;
    };
  };
} fRectangle;

typedef struct {
  union {
    iPoint lower_left;
    struct {
      int lower_left_x;
      int lower_left_y;
    };
  };
  union {
    iPoint upper_right;
    struct {
      int upper_right_x;
      int upper_right_y;
    };
  };
} iRectangle;

typedef struct {
  union {
    fPoint start_point;
    struct {
      float x0, y0;
    };
  };
  union {
    fPoint end_point;
    struct {
      float x1, y1;
    };
  };
} fLine;

typedef struct {
  wchar_t character;
  int* contours;
  int n_contours;
  fPoint* points;
  int total_point_count;
  fRectangle* bbox;
} Shape;

typedef struct {
  int* contours;
  int n_contours;
  iPoint* points;
  int total_point_count;
} RasterShape;

typedef struct Edge {
  int x_intercept;
  float m, b; // y = m*x + b
  float delta_x;
  float delta_y;
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
  struct Edge* prev_edge;
  struct Edge* next_edge;
} Edge;

typedef struct {
  Edge* entries;
  int count;
} EdgeList;

typedef struct {
  iPoint** contours;
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
    fVector3 col1;
  };
  union {
    struct {
      float x2;
      float y2;
      float z2;
    };
    fVector3 col2;
  };
  union {
    struct {
      float x3;
      float y3;
      float z3;
    };
    fVector3 col3;
  };
} fMatrix3;

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
  uint8_t* buffer;
  int square_side;
} RasterSubpixel;

typedef struct {
  uint8_t* pixel_scanline;
  int surface_subpixel_count;
  int subpixel_count;
  int pixel_count_y, pixel_count_x;
  uint8_t* at_pixel;
  int pixel_square_side;
  uint8_t* subpixel_buffer;
  int at_pixel_i, at_pixel_j;
} RasterSubpixelReader;

typedef struct {
  int subsampling_factor;
  float* blackness_levels;
  int width;
  int height;
  int min_x, min_y;
  int max_x, max_y;
  int subpixel_count;
  uint8_t* subpixel_buffer;
} RasterSurface;

typedef struct {
  fPoint lower_left;
  fPoint upper_right;
  float width;
  float height;
} ViewWindow;

typedef struct {
  uint32_t* backbuffer;
  void* framebuffer;
  int framebuffer_size_bytes;
  int backbuffer_size_pixels;
  int width;
  int height;
  int bytes_per_pixel;
  int bits_per_pixel;
} DeviceWindow;

