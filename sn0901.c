#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <math.h>

#define count 1234
#define orange (struct color){255,111,0,255}
#define green (struct color){0,255,0,255}
#define gray (struct color){123,123,123,255}
#define pl s.scene[0]

struct v2 { float x, y; };
struct color { char r, g, b, a; };
struct task { char name[11], req[11], c, exp, coin, is_spawned; };
struct hunt { char name[11], c, is_spawned; };
struct item { char name[11], item_type, buff, tier, is_spawned; short price; };
enum type { tile, character, enter, item, npc };
enum tile { grass, stone };
enum item_type { none, weapon, gear };
enum map { world, dungeon };
struct frame { int width; int height; unsigned char* pixels; } frame;
struct node {
    char name[11], type, map, is_spawned, is_attached;
    unsigned char x, y, lvl; short at, link, hp; struct item item;
};
struct state {
    struct node scene[count];
    int nodes, frames, spawned;
    char quit, log, progress;
    int x, y, coin, exp, armor, attack;
    struct task tasks[8];
    struct hunt hunt[8];
    struct item inventory[10];
    struct item store[10];
};
struct state s, def;

void reset() { s = def; srand(0); }

int coll(int x, int y) {
    if (x < 0 || x> 7 || y < 0 || y>7) return 0;
    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (s.scene[i].x == x && s.scene[i].y == y &&
            s.scene[i].type == tile && s.scene[i].map== s.scene[0].map)
            return 1;
    }
    return 0;
}
char get_hp(int i) {
    return s.scene[i].lvl * 3 + 5;
}
char get_armor(int i) { 
    char a = 0;
    if (i == 0 && s.inventory[1].is_spawned && s.inventory[1].item_type == gear)
    { a=a+ s.inventory[1].tier+ s.inventory[1].buff; }
    return a;
}
char get_attack(int i) {
    char a = 0;
    if (i==0 && s.inventory[0].is_spawned && s.inventory[0].item_type == weapon)
    { a = a + s.inventory[0].tier + s.inventory[0].buff; }
    return pl.lvl + a;
}
void equip(char a, char b) {
    struct item sa = s.inventory[a];
    struct item sb = s.inventory[b];
    s.inventory[b] = sa;
    s.inventory[a] = sb;
}
void shuffle() {
    for (int i = 0; i < 10; i++) {
        if (s.inventory[i].is_spawned)
        equip(i, rand() % 10);
    }

}
char inv_slot() {
    for (int i = 0; i < 10; i++) {
        if (!s.inventory[i].is_spawned)
            return i;
    }
    return -1;
}

void pick_up(int i) {
    if (s.scene[i].is_spawned && s.scene[i].type == item)
    {
        int slot = inv_slot();
        s.inventory[slot] = s.scene[i].item;
        s.inventory[slot].is_spawned = 1;
        s.scene[i].is_spawned = 0;
        printf("took %s\n", s.scene[i].item.name);

    }
}
void drop(int id) {
    if (!s.inventory[id].is_spawned) return;
    spawn((struct node) {
        .type = item, .lvl = 1, .type = item, .x = pl.x, .y = pl.y,  .item = s.inventory[id]  });
    s.inventory[id] = (struct item){ 0 };
}
void buy(int id) {

    int slot = inv_slot(); 
    if (!s.store[id].is_spawned) return;

        if (slot > -1 && s.coin> s.store[id].price) {
            s.inventory[slot] = s.store[id];
        s.coin -= s.store[id].price;
        printf("\nbought %s\n", s.store[id].name);
    }

}
void sell(int id) {
    if (!s.inventory[id].is_spawned) return;
        s.coin += s.inventory[id].price/2;
        printf("\sold %s\n", s.inventory[id].name);
        s.inventory[id] = (struct item){ 0 };
}
int attack(int a, int b) {
    printf("\n%s@%s[%i]{%i", s.scene[a].name, s.scene[b].name, s.scene[b].lvl, s.scene[b].hp);
    if (rand() % 4 != 0) {
        int d = get_attack(a);
        if (rand() % 4 == 0) d *= 2;
        printf(" hit:%d", d);
        s.scene[b].hp -= d;
    }
    else printf(" miss");

}
int fight(int a, int b) {
    while (1) {
        attack(a, b);
        if (s.scene[b].hp < 1) { printf("|\n"); return a; }
        attack(b, a);
        if (s.scene[a].hp < 1) { printf("(|)\n"); return b; }
    }
}

