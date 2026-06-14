
#include <stdio.h>
#include "sn.h"
#include "math.h"
#include <stdlib.h>
#include "main.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
struct rule {
    char start[8], replace[8];
    char it, len, letter;
    float angle;
};
struct trans { struct v2 p; float rot; };
struct state s, def;
void turtle(
    struct trans t, struct rule rule, int depth);
short sound2() {
    double st = 6.283 / (44100. / s.mx / 2.);
    double n = sin(s.sa* 6.283 *s.n /44100.)*2222;
    s.sa++;
    s.smp += st;
    double out = 0;
    out = sin(s.smp * sin(s.smp / s.my * 4.)) * sin(s.smp / 1111.) * 123.;
    out += n;
    return out;

};

void input(int key) {}
void host() {
    s.is_host = 1;
    udp_init();
}
void join() {

}
void text() {

}
int player() {
    for (int i = 0; i < SC; i++) {
        if (s.scene[i].is_spawned && s.scene[i].is_controlled) return i;
        return -1;
    };
}
float hf(float x, float y) {

    return 
        sin(x * sin(y / 2212.)) * sin(len((struct v2) {x,y})/13111.);
}

void play(int id) {
    double freq = 440.0 * pow(2.0, (id - 69) / 12.0);
    printf("%i %f played!\n", id, freq);
    spawn((struct node) { (struct v2) { s.t * 55, (id-21)*3 }, .is_block=1, .sx=6, .c=white });
    s.n = freq;
    for (int i = 1; i < 5; i++) {
        turtle(
            (struct trans) {
            s.t * 55, (id - 21) * 3, 0.
        },
            (struct rule) {
            "F", "FF+", 8, 12, 'F', i + sin(F / 11.) / 1111.
        }, 0);
    }
}

float lerp(float a, float b, float f) {
	float r = a * (1.0 - f) + (b * f);
	return r;
}
float len(struct v2 v) { return sqrt(v.x * v.x + v.y * v.y); };
float dist(struct v2 a, struct v2 b) { return len((struct v2) { a.x - b.x, a.y - b.y }); };

struct v2 forward2(float a) {
    struct v2 forward;
    float ad = a ;
    forward.x = cosf(ad);
    forward.y = sinf(ad);
    return forward;
}

void reset() { s = def; init(); s.frame = 345; }
void load() {
    FILE* fptr = fopen("save_nr", "rb");
    if (fptr) {
        fread(&s, sizeof(s), 1, fptr);
        fclose(fptr);
    }
}
void save() {
    FILE* fptr = fopen("save_nr", "wb");
    if (fptr) {
        fwrite(&s, sizeof(s), 1, fptr);
        fclose(fptr);
    }
}

int spawn(struct node n) {
    if (s.scene[(s.scene_cur)% SC].is_block) {
        s.scene_cur++; return -1;
    }
    n.is_spawned = 1;
    n.t.z = (rand() % 222)/222.;
    s.scene[s.scene_cur % SC] = n;
    int r = s.scene_cur;
    s.scene_cur++;
    return r;
}

int hit(int a, int b) {
    return v2_circle(s.scene[a].t, s.scene[b].t, s.scene[b].sx);
}
int spawn_r(int x, int y) {
    struct color c;
    if (s.pick.a == 0) c = hsv_to_rgb(rand() % 360, 1, 1);
    else c = s.pick;
    spawn((struct node) {
        (struct v2) {x, y},
            .c = c,
            .sx = 2 + rand() % 7,
            .is_la = !(rand() % 11),
            .is_lb = !(rand() % 11),
            .la = rand() % 111,
            .lb = rand() % 111,
            .v.x = (111 / 2 - rand() % 111) / 111.,
            .v.y = (111 / 2 - rand() % 111) / 111.,
            .a = (111 / 2 - rand() % 111) / 11111.,
            .rot = line_angle_rad( (struct v2) { s.mx, s.my }, (struct v2) {x, y})
    });
}

