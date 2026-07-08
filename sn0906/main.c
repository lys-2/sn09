#include <stdio.h>
#include <stdlib.h>
#define SC 1234567

struct item { int id, count; };
struct set { struct item d[SC]; int len; };
struct node { char name[8], is_spawned, is_root, depth; int at, count; };
struct state { struct node c[SC]; int cur; char bin[12345]; struct set select; };

struct set get_child(int id);

struct state s;
int spawn(struct node n) {
	n.is_spawned = 1;
	n.count = 1;
	s.c[s.cur] = n;
	s.cur++;
	return s.cur - 1;
}
void spawn2(char name[8], int at) {
	spawn((struct node) { name, .at = at });
}
void spawn_copy(int id, int at) {
	struct set g;
	g = get_child(id);
	int n = spawn(s.c[id]);
	s.c[n].at = at;
	for (int i = 0; i < g.len; i++) {
		spawn_copy(g.d[i].id, n);
	}
}
int get_parent(int id, int d) {
	if (!d) return s.c[id].at;
	else return get_parent(s.c[id].at, d - 1);
	//   printf("d%i", d);
}

struct set get_child(int id) {
	struct set g = { 0 };
	for (int i = 0; i < SC; i++) {
		if (!s.c[i].is_spawned) continue;
		if (s.c[i].is_root) continue;
		if (s.c[i].at == id) {
			g.d[g.len].id = i;
			g.len++;
		}
	};
	return g;
}
void print_tree(int id, int level) {

	for (int i = 0; i < level; i++) printf(".");
	printf("%s\n", s.c[id].name);
	struct set children = get_child(id);
	for (int i = 0; i < children.len; i++) {
		print_tree(children.d[i].id, level + 1);
	}
}
void move(int id, int to) {
	s.c[id].at = to;
}
void loop() {
	while (1) {}
}
void init() {
	FILE* fptr = fopen("scene.txt", "rb");
	if (fptr) {
		fread(&s.bin, sizeof(s.bin), 1, fptr);
		fclose(fptr);
	}
	printf("%s", s.bin);
	char w[64], d = 0, cur = 0, is_word = 0, depth = 0, ld = 0;
	int last = 0;
	strcpy(w, "node");
	struct node n = { 0 };
	struct node def = { 0 };
	for (int i = 0; i < 12345; i++) {
		n = def;
		if (!s.bin[i]) break;
		if (s.bin[i] == '\r') continue;
		if (s.bin[i] == '\n' && cur == 0) continue;
		if (s.bin[i] == ' ' && !is_word) { depth++, cur--; };
		if (s.bin[i] != ' ') { is_word = 1; };
		if (is_word) w[cur] = s.bin[i];
		cur++;
		if (s.bin[i] == '\n' && cur != 0) {
			w[cur - 1] = 0;
			n.is_root = depth == 0;
			n.depth = depth;
			if (depth > ld) n.at = last;
			if (depth <= ld) n.at = get_parent(last, ld - depth);
			strcpy(n.name, w);
			last = spawn(n);
			ld = depth;
			cur = 0;
			depth = 0;
			is_word = 0;
		};
	};

	spawn_copy(4, 17);
	spawn_copy(4, 17);
	spawn_copy(4, 17);
	spawn_copy(4, 17);
	spawn_copy(4, 17);


	printf("H1!\n");
	struct set g;
	for (int i = 0; i < SC; i++) {
		if (!s.c[i].is_spawned) continue;
		printf("~%i|%s", i, s.c[i].name);
		if (!s.c[i].is_root)
			printf("@%i|%s", s.c[i].at, s.c[s.c[i].at].name);
		printf("\n");
	    g = get_child(i);
		for (int j = 0; j < g.len; j++) {
			printf(".. %s \n", s.c[g.d[j].id].name);
		}
	}
	print_tree(0,0);
}
void main() {
	init();
	loop();
	return 0; }