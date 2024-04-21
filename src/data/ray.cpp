//
// Created by Alether on 4/17/2024.
//

#include "ray.h"
#include "boundingbox.h"

using namespace std;
using namespace ale;


//https://tavianator.com/2022/ray_box_boundary.html
optional<float> Ray::tryIntersect(const Ray &ray,
                                  const BoundingBox &box,
                                  float limitTMin = 0,
                                  float limitTMax = INFINITY) {
    double tx1 = (box.min.x - ray.origin.x) * ray.invDir.x;
    double tx2 = (box.max.x - ray.origin.x) * ray.invDir.x;

    double tmin = std::min(tx1, tx2);
    double tmax = std::max(tx1, tx2);

    double ty1 = (box.min.y - ray.origin.y) * ray.invDir.y;
    double ty2 = (box.max.y - ray.origin.y) * ray.invDir.y;

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    if (tmax < tmin) {
        return nullopt;
    }

    if (tmin > limitTMin && tmin < limitTMax) {
        return tmin;
    } else if (tmax > limitTMin && tmax < limitTMax) {
        return tmax;
    }
    return nullopt;
}
