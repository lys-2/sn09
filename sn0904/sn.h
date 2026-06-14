#define orange (struct color){255,111,0,255}
#define cyan (struct color){0,255,255,255}
#define green (struct color){0,255,0,255}
#define yellow (struct color){255,255,0,255}
#define red (struct color){255,0,0,255}
#define blue (struct color){0,0,255,255}
#define magenta (struct color){255,0,255,255}
#define white (struct color){255,255,255,255}
#define F s.frame
#define SC 1234

struct frame { int width; int height; unsigned char* pixels; };
struct v2 { float x, y, z; };
struct v3 { float x, y, z; };
struct tri { int a, b, c; };
enum node_state { idle, walk, run, hi, sit, lay, bye };
enum status { none, wet, dirt };
struct ex { float x, y, r; char sa, sb;};
struct pack { struct ex n[32]; };
struct color { unsigned char r, g, b, a; };

struct address { unsigned char a[4]; };

enum type { Tnode, Tline, Ttri, Tchar, Tbox };
struct node {
    struct v2 t; struct color c;
    char is_spawned, is_la, is_lb, is_block, is_controlled, auto_spawn;
    float sx, sy, sz, rot, a, dur;
    struct v2 v; char la, lb, at;
    char type;

};

struct state {
    double n;
    int sa;
    double smp;
    int frame, actions; float t, d;
    struct color pick; struct node scene[SC]; int scene_cur, mx, my;
    char is_host, is_remote;
};
extern struct state s, def;

int spawn(struct node n);

void move(int x, int y);
int hit(int a, int b);
void play(int id);
void load();
void save();

int player();
float hf(float x, float y);
float len(struct v2 v);
void paint(struct frame f, int id);
void clear(struct frame f);
struct color hsv_to_rgb(float h, float s, float v);
void colors(struct frame f, int id);
struct color pick(struct frame f, int x, int y, int id);
void point(struct frame f, int x, int y, struct color c);
void ring2(struct frame f, struct v2 centre, struct color c, float radius);
void circle(struct frame f, struct v2 centre, struct color c, float radius);
void tri(struct frame f, struct v2 a, struct v2 b, struct v2 c,
    struct v2 o, struct color cl, float r);
void line(struct frame f, struct v2 a, struct v2 b, struct v2 o,
    struct color c, float an);
void line2(struct frame f, struct v2 a, struct v2 b, struct v2 o, struct color c, float r);
float line_angle_rad(struct v2 a, struct v2 b);
int v2_circle(struct v2 a, struct v2 b, float r);
void reset();

short sound2();