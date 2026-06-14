#include <windows.h>

struct wframe {
    BITMAPINFO frame_bitmap_info;
    HBITMAP frame_bitmap;
    HWND window_handle;
    HDC device_context, frame_device_context;
    struct frame f;
};

#define windows 6
struct wframe tex[windows];