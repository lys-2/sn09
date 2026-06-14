#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include "sn.h"
#include "wa.h"
#include "Wm.h"
#include "wind.h"
#include "main.h"
#include "Wgl.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// High-frequency sub-pixel coordinates
double g_floatingMouseX = 0.0;
double g_floatingMouseY = 0.0;
LARGE_INTEGER frequency;
LARGE_INTEGER startTime;
LARGE_INTEGER endTime;
double elapsedTime = 0;

int quit;

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

void wtext(char* t, float x, float y, int m, int id) {

    font = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, "Comic Sans MS");

    SelectObject(tex[id].frame_device_context, font);
    SetTextColor(tex[id].frame_device_context, RGB(111, 255, 255));
    SetBkMode(tex[id].frame_device_context, m);
    SetBkColor(tex[id].frame_device_context, RGB(0, 111, 111));
    RECT r = { x,  tex[id].f.height - y, x + 222,  tex[id].f.height - y + 33 };
    DrawTextA(tex[id].frame_device_context, t, -1, &r, 0);

    DeleteObject(font);
}

void wpaint(int id, HWND window_handle) {
    tex[id].device_context = BeginPaint(window_handle, &ps);
    char str[64];
    clear(tex[id].f);

    paint(tex[id].f, id);
    sprintf(str, "Hi! f%iK%i a%i", s.frame / 1000, (int)(1./elapsedTime*1000.), s.actions);
    if (id == 1) wtext(str, 1, tex[id].f.height - 11, OPAQUE, id);

    BitBlt(tex[id].device_context,
        ps.rcPaint.left,
        ps.rcPaint.top,
        ps.rcPaint.right - ps.rcPaint.left,
        ps.rcPaint.bottom - ps.rcPaint.top,
        tex[id].frame_device_context,
        ps.rcPaint.left, ps.rcPaint.top,
        SRCCOPY);

    EndPaint(window_handle, &ps);
}

void vproc() {
    process(elapsedTime);
}

