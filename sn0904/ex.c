#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

// Forward declaration of the window procedure callback
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {


    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"WN";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    // 2. Create the First Window
    HWND hwnd1 = CreateWindowExW(
        0, L"WN", L"First Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, hInstance, NULL
    );

    // 3. Create the Second Window (Reusing the same class)
    HWND hwnd2 = CreateWindowExW(
        0, L"WN", L"Second Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, hInstance, NULL
    );

    // 4. Reveal both windows on the screen
    ShowWindow(hwnd1, nCmdShow);
    ShowWindow(hwnd2, nCmdShow);

    // 5. The Unified Message Loop handles all windows on this thread
    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}

// 6. The Shared Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    static int openWindowCount = 2;
    switch (uMsg) {
    case WM_DESTROY:
        openWindowCount--;
        // Only terminate the main loop when the last remaining window closes
        if (openWindowCount == 0) {
            PostQuitMessage(0);
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw different text depending on which window is painting
        wchar_t title[256];
        GetWindowTextW(hwnd, title, 256);
        TextOutW(hdc, 20, 20, title, lstrlenW(title));

        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
