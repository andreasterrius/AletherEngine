//
// Created by Alether on 5/4/2024.
//

#ifndef ALETHERENGINE_SDF_MODEL_H
#define ALETHERENGINE_SDF_MODEL_H

#include<vector>
#include<glm/glm.hpp>
#include"data/boundingbox.h"
#include<functional>
#include<optional>

using namespace glm;
using namespace std;

namespace ale {

class Model;
class LineRenderer;

class Texture3D {
public:
    unsigned int id;

    Texture3D(vector<vector<vector<float>>> distances);
};

// This is a 3d array representative given a mesh
class SdfModel {
private:
    vec3 cubeSize;
    int cubeCount; // for 1 dimension

    optional<Texture3D> texure3D;

public:
    //TODO: move to private, public is for debugging
    vector<vector<vector<float>>> distances;
    vector<vector<vector<vec3>>> positions; // Only used for debugging

    BoundingBox boundingBox; //for debug purposes

    SdfModel(Model &model, int cubeCount = 16);

    // returns small cubes that creates the sdf
    void loopOverCubes(function<void(int, int, int, BoundingBox)> func);
};

}


#endif //ALETHERENGINE_SDF_MODEL_H
