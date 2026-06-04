#include "sn.h"
#include "math.h"

float lerp(float a, float b, float f) {
	float r = a * (1.0 - f) + (b * f);
	return r;
}
float len(struct v2 v) { return sqrt(v.x * v.x + v.y * v.y); };
float dist(struct v2 a, struct v2 b) { return len((struct v2) { a.x - b.x, a.y - b.y }); };

void spawn(struct node n) {
    n.is_spawned = 1;
    s.scene[s.scene_cur % 123] = n;
    s.scene_cur++;
}

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
    for (int i = 0; i < f.width * f.height; i++) {
        int y = i / f.width;
        int x = i % f.width;
        f.pixels[id % 4 + i * 4] += x % 133;
    }
    for (int i = 0; i < 123; i++) {
        point(f, s.scene[i].t.x, s.scene[i].t.y, s.scene[i].c);
        ring2(f, (struct v2) { s.scene[i].t.x, s.scene[i].t.y }, s.scene[i].c, 5);
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
void ring2(struct frame f, struct v2 centre, struct color c, float radius) {
    for (int deg = 0; deg < 360; deg++) {
        float rad = deg * 3.14159265f / 180.0f;
        int x = centre.x + (int)(radius * cos(rad));
        int y = centre.y + (int)(radius * sin(rad));
        point(f, x, y, c);
    }
}

// Bresenham's line – integer only
void draw_line2(struct frame f, struct v2 a, struct v2 b, struct v2 o, struct color c, float r) {
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
int box(int a, int b, int c, int d) {

}

int card(int x, int y, int cx, int cy, int w, int h) {
    if ((x > cx && x < cx + w) && (y > cy && y < cy + h)) return 1;
    else   return 0;
}

struct color hsv_to_rgb(float h, float s, float v) {
    struct color c;
    if (s == 0.0f) {
        // Achromatic (grey)
        c.r = c.g = c.b = (unsigned char)(v * 255.0f);
        return c;
    }
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