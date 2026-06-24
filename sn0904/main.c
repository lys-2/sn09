#include <stdio.h>
#include <stdlib.h>  
#include "main.h"
#include "sn.h"


#if defined(__linux__)
#include <time.h>
#include <unistd.h>
void main() {
    init();
    while (1) {
        process(11.);
        sleep(1);
        F++;
    }

}
#endif

#if defined(_WIN32)

#include <windows.h>
#include "Wa.h"
#include "wind.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR pCmdLine, int nCmdShow) {
    wwin(hInstance);
}

#endif

void init() {
    udp_cleanup();

    int n =  text_new("Player");
        spawn((struct node) {
 (struct v2) { 33, 33 }, .c = red, .sx = 13, .is_block =1,
            .is_controlled=1,.type=Tchar, .name=n });

        n = text_new("Player 2");
        spawn((struct node) {
 (struct v2) { 77, 33 }, .c = red, .sx = 13, .is_block =1, .type=Tchar, .name=n });

    srand(111);
	printf("Hi!\r\n");
    for (int i = 0; i < 111; i++) {
        spawn_r(rand() % 256, rand() % 128);
    }
    spawn((struct node) {.type=Tmesh });
    int ui = spawn((struct node) {.type=Tscene });
    for (int i = 0; i < 11; i++) {
        spawn((struct node) { (struct v2) { rand() % 256, 77 }, .type = Tbutton, .at = ui });
    }
};

void process(double d) {

    if (s.t > 120) reset();
    // Update positions and rotations (skip blocked nodes)
    for (int i = 0; i < SC; i++) {
        if (s.scene[i].is_block) continue;   // blocked nodes don't move
        s.scene[i].t.x += s.scene[i].v.x * d / 3.0;
        s.scene[i].t.y += s.scene[i].v.y * d / 3.0;
        s.scene[i].rot += s.scene[i].a * d;
        s.scene[i].t.z = hf(s.scene[i].t.x, s.scene[i].t.y)*3.;
        if (!(rand()%112))  s.scene[i].is_spawned=0;
    }

    // Collision detection between all pairs (only once per pair)
    for (int i = 0; i < SC; i++) {
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