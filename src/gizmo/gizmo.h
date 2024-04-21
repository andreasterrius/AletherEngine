//#ifndef HELLO_C_GIZMO_H
//#define HELLO_C_GIZMO_H
//
//#include<glm/glm.hpp>
//#include<memory>
//#include<vector>
//
//using namespace glm;
//
//namespace ale {
//
//typedef enum AlGizmo_ModelType {
//    ArrowX, ArrowY, ArrowZ,
//    PlaneXY, PlaneXZ, PlaneYZ,
//    RotationXY, RotationXZ, RotationYZ
//} AlGizmo_ModelType;
//
//typedef enum AlGizmo_ActiveAxis {
//    X, Y, Z, XY, XZ, YZ
//} AlGizmo_ActiveAxis;
//
//typedef enum AleGizmo_Type {
//    Translate, Scale, Rotate
//} AlGizmo_Type;
//
//static const float AlGizmo_MaximumDistanceScale = 1000.0f;
//
//typedef struct AlGizmo_InitialClickInfo {
//    bool exist;
//    AlGizmo_ActiveAxis activeAxis;
//    vec3 position;
//    // This is the position of the first initial click (only returns value for activeAxis)
//    vec3 firstRayPlaneHitPos;
//    vec3 initialSelfPos;
//    // This is the position of last frame ray-plane intersection (only returns value for activeAxis)
//    vec3 lastFrameRayPlaneHitPos;
//} AlGizmo_InitialClickInfo;
//
//typedef struct AlGizmo_GrabAxis {
//    RayCollision rayCollision;
//    AlGizmo_ActiveAxis activeAxis;
//} AlGizmo_GrabAxis;
//
//class AlGizmo {
//#define MODELS_LEN 9
//    // Meshes
//    std::vector<RlModel> models;
//
//    // State
//    bool isHidden;
//    vec3 position;
//    float scale;
//
//    // if not null, then user has selected one of the axis
//    AlGizmo_InitialClickInfo initialClickInfo;
//
//public:
//    AlGizmo_Type gizmoType;
//
//    AlGizmo();
//
//    /// This usually happens when user press and holding left click (handled by caller)
//    /// This function is paired with release()
//    /// Return bool if it's holding something
//    bool tryHold(Transform *transform, vec2 mousePos, Camera3D camera);
//
//    void release();
//
//    void render();
//
//    void hide();
//
//private:
//    void scaleAll();
//
//    AlGizmo_GrabAxis grabAxis(Ray ray);
//
//    /// This returns the hit point position of ONLY the activeAxis (all else will be 0)
//    RayCollision rayPlaneIntersection(Ray ray, AlGizmo_ActiveAxis activeAxis, Vector3 planeCoord);
//
//    /// This will return the new position (not the delta)
//    /// I want it to snap to the where the mouse ray is intersecting the activeAxis
//    Vector3 handleTranslate(AlGizmo_ActiveAxis activeAxis, Vector3 rayPlaneHitPoint);
//
//    /// Unlike translation, this will return the delta instead
//    //static void alGizmoGetRayplaneHitPosDelta(AleGizmo *self, AleGizmo_ActiveAxis activeAxis, Vector3 rayPlaneHitPoint, Vector3 );
//    //
//    //static void alGizmoHandleRotate() {
//    ////    if (activeAxis == XY) {
//    ////        initialRayPos.x =
//    ////                self->initialClickInfo.firstRayPlaneHitPos.x - self->initialClickInfo.initialSelfPos.x;
//    ////        initialRayPos.y =
//    ////                self->initialClickInfo.firstRayPlaneHitPos.y - self->initialClickInfo.initialSelfPos.y;
//    ////    } else if (activeAxis == YZ) {
//    ////        initialRayPos.z =
//    ////                self->initialClickInfo.firstRayPlaneHitPos.z - self->initialClickInfo.initialSelfPos.z;
//    ////        initialRayPos.y =
//    ////                self->initialClickInfo.firstRayPlaneHitPos.y - self->initialClickInfo.initialSelfPos.y;
//    ////    } else if (activeAxis == XZ) {
//    ////        initialRayPos.x =
//    ////                self->initialClickInfo.firstRayPlaneHitPos.x - self->initialClickInfo.initialSelfPos.x;
//    ////        initialRayPos.z =
//    ////                self->initialClickInfo.firstRayPlaneHitPos.z - self->initialClickInfo.initialSelfPos.z;
//    ////    }
//    //}
//};
//}
//
//#endif //HELLO_C_GIZMO_H
