//
// Created by Alether on 4/18/2024.
//

#ifndef ALETHERENGINE_LINE_RENDERER_H
#define ALETHERENGINE_LINE_RENDERER_H

#include "../data/shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <utility>
#include <vector>

using namespace glm;
using namespace std;

static vec3 WHITE = vec3(1.0, 1.0, 1.0);

namespace ale {

class Ray;
class Transform;
class BoundingBox;

class LineRenderer {
private:
  struct Data {
    vec3 startPos;
    vec3 color;
  };
  Shader lineShader;
  unsigned int linesVAO, linesVBO;
  vector<Data> lineData;

  Shader boxShader;
  unsigned int boxVAO, boxVBO, boxInstanceVBO;
  vector<vec3> boxData;

public:
  LineRenderer();

  void queueLine(Ray &ray, vec3 color = WHITE);

  void queueLine(vec3 start, vec3 end, vec3 color = WHITE);

  void queueBox(Transform transform, BoundingBox bb, vec3 color = WHITE);

  void queueUnitCube(Transform transform);

  void render(mat4 projection, mat4 view);
};
} // namespace ale

#endif // ALETHERENGINE_LINE_RENDERER_H
