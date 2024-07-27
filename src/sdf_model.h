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
#include"data/shader.h"
#include"data/ray.h"
#include<utility>

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

    optional<Texture3D> texture3D;

public:
    //TODO: move to private, public is for debugging
    vector<vector<vector<float>>> distances;
    vector<vector<vector<vec3>>> positions; // Only used for debugging
    vector<pair<vec3, vec3>> faceNormals; // Only used for debugging

    vec3 facePoint;
    vector<tuple<vec3,vec3, vec3>> isectPoints; //debugs

    BoundingBox outerBB; //bb for sdf
    BoundingBox bb; //mesh bb

    SdfModel(Model &model, int cubeCount = 16);

    // returns small cubes that creates the sdf
    void loopOverCubes(function<void(int, int, int, BoundingBox)> func);

    void bindToShader(Shader shader);

    void writeToFile(string path);

    bool findHitPositions(Ray debugRay, vector<vec3> *debugHitPos);
};

}


#endif //ALETHERENGINE_SDF_MODEL_H
