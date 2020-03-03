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

#include "drawing.h"

global Arena arena;

#include "drawing.c"
#include "x11_main.h"

xcb_image_t* create_x11_framebuffer(xcb_connection_t* conn, X11DeviceWindow* device_window) {
  const xcb_setup_t* setup = xcb_get_setup(conn);
  xcb_format_t* format_at = xcb_setup_pixmap_formats(setup);
  xcb_format_t* format_end = format_at + xcb_setup_pixmap_formats_length(setup);
  xcb_format_t* format = 0;
  for(; format_at != format_end; ++format_at) {
    if((format_at->depth == device_window->depth) && (format_at->bits_per_pixel == device_window->bits_per_pixel)) {
      //printf("format %p has pad %d depth %d, bpp %d\n", format_at, format_at->scanline_pad, device_window->depth, device_window->bits_per_pixel);
      format = format_at;
      break;
    }
  }

  if (!format) {
    return 0;
  }

  device_window->framebuffer = push_array(uint8_t, device_window->framebuffer_size_bytes);
  xcb_image_t* result = xcb_image_create(device_window->width, device_window->height, XCB_IMAGE_FORMAT_Z_PIXMAP,
                                         format->scanline_pad, format->depth, format->bits_per_pixel,
                                         0, setup->image_byte_order, XCB_IMAGE_ORDER_LSB_FIRST,
                                         device_window->framebuffer, device_window->framebuffer_size_bytes, device_window->framebuffer);
  return result;
}

int main(int argc, char** argv) {
  uint32_t values[2];

  arena.memory = malloc(20*MEGABYTE);
  if (!arena.memory) {
    printf("ERROR\n");
    return -1;
  }
  arena.avail = arena.memory;

  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  if (!connection) {
    printf("ERROR\n");
    return -1;
  }

  const xcb_setup_t* setup = xcb_get_setup(connection);
  xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;
  //printf("root depth %d\n",screen->root_depth);

  X11DeviceWindow device_window = {};
  device_window.width = 400;
  device_window.height = 400;
  device_window.bytes_per_pixel = 4;
  device_window.bits_per_pixel = device_window.bytes_per_pixel*8;
  device_window.backbuffer_size_pixels = device_window.width*device_window.height;
  device_window.framebuffer_size_bytes = device_window.backbuffer_size_pixels*device_window.bytes_per_pixel;
  device_window.depth = 24; // FIXME: What is this and why should it be equal to 24?

  device_window.backbuffer = push_array(uint32_t, device_window.width * device_window.height);
  xcb_image_t* x11_image = create_x11_framebuffer(connection, &device_window);
  if (x11_image == 0) {
    printf("ERROR\n");
    xcb_disconnect(connection);
    return -1;
  }

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

  mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
  values[0] = screen->black_pixel;
  values[1] = 0x00ffffff;

  xcb_gcontext_t gc = xcb_generate_id(connection);
  xcb_create_gc(connection, gc, pixmap, mask, values);

  xcb_image_put(connection, pixmap, gc, x11_image, 0, 0, 0);

  /* Show the window */
  xcb_map_window(connection, x11_window);
  xcb_flush(connection);

  draw((DeviceWindow*)&device_window);
  copy_backbuffer_to_framebuffer((DeviceWindow*)&device_window);

  xcb_image_put(connection, pixmap, gc, x11_image, 0, 0, 0);
  xcb_copy_area(connection, pixmap, x11_window, gc, 0, 0, 0, 0, x11_image->width, x11_image->height);
  xcb_flush(connection);

  bool is_running = true;
  while (is_running) {
    xcb_generic_event_t* e = 0;
    if (e = xcb_wait_for_event(connection)) {
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
          is_running = false;
          break;

        case XCB_BUTTON_PRESS:
          break;
      }
      free(e);
    }
  }

  xcb_free_pixmap(connection, pixmap);
  xcb_disconnect(connection);

  return 0;
}

