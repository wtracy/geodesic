#include <stdio.h>
#include <vector>
#include <assert.h>
#include <SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include "vector3d.h"

struct Face {
  int vA;
  int vB;
  int vC;

  //Face(int aa, int ab, int ac): a(aa), b(ab), c(ac) {}

  void set(int a, int b, int c) {
    vA = a;
    vB = b;
    vC = c;
  }
};

void printGLError(GLenum error) {
  switch(error) {
    case GL_NO_ERROR: 
      break;
    case GL_INVALID_ENUM:
      fprintf(stderr, "Invalid enum\n");
      break;
    case GL_INVALID_VALUE:
      fprintf(stderr, "Invalid value\n");
      break;
    default: 
      fprintf(stderr, "Unknown GL error\n");
  }
}


int enqueue(std::vector<vector3d_t> &vertices, vector3d_t vector) {
  vertices.push_back(vector);
  return vertices.size() - 1;
}


void makeFace(int a, int b, int c, std::vector<Face> &faces) {
  Face face;
  face.set(a, b, c);
  faces.push_back(face);
}


void subdivide(int a, int b, int c, std::vector<vector3d_t> &vertices, std::vector<Face> &faces, int count) {
  vector3d_t vector(0, 0, 0);

  vector = vertices[a];
  vector += vertices[b];
  vector.normalize();
  int d = enqueue(vertices, vector);

  vector = vertices[b];
  vector += vertices[c];
  vector.normalize();
  int e = enqueue(vertices, vector);

  vector = vertices[c];
  vector += vertices[a];
  vector.normalize();
  int f = enqueue(vertices, vector);
  
  if (count <= 0) {
    makeFace(a, d, f, faces);
    makeFace(b, d, e, faces);
    makeFace(c, e, f, faces);
    makeFace(d, e, f, faces);
  } else {
    subdivide(a, d, f, vertices, faces, count - 1);
    subdivide(b, d, e, vertices, faces, count - 1);
    subdivide(c, e, f, vertices, faces, count - 1);
    subdivide(d, e, f, vertices, faces, count - 1);
  }
}


int main(int argc, char**argv) {
  assert(sizeof(PFLOAT) == sizeof(GLfloat));
  assert(sizeof(vector3d_t) == 3 * sizeof(GLfloat));
  assert(sizeof(Face) == 3 * sizeof(GLint));

  std::vector<vector3d_t> vertices;
  std::vector<Face> faces;
  bool stillGoing = true;

  /* initialize SDL */
  SDL_Init(SDL_INIT_VIDEO);

  /* set the title bar */
  SDL_WM_SetCaption("Geodesic", "Geodesic");

  /* create window */
  SDL_Surface* screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL);

  glClearColor(1, 1, 1, 1);

  glViewport(0, 0, 640, 480);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(-1, 1, 1, -1, 1, -1);

  glMatrixMode(GL_MODELVIEW);

  glEnable(GL_TEXTURE_2D);

  glLoadIdentity();

  // generate mesh
  vector3d_t vector;
  vector.set(0, 0, 1);
  vertices.push_back(vector);
  vector.set(sin(M_PI*2/3), 0, cos(M_PI*2/3));
  vertices.push_back(vector);
  vector.set(sin(-M_PI*2/3), 0, cos(-M_PI*2/3));
  vertices.push_back(vector);
  vector.set(0, 1, 0);
  vertices.push_back(vector);

  subdivide(0, 1, 3, vertices, faces, 1);
  subdivide(1, 2, 3, vertices, faces, 1);
  subdivide(2, 0, 3, vertices, faces, 1);

  
  for (int i = 0; i < vertices.size(); ++i) {
    vector = vertices[i];
  }
  for (int i = 0; i < faces.size(); ++i) {
    Face face;
    face = faces[i];
  }

  GLfloat vertexBuffer[] = {0, 1, 1.0, -1, -1, 1.0, 1, -1, 1.0};

  while (stillGoing) {

    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      stillGoing = false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0, 0, 0);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(90, 1, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
    printGLError(glGetError());

    glDrawElements(GL_LINES, faces.size() * 3, GL_UNSIGNED_INT, &faces[0]);
    printGLError(glGetError());

    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);

    SDL_GL_SwapBuffers();
  }

  SDL_Quit();
  return 0;
}
