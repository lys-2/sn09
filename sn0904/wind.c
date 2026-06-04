#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include "sn.h"
#define windows 31

int quit;

struct wframe {
    BITMAPINFO frame_bitmap_info;
    HBITMAP frame_bitmap;
    HWND window_handle;
    HDC device_context, frame_device_context;
    struct frame f;
};

struct wframe tex[windows];

PAINTSTRUCT ps;
WNDCLASS window_class;
MSG msg;
HFONT font;
HANDLE hConsole;
HWND consoleWindow;

int get_window(HWND window_handle) {
    for (int i = 0; i < windows; i++) {
        if (tex[i].window_handle == window_handle) { return i; }
    }
    return -1;
}

void text(char* t, float x, float y, int m, int id) {

    font = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, "Comic Sans MS");

    SelectObject(tex[id].frame_device_context, font);
    SetTextColor(tex[id].frame_device_context, RGB(111, 255, 255));
    SetBkMode(tex[id].frame_device_context, m);
    SetBkColor(tex[id].frame_device_context, RGB(0, 111, 111));
    RECT r = { x,  tex[id].f.height - y, x + 33,  tex[id].f.height - y + 33 };
    DrawTextA(tex[id].frame_device_context, t, -1, &r, 0);

    DeleteObject(font);
}

LRESULT CALLBACK wpm(HWND window_handle,
    UINT message, WPARAM wParam, LPARAM lParam) {

    int id = get_window(window_handle);

    if (id == -1) {

        return DefWindowProc(window_handle, message, wParam, lParam);

    }

    int x = LOWORD(lParam);
    int y = tex[id].f.height-1 - HIWORD(lParam);

    if (message == WM_KEYDOWN && wParam == 'R') {}
    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) { quit = 1; }
    if (message == WM_LBUTTONDOWN) {
        point(tex[id].f, x, y, s.pick);
    }
    if (message == WM_RBUTTONDOWN) {
        s.pick = pick(tex[id].f, x, y, id);
    }

    switch (message) {
    case WM_QUIT: {} break;
    case WM_DESTROY: { quit = 1; } break;
    case WM_MOUSEMOVE: {
        spawn((struct node) { (
            struct v2) {x,y},
            s.pick
        });
        printf("m%i %i ~%i~\n", x, y, id);
    } break;

    case WM_PAINT: {

        tex[id].device_context = BeginPaint(window_handle, &ps);

        clear(tex[id].f);
        if (id==0) 
            colors(tex[id].f, id);
        else
            paint(tex[id].f, id);

        char str[64]; sprintf(str, "Hi!");
        text(str, 1, tex[id].f.height - 11, OPAQUE, id);

        BitBlt(tex[id].device_context,
            ps.rcPaint.left,
            ps.rcPaint.top,
            ps.rcPaint.right - ps.rcPaint.left,
            ps.rcPaint.bottom - ps.rcPaint.top,
            tex[id].frame_device_context,
            ps.rcPaint.left, ps.rcPaint.top,
            SRCCOPY);

        EndPaint(window_handle, &ps);

        //  SetWindowTextA(window_handle, "snry rpg sn0833");

    } break;

    case WM_SIZE: {

        tex[id].frame_bitmap_info.bmiHeader.biWidth = LOWORD(lParam);
        tex[id].frame_bitmap_info.bmiHeader.biHeight = HIWORD(lParam);

        if (tex[id].frame_bitmap) DeleteObject(tex[id].frame_bitmap);
        tex[id].frame_bitmap = CreateDIBSection(
            NULL,
            &tex[id].frame_bitmap_info,
            DIB_RGB_COLORS,
            (void**)&tex[id].f.pixels,
            0,
            0
        );
        tex[id].f.width = LOWORD(lParam);
        tex[id].f.height = HIWORD(lParam);
        SelectObject(tex[id].frame_device_context, tex[id].frame_bitmap);
        InvalidateRect(window_handle, NULL, TRUE);

    } break;

    default: {
        return DefWindowProc(window_handle, message, wParam, lParam);
    }
    }
    return 0;
}

void console() {
    FILE* conin = stdin;
    FILE* conout = stdout;
    FILE* conerr = stderr;
    AllocConsole();
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    freopen_s(&conin, "CONIN$", "r", stdin);
    freopen_s(&conout, "CONOUT$", "w", stdout);
    freopen_s(&conerr, "CONOUT$", "w", stderr);
    //SetConsoleTitleA("console ");
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR pCmdLine, int nCmdShow) {

    window_class.lpfnWndProc = wpm;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = "CLASS";
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&window_class);

    for (int i = 0; i < windows; i++) {
        tex[i].frame_bitmap_info.bmiHeader.biSize =
            sizeof(tex[i].frame_bitmap_info.bmiHeader);
        tex[i].frame_bitmap_info.bmiHeader.biPlanes = 1;
        tex[i].frame_bitmap_info.bmiHeader.biBitCount = 32;
        tex[i].frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
        tex[i].frame_device_context = CreateCompatibleDC(0);

        tex[i].window_handle = CreateWindowA(
            "CLASS", L"snry template", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            123 + i*4, 123 + i, 512 - i*2, 256 - i+39, NULL, NULL, hInstance, NULL
        );

            tex[i].f.width = 512 - i * 2;
            tex[i].f.height = 256 - i;
            tex[i].frame_bitmap_info.bmiHeader.biWidth = tex[i].f.width;
            tex[i].frame_bitmap_info.bmiHeader.biHeight = tex[i].f.height;
            tex[i].frame_bitmap = CreateDIBSection(NULL,
                &tex[i].frame_bitmap_info, DIB_RGB_COLORS,
                (void**)&tex[i].f.pixels, 0, 0);
            SelectObject(tex[i].frame_device_context, tex[i].frame_bitmap);

            clear(tex[i].f);
            paint(tex[i].f, i);
            tex[i].device_context = BeginPaint(tex[i].window_handle, &ps);
            BitBlt(tex[i].device_context,
                ps.rcPaint.left,
                ps.rcPaint.top,
                ps.rcPaint.right - ps.rcPaint.left,
                ps.rcPaint.bottom - ps.rcPaint.top,
                tex[i].frame_device_context,
                ps.rcPaint.left, ps.rcPaint.top,
                SRCCOPY);

            EndPaint(tex[i].window_handle, &ps);
            InvalidateRect(tex[i].window_handle, NULL, TRUE);

        
    }

    console();
    consoleWindow = GetConsoleWindow();
    SetWindowPos(consoleWindow, 0, 33, 432, 512, 256, 0);
    SetForegroundWindow(tex[windows-1].window_handle);

    while (!quit) {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }
        for (int i = 0; i < windows; i++) {

            InvalidateRect(tex[i].window_handle, NULL, FALSE);
            UpdateWindow(tex[i].window_handle);
        }
        F++;
       Sleep(1);
    }
    return 0;

}