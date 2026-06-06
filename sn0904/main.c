#include <stdio.h>
#include <windows.h>

#include "sn.h"
#include "mat.h"

void init() {
	printf("Hi!");
};
void process(double d) {
	for (int i = 0; i < 123; i++) {
		s.scene[i].t.x += s.scene[i].v.x;
		s.scene[i].t.y += s.scene[i].v.y;
		s.scene[i].rot += s.scene[i].a;

	}
	s.t += d/1000.;
	//Sleep(33);
};

/*main() { 
	printf("Hello!\n"); 
	lerp(1,2,3);
	F++;
	mat();
	while (1) {};
	return 0;
}*/