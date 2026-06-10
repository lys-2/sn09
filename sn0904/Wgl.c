#include <windows.h>
#include <gl/gl.h>
#include "sn.h"
#include "math.h"
#include "wind.h"
#pragma comment(lib, "opengl32.lib")

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

    // Face 4: Bottom (Vertices 1, 3, 2)
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[2]);

    glEnd();
}

float b2f(int b) {
    return -1+(b / 255.);
}

HDC hdc;
HGLRC hglrc;

void sw () {
     wglMakeCurrent(hdc, hglrc);
}
void gloop(HDC hdc) {
    // Make the context current (you must store hdc and hglrc globally or per window)
    // wglMakeCurrent(hdc, hglrc); // uncomment if not already current

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the camera (modelview)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Move camera back 5 units along Z
    glTranslatef(0.0f, 0.0f, -5.0f);
    // Rotate the whole world based on mouse Y (optional)
    glRotatef(s.my / 2.0f, 1.0f, 0.0f, 0.0f);

    // Draw the tetrahedron (it rotates by itself)
    glPushMatrix();
    glRotatef(s.t * 111.0f, 0.0f, 1.0f, 0.0f);   // rotate around Y axis
    drawTetrahedron();
    glPopMatrix();

    // Draw scene triangles (give them a Z value inside the frustum, e.g., -2.0)
    for (int i = 0; i < 123; i++) {
        if (!s.scene[i].is_spawned) continue;
        glPushMatrix();
        // Map X and Y from screen coordinates to world coordinates
        // Assuming screen width 512, height 256, and we want a range of -2..2 in X and -1..1 in Y
        float wx = (s.scene[i].t.x / 256.0f) * 4.0f - 2.0f;  // example mapping
        float wy = (s.scene[i].t.y / 128.0f) * 2.0f - 1.0f;
        glTranslatef(wx, wy, -2.0f);  // Z = -2 (inside frustum)
        glBegin(GL_TRIANGLES);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-0.2f, -0.2f, 0.0f);
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f(0.2f, -0.2f, 0.0f);
        glColor3f(sin(s.t * 112.0f), 1.0f, 1.0f);
        glVertex3f(0.0f, 0.2f, 0.0f);
        glEnd();
        glPopMatrix();
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

    RECT rect;
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
    double right = top * aspect;
    glFrustum(-right, right, -top, top, 0.1, 100.0);

    // Switch to modelview matrix for object transforms
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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