LRESULT CALLBACK wpm(HWND window_handle,
    UINT message, WPARAM wParam, LPARAM lParam) {

    int id = get_window(window_handle);

    if (id == -1) {

        return DefWindowProc(window_handle, message, wParam, lParam);

    }

    int x = LOWORD(lParam);
    int y = tex[id].f.height - HIWORD(lParam);
    if (message == WM_KEYDOWN && wParam == 'W') { move2(0, 1); }
    if (message == WM_KEYDOWN && wParam == 'J') { load(); }
    if (message == WM_KEYDOWN && wParam == 'K') { save(); }
    if (message == WM_KEYDOWN && wParam == 'E') { s.scene[player()].t.z+=1; }
    if (message == WM_KEYDOWN && wParam == 'Q') { s.scene[player()].t.z-=1; }
    if (message == WM_KEYDOWN && wParam == 'H') { host(); }
    if (message == WM_KEYDOWN && wParam == 'N') { join(); }
    if (message == WM_KEYDOWN && wParam == 'C') { udp_send("Hello!"); }
    if (message == WM_KEYDOWN && wParam == 'R') {
        reset();
    }
    if (message == WM_KEYDOWN && wParam == 'Q') {
        spawn(
            (struct node) {( struct v2) { s.mx, s.my }, .is_block = 1, .sx=33, .c=white }
        );
    }
    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) { quit = 1; }
    if (message == WM_LBUTTONDOWN) {
        
    }
    if (message == WM_RBUTTONDOWN) {
        s.pick = pick(tex[id].f, x, y, id);
    }

    switch (message) {
    case WM_QUIT: {} break;
    case WM_DESTROY: { 
        quit = 1; 
    } break;
    case WM_MOUSEMOVE: {
        if (id == 1) {
            s.mx = x;
            s.my = y;
            move(x, y);
           // printf("m%i %i ~%i~\n", x, y, id);
          //  wave_step = 6.283 / (SAMPLING_RATE / (float)x / 11.);

        }
    } break;

    case WM_PAINT: {
        if (id == 3) return DefWindowProc(window_handle, message, wParam, lParam);
        if (id == 4) return DefWindowProc(window_handle, message, wParam, lParam);
       wpaint(id, window_handle);

    } break;
    case WM_MOVING: {
        process(15.);
        for (int i = 0; i < windows; i++) {

            InvalidateRect(tex[i].window_handle, NULL, FALSE);
            UpdateWindow(tex[i].window_handle);
        }

    } break;

    case WM_SIZING: {
        process(5.1);
        for (int i = 0; i < windows; i++) {
            InvalidateRect(tex[i].window_handle, NULL, FALSE);
            UpdateWindow(tex[i].window_handle);
        }

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
        if (id == 3) resize(tex[id].f.width , tex[id].f.height);
        if (id==4)
        resize2(tex[id].f.width, tex[id].f.height);
        SelectObject(tex[id].frame_device_context, tex[id].frame_bitmap);
        InvalidateRect(window_handle, NULL, TRUE);

    } break;
    case WM_INPUT: {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        if (dwSize > 0) {
            LPBYTE lpb = (LPBYTE)malloc(dwSize);
            if (lpb == NULL) return 0;
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize) {
                RAWINPUT* raw = (RAWINPUT*)lpb;
                if (raw->header.dwType == RIM_TYPEMOUSE) {
                    if ((raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0) {
                        long deltaX = raw->data.mouse.lLastX;
                        long deltaY = raw->data.mouse.lLastY;
/*                        s.mx += deltaX;
                        s.my += deltaY;*/
                    }
                }
            }
            free(lpb);
        }
        return 0;
    }
    
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
void wwin(HINSTANCE hInstance) {

    window_class.lpfnWndProc = wpm;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = "CLASS";
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.style = CS_OWNDC;
    RegisterClassA(&window_class);

    int w, h, x, y;
    for (int i = 0; i < windows; i++) {
        if (i == 5) { w = 256; h = 123; x = 678; y = 488; }
        if (i == 4) { w = 256; h = 128; x = 678; y = 333; }
        if (i == 3) { w = 256; h = 123; x = 678; y = 145; }
        if (i == 2) {
            w = 32*16;
            h = 32;
            x = 345;
            y = 67;
        }
        if (i == 1) {
             w = 512 - i * 2;
             h = 256 - i;
             x = 123 + i * 4;
             y = 178 + i;
        }
        if (i == 0) {
             w = 256;
             h = 64;
             x = 33;
             y = 67 ;
        }


        tex[i].frame_bitmap_info.bmiHeader.biSize =
            sizeof(tex[i].frame_bitmap_info.bmiHeader);
        tex[i].frame_bitmap_info.bmiHeader.biPlanes = 1;
        tex[i].frame_bitmap_info.bmiHeader.biBitCount = 32;
        tex[i].frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
        tex[i].frame_device_context = CreateCompatibleDC(0);

        RECT rc = { 0, 0, w, h };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        int ww = rc.right - rc.left;
        int wh = rc.bottom - rc.top;
        tex[i].window_handle = CreateWindowA(
            "CLASS", L"snry template", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_POPUP,
            x, y, ww, wh, NULL, NULL, hInstance, NULL
        );

            tex[i].f.width = w;
            tex[i].f.height = h;
            tex[i].frame_bitmap_info.bmiHeader.biWidth = tex[i].f.width;
            tex[i].frame_bitmap_info.bmiHeader.biHeight = tex[i].f.height;
            tex[i].frame_bitmap = CreateDIBSection(NULL,
                &tex[i].frame_bitmap_info, DIB_RGB_COLORS,
                (void**)&tex[i].f.pixels, 0, 0);
            SelectObject(tex[i].frame_device_context, tex[i].frame_bitmap);
            COLORREF titleBackgroundColor = RGB(45, 45, 45, 35);
            DwmSetWindowAttribute(tex[i].window_handle, DWMWA_CAPTION_COLOR, &titleBackgroundColor, sizeof(titleBackgroundColor));

            wpaint(i, tex[i].window_handle);


            InvalidateRect(tex[i].window_handle, NULL, FALSE);
    }

    RAWINPUTDEVICE Rid = { 0 };
    Rid.usUsagePage = 0x01;          // HID usage page for generic desktop
    Rid.usUsage = 0x02;          // HID usage for mouse
    Rid.dwFlags = RIDEV_INPUTSINK; // receive messages even when not foreground
    Rid.hwndTarget = tex[0].window_handle;
    RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
    QueryPerformanceFrequency(&frequency);

    while (!quit) {
        process(elapsedTime);
        udp_poll();
        QueryPerformanceCounter(&startTime);


        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }
        if (F == 1) {
            init();
        }
        if (F == 11) {
            wgl2(tex[4].window_handle);
        }
        if (F == 211) {
            waudio();
            wmidi();
        }
        if (F == 111) {
            wgl(tex[3].window_handle);
            gloop(tex[3].device_context);
        }

        if (F == 33) {
            console();
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            consoleWindow = GetConsoleWindow();
            SetWindowPos(consoleWindow, 0, 33, 432, 512, 256, 0);
            SetForegroundWindow(tex[1].window_handle);

        }

        for (int i = 0; i < windows; i++) {
            if (i == 3 && F > 111) {
                gloop(tex[i].device_context);
            }
            if (i == 4 && F>11) {
                gloop2();
            }
            InvalidateRect(tex[i].window_handle, NULL, FALSE);
            UpdateWindow(tex[i].window_handle);
        }

        QueryPerformanceCounter(&endTime);
        elapsedTime = (double)(endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        F++;
        
    }
    return 0;

}