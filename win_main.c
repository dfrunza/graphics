// -*- coding: utf-8 -*-

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#include <tchar.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <wchar.h>
#include <malloc.h>

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
#include "win_main.h"

LRESULT CALLBACK winproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  persistent WinDeviceWindow* device_window = 0;
  LRESULT result = 0;
  switch (msg) {
    case WM_CLOSE:
    case WM_DESTROY:
      device_window->is_running = false;
      break;

    case WM_CREATE:
      {
        CREATESTRUCT* create_struct = (CREATESTRUCT*)lparam;
        device_window = (WinDeviceWindow*)create_struct->lpCreateParams;
      }
      break;

    case WM_PAINT:
      {
        PAINTSTRUCT paint_struct = {};
        HDC paint_hdc = BeginPaint(hwnd, &paint_struct);
        StretchDIBits(paint_hdc, 0, 0, device_window->width, device_window->height,
                      0, 0, device_window->width, device_window->height,
                      device_window->framebuffer, &device_window->bitmap_info, DIB_RGB_COLORS, SRCCOPY);
        EndPaint(hwnd, &paint_struct);
      }
      break;

    case WM_KEYDOWN:
      device_window->is_running = false;
      break;

    default:
      result = DefWindowProc(hwnd, msg, wparam, lparam);
      break;
    }
  return result;
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prev_hinstance, LPSTR cmd_str, int cmd_show) {
  arena.memory = malloc(20*MEGABYTE);
  if (!arena.memory) {
    printf("ERROR\n");
    exit(1);
  }
  arena.avail = arena.memory;

  char* title = "Drawing";
  WNDCLASSEX winclass = {};
  winclass.cbSize = sizeof(WNDCLASSEX);
  winclass.style = CS_DBLCLKS;
  winclass.lpfnWndProc = &winproc;
  winclass.hInstance = hinstance;
  winclass.hIcon = LoadIcon(0, IDI_WINLOGO);
  winclass.hCursor = LoadCursor(0, IDC_ARROW);
  winclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winclass.lpszClassName = title;

  if (!RegisterClassEx(&winclass)) {
    printf("ERROR\n");
    return 0;
  }

  WinDeviceWindow device_window = {};
  device_window.width = 100;
  device_window.height = 100;
  device_window.bytes_per_pixel = 4;
  device_window.bits_per_pixel = device_window.bytes_per_pixel*8;
  device_window.backbuffer_size_pixels = device_window.width*device_window.height;
  device_window.framebuffer_size_bytes = device_window.backbuffer_size_pixels*device_window.bytes_per_pixel;

  DWORD window_style = WS_OVERLAPPEDWINDOW;
  HWND hwnd = CreateWindow(title, title, window_style,
                           0, 0, device_window.width, device_window.height,
                           0, 0, hinstance, &device_window);
  RECT window_rect = {};
  GetWindowRect(hwnd, &window_rect);
  RECT client_rect = {};
  GetClientRect(hwnd, &client_rect);
  RECT adjusted_window_rect = {};
  adjusted_window_rect.left = window_rect.left;
  adjusted_window_rect.top = window_rect.top;
  adjusted_window_rect.right = window_rect.right + (device_window.width - client_rect.right);
  adjusted_window_rect.bottom = window_rect.bottom + (device_window.height - client_rect.bottom);
  SetWindowPos(hwnd, 0, adjusted_window_rect.left, adjusted_window_rect.top, adjusted_window_rect.right, adjusted_window_rect.bottom, 0);

  device_window.backbuffer = push_array(uint32_t, device_window.width*device_window.height);

  BITMAPINFO* bmi = &device_window.bitmap_info;
  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = device_window.width;
  bmi->bmiHeader.biHeight = -device_window.height;  /* top-down */
  bmi->bmiHeader.biPlanes = 1;
  bmi->bmiHeader.biBitCount = device_window.bits_per_pixel;
  bmi->bmiHeader.biCompression = BI_RGB;
  bmi->bmiHeader.biSizeImage = 0;  /* May be set to 0 for BI_RGB bitmaps */
  device_window.framebuffer = push_array(uint8_t, device_window.framebuffer_size_bytes);

  draw((DeviceWindow*)&device_window);
  copy_backbuffer_to_framebuffer((DeviceWindow*)&device_window);

  ShowWindow(hwnd, cmd_show);
  UpdateWindow(hwnd);

  device_window.is_running = true;
  MSG msg = {};
  while (device_window.is_running) {
    if (GetMessage(&msg, 0, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  return msg.wParam;
}
