//
// Created by Alether on 5/4/2024.
//

#ifndef ALETHERENGINE_SDF_MODEL_H
#define ALETHERENGINE_SDF_MODEL_H

#include<vector>
#include<glm/glm.hpp>
#include"data/boundingbox.h"
#include<functional>

using namespace glm;
using namespace std;

namespace ale {

class Model;
class LineRenderer;

// This is a 3d array representative given a mesh
class SdfModel {
private:
    vec3 cubeSize;
    int cubeCount; // for 1 dimension
    vector<vector<vector<float>>> distances;

public:
    BoundingBox boundingBox; //for debug purposes

    SdfModel(Model &model, int cubeCount = 16);

    // returns small cubes that creates the sdf
    void loopOverCubes(function<void(BoundingBox)> func);
};

}


#endif //ALETHERENGINE_SDF_MODEL_H
