#include "sn.h"
#include "math.h"
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

int sa = 0;
double smp = 0;
short sound2() {
    sa++;
    double st = 6.283 / ((float)44100 / s.mx/2.);
    smp += st;
    return sin(smp* sin(smp/s.my/4.))* sin(smp/11111.)*11111.;

};

float lerp(float a, float b, float f) {
	float r = a * (1.0 - f) + (b * f);
	return r;
}
float len(struct v2 v) { return sqrt(v.x * v.x + v.y * v.y); };
float dist(struct v2 a, struct v2 b) { return len((struct v2) { a.x - b.x, a.y - b.y }); };

void spawn(struct node n) {
    if (s.scene[(s.scene_cur)% 123].is_block) {
        s.scene_cur++; return;
    }
    n.is_spawned = 1;
    s.scene[s.scene_cur % 123] = n;
    s.scene_cur++;
}

int hit(int a, int b) {
    return v2_circle(s.scene[a].t, s.scene[b].t, s.scene[b].sx);
}

void move(int x, int y) {
    struct color c;
    if (s.pick.a == 0) c = hsv_to_rgb(rand() % 360, 1, 1);
    else c = s.pick;

    spawn((struct node) {
        ( struct v2) { s.mx, s.my },
            .c = c,
            .sx = 2 + rand() % 7,
            .is_la = !(rand()%11),
            .is_lb = !(rand() % 11),
            .la = rand() % 111,
            .lb = rand() % 111,
            .v.x = (111/2-rand()%111)/111.,
            .v.y =  (111/2-rand()%111)/111.,
            .a = (111/2-rand()%111)/11111.,
            .rot = line_angle_rad(
                (struct v2) { s.mx, s.my},
                (struct v2) {  x, y   }
            ) });
    s.mx = x;
    s.my = y;
    s.actions++;
};

void point(struct frame f, int x, int y, struct color c) {
    if (x >= 0 && x < f.width && y >= 0 && y < f.height) {
        f.pixels[0 + (x + y * f.width) * 4] = c.b;
        f.pixels[1 + (x + y * f.width) * 4] = c.g;
        f.pixels[2 + (x + y * f.width) * 4] = c.r;
    }
}
struct color pick(struct frame f, int x, int y, int id) {
    return (struct color) {
        f.pixels[2 + (x + y * f.width) * 4],
            f.pixels[1 + (x + y * f.width) * 4],
            f.pixels[0 + (x + y * f.width) * 4],
            255
    };
}

void paint(struct frame f, int id) {

    if (id == 0) {
        colors(f, id);
    }
    if (id == 2) {
        for (int i = 0; i < f.width * f.height; i++) {
            int x = i % f.width;
            f.pixels[1 % 4 + i * 4] = (255-(x % 32)*8)/3;
        }
        circle(f, (struct v2) { fmod(s.t/2.,1.)*32*16, 16 },
            orange, 11);
        circle(f, (struct v2) { fmod(s.t / 512., 1.) * 32 * 16, 4 },
            red, 2);
        ring2(f, (struct v2) { fmod(s.t / 64., 1.) * 32 * 16, 8 },
            cyan, 4);
    }
    if (id == 1) 
    {

    for (int i = 0; i < f.width * f.height; i++) {
        int y = i / f.width;
        int x = i % f.width;
        f.pixels[1 % 4 + i * 4] = (x%7)*2;
        f.pixels[1 % 4 + i * 4] += (x%11)*3;
        f.pixels[0 + i * 4] = (y%13)*2;
        f.pixels[0 + i * 4] += (y%17)*3;
    }
    for (int i = 0; i < 123; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (!s.scene[i].is_la || !s.scene[i].is_lb) continue;

        tri(f,
            (struct v2) {s.scene[i].t.x, s.scene[i].t.y},
            (struct v2) { s.scene[s.scene[i].la].t.x, s.scene[s.scene[i].la].t.y },
            (struct v2) { s.scene[s.scene[i].lb].t.x, s.scene[s.scene[i].lb].t.y },
            (struct v2) { 0, 0 },
            s.scene[i].c,
            0
        );
    }
    for (int i = 0; i < 123; i++) {
        if (!s.scene[i].is_spawned) continue;
        if (!s.scene[i].is_la) continue;
        line(f, 
            (struct v2) { s.scene[i].t.x, s.scene[i].t.y },
            (struct v2) { 
            s.scene[s.scene[i].la].t.x,
            s.scene[s.scene[i].la].t.y
        },
            (struct v2) { 0, 0 },
            blue,
            0
            );
    }
    for (int i = 0; i < 123; i++) {
        if (!s.scene[i].is_spawned) continue;

        point(f, s.scene[i].t.x, s.scene[i].t.y, s.scene[i].c);
        circle(f, (struct v2) { s.scene[i].t.x, s.scene[i].t.y },
            s.scene[i].c, s.scene[i].sx);
    }
        for (int i = 0; i < 123; i++) {
            if (!s.scene[i].is_spawned || s.scene[i].is_block) continue;

        line(f, 
            (struct v2) { s.scene[i].t.x, s.scene[i].t.y },
            (struct v2) { s.scene[i].t.x+ s.scene[i].sx*2, s.scene[i].t.y },
            (struct v2) { s.scene[i].t.x, s.scene[i].t.y },
            white,
            s.scene[i].rot
            );
    }
        }
}
void colors(struct frame f, int id) {
    for (int i = 0; i < f.width * f.height; i++) {
        int x = i % f.width;
        int y = i / f.width;
        float hue = (float)x / f.width * 360.0f;  // hue from 0 to 360 across width
        struct color c = hsv_to_rgb(hue, 1.0f, 1.0f);
        int idx = i * 4;
        f.pixels[idx + 0] = c.b;
        f.pixels[idx + 1] = c.g;
        f.pixels[idx + 2] = c.r;
        if (y < 32) {
            f.pixels[idx + 0] = s.pick.b;
            f.pixels[idx + 1] = s.pick.g;
            f.pixels[idx + 2] = s.pick.r;
        }
        // alpha unchanged (keep as is)
    }
}

