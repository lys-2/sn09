#define orange (struct color){255,111,0,255}
#define cyan (struct color){0,255,255,255}
#define green (struct color){0,255,0,255}
#define yellow (struct color){255,255,0,255}
#define red (struct color){255,0,0,255}
#define blue (struct color){0,0,255,255}
#define magenta (struct color){255,0,255,255}
#define white (struct color){255,255,255,255}
#define F s.frame

struct frame { int width; int height; unsigned char* pixels; };
struct v2 { float x, y; };
struct tri { int a, b, c; };
struct color { char r, g, b, a; };
enum type { Tnode, Tline, Ttri };
struct node {
    struct v2 t; struct color c; char is_spawned, is_linked; float sx, sy, rot, a;
    struct v2 v; char la, lb, at;
    enum type type;
};

struct state { 
    int frame, actions; float t;
    struct color pick; struct node scene[123]; int scene_cur, mx, my;
};
struct state s;

void spawn(struct node n);

void move(int x, int y);

void paint(struct frame f, int id);
void clear(struct frame f);
struct color hsv_to_rgb(float h, float s, float v);
void colors(struct frame f, int id);
struct color pick(struct frame f, int x, int y, int id);
void point(struct frame f, int x, int y, struct color c);
void ring2(struct frame f, struct v2 centre, struct color c, float radius);
void circle(struct frame f, struct v2 centre, struct color c, float radius);

void line(struct frame f, struct v2 a, struct v2 b, struct v2 o,
    struct color c, float an);
float line_angle_rad(struct v2 a, struct v2 b);
void init();
void process();