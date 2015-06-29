#include <stdio.h>
#include <vector>
#include <map>
#include <utility>
#include <assert.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <SDL.h>
#include "vector3d.h"



struct Face {
  int a;
  int b;
  int c;

  void set(int _a, int _b, int _c) {
    a = _a;
    b = _b;
    c = _c;
  }
};

struct UnpackedFace {
  int a1;
  int a2;
  int b1;
  int b2;
  int c1;
  int c2;

  UnpackedFace(int _a1, int _a2, int _b1, int _b2, int _c1, int _c2) {
    a1 = _a1;
    a2 = _a2;
    b1 = _b1;
    b2 = _b2;
    c1 = _c1;
    c2 = _c2;
  }
};

std::vector<vector3d_t> vertices;
std::map<std::pair<int, int>, int> midpoints;
std::vector<Face> faces;
std::vector<UnpackedFace> unpackedFaces;


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

int getMidpoint(int a, int b) {
  std::map<std::pair<int, int>, int>::iterator it;
  it = midpoints.find(std::pair<int, int>(a, b));

  if (it == midpoints.end()) {
    vector3d_t midpoint = vertices[a];
    midpoint += vertices[b];
    midpoint.normalize();
    vertices.push_back(midpoint);
    int midpoint_index = vertices.size() - 1;

    midpoints[std::pair<int, int>(a, b)] = midpoint_index;
    midpoints[std::pair<int, int>(b, a)] = midpoint_index;

    return midpoint_index;
  } else {
    const std::pair<const std::pair<int, int>, int> value = *it;
    return value.second;
  }
}


/* Breaks the face into four triangles. The original triangle has vertices
 * a, b, and c, while the children share both those and the new vertices
 * d, e, and f.
 */
void subdivide(int a, int b, int c,
               int count) {
  if (count > 0) { // if we haven't recursed too far
    int d = getMidpoint(a, b);
    int e = getMidpoint(b, c);
    int f = getMidpoint(c, a);

    subdivide(a, d, f, count - 1);
    subdivide(b, d, e, count - 1);
    subdivide(c, e, f, count - 1);
    subdivide(d, e, f, count - 1);
  } else {
    Face f;
    f.set(a, b, c);
    faces.push_back(f);

    UnpackedFace uf(a, b, b, c, c, a);
    unpackedFaces.push_back(uf);
  }
}


int main(int argc, char**argv) {
  bool stillGoing = true;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_WM_SetCaption("Geodesic", "Geodesic");
  SDL_Surface* screen = SDL_SetVideoMode(480, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL);
  glClearColor(1, 1, 1, 1);
  glViewport(0, 0, 480, 480);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0, 1.0, 1.0, -1.0, 1, -1);
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

  int divisions = 4;
  subdivide(0, 1, 3, divisions);
  subdivide(1, 2, 3, divisions);
  subdivide(2, 0, 3, divisions);
  
  while (stillGoing) {
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      stillGoing = false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0, 0, 0);
    glPushMatrix();
    glLoadIdentity();
    //glRotatef(clock() / 2000, 1, 0, 0);
    glRotatef(90, 1, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
    //glDrawArrays(GL_POINTS, 0, vertices.size() - 0);
    glDrawElements(GL_LINES, unpackedFaces.size()*6, GL_UNSIGNED_INT, &unpackedFaces[0]);
    //glDrawElements(GL_TRIANGLES, faces.size()*3, GL_UNSIGNED_INT, &faces[0]);

    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);

    SDL_GL_SwapBuffers();
  }

  SDL_Quit();
  return 0;
}
