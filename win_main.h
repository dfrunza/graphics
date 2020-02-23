typedef struct {
  DeviceWindow;
  bool is_running;
  BITMAPINFO bitmap_info;
  void* framebuffer;
} WinDeviceWindow;
