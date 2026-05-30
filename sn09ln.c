#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <math.h>

#define count 12345
#define orange (struct color){255,111,0,255}
#define cyan (struct color){0,255,255,255}
#define green (struct color){0,255,0,255}
#define yellow (struct color){255,255,0,255}
#define red (struct color){255,0,0,255}
#define blue (struct color){0,0,255,255}
#define magenta (struct color){255,0,255,255}
HICON hLarge, hSmall;

struct v2 { float x, y; };
struct line { int a, b; };
struct tri { int a, b, c; };
struct data { int cur, len; };
struct path { int cur, len; };
struct mesh { struct data p, l, t; };
struct color { char r, g, b, a; };
enum type { leaf, tile, button, mesh, path };
struct frame { int width; int height; unsigned char* pixels; } frame;
struct node { char name[8], type, is_spawned, is_attached, is_hover;
float x, y, sx, sy, r, at; struct color c; int path, desc, mesh;
};
struct state {
    struct node scene[count];
    int nodes, frames, spawned;
    char quit, log;
    unsigned char text[12345];
    struct v2 points[12345];
    struct line lines[1234];
    struct tri tris[1234];
    struct mesh meshes[123];
    struct data path[123];
    int x, y, text_cur, point_cur, line_cur, tri_cur, mesh_cur;
};
struct state s, def;

float lerp(float a, float b, float f) {
    float r = a * (1.0 - f) + (b * f);
    return r;
}
float len(struct v2 v) { return sqrt(v.x * v.x + v.y * v.y); };
float dist(struct v2 a, struct v2 b) { return len((struct v2) { a.x - b.x, a.y - b.y }); };

void delete(int id) {
    s.scene[id].is_spawned = 0; s.spawned--;
   // printf("- %i\n", id);
}
int slot() {
    if (!s.scene[s.nodes % count].is_spawned) return s.nodes % count;
    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) return i;
    }
    return -1;
}
int spawn(struct node n) {
    int sl = slot();
    if (sl == -1) return -1;
    n.is_spawned = 1;
    s.scene[sl] = n;
    s.spawned++;
    s.nodes++;
   // printf("++ %i\n", sl);
    return sl;
}

void point(struct frame f, int x, int y, struct color c) {
    if (x >= 0 && x < f.width && y >= 0 && y < f.height) {
        f.pixels[0 + (x + y * f.width) * 4] = c.b;
        f.pixels[1 + (x + y * f.width) * 4] = c.g;
        f.pixels[2 + (x + y * f.width) * 4] = c.r;
    }
}

struct v2 rot(struct v2 a, float r) {
    return (struct v2) {
        a.x* cos(r) - a.y * sin(r),
            a.x* sin(r) + a.y * cos(r)
    };
}
struct v2 rot_or(struct v2 a, struct v2 o, float r) {
    a = rot((struct v2) { a.x - o.x, a.y - o.y }, r);
    return   (struct v2) { a.x + o.x, a.y + o.y };
}

void draw_line(struct frame f, struct v2 a, struct v2 b, struct v2 o,
    struct color c, float r) {
    a = rot_or(a, o, r);
    b = rot_or(b, o, r);
    int d = dist(a, b);
    for (int i = 0; i <= d; i++) {
        point(f,
            lerp(a.x, b.x, i / (float)d),
            lerp(a.y, b.y, i / (float)d), c);
    }
}

void ring(struct frame f, struct v2 a,
    struct color c, float r) {
    for (int i = 0; i <= 111; i++) {
        a = rot_or((struct v2) { a.x, a.y + r }, a, i);
        point(f, a.x, a.y, c);
    }
}

void tri(struct frame f, struct v2 a, struct v2 b, struct v2 c,
    struct v2 o, struct color cl, float r) {
    a = rot_or(a, o, r);
    b = rot_or(b, o, r);
    c = rot_or(c, o, r);
    int d = dist(b, c);
    for (int i = 0; i <= d; i++) {
        draw_line(f,
            a,
            (struct v2) {
            lerp(b.x, c.x, i / (float)d),
                lerp(b.y, c.y, i / (float)d)
        },
            o, cl, 0
        );
    };
}

int card(int x, int y, int cx, int cy, int w, int h ) {
    if ((x > cx && x < cx + w) && (y > cy && y < cy + h)) return 1;
    else   return 0;
}

int append(char* str) {
    int a, c = 0;
    a = s.text_cur;
    while (1) {
        s.text[c+s.text_cur] = str[c];
        if (str[c] == 0) { s.text_cur += c+1;  return a; }
        c++;
    }

}

