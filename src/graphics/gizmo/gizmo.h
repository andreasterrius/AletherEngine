#ifndef HELLO_C_GIZMO_H
#define HELLO_C_GIZMO_H

#include "src/data/transform.h"
#include "src/graphics/shader.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace ale {

class Model;
class Camera;
class Ray;

typedef enum Gizmo_ModelType {
  ArrowX,
  ArrowY,
  ArrowZ,
  PlaneXY,
  PlaneXZ,
  PlaneYZ,
  RotationXY,
  RotationXZ,
  RotationYZ,
  ScaleX,
  ScaleY,
  ScaleZ,
  ScaleAll,
} Gizmo_ModelType;

typedef enum Gizmo_ActiveAxis { X, Y, Z, XY, XZ, YZ, All } Gizmo_ActiveAxis;

typedef enum Gizmo_Type { Translate, Scale, Rotate } Gizmo_Type;

static const float Gizmo_MaximumDistanceScale = 1000.0f;

typedef struct Gizmo_InitialClickInfo {
  bool exist;
  Gizmo_ActiveAxis activeAxis;
  glm::vec3 position;
  // This is the position of the first initial click (only returns value for
  // activeAxis)
  glm::vec3 firstRayPlaneHitPos;
  glm::vec3 initialSelfPos;
  // This is the position of last frame ray-plane intersection (only returns
  // value for activeAxis)
  glm::vec3 lastFrameRayPlaneHitPos;

  glm::vec2 initialMousePos;
  glm::vec2 lastFrameMousePos;
} Gizmo_InitialClickInfo;

typedef struct Gizmo_GrabAxis {
  glm::vec3 rayCollisionPosition;
  Gizmo_ActiveAxis activeAxis;
} Gizmo_GrabAxis;

class Gizmo {
#define MODELS_LEN 13
public:
  // Meshes
  std::vector<Model> models;

  const glm::vec3 RED_Z = glm::vec3(1.0, 0.0, 0.0);
  const glm::vec3 GREEN_X = glm::vec3(0.0, 1.0, 0.0);
  const glm::vec3 BLUE_Y = glm::vec3(0.0, 0.0, 1.0);
  const glm::vec3 YELLOW_ALL = glm::vec3(1.0, 1.0, 0.0);

  // State
  bool is_hidden;
  bool is_dragging;

  //    vec3 position;
  //    float scale;
  Transform transform;
  std::optional<entt::entity> selected_entity;

  // if exist = true, then user has selected one of the axis
  Gizmo_InitialClickInfo initial_click_info;

  // Remember the first and last transform
  Transform initial_transform;
  Transform last_transform;
  bool just_released; // don't like this but ok for now I guess.

  Shader gizmo_shader;

public:
  Gizmo_Type gizmoType;
  // not supported yet
  bool isLocalSpace;

  Gizmo();

private:
  /// This usually happens when user press and holding left click (handled by
  /// caller) This function is paired with release() Return bool if it's holding
  /// something
  bool try_hold(Transform *transform, Camera &camera, glm::vec2 mousePos,
                Ray mouseRay);

  void show(Transform transform);

  void hide();

public:
  // returns whether the press should be propagated or not
  bool handle_press(Ray &mouse_ray, Camera &camera, glm::vec2 mouse_pos,
                    entt::registry &world);
  void handle_release();
  void tick(const Ray &mouse_ray, Camera &camera, glm::vec2 mouse_pos,
            entt::registry &world);
  void render(Camera camera, glm::vec3 lightPos);
  void change_mode(Gizmo_Type gizmoType);
  void change_space(bool isLocalSpace);
  std::optional<std::tuple<entt::entity, Transform, Transform>>
  get_release_info();

private:
  void scale_all();

  std::optional<Gizmo_GrabAxis> grab_axis(Ray ray);

  /// This returns the hit point position of ONLY the activeAxis (all else will
  /// be 0)
  std::optional<glm::vec3> ray_plane_intersection(Ray ray,
                                                  Gizmo_ActiveAxis activeAxis,
                                                  glm::vec3 planeCoord,
                                                  Camera &camera);

  /// This will return the new position (not the delta)
  /// I want it to snap to the where the mouse ray is intersecting the
  /// activeAxis
  glm::vec3 handle_translate(Gizmo_ActiveAxis activeAxis,
                             glm::vec3 rayPlaneHitPoint);

  /// Unlike translation, this will return the delta instead
  // static void GizmoGetRayplaneHitPosDelta(AleGizmo *self, AleGizmo_ActiveAxis
  // activeAxis, Vector3 rayPlaneHitPoint, Vector3 );
  //
  // static void GizmoHandleRotate() {
  ////    if (activeAxis == XY) {
  ////        initialRayPos.x =
  ////                self->initialClickInfo.firstRayPlaneHitPos.x -
  /// self->initialClickInfo.initialSelfPos.x; /        initialRayPos.y = /
  /// self->initialClickInfo.firstRayPlaneHitPos.y -
  /// self->initialClickInfo.initialSelfPos.y; /    } else if (activeAxis == YZ)
  ///{ /        initialRayPos.z = / self->initialClickInfo.firstRayPlaneHitPos.z
  ///- self->initialClickInfo.initialSelfPos.z; /        initialRayPos.y = /
  /// self->initialClickInfo.firstRayPlaneHitPos.y -
  /// self->initialClickInfo.initialSelfPos.y; /    } else if (activeAxis == XZ)
  ///{ /        initialRayPos.x = / self->initialClickInfo.firstRayPlaneHitPos.x
  ///- self->initialClickInfo.initialSelfPos.x; /        initialRayPos.z = /
  /// self->initialClickInfo.firstRayPlaneHitPos.z -
  /// self->initialClickInfo.initialSelfPos.z; /    }
  //}
};

} // namespace ale

#endif // HELLO_C_GIZMO_H