int lap(int i) {
    printf("+");
    if (s.scene[i].type == enter) {
        printf("JUMP to %i", s.scene[i].link);
        s.scene[0].map = s.scene[s.scene[i].link].map;
        s.scene[0].x = s.scene[s.scene[i].link].x - 1;
        s.scene[0].y = s.scene[s.scene[i].link].y;
    };
    if (s.scene[i].type == item) {
        printf("\nfound %s item\n", s.scene[i].item.name);
        pick_up(i);
    };
    if (s.scene[i].type == npc) {
        printf("\nfound %s[%i] NPC\n", s.scene[i].name, s.scene[i].lvl);
        int res = fight(0, i);
        if (res != 0) { reset(); }
        else { 
            printf("%s[%i] won! +%iexp\n", s.scene[res].name, s.scene[res].lvl, 43);
            s.scene[i].is_spawned = 0; s.exp += 43; s.progress++;
        }
    };
}

void move(int x, int y) {
    if (coll(s.scene[0].x + x, s.scene[0].y + y)) {
        s.scene[0].x += x;
        s.scene[0].y += y;
    }
    else return;

    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (s.scene[i].x == s.scene[0].x && s.scene[i].y == s.scene[0].y &&
            s.scene[i].map == s.scene[0].map && i != 0)
        {
            lap(i);
        }
    }

}

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
    if (s.scene[sl].type == character || s.scene[sl].type == npc) 
    { s.scene[sl].hp = get_hp(sl); }
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

