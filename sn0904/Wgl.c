#include <windows.h>
#include <gl/gl.h>
#include "sn.h"
#include "math.h"
#include "wind.h"
#pragma comment(lib, "opengl32.lib")
void sw();

GLfloat vertices[4][3] = {
    { 0.0f,  1.0f,  0.0f },   // Top vertex (0)
    {-1.0f, -1.0f,  1.0f },   // Front-left vertex (1)
    { 1.0f, -1.0f,  1.0f },   // Front-right vertex (2)
    { 0.0f, -1.0f, -1.0f }    // Back vertex (3)
};

void drawTetrahedron() {
    glBegin(GL_TRIANGLES);

    // Face 1: Front (Vertices 0, 1, 2)
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);

    // Face 2: Right (Vertices 0, 2, 3)
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);

    // Face 3: Left (Vertices 0, 3, 1)
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[1]);

    glColor3f(1.0f, 0.0f, 1.0f); 
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[2]);

    glEnd();
}

float b2f(int b) {
    return -1+(b / 255.);
}

void resize(int w, int h) {
    sw();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double aspect = (double)w / (double)h;
    double fov = 22.0 * 3.14159 / 180.0;
    double top = 0.1 * tan(fov / 2.0);
    double right = -top * aspect;
    glFrustum(-right, right, -top, top, 0.1, 113111.0);
    glViewport(0, 0, w, h);


}

HDC hdc;
HGLRC hglrc;
RECT rect;
int p;
void sw () {
     wglMakeCurrent(hdc, hglrc);
}
void gloop(HDC hdc) {

    sw();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.4f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    p = player();
    if (p == -1) return;


    glRotatef(s.scene[p].rot * 57.29, 0.0f, 1.0f, 0.0f);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    glRotatef(90., 1.0f, 0.0f, 0.0f);
    glTranslatef(-s.scene[p].t.x, -s.scene[p].t.y, 11);

    glPushMatrix();
    glTranslatef(111., 111., -21.);
    glRotatef(s.t * 57.29f, 0.0f, 1.0f, 0.0f);
    glScalef(11., 11., 11.);
    drawTetrahedron();
    glPopMatrix();

    for (int i = 0; i < SC; i++) {
        if (!s.scene[i].is_spawned) continue;
        glPushMatrix();
        // Map X and Y from screen coordinates to world coordinates
        // Assuming screen width 512, height 256, and we want a range of -2..2 in X and -1..1 in Y
        float wx = s.scene[i].t.x;  // example mapping
        float wy = s.scene[i].t.y;
        float wz = s.scene[i].t.z;
        glTranslatef(wx, wy, wz);  // Z = -2 (inside frustum)
     //   glRotatef(90., 1.0f, 0.0f, 0.0f);
        glScalef(s.scene[i].sx*11.0f, s.scene[i].sx*11.0f, 1.0f);
        glRotatef(s.scene[i].rot * 57., 0.0f, 0.0f, 1.0f);
        if (!(rand()%3)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_TRIANGLES);
        glColor3f(s.scene[i].c.r/255., s.scene[i].c.g/255., s.scene[i].c.b/255.);
        glVertex3f(-0.2f, -0.2f, 0.0f);
        glColor3f(s.scene[i].c.r / 255., s.scene[i].c.g / 255., s.scene[i].c.b / 255.);
        glVertex3f(0.2f, -0.2f, 0.0f);
        glColor3f(1., 1., s.scene[i].c.b / 255.);
        glVertex3f(0.0f, 0.2f, 0.f);
        glEnd();
        glPopMatrix();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glPushMatrix();
    glRotatef(-90., 1.0f, 0.0f, 0.0f);

    glTranslatef(0., -6., 0.);
    glScalef(1111.,1.,1131.);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, .1f, 0.0f);
    glVertex3f(0.f, -1.2f, -1.0f);
    glColor3f(0.0f, .3f, 1.0f);
    glVertex3f(-1.2f, -1.f, 1.0f);
    glColor3f(0., 1.0f, 1.0f);
    glVertex3f(1.0f, -1.f, 1.0f);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glColor4f(1.0f, .1f, 1.0f, .1);
    glVertex3f(s.mx, s.my, 0.0f);
    glColor4f(0.0f, .3f, 1.0f, .1);
    glVertex3f(s.mx, s.my, -22.0f);
    glEnd();
    glPopMatrix();

    for (int i = 0; i < 1024; i++) {
        int x = 12343 - rand() % 23454;
        int y = 12343 - rand() % 23455;
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        glColor3f(1.0f, .1f, 1.0f);
        glVertex3f(x, y, hf(x, y)*123.);
        glEnd();
    }
    SwapBuffers(hdc);
}


int wgl(HWND hwnd) {

    hdc = GetDC(hwnd);
    // 4. Define and set the Pixel Format Description
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int format = ChoosePixelFormat(hdc, &pfd);
    if (!format) return 0;
    if (!SetPixelFormat(hdc, format, &pfd)) return 0;

    // 5. Create and activate the OpenGL Rendering Context (WGL)
    hglrc = wglCreateContext(hdc);
    if (!hglrc) return 0;
    if (!wglMakeCurrent(hdc, hglrc)) return 0;


    GetClientRect(hwnd, &rect);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double aspect = (double)w / (double)h;
    double fov = 22.0 * 3.14159 / 180.0;
    double top = 0.1 * tan(fov / 2.0);
    double right = -top * aspect;
    glFrustum(-right, right, -top, top, 0.1, 111113.0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);


    // 1. Define the function pointer type for wglSwapIntervalEXT
    typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC) (int);

    // 2. Retrieve the function pointer
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

    // 3. Call this AFTER creating and making your OpenGL context current

        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        if (wglSwapIntervalEXT != NULL) {
            wglSwapIntervalEXT(0); // 0 unlocks the framerate, 1 locks to VSync
        }
    

    // 8. Cleanup resources before exit
/*    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);*/

}
