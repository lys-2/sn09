#include <windows.h>
void text(char* t, float x, float y, int m, int id);
struct wframe {
    BITMAPINFO frame_bitmap_info;
    HBITMAP frame_bitmap;
    HWND window_handle;
    HDC device_context, frame_device_context;
    struct frame f;
};

#define windows 6
struct wframe tex[windows];