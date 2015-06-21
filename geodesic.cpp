#include <stdio.h>
#include <vector>
#include "vector3d.h"

struct Face {
  int a;
  int b;
  int c;

  Face(int aa, int ab, int ac): a(aa), b(ab), c(ac) {}

  void set(int aa, int ab, int ac) {
    a = aa;
    b = ab;
    c = ac;
  }
};


int enqueue(std::vector<vector3d_t> &vertices, vector3d_t vector) {
  vertices.push_back(vector);
  return vertices.size() - 1;
}


void makeFace(int a, int b, int c, std::vector<Face> &faces) {
  Face face(a, b, c);
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
  std::vector<vector3d_t> vertices;
  std::vector<Face> faces;

  printf("<?xml version=\"1.0\"?>\n<scene type=\"triangle\">");
  printf("<material name=\"Yellow.001\"><color r=\"0.859948\" g=\"0.841801\" b=\"0.185443\" a=\"1\"/><type sval=\"shinydiffusemat\"/></material>");
  printf("<light name=\"Lamp.001\"><color r=\"1\" g=\"1\" b=\"1\" a=\"1\"/><corner x=\"-0.25\" y=\"02.25\" z=\"2\"/><from x=\"0\" y=\"02.5\" z=\"2\"/><point1 x=\"-0.25\" y=\"02.75\" z=\"2\"/><point2 x=\"0.25\" y=\"02.25\" z=\"2\"/><power fval=\"5\"/><type sval=\"arealight\"/></light>");
  printf("<camera name=\"cam\"><from x=\"0\" y=\"3.9\" z=\"0\"/><resx ival=\"375\"/><resy ival=\"375\"/><to x=\"0\" y=\"3.7\" z=\"0\"/><type sval=\"perspective\"/><up x=\"0\" y=\"-3.9\" z=\"2\"/></camera>");
  printf("<integrator name=\"default\"><type sval=\"photonmapping\"/></integrator>");
  printf("<integrator name=\"volintegr\"><type sval=\"none\"/></integrator>");
  printf("<render><AA_inc_samples ival=\"2\"/><AA_minsamples ival=\"4\"/><AA_passes ival=\"2\"/><AA_pixelwidth fval=\"1.5\"/><AA_threshold fval=\"0.05\"/><camera_name sval=\"cam\"/><filter_type sval=\"mitchell\"/><gamma fval=\"2.2\"/><height ival=\"375\"/><integrator_name sval=\"default\"/><volintegrator_name sval=\"volintegr\"/><width ival=\"375\"/></render>");

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

  subdivide(0, 1, 3, vertices, faces, 2);
  subdivide(1, 2, 3, vertices, faces, 2);
  subdivide(2, 0, 3, vertices, faces, 2);
  
  printf("<mesh vertices=\"%d\" faces=\"%d\">", vertices.size(), faces.size());
  for (int i = 0; i < vertices.size(); ++i) {
    vector = vertices[i];
    printf("<p x=\"%f\" y=\"%f\" z=\"%f\" />", vector.x, vector.y, vector.z);
  }
  printf("<set_material sval=\"Yellow.001\"/>");
  for (int i = 0; i < faces.size(); ++i) {
    Face face(0, 0, 0);
    face = faces[i];
    printf("<f a=\"%d\" b=\"%d\" c=\"%d\"/>", face.a, face.b, face.c);
  }
  printf("</mesh>");

  printf("</scene>");
}