int add_point(float x, float y) {
    s.points[s.point_cur].x = x;
    s.points[s.point_cur].y = y;
    s.point_cur++;
    return s.point_cur;
}
int add_line(int a, int b) {
    s.lines[s.line_cur].a = a;
    s.lines[s.line_cur].b = b;
    s.line_cur++;
    return s.line_cur;
}
int add_tri(int a, int b, int c) {
    s.tris[s.tri_cur].a = a;
    s.tris[s.tri_cur].b = b;
    s.tris[s.tri_cur].c = c;
    s.tri_cur++;
    return s.tri_cur;
}
int add_path(float x, float y) {
    s.points[s.point_cur].x = x;
    s.points[s.point_cur].y = y;
    s.point_cur++;
    return s.point_cur;
}

int add_mesh(struct data p, struct data l, struct data t) {
    s.meshes[s.mesh_cur].p = p;
    s.meshes[s.mesh_cur].l = l;
    s.meshes[s.mesh_cur].t = t;
    s.mesh_cur++;
}
int draw_mesh(struct frame f, int id, struct v2 v, float r) {

    ring(f, v, yellow, 1);

    for (int i = 0; i < s.meshes[id].t.len; i++) {
        struct tri t = s.tris[i];
        tri(f,
            (struct v2)  {s.points[t.a].x+v.x,  s.points[t.a].y+v.y  },
            (struct v2)  {s.points[t.b].x+v.x,  s.points[t.b].y+v.y  },
            (struct v2)  {s.points[t.c].x+v.x,  s.points[t.c].y+v.y  },
            v,
            green,
            r
        );
    }
    for (int i = 0; i < s.meshes[id].l.len; i++) {
        struct line l = s.lines[i];
        draw_line(f,
            (struct v2)  {s.points[l.a].x+v.x,  s.points[l.a].y+v.y  },
            (struct v2)  {s.points[l.b].x+v.x,  s.points[l.b].y+v.y  },
            v,
            blue,
            r
        );
    }
    for (int i = 0; i < s.meshes[id].p.len; i++) {
        struct v2 p = s.points[s.meshes[id].p.cur + i];
        p = rot(p, r);
        point(
            f,
            p.x + v.x,
            p.y + v.y,
            orange
        );
    }
}

int draw_path(struct frame f, int id) {
    for (int i = 0; i < s.path[id].len-2; i++) {
        draw_line(f,
            s.points[s.path[id].cur+i],
            s.points[s.path[id].cur+1+i],
            (struct v2)  {0, 0 },
            magenta,
            0
        );
    }
}

void hover(int id) {}
int click(int x, int y) { 
    printf("click!\n");

    for (int i = 0; i < count; i++) {
            if (s.scene[i].type == button) {
                if (card(x, y, s.scene[i].x, s.scene[i].y, s.scene[i].sx, s.scene[i].sy)) {
                    printf("BUTTON %i %s\n", i, &s.text[s.scene[i].path]);
                    STARTUPINFO si;
                    PROCESS_INFORMATION pi;
                    ZeroMemory(&si, sizeof(si));
                    si.cb = sizeof(si);
                    ZeroMemory(&pi, sizeof(pi));
                    CreateProcessA(
                        &s.text[s.scene[i].path], // Executable path
                        NULL,                                  // Command line arguments
                        NULL,                                  // Process handle not inheritable
                        NULL,                                  // Thread handle not inheritable
                        FALSE,                                 // Set handle inheritance to FALSE
                        0,                                     // No creation flags
                        NULL,                                  // Use parent's environment block
                        NULL,                                  // Use parent's starting directory 
                        &si,                                   // Pointer to STARTUPINFO structure
                        &pi);                          // Pointer to PROCESS_INFORMATION structure
                    return i;
                }
            }
        }
    }


