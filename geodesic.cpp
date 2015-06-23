#include <stdio.h>
#include <vector>
#include <assert.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <SDL.h>
#include "vector3d.h"


struct Edge {
  int a;
  int b;
  int center;

  int childA;
  int childB;

  void set(int aIn, int bIn) {
    a = aIn;
    b = bIn;
    center = -1;
  }

  /* Generates a new vertex at the midpoint of the line (projected onto the 
   * sphere) and two "child"
   * lines spanning the midpoint and the respective end points of the
   * original line.
   */
  int split(std::vector<vector3d_t>& vertices, std::vector<Edge>& edges) {
    if (center == -1) { // if we haven't already done this
      vector3d_t v(0, 0, 0);

      // Create the new vertex
      v += vertices[a];
      v += vertices[b];
      v.normalize();
      vertices.push_back(v);
      center = vertices.size() - 1;

      // Create the new edges
      Edge edge;

      edge.set(a, center);
      edges.push_back(edge);
      childA = edges.size() - 1;

      edge.set(center, b);
      edges.push_back(edge);
      childB = edges.size() - 1;
    }
    return center;
  }
};

struct Face {
  int ab;
  int bc;
  int ca;

  void set(int a, int b, int c) {
    ab = a;
    bc = b;
    ca = c;
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




/* Breaks the face into four triangles. The original triangle has vertices
 * a, b, and c, while the children share both those and the new vertices
 * d, e, and f.
 */
void subdivide(Face& face, std::vector<vector3d_t> &vertices, 
               std::vector<Edge>& edges, 
               std::vector<Face>& faces, 
               int count) {
  if (count > 0) { // if we haven't recursed too far
    Edge temp;

    /* We always call split() on a temp variable because the addition of new lines
     * will cause the std::vector to resize, and it might yank our edge
     * out from under us.
     */

    /* Generate vertex d and attached lines. */
    temp = edges[face.ab];
    int d = temp.split(vertices, edges);
    edges[face.ab] = temp;
    int ad = edges[face.ab].childA;
    int db = edges[face.ab].childB;

    // Generate vertex e and attached lines
    temp = edges[face.bc];
    int e = temp.split(vertices, edges);
    edges[face.bc] = temp;
    int be = edges[face.bc].childA;
    int ec = edges[face.bc].childB;

    // Generate vertex f and attached lines
    temp = edges[face.ca];
    int f = temp.split(vertices, edges);
    edges[face.ca] = temp;
    int cf = edges[face.ca].childA;
    int fa = edges[face.ca].childB;

    Edge edge;

    // Create the edges interior to the original triangle
    edge.set(d, e);
    edges.push_back(edge);
    int de = edges.size() - 1;

    edge.set(e, f);
    edges.push_back(edge);
    int ef = edges.size() - 1;

    edge.set(f, d);
    edges.push_back(edge);
    int fd = edges.size() - 1;

    Face face;

    // Generate faces inside the new lines, and continue subdividing them
    face.set(ad, fd, fa);
    faces.push_back(face);
    subdivide(face, vertices, edges, faces, count - 1);

    face.set(be, de, db);
    faces.push_back(face);
    subdivide(face, vertices, edges, faces, count - 1);

    face.set(cf, ef, ec);
    faces.push_back(face);
    subdivide(face, vertices, edges, faces, count - 1);

    face.set(de, ef, fd);
    faces.push_back(face);
    subdivide(face, vertices, edges, faces, count - 1);
  }
}


int main(int argc, char**argv) {
  assert(sizeof(PFLOAT) == sizeof(GLfloat));
  assert(sizeof(vector3d_t) == 3 * sizeof(GLfloat));
  assert(sizeof(Face) == 3 * sizeof(GLint));

  std::vector<vector3d_t> vertices;
  std::vector<Edge> edges;
  std::vector<Face> faces;
  bool stillGoing = true;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_WM_SetCaption("Geodesic", "Geodesic");
  SDL_Surface* screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL);
  glClearColor(0, 0, 0, 1);
  glViewport(0, 0, 640, 480);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-2, 2, 2, -2, 1, -1);
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

  Edge edge;
  edge.set(0, 1);
  edges.push_back(edge);
  edge.set(3, 0);
  edges.push_back(edge);
  edge.set(1, 2);
  edges.push_back(edge);
  edge.set(3, 1);
  edges.push_back(edge);
  edge.set(2, 0);
  edges.push_back(edge);
  edge.set(2, 3);
  edges.push_back(edge);

  int subdivisions = 6;
  Face face;
  face.set(0, 1, 3);
  faces.push_back(face);
  face.set(2, 3, 5);
  faces.push_back(face);
  face.set(4, 5, 1);
  faces.push_back(face);

  subdivide(faces[0], vertices, edges, faces, subdivisions);
  subdivide(faces[1], vertices, edges, faces, subdivisions);
  subdivide(faces[2], vertices, edges, faces, subdivisions);

  printf("Vertex count: %d\n", vertices.size());
  
  while (stillGoing) {
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      stillGoing = false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0, 1, 0);
    glPushMatrix();
    glLoadIdentity();
    //glRotatef(clock() / 1000, 1, 0, 0);
    glRotatef(90, 1, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
    glDrawArrays(GL_POINTS, 0, vertices.size() - 0);

    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);

    SDL_GL_SwapBuffers();
  }

  SDL_Quit();
  return 0;
}
