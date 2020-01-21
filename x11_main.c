// -*- coding: utf-8 -*-

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <wchar.h>

#define local static
#define global static
#define internal static
#define true 1u
#define false 0u
#define bool uint32_t
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE
#define PI 3.141592f
#define FLOAT_EPSILON 0.000001

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

#include "drawing_struct.h"

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

global Color COLOR_RED = {.R=255, .G=0, .B=0};
global Color COLOR_GREEN = {.R=255, .G=0, .B=0};
global Color COLOR_BLUE = {.R=0, .G=0, .B=255};
global Color WHITE = {.R=255, .G=255, .B=255};
global Color BLACK = {.R=0, .G=0, .B=0};

#include "drawing.c"

xcb_image_t*
create_x11_image(xcb_connection_t* conn, DeviceWindow* device_window) {
  const xcb_setup_t* setup = xcb_get_setup(conn);
  xcb_format_t* fmt_at = xcb_setup_pixmap_formats(setup);
  xcb_format_t* fmt_end = fmt_at + xcb_setup_pixmap_formats_length(setup);
  xcb_format_t* fmt = 0;
  for(; fmt_at != fmt_end; ++fmt_at) {
    if((fmt_at->depth == device_window->depth) && (fmt_at->bits_per_pixel == device_window->bits_per_pixel)) {
      //printf("fmt %p has pad %d depth %d, bpp %d\n", fmt_at, fmt_at->scanline_pad, device_window->depth, device_window->bits_per_pixel);
      fmt = fmt_at;
      break;
    }
  }

  if (fmt == 0) {
    return 0;
  }

  int image_size_bytes = device_window->width * device_window->height * device_window->bytes_per_pixel;
  uint8_t* image_bytes = push_array(uint8_t, image_size_bytes);
  xcb_image_t* result = xcb_image_create(device_window->width, device_window->height, XCB_IMAGE_FORMAT_Z_PIXMAP,
                                         fmt->scanline_pad, fmt->depth, fmt->bits_per_pixel,
                                      0, setup->image_byte_order, XCB_IMAGE_ORDER_LSB_FIRST,
                                      image_bytes, image_size_bytes, image_bytes);
  return result;
}

void
blit_device_window_to_x11_image(DeviceWindow* device_window, xcb_image_t* x11_image) {
#if 0
  // verbatim
  for (int i = 0; i < image_height; ++i) {
    uint32_t* src_line = image_buffer + image_width*i;
    uint32_t* dest_line = (uint32_t*)image->data + image_width*i;
    for (int j = 0; j < image_width; ++j) {
      uint32_t p = src_line[j];
      dest_line[j] = p;
    }
  }
#else
  // vertically flipped
  for (int i = 0; i < device_window->height; ++i) {
    uint32_t* src_line = device_window->pixel_buffer + device_window->width*i;
    uint32_t* dest_line = (uint32_t*)x11_image->data + device_window->width*(device_window->height-1) - device_window->width*i;
    for (int j = 0; j < device_window->width; ++j) {
      uint32_t p = src_line[j];
      dest_line[j] = p;
    }
  }
#endif
}

int
main(int argc, char** argv) {
  uint32_t values[2];

  arena.memory = malloc(20*MEGABYTE);
  if (!arena.memory) {
    printf("ERROR\n");
    exit(1);
  }
  arena.avail = arena.memory;

  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  if (!connection) {
    printf("ERROR\n");
    exit(1);
  }

  const xcb_setup_t* setup = xcb_get_setup(connection);
  xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;
  //printf("root depth %d\n",screen->root_depth);

  DeviceWindow device_window = {0};
  device_window.width = 100;
  device_window.height = 100;
  device_window.bytes_per_pixel = 4;
  device_window.bits_per_pixel = device_window.bytes_per_pixel*8;
  device_window.depth = 24; // FIXME: What is this and why should it be equal to 24?

  xcb_image_t* x11_image = create_x11_image(connection, &device_window);
  if (x11_image == 0) {
    printf("ERROR\n");
    xcb_disconnect(connection);
    return 1;
  }
  device_window.pixel_buffer = push_array(uint32_t, device_window.width * device_window.height);

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_BUTTON_PRESS;

  xcb_window_t x11_window = xcb_generate_id(connection);
  xcb_create_window(connection, device_window.depth/*XCB_COPY_FROM_PARENT?*/, x11_window, screen->root,
                    0/*x*/, 0/*y*/, x11_image->width, x11_image->height, 0/* border_width */,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

  char* title = "Drawing";
  xcb_icccm_set_wm_name(connection, x11_window, XCB_ATOM_STRING, 8, strlen(title), title);

  xcb_pixmap_t pixmap = xcb_generate_id(connection);
  xcb_create_pixmap(connection, device_window.depth, pixmap, x11_window, x11_image->width, x11_image->height);

  /* Create pixmap plot gc */
  mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
  values[0] = screen->black_pixel;
  values[1] = 0x00ffffff;

  xcb_gcontext_t gc = xcb_generate_id (connection);
  xcb_create_gc(connection, gc, pixmap, mask, values);

  /* Put the image into the pixmap */
  xcb_image_put(connection, pixmap, gc, x11_image, 0, 0, 0);

  /* Show the window */
  xcb_map_window(connection, x11_window);
  xcb_flush(connection);

  draw(&device_window);
  //draw_string(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n0123456789\n~!@#$%^&*()_+-{}|:\"<>?`[]\\;',./", &drawing_surface);
  blit_device_window_to_x11_image(&device_window, x11_image);

  xcb_image_put(connection, pixmap, gc, x11_image, 0, 0, 0);
  xcb_copy_area(connection, pixmap, x11_window, gc, 0, 0, 0, 0, x11_image->width, x11_image->height);
  xcb_flush(connection);

  xcb_generic_event_t* e;
  int done = false;
  while (!done && (e = xcb_wait_for_event(connection))) {
    switch (e->response_type) {
      case XCB_EXPOSE: {
        xcb_expose_event_t* ee = (xcb_expose_event_t*)e;
        //printf("expose %d,%d - %d,%d\n", ee->x, ee->y, ee->width, ee->height);
        xcb_copy_area(connection, pixmap, x11_window, gc,
                      ee->x, ee->y, ee->x, ee->y, ee->width, ee->height);
        xcb_flush(connection);
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

  xcb_free_pixmap(connection, pixmap);
  xcb_disconnect (connection);

  return 0;
}

