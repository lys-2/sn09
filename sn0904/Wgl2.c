#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <GL/gl.h>
#include <stdlib.h>
#include "sn.h"


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// 1. Define missing modern OpenGL primitive types
typedef char GLchar;
typedef int GLsizei;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

// --- MODERN OPENGL PROFILES, EXTENSIONS, & MACROS ---
#define GL_FRAGMENT_SHADER    0x8B30
#define GL_VERTEX_SHADER      0x8B31
#define GL_COMPILE_STATUS     0x8B81
#define GL_LINK_STATUS        0x8B82
#define GL_INFO_LOG_LENGTH    0x8B84

// Function pointer signatures for Modern OpenGL shader control
typedef GLuint(WINAPI* PFNGLCREATESHADERPROC)(GLenum type);
typedef void  (WINAPI* PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void  (WINAPI* PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void  (WINAPI* PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void  (WINAPI* PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint(WINAPI* PFNGLCREATEPROGRAMPROC)(void);
typedef void  (WINAPI* PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void  (WINAPI* PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void  (WINAPI* PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void  (WINAPI* PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void  (WINAPI* PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint(WINAPI* PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void  (WINAPI* PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);

// Global Function Pointers
PFNGLCREATESHADERPROC     glCreateShader = NULL;
PFNGLSHADERSOURCEPROC     glShaderSource = NULL;
PFNGLCOMPILESHADERPROC    glCompileShader = NULL;
PFNGLGETSHADERIVPROC      glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLCREATEPROGRAMPROC    glCreateProgram = NULL;
PFNGLATTACHSHADERPROC     glAttachShader = NULL;
PFNGLLINKPROGRAMPROC      glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC     glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLUSEPROGRAMPROC       glUseProgram = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM1FPROC        glUniform1f = NULL;

// Global WinAPI variables
HWND   g_hWnd = NULL;
HDC    g_hDC = NULL;
HGLRC  g_hRC = NULL;
BOOL   g_isRunning = TRUE;
GLuint g_ShaderProgram = -1;
GLint  g_TimeLocation = -1;
GLint  g_mx = -1;
GLfloat  g_sx = -1;
GLint  g_sy = -1;
float sx, sy;

// GLSL Shader Source Strings
const char* vertexShaderSource =
"#version 120\n" // Targeted compatibility mode matching classic contexts
"void main() {\n"
"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"   gl_FrontColor = gl_Color;\n"
"}\n";

char* fragmentShaderSource =
"#version 120\n"
"uniform float u_time;\n" // Uniform scalar value updated every single loop frame
"void main() {\n"
"   vec4 baseColor = gl_Color;\n"
"   float pulse = (sin(u_time * 3.0) + 1.0) * 0.5;\n" // Custom mathematical pulsing logic
"   gl_FragColor = vec4(baseColor.rgb * pulse , 1.0);\n"
"}\n";

char* sh;

void loads() {
    FILE* fptr = fopen("frag.glsl", "rb");
    if (fptr) {
        fseek(fptr, 0, SEEK_END);
        long len = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);

         sh = (char*)malloc(len + 1);
        fread(sh, 1, len, fptr);
        fclose(fptr);
        sh[len] = '\0';
    }
}

// Forward Declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void SetupPixelFormat(HDC hDC);
void LoadGLExtensions(void);
GLuint CompileAndLinkShaders(const char* vertSrc, const char* fragSrc);

// Runtime dynamic GLSL compilation engine configuration code helper
GLuint CompileAndLinkShaders(const char* vertSrc, const char* fragSrc) {
    GLint success;
    char infoLog[512];

    // Compile Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSrc, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        MessageBox(NULL, infoLog, "Vertex Shader Compile Error", MB_ICONERROR);
    }

    // Compile Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        MessageBox(NULL, infoLog, "Fragment Shader Compile Error", MB_ICONERROR);
    }

    // Link Shader Program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        MessageBox(NULL, infoLog, "Shader Program Link Error", MB_ICONERROR);
    }

    return program;
}

void sw2() {
    wglMakeCurrent(g_hDC, g_hRC);
}
void gloop2() {

  //  wglMakeCurrent(g_hDC, g_hRC);
    sw2();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Ensure we don't apply old model views to our screenspace coordinates
    glLoadIdentity();

    float time = (float)GetTickCount() / 1000.0f;

    glUseProgram(g_ShaderProgram);
    if (g_TimeLocation != -1) {
        glUniform1f(g_TimeLocation, time);
    }
    glUniform1f(g_mx, (float)s.mx);
    glUniform1f(g_sx, sx);
    glUniform1f(g_sy, sy);

    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
    glUseProgram(0);
    SwapBuffers(g_hDC);

}
void resize2(int x, int y) {
    sw2();
    sx = x;
    sy = y;
    glViewport(0, 0, x, y);

}
void wgl2(HWND hwnd) {
    WNDCLASSEX wc = { 0 };
    MSG msg;
    
    g_hDC = GetDC(hwnd);
    SetupPixelFormat(g_hDC);
    g_hRC = wglCreateContext(g_hDC);
   wglMakeCurrent(g_hDC, g_hRC);

    // Load pipeline extension function address entries before compiling
    LoadGLExtensions();
    loads();
    if (!sh) {
        sh = (char*)fragmentShaderSource;   // use the hardcoded one
    }
    g_ShaderProgram = CompileAndLinkShaders(vertexShaderSource, sh);
    if (g_ShaderProgram<0) return 1;
    g_TimeLocation = glGetUniformLocation(g_ShaderProgram, "u_time");
    g_mx = glGetUniformLocation(g_ShaderProgram, "u_mx");
    g_sx = glGetUniformLocation(g_ShaderProgram, "u_sx");
    g_sy = glGetUniformLocation(g_ShaderProgram, "u_sy");
    resize2(256.,128.);
    // 1. Define the function pointer type for wglSwapIntervalEXT
    typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC) (int);

    // 2. Retrieve the function pointer
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

    // 3. Call this AFTER creating and making your OpenGL context current

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT != NULL) {
        wglSwapIntervalEXT(0); // 0 unlocks the framerate, 1 locks to VSync
    }
    
    gloop2();

  //  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

/*    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    return (int)msg.wParam;*/
}

// Map entry point addresses dynamically from operational graphics drivers
void LoadGLExtensions(void) {
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
}

void SetupPixelFormat(HDC hDC) {
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixelFormat, &pfd);
}

/*LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}*/