void draw_line(struct frame f, struct v2 a, struct v2 b,
    struct color c) {
    int d = dist(a, b);
    for (int i = 0; i <= d; i++) {
        point(f,
            lerp(a.x, b.x, i / (float)d),
            lerp(a.y, b.y, i / (float)d), c);
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
void ring(struct frame f, struct v2 a,
    struct color c, float r) {
    for (int i = 0; i <= 123; i++) {
        a = rot_or((struct v2) { a.x, a.y + r}, a, i);
        point(f, a.x, a.y, c);
    }
}

void load() {
    FILE* fptr = fopen("save_nr", "rb");
    fread(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}
void init() {
    srand(0);
    printf("Hi!\n");
    spawn((struct node) { "player", .type = character, .lvl = 1 });
    s.coin += 678;
    spawn((struct node) { "dummy", .type = npc, .lvl = 1, .x = 0, .y = 2, });
    spawn((struct node) { "drake", .type = npc, .lvl = 9, .x = 7, .y = 7, });
    spawn((struct node) { .type = item, .lvl=1, .type=item, .x=1, .y=3,
        .item = (struct item) {.name="stick+1", .item_type=weapon, .tier=1, .buff=1 }
    });
    spawn((struct node) {
        .type = item, .lvl = 1, .type = item, .x = 1, .y = 7,
            .item = (struct item){ .name = "set 2", .item_type = gear, .tier = 2 }
    });

    s.inventory[1] = (struct item){ .is_spawned = 1, .name = "base set",
        .item_type = gear, .tier = 1 };

    s.store[0] = (struct item){ .is_spawned = 1, .name = "potion", .price = 100 };
    s.store[1] = (struct item){ .is_spawned = 1, .name = "noodle", .price = 50 };
    s.store[2] = (struct item){ .is_spawned = 1, .name = "veg", .price = 50 };
    s.store[3] = (struct item){ .is_spawned = 1, .name = "fruit", .price = 50 };

    for (int i = 0; i < 64; i++) {
        if (rand() < 26666) {
            spawn((struct node) { .type = tile, .x = i % 8, .y = i / 8 });
        }
    }
    for (int m = 1; m < 8; m++) {

        for (int i = 0; i < 64; i++) {
            if (rand() < 26666) {
                spawn((struct node) { .type = tile, .map = m, .x = i % 8, .y = i / 8 });
            }
        }
    }
    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned || s.scene[i].type!=tile) continue;
        if (rand()%13==0 ) {
            spawn((struct node) { "wasp", .type = npc, .lvl = 1,
                .x=s.scene[i].x, .y= s.scene[i].y, .map=s.scene[i].map });
        }
    }

    int a = 0;
    int b = 0;
   a = spawn((struct node) { .type = enter, .x = 4, .y = 4 });
   b = spawn((struct node) { .type = enter, .x = 4, .y = 4, .map=1 });
   s.scene[a].link = b;
   s.scene[b].link = a;
   a = spawn((struct node) { .type = enter, .x = 2, .y = 2 });
   b = spawn((struct node) { .type = enter, .x = 5, .y = 5, .map = 2 });
   s.scene[a].link = b;
   s.scene[b].link = a;
   a = spawn((struct node) { .type = enter, .x = 3, .y = 3 });
   b = spawn((struct node) { .type = enter, .x = 3, .y = 3, .map = 3 });
   s.scene[a].link = b;
   s.scene[b].link = a;

};

void save() {
    FILE* fptr = fopen("save_nr", "wb");
    if (fptr) fwrite(&s, sizeof(s), 1, fptr);
    fclose(fptr);
}

void process(float dt) {

    for (int i = 0; i < count; i++) {
        if (!s.scene[i].is_spawned) continue;
    }
    srand(s.frames%1234);
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
    SetTextColor(frame_device_context, RGB(0, 255, 0));
    SetBkMode(frame_device_context, m);
    SetBkColor(frame_device_context, RGB(11, 11, 11));
    RECT r = { x, frame.height - y, x + 345, frame.height - y + 64 };
    DrawTextA(frame_device_context, t, -1, &r, 0);
}
void paint(struct frame f) {
    for (int i = 0; i < f.width * f.height; i++) {
        int y = i / f.width;
        int x = i % f.width;
        f.pixels[1 + i * 4] = x % 33;
        f.pixels[i * 4] = (y / 5) % 77;
    }

    for (int i = 0; i < count; i++) {
        if (s.scene[i].is_spawned) {
            if (s.scene[i].type==tile && s.scene[i].map==world) {
            ring(f, (struct v2) { 22+s.scene[i].x*16, 22+s.scene[i].y*16}, orange,
                7);
            }
            if (s.scene[i].type == tile && s.scene[i].map != world
                && s.scene[0].map== s.scene[i].map) {
                ring(f, (struct v2) { 222 + s.scene[i].x * 16, 22 + s.scene[i].y * 16 }, gray,
                    7);
            }
            if (s.scene[i].type==character) {
            ring(f, (struct v2) { (200*(s.scene[0].map!=0))+22+s.scene[i].x*16, 22+s.scene[i].y*16}, green,
                5);
            }
        if (s.scene[i].type==enter && s.scene[0].map == s.scene[i].map) {
            ring(f, (struct v2) { 200*(s.scene[i].map>0)+22+s.scene[i].x*16,
                22+s.scene[i].y*16

            }, green,
                2);
            }
        if (s.scene[i].type == item && s.scene[0].map == s.scene[i].map) {
            text("~", 200 * (s.scene[i].map > 0) + 12 + s.scene[i].x * 16,
                32 + s.scene[i].y * 16, 0);
        }
        if (s.scene[i].type == npc && s.scene[0].map == s.scene[i].map) {
            text("^", 200 * (s.scene[i].map > 0) + 12 + s.scene[i].x * 16,
                32 + s.scene[i].y * 16, 0);
        }

        }

    }

    for (int i = 0; i < 10; i++) { 
        ring(f, (struct v2) {  frame.width-8, 20+i*16  }, gray, 3);
        if (s.inventory[i].is_spawned) {
            ring(f, (struct v2) { frame.width - 8 - 3, 20 + i * 16 }, green, 2);
            text(s.inventory[i].name, frame.width - 48, (20 + i * 16) + 16, 0);
        }
    }
    for (int i = 0; i < 10; i++) {
        if (s.store[i].is_spawned) {
            ring(f, (struct v2) { frame.width - 86, 20 + i * 16 }, green, 2);
            char str[64]; sprintf(str, "%s[%i]", s.store[i].name, s.store[i].price);
            text(str, frame.width - 148, (20 + i * 16) + 16, 0);
        }
    }


    char str[64]; sprintf(str, "Hi! %i/:%i\n f:%ik", count, s.spawned, s.frames / 1000);
    text(str, 33, frame.height, OPAQUE);
    sprintf(str, "PROG %i/8", s.progress);
    text(str, 33, frame.height - 33, OPAQUE);

    sprintf(str, "PLAYER L%i[%i] H%i/%i R%i D%i M%i/%i/%i C:%i",
        pl.lvl, s.exp, pl.hp, get_hp(0), get_armor(0), get_attack(0), pl.map, pl.x, pl.y, s.coin);
    text(str, 33, frame.height-45, OPAQUE);

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
    if (message == WM_KEYDOWN && wParam == 'W') move(0, 1);
    if (message == WM_KEYDOWN && wParam == 'S') move(0, -1);
    if (message == WM_KEYDOWN && wParam == 'A') move(-1, 0);
    if (message == WM_KEYDOWN && wParam == 'D') move(1, 0);
    if (message == WM_KEYDOWN && wParam == 'Q') { shuffle(); }
    if (message == WM_KEYDOWN && wParam == 'T') { buy(rand() % 4); }
    if (message == WM_KEYDOWN && wParam == 'Y') { sell(rand() % 10); }
    if (message == WM_KEYDOWN && wParam == 'F') {  drop(rand()%10);   }
    
    if (message == WM_KEYDOWN && wParam == VK_ESCAPE) { s.quit = 1; }
    if (message == WM_RBUTTONDOWN) {

    }
    if (message == WM_LBUTTONDOWN) {
    }

    switch (message) {
    case WM_QUIT: {} break;
    case WM_DESTROY: {
        s.quit = 1;
    } break;
    case WM_MOUSEMOVE: {
        s.x = x;
        s.y = y;

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

    font = CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Comic Sans MS");

    while (!s.quit) {
        process(.01);

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }

        if (s.frames == 1) {
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