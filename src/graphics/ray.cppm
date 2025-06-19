//
// Created by Alether on 4/17/2024.
//
module;

#include <glm/glm.hpp>
#include <optional>
#include <string>

export module graphics:ray;
import data;

using namespace std;
using namespace ale;
using namespace ale::data;
using namespace glm;

export namespace ale::graphics {
class Ray {
public:
  glm::vec3 origin;
  glm::vec3 dir;
  glm::vec3 invDir;

  Ray(glm::vec3 origin, glm::vec3 dir) : origin(origin), dir(normalize(dir)) {
    this->invDir = glm::vec3(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);
  }

  // https://tavianator.com/2022/ray_box_boundary.html
  optional<float> intersect(const BoundingBox &boxR, float limitTMin = 0,
                            float limitTMax = INFINITY) {
    // BoundingBox box = boxR.apply_scale(transform);
    BoundingBox box = boxR;
    double tx1 = (box.min.x - origin.x) * invDir.x;
    double tx2 = (box.max.x - origin.x) * invDir.x;
    double tmin = std::min(tx1, tx2);
    double tmax = std::max(tx1, tx2);

    double ty1 = (box.min.y - origin.y) * invDir.y;
    double ty2 = (box.max.y - origin.y) * invDir.y;
    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    double tz1 = (box.min.z - origin.z) * invDir.z;
    double tz2 = (box.max.z - origin.z) * invDir.z;
    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    if (tmax < tmin) {
      return nullopt;
    } else if (tmin > limitTMin && tmin < limitTMax) {
      return tmin;
    } else if (tmax > limitTMin && tmax < limitTMax) {
      return tmax;
    }
    return nullopt;
  }

  glm::vec3 resolve(float t) { return this->origin + t * this->dir; }

  Ray apply_transform_inversed(Transform t) {
    auto inv_mat = inverse(t.get_model_matrix());
    auto O = inv_mat * vec4(origin, 1.0);
    auto D = normalize(inv_mat * vec4(dir, 0.0));
    return Ray(O, D);
  };

  std::string to_string() {
    return "o:(" + std::to_string(origin.x) + "." + std::to_string(origin.y) +
           "," + std::to_string(origin.z) + ")" + " | d:(" +
           std::to_string(dir.x) + "." + std::to_string(dir.y) + "," +
           std::to_string(dir.z) + ")";
  }
};
} // namespace ale::graphics
