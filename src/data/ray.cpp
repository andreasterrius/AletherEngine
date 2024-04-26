//
// Created by Alether on 4/17/2024.
//

#include "ray.h"
#include "boundingbox.h"

using namespace std;
using namespace ale;


//https://tavianator.com/2022/ray_box_boundary.html
optional<float> Ray::tryIntersect(mat4 transform,
                                  const BoundingBox &box,
                                  float limitTMin,
                                  float limitTMax) {
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

vec3 Ray::resolveT(float t) {
    return this->origin + t * this->dir;
}
