//
// Created by Alether on 4/17/2024.
//

#ifndef ALETHERENGINE_RAY_H
#define ALETHERENGINE_RAY_H

#include<string>
#include<glm/glm.hpp>
#include<optional>

using namespace glm;
using namespace std;


namespace ale {

class BoundingBox;

class Ray {
public:
    vec3 origin;
    vec3 dir;
    vec3 invDir;

    Ray(vec3 origin, vec3 dir) : origin(origin), dir(normalize(dir)){}

    optional<float> tryIntersect(const Ray &ray, const BoundingBox &box, float limitTMin, float limitTMax);

    string toString() {
        return "o:(" + to_string(origin.x) + "." + to_string(origin.y) + "," + to_string(origin.z) + ")" +
                " | d:(" + to_string(dir.x) + "." + to_string(dir.y) + "," + to_string(dir.z) + ")";
    }
};

}


#endif //ALETHERENGINE_RAY_H
