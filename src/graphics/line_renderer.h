//
// Created by Alether on 4/18/2024.
//

#ifndef ALETHERENGINE_LINE_RENDERER_H
#define ALETHERENGINE_LINE_RENDERER_H

#include <glm/glm.hpp>
#include <vector>
#include "model.h"
#include "shader.h"

import color;

namespace ale {

class Ray;
class Transform;
class BoundingBox;

class LineRenderer {
private:
  struct Data {
    glm::vec3 startPos;
    glm::vec3 color;
  };
  Shader lineShader;
  unsigned int linesVAO, linesVBO;
  vector<Data> lineData;

  Shader boxShader;
  unsigned int boxVAO, boxVBO, boxInstanceVBO;
  vector<glm::vec3> boxData;

  Model create_box();

public:
  LineRenderer();

  void queue_line(Ray &ray, glm::vec3 color = WHITE);

  void queue_line(glm::vec3 start, glm::vec3 end, glm::vec3 color = WHITE);

  void queue_box(Transform transform, BoundingBox bb, glm::vec3 color = WHITE);

  void queue_unit_cube(Transform transform);

  void render(glm::mat4 projection, glm::mat4 view);
};
} // namespace ale

#endif // ALETHERENGINE_LINE_RENDERER_H