void clear(struct frame f) {
    for (int i = 0; i < f.width * f.height; i++) {
        for (int j = 0; j < 4; j++) {
            f.pixels[j + (i * 4)] = 0;
        }
    }
}

struct v2 rot(struct v2 a, float r) {
    return (struct v2) {
        a.x* cos(r) - a.y * sin(r),
            a.x* sin(r) + a.y * cos(r)
    };
}

float line_angle_rad(struct v2 a, struct v2 b) {
    return atan2f(b.y - a.y, b.x - a.x);
}

struct v2 rot_or(struct v2 a, struct v2 o, float r) {
    a = rot((struct v2) { a.x - o.x, a.y - o.y }, r);
    return   (struct v2) { a.x + o.x, a.y + o.y };
}

void line(struct frame f, struct v2 a, struct v2 b, struct v2 o,
    struct color c, float an) {
    a = rot_or(a, o, an);
    b = rot_or(b, o, an);
    int d = dist(a, b);
    d = 128;
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
void ring2(struct frame f, struct v2 o, struct color c, float radius) {
    for (int i = 0; i < (int)radius*16; i++) {
        int x = o.x + (int)(radius * cos(i));
        int y = o.y + (int)(radius * sin(i));
        point(f, x, y, c);
    }
}
void circle(struct frame f, struct v2 centre, struct color c, float radius) {
    for (int i = 1; i < 4; i++) {

        ring2(f, centre, c, radius);
    }
}

// Bresenham's line – integer only
void line2(struct frame f, struct v2 a, struct v2 b, struct v2 o, struct color c, float r) {
    a = rot_or(a, o, r);
    b = rot_or(b, o, r);
    int x0 = (int)a.x, y0 = (int)a.y;
    int x1 = (int)b.x, y1 = (int)b.y;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        point(f, x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
void tri(struct frame f, struct v2 a, struct v2 b, struct v2 c,
    struct v2 o, struct color cl, float r) {
    a = rot_or(a, o, r);
    b = rot_or(b, o, r);
    c = rot_or(c, o, r);
    line(f,  b,c,  o, cl, 0  );
    int d = dist(b, c);
     d =64;
    for (int i = 0; i <= d; i++) {
        line(f,
            a,
            (struct v2) {
            lerp(b.x, c.x, i / (float)d),
                lerp(b.y, c.y, i / (float)d)
        },
            o, cl, 0
        );
    };
}
int box_draw(int a, int b, int c, int d, float r) {

}

int v2_box(int x, int y, int cx, int cy, int w, int h) {
    if ((x > cx && x < cx + w) && (y > cy && y < cy + h)) return 1;
    else   return 0;
}
int v2_circle(struct v2 v, struct v2 c, float r) {
    if (dist(v, c) < r) return 1;
    return 0;
}
struct color hsv_to_rgb(float h, float s, float v) {
    struct color c;

    h = fmodf(h, 360.0f);          // wrap hue
    float hh = h / 60.0f;
    int i = (int)hh;
    float ff = hh - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - (s * ff));
    float t = v * (1.0f - (s * (1.0f - ff)));

    switch (i) {
    case 0: c.r = (unsigned char)(v * 255); c.g = (unsigned char)(t * 255); c.b = (unsigned char)(p * 255); break;
    case 1: c.r = (unsigned char)(q * 255); c.g = (unsigned char)(v * 255); c.b = (unsigned char)(p * 255); break;
    case 2: c.r = (unsigned char)(p * 255); c.g = (unsigned char)(v * 255); c.b = (unsigned char)(t * 255); break;
    case 3: c.r = (unsigned char)(p * 255); c.g = (unsigned char)(q * 255); c.b = (unsigned char)(v * 255); break;
    case 4: c.r = (unsigned char)(t * 255); c.g = (unsigned char)(p * 255); c.b = (unsigned char)(v * 255); break;
    default: c.r = (unsigned char)(v * 255); c.g = (unsigned char)(p * 255); c.b = (unsigned char)(q * 255); break;
    }
    return c;
}