void move(int x, int y) {

    s.mx = x;
    s.my = y;
    spawn_r(x,y);
    for (int i = 0; i < SC; i++) {
        if (s.scene[i].is_controlled) {
            s.scene[i].rot = line_angle_rad((struct v2) {
                s.scene[i].t.x, s.scene[i].t.y},
                (struct v2) {x, y});        
        }
    }

    s.actions++;
};
void move2(int x, int y) {
    for (int i = 0; i < SC; i++) {
        if (s.scene[i].is_controlled) {
            struct v2 f = forward2(s.scene[i].rot);
            s.scene[i].t.x += f.x*s.d;
            s.scene[i].t.y += f.y*s.d;
        }
    }
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
        f.pixels[1 % 4 + i * 4] = ((1.+hf(x,y))/2.)*66.;
      //  f.pixels[1 % 4 + i * 4] += (x%11)*3;
/*        f.pixels[0 + i * 4] = (y%13)*2;
        f.pixels[0 + i * 4] += (y%17)*3;*/
    }
    for (int i = 0; i < SC; i++) {

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
    for (int i = 0; i < SC; i++) {
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
    for (int i = 0; i < SC; i++) {
        if (!s.scene[i].is_spawned) continue;

        point(f, s.scene[i].t.x, s.scene[i].t.y, s.scene[i].c);
        circle(f, (struct v2) { s.scene[i].t.x, s.scene[i].t.y },
            s.scene[i].c, s.scene[i].sx);
    }
        for (int i = 0; i < SC; i++) {
            if (!s.scene[i].is_spawned ) continue;

        line(f, 
            (struct v2) { s.scene[i].t.x, s.scene[i].t.y },
            (struct v2) { s.scene[i].t.x+ s.scene[i].sx*2, s.scene[i].t.y },
            (struct v2) { s.scene[i].t.x, s.scene[i].t.y },
            white,
            s.scene[i].rot
            );
    }

        }

    for (int i = 0; i < SC; i++) {
        if (!s.scene[i].is_spawned || !s.scene[i].is_controlled) continue;

#if defined(_WIN32)
        wtext("playe1r7", s.scene[i].t.x, s.scene[i].t.y, 0, 1);
#endif

    };
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
void circle(struct frame f, struct v2 o, struct color c, float r) {
    for (int i = 1; i < 14; i++) {

        ring2(f, o, c, r/i);
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
int box_draw(float sx, float sy, struct v2 o, float r) {

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

struct v2 forward(struct v2 p, float angle, float d) {
    struct v2 f = { cos(angle), sin(angle) };
    p.x += f.x * d;
    p.y += f.y * d;
    return p;
}

char* lsys(char* st, char* r, int d) {

    if (!d) return st;

    char str[12345] = { 0 };
    char c[2]; c[1] = 0;
    int i = 0;
    while (st[i]) {
        if (st[i] == 'F') { strcat(&str, r); }
        else { c[0] = st[i]; strcat(&str, &c); }
        i++;
    }
    //printf("%s, %i\n", &str, d); 
    return lsys(&str, r, d - 1);

}
void turtle(
    struct trans t, struct rule rule, int depth) {

    char* str = lsys(rule.start, rule.replace, rule.it);
    struct v2 a = t.p;
    struct v2 b = { 0 };
    struct v2 ps = { 0 };
    float rs = 0;

    float rot = t.rot;
    int i = 0;
    printf("str %s", str);
    while (str[i] != 0) {
        if (str[i] == '+') { rot += 3.1415 / rule.angle; }
        if (str[i] == '-') { rot -= 3.1415 / rule.angle; }
        if (str[i] == '[') { rs = rot; ps = a; }
        if (str[i] == ']') { rot = rs; a = ps; }

        if (str[i] == 'F') {
            b = forward(a, rot, rule.len);
            spawn((struct node) {
                (struct v2) {
                a.x, a.y
            },
                    .c = hsv_to_rgb(rand() % 360, 1, 1),
                    .sx = 2 + rand() % 7,
                    .is_la = 1,
                    .la = 1,
                    .rot = line_angle_rad((struct v2) { s.mx, s.my }, (struct v2) { a.x, a.y })
            });
            a = b;
        }
        i++;
    }

}