void load() {
    FILE* fptr = fopen("save_nr", "rb");
    fread(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}
void init() {
    printf("Hi!\n");

    add_point(0, 0);
    add_point(0, 8);
    add_point(8, 8);
    add_point(14, 15);
    add_point(14, 25);
    add_line(2, 4);
    add_line(0, 3);
    add_tri(0, 1, 2);

    add_mesh((struct data) { 0, 5 }, (struct data) { 0, 2 }, (struct data) { 0, 1 });
    s.path[0].cur = 6;

    for (int i = 0; i < 5; i++) {
         spawn((struct node) { "mesh", mesh, .x = i * 16, .y = 123, 24, 24, .r = i/12. });
    }

    int text, slot = 0;
    slot = spawn((struct node) { "start", button, .x = 111, .y = 55, 67, 33 });
    text = append("sn09/sn09c.exe");
    s.scene[slot].path = text;
    slot = spawn((struct node) { "button", button, .x=44, .y=33, 68, 33 });
    text = append("button2");
    s.scene[slot].path = text;
    slot = spawn((struct node) { "nt", button, .x = 123, .y = 33, 24, 24 });
    text = append("C:\\Windows\\System32\\notepad.exe");
    s.scene[slot].path = text;
    slot = spawn((struct node) { "pt", button, .x = 155, .y = 33, 24, 24 });
    text = append("C:\\Windows\\System32\\mspaint.exe");
    s.scene[slot].path = text;
    slot = spawn((struct node) { "mc", button, .x = 234, .y = 33, 24, 24 });
    text = append("mc-windows/mcat.exe");
    s.scene[slot].path = text;

    ExtractIconExA("mc-windows/mcat.exe", 0, &hLarge, &hSmall, 1);

};

void save() {
    FILE* fptr = fopen("save_nr", "wb");
    if (fptr) fwrite(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}
void reset() { s = def;  }

void process(float dt) {

    if (s.frames < count) {
        //   spawn((struct node) {.x = rand() % frame.width, .y = rand() % frame.height});
    }

    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (s.scene[i].type == leaf && rand() % 12 == 8) {
            s.scene[i].y += 2 - rand() % 5;
            s.scene[i].x += 2 - rand() % 5;
        }
        if (s.scene[i].type == leaf && rand() % 115 == 0) delete(i);
        if (s.scene[i].type == mesh && rand() % 1234 == 0) delete(i);
        if (s.scene[i].type == mesh && rand() % 115 == 0) s.scene[i].r+=.1;
    }
    s.frames++;
}

void clear(struct frame f) {
    for (int i = 0; i < f.width * f.height; i++) {
        for (int i2 = 0; i2 < 4; i2++) {
            f.pixels[i2 + i * 4] = 0;
        }
    }
}

static HDC device_context, frame_device_context;
HFONT font;
void text(char* t, float x, float y, int m) {
    SelectObject(frame_device_context, font);
    SetTextColor(frame_device_context, RGB(255, (s.frames / 22) % 255, 255));
    SetBkMode(frame_device_context, m);
    SetBkColor(frame_device_context, RGB(0, 111, 111));
    RECT r = { x, frame.height-y, x+234, frame.height-y+64 };
    DrawTextA(frame_device_context, t, -1, &r, 0);
}
void paint(struct frame f) {
    for (int i = 0; i < f.width * f.height; i++) {
        int y = i / f.width;
        int x = i % f.width;
        f.pixels[1 + i * 4] = x % 33;
        f.pixels[i * 4] = (y / 5) % 77;
        f.pixels[0 + i * 4] = sin(dist(
            (struct v2) { x, y },
            (struct v2) { 111, 111 })/11.)*s.frames/22.;

    }

    for (int i = 0; i < count; i++) {
        if (s.scene[i].is_spawned) {
            point(f, s.scene[i].x, s.scene[i].y,
                cyan
            );
            if (s.scene[i].type == mesh) {
                draw_mesh(f, 0, (struct v2) { s.scene[i].x, s.scene[i].y }, s.scene[i].r);

            }
        }

    }

    draw_path(f, 0);

    for (int i = 0; i < count; i++) {
        if (s.scene[i].is_spawned) {

            if (s.scene[i].type == button) {
                for (int i2 = 0; i2 < f.width * f.height; i2++) {
                    int y = i2 / f.width;
                    int x = i2 % f.width;
                    if (card(x, y, s.scene[i].x, s.scene[i].y, s.scene[i].sx, s.scene[i].sy)) {
                        point(f, x, y, (struct color) {
                            s.scene[i].is_hover * 255, 44, 44, 255
                        });
                    }
                }
                text(s.scene[i].name, s.scene[i].x, s.scene[i].y + s.scene[i].sy, 1);
                DrawIcon(frame_device_context, 0, 0, hSmall);
            }

        }

    }

    char str[64]; sprintf(str, "Hi! %i/:%i\n f:%ik", count, s.spawned, s.frames / 1000);
    text(str, 33, frame.height, OPAQUE);
}

static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap;
static PAINTSTRUCT ps;
static WNDCLASS window_class;
static HWND window_handle;
MSG msg;
LRESULT CALLBACK wpm(HWND window_handle,
    UINT message, WPARAM wParam, LPARAM lParam) {

    int x = LOWORD(lParam);
    int y = frame.height - HIWORD(lParam);

    if (message == WM_KEYDOWN && wParam == 'R') reset();
    if (message == WM_KEYDOWN && wParam == 'L') load();
    if (message == WM_KEYDOWN && wParam == 'J') save();
    if (message == WM_KEYDOWN && wParam == 'Q') {
        printf("L click! %i %i\n", x, y);
        for (int i = 0; i < count; i++) {
            if (!s.scene[i].is_spawned || s.scene[i].type != tile) continue;
            if (dist((struct v2) { s.x, s.y }, (struct v2) { s.scene[i].x, s.scene[i].y }) < 30.)
                delete(i);
        }
        for (int i = 0; i < frame.width * frame.height; i++) {
            int my = i / frame.width;
            int mx = i % frame.width;
            if (dist((struct v2) { s.x, s.y }, (struct v2) { mx, my }) < 30.)
                spawn((struct node) { .type = tile, "asd", .x = mx, .y = my, .c = orange });
        }
    };
    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) { s.quit = 1; }
    if (message == WM_RBUTTONDOWN) {
        for (int i = 0; i < count; i++) {
            if (!s.scene[i].is_spawned || s.scene[i].type != tile) continue;
            if (dist((struct v2) { x, y }, (struct v2) { s.scene[i].x, s.scene[i].y }) < 30.)
                delete(i);
        }
    }
    if (message == WM_LBUTTONDOWN) {
        click(x, y);
    }

    switch (message) {
    case WM_QUIT: {} break;
    case WM_DESTROY: {
        s.quit = 1;
    } break;
    case WM_MOUSEMOVE: {
        s.x = x;
        s.y = y;
        spawn((struct node) { .type = leaf, .x = x, .y = y, .c = cyan });
        spawn((struct node) { .type = mesh, .x = x, .y = y, .c = cyan, .r=rand() });

        add_point(x, y);
        s.path[0].len++;

        for (int i = 0; i < count; i++) {
            if (s.scene[i].is_spawned) {
                if (s.scene[i].type == button) {
                    s.scene[i].is_hover = 0;
                    if (card(x, y, s.scene[i].x, s.scene[i].y,
                        s.scene[i].sx, s.scene[i].sy))
                    { s.scene[i].is_hover = 1; }
                }
            }
        }
    } break;

    case WM_PAINT: {

        device_context = BeginPaint(window_handle, &ps);

        clear(frame);
        paint(frame);

        BitBlt(device_context,
            ps.rcPaint.left,
            ps.rcPaint.top,
            ps.rcPaint.right - ps.rcPaint.left,
            ps.rcPaint.bottom - ps.rcPaint.top,
            frame_device_context,
            ps.rcPaint.left, ps.rcPaint.top,
            SRCCOPY);

        EndPaint(window_handle, &ps);
        //  SetWindowTextA(window_handle, "snry rpg sn0833");

    } break;

    case WM_SIZE: {

        frame_bitmap_info.bmiHeader.biWidth = LOWORD(lParam);
        frame_bitmap_info.bmiHeader.biHeight = HIWORD(lParam);

        if (frame_bitmap) DeleteObject(frame_bitmap);
        frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&frame.pixels, 0, 0);
        SelectObject(frame_device_context, frame_bitmap);

        frame.width = LOWORD(lParam);
        frame.height = HIWORD(lParam);

    } break;

    default: {
        return DefWindowProc(window_handle, message, wParam, lParam);
    }
    }
    return 0;
}

HANDLE hConsole;
HWND consoleWindow;
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
    window_class.lpszClassName = "My Window Class";
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&window_class);

    frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
    frame_bitmap_info.bmiHeader.biPlanes = 1;
    frame_bitmap_info.bmiHeader.biBitCount = 32;
    frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
    frame_device_context = CreateCompatibleDC(0);
    window_handle = CreateWindow("My Window Class", L"snry template", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        123, 123, 512, 256, NULL, NULL, hInstance, NULL);
    if (window_handle == NULL) { return -1; }

    font = CreateFont(24, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Comic Sans MS");

    while (!s.quit) {
        process(.01);

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }

        if (s.frames == 2) {
            console();
            consoleWindow = GetConsoleWindow();
            SetWindowPos(consoleWindow, 0, 33, 432, 512, 256, 0);
            SetForegroundWindow(window_handle);

            init();
        }

        InvalidateRect(window_handle, NULL, FALSE);
        UpdateWindow(window_handle);

    }
    return 0;

}