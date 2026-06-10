#include <stdio.h>
#include <windows.h>

#include "sn.h"
#include "mat.h"
#include "wind.h"
#include "Wa.h"

void init() {
    srand(GetTickCount());
	printf("Hi!\r\n");
    for (int i = 0; i < 111; i++) {
        move(rand() % tex[1].f.width, rand() % tex[1].f.height);
    }
    spawn((struct node) {
        (struct v2) { 33, 33 }, .c = red, .sx = 13, .is_block =1, .is_controlled=1 });
};
void process(double d) {
    if (s.t > 120) reset();
    // Update positions and rotations (skip blocked nodes)
    for (int i = 0; i < 123; i++) {
        if (s.scene[i].is_block) continue;   // blocked nodes don't move
        s.scene[i].t.x += s.scene[i].v.x * d / 3.0;
        s.scene[i].t.y += s.scene[i].v.y * d / 3.0;
        s.scene[i].rot += s.scene[i].a * d;
    }

    // Collision detection between all pairs (only once per pair)
    for (int i = 0; i < 123; i++) {
        if (!s.scene[i].is_spawned) continue;
        for (int j = 0; j < 123; j++) {
            if (!s.scene[j].is_spawned) continue;
            if (i==j) continue;
          //  if (hit(i, j)) {
                if (0) {
                s.scene[i].v.x = 0.;
                s.scene[i].v.y = 0.;
                s.scene[i].c = white;
               // printf("hit");
            }
        }
    }

    s.d = d;
    s.t += d / 1000.0;
}

/*main() { 
	printf("Hello!\n"); 
	lerp(1,2,3);
	F++;
	mat();
	while (1) {};
	return 0;
}*/