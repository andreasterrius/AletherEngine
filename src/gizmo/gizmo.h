#ifndef HELLO_C_GIZMO_H
#define HELLO_C_GIZMO_H

#include<glm/glm.hpp>
#include<memory>
#include<vector>
#include<optional>
#include"../data/shader.h"
#include"../data/transform.h"

using namespace std;
using namespace glm;

namespace ale {

class Model;
class Camera;
class Ray;

typedef enum Gizmo_ModelType {
    ArrowX, ArrowY, ArrowZ,
    PlaneXY, PlaneXZ, PlaneYZ,
    RotationXY, RotationXZ, RotationYZ
} Gizmo_ModelType;

typedef enum Gizmo_ActiveAxis {
    X, Y, Z, XY, XZ, YZ
} Gizmo_ActiveAxis;

typedef enum Gizmo_Type {
    Translate, Scale, Rotate
} Gizmo_Type;

static const float Gizmo_MaximumDistanceScale = 1000.0f;

typedef struct Gizmo_InitialClickInfo {
    bool exist;
    Gizmo_ActiveAxis activeAxis;
    vec3 position;
    // This is the position of the first initial click (only returns value for activeAxis)
    vec3 firstRayPlaneHitPos;
    vec3 initialSelfPos;
    // This is the position of last frame ray-plane intersection (only returns value for activeAxis)
    vec3 lastFrameRayPlaneHitPos;
} Gizmo_InitialClickInfo;

typedef struct Gizmo_GrabAxis {
    vec3 rayCollisionPosition;
    Gizmo_ActiveAxis activeAxis;
} Gizmo_GrabAxis;

class Gizmo {
#define MODELS_LEN 9
    // Meshes
    std::vector<Model> models;

    // State
    bool isHidden;
//    vec3 position;
//    float scale;
    Transform transform;

    // if not null, then user has selected one of the axis
    Gizmo_InitialClickInfo initialClickInfo;

    Shader basicColorShader;

public:
    Gizmo_Type gizmoType;

    Gizmo();

    /// This usually happens when user press and holding left click (handled by caller)
    /// This function is paired with release()
    /// Return bool if it's holding something
    bool tryHold(Transform *transform, Ray mouseRay, Camera camera);

    void release();

    void render(Camera camera, vec3 lightPos, vec2 screenSize);

    void hide();

private:
    void scaleAll();

    optional<Gizmo_GrabAxis> grabAxis(Ray ray);

    /// This returns the hit point position of ONLY the activeAxis (all else will be 0)
    optional<vec3> rayPlaneIntersection(Ray ray, Gizmo_ActiveAxis activeAxis, vec3 planeCoord);

    /// This will return the new position (not the delta)
    /// I want it to snap to the where the mouse ray is intersecting the activeAxis
    vec3 handleTranslate(Gizmo_ActiveAxis activeAxis, vec3 rayPlaneHitPoint);

    /// Unlike translation, this will return the delta instead
    //static void GizmoGetRayplaneHitPosDelta(AleGizmo *self, AleGizmo_ActiveAxis activeAxis, Vector3 rayPlaneHitPoint, Vector3 );
    //
    //static void GizmoHandleRotate() {
    ////    if (activeAxis == XY) {
    ////        initialRayPos.x =
    ////                self->initialClickInfo.firstRayPlaneHitPos.x - self->initialClickInfo.initialSelfPos.x;
    ////        initialRayPos.y =
    ////                self->initialClickInfo.firstRayPlaneHitPos.y - self->initialClickInfo.initialSelfPos.y;
    ////    } else if (activeAxis == YZ) {
    ////        initialRayPos.z =
    ////                self->initialClickInfo.firstRayPlaneHitPos.z - self->initialClickInfo.initialSelfPos.z;
    ////        initialRayPos.y =
    ////                self->initialClickInfo.firstRayPlaneHitPos.y - self->initialClickInfo.initialSelfPos.y;
    ////    } else if (activeAxis == XZ) {
    ////        initialRayPos.x =
    ////                self->initialClickInfo.firstRayPlaneHitPos.x - self->initialClickInfo.initialSelfPos.x;
    ////        initialRayPos.z =
    ////                self->initialClickInfo.firstRayPlaneHitPos.z - self->initialClickInfo.initialSelfPos.z;
    ////    }
    //}
};
}

#endif //HELLO_C_GIZMO_H
