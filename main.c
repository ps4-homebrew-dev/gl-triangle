/*
 * Usage: ps4-gl-triangle [seconds]   (default 30; 0 runs until killed)
 *
 * SPDX-License-Identifier: MIT
 */

#define _POSIX_C_SOURCE 200809L /* clock_gettime */

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define LOG(...)                                                                                   \
   do {                                                                                            \
      fprintf(stderr, "gl-triangle: " __VA_ARGS__);                                                \
      fputc('\n', stderr);                                                                         \
      fflush(stderr);                                                                              \
   } while (0)

static double
now_seconds(void)
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static const char vertex_source[] =
   "attribute vec2 position;\n"
   "attribute vec3 color;\n"
   "uniform float time;\n"
   "uniform float aspect;\n"
   "varying vec3 v_color;\n"
   "void main() {\n"
   "   float c = cos(time), s = sin(time);\n"
   "   vec2 p = vec2(c * position.x - s * position.y, s * position.x + c * position.y);\n"
   "   gl_Position = vec4(p.x / aspect, p.y, 0.0, 1.0);\n"
   "   v_color = color;\n"
   "}\n";

static const char fragment_source[] =
   "precision mediump float;\n"
   "varying vec3 v_color;\n"
   "void main() {\n"
   "   gl_FragColor = vec4(v_color, 1.0);\n"
   "}\n";

static GLuint
compile(GLenum type, const char *source)
{
   GLuint shader = glCreateShader(type);
   GLint ok = 0;
   glShaderSource(shader, 1, &source, NULL);
   glCompileShader(shader);
   glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
   if (!ok) {
      char log[1024];
      glGetShaderInfoLog(shader, sizeof(log), NULL, log);
      LOG("shader compile failed: %s", log);
      exit(1);
   }
   return shader;
}

int
main(int argc, char **argv)
{
   const double run_seconds = argc > 1 ? atof(argv[1]) : 30.0;
   int fd = open("/dev/console", O_WRONLY);
   dup2(fd, 1);
   dup2(fd, 2);

   /* The display: the videoout platform is EGL's native one. */
   EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   EGLint major, minor;
   if (display == EGL_NO_DISPLAY || !eglInitialize(display, &major, &minor)) {
      LOG("eglInitialize failed: 0x%04x", eglGetError());
      return 1;
   }
   LOG("EGL %d.%d, %s", major, minor, eglQueryString(display, EGL_VENDOR));

   if (!eglBindAPI(EGL_OPENGL_ES_API)) {
      LOG("eglBindAPI failed: 0x%04x", eglGetError());
      return 1;
   }

   const EGLint config_attribs[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
      EGL_NONE,
   };
   EGLConfig config;
   EGLint count = 0;
   if (!eglChooseConfig(display, config_attribs, &config, 1, &count) || count == 0) {
      LOG("no EGL config for a window: 0x%04x", eglGetError());
      return 1;
   }

   /* The display is the only window: any non-zero native window names it. */
   EGLSurface surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)1, NULL);
   if (surface == EGL_NO_SURFACE) {
      LOG("eglCreateWindowSurface failed: 0x%04x", eglGetError());
      return 1;
   }

   const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
   EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
   if (context == EGL_NO_CONTEXT) {
      LOG("eglCreateContext failed: 0x%04x", eglGetError());
      return 1;
   }
   if (!eglMakeCurrent(display, surface, surface, context)) {
      LOG("eglMakeCurrent failed: 0x%04x", eglGetError());
      return 1;
   }
   eglSwapInterval(display, 1);

   EGLint width, height;
   eglQuerySurface(display, surface, EGL_WIDTH, &width);
   eglQuerySurface(display, surface, EGL_HEIGHT, &height);
   LOG("GL: %s / %s", (const char *)glGetString(GL_VERSION), (const char *)glGetString(GL_RENDERER));
   LOG("surface: %dx%d", width, height);

   /* The triangle. */
   GLuint program = glCreateProgram();
   glAttachShader(program, compile(GL_VERTEX_SHADER, vertex_source));
   glAttachShader(program, compile(GL_FRAGMENT_SHADER, fragment_source));
   glBindAttribLocation(program, 0, "position");
   glBindAttribLocation(program, 1, "color");
   glLinkProgram(program);
   GLint linked = 0;
   glGetProgramiv(program, GL_LINK_STATUS, &linked);
   if (!linked) {
      LOG("program link failed");
      return 1;
   }
   glUseProgram(program);
   const GLint time_location = glGetUniformLocation(program, "time");
   glUniform1f(glGetUniformLocation(program, "aspect"), (float)width / (float)height);

   static const GLfloat vertices[] = {
      /* x, y, r, g, b */
      0.0f, 0.6f, 1.0f, 0.25f, 0.2f,
      -0.52f, -0.3f, 0.2f, 0.9f, 0.3f,
      0.52f, -0.3f, 0.25f, 0.4f, 1.0f,
   };
   GLuint buffer;
   glGenBuffers(1, &buffer);
   glBindBuffer(GL_ARRAY_BUFFER, buffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glViewport(0, 0, width, height);

   /* Draw until the time is up. */
   const double start = now_seconds();
   double last_report = start;
   unsigned long frames = 0, frames_since_report = 0;
   for (;;) {
      const double t = now_seconds() - start;
      if (run_seconds > 0.0 && t >= run_seconds)
         break;

      const float pulse = 0.5f + 0.5f * sinf((float)t * 1.5f);
      glClearColor(0.05f + 0.15f * pulse, 0.05f, 0.20f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      glUniform1f(time_location, (float)t);
      glDrawArrays(GL_TRIANGLES, 0, 3);

      if (!eglSwapBuffers(display, surface)) {
         LOG("eglSwapBuffers failed: 0x%04x", eglGetError());
         break;
      }

      frames++;
      frames_since_report++;
      const double now = now_seconds();
      if (now - last_report >= 5.0) {
         LOG("%.1f fps", frames_since_report / (now - last_report));
         last_report = now;
         frames_since_report = 0;
      }
   }
   LOG("%lu frames, exiting", frames);

   eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(display, context);
   eglDestroySurface(display, surface);
   eglTerminate(display);
   return 0;
}
