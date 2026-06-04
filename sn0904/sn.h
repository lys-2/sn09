#define orange (struct color){255,111,0,255}
#define cyan (struct color){0,255,255,255}
#define green (struct color){0,255,0,255}
#define yellow (struct color){255,255,0,255}
#define red (struct color){255,0,0,255}
#define blue (struct color){0,0,255,255}
#define magenta (struct color){255,0,255,255}
#define F s.frame

struct frame {
    int width; int height; unsigned char* pixels;
};

void paint(struct frame f, int id);
void clear(struct frame f);
struct color hsv_to_rgb(float h, float s, float v);
void colors(struct frame f, int id);
struct color pick(struct frame f, int x, int y, int id);
void point(struct frame f, int x, int y, struct color c);
void ring2(struct frame f, struct v2 centre, struct color c, float radius);

struct v2 { float x, y; };
struct line { int a, b; };
struct tri { int a, b, c; };
struct data { int cur, len; };
struct path { int cur, len; };
struct mesh { struct data p, l, t; };
struct color { char r, g, b, a; };
enum type { leaf, tile, button, mesh, path };
struct node {
    struct v2 t; struct color c; char is_spawned;
};

struct state { int frame; struct color pick; struct node scene[123]; int scene_cur; };
struct state s;

void spawn(struct node n);

void init();
void process();