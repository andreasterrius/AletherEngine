//
// Created by alether on 9/26/23.
//
#include "gizmo.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "src/data/file_system.h"
#include "src/data/transform.h"
#include "src/graphics/camera.h"
#include "src/graphics/model.h"
#include "src/graphics/ray.h"
#include "src/graphics/static_mesh.h"

using afs = ale::FileSystem;
using namespace std;
using namespace glm;

namespace ale {
ale::Gizmo::Gizmo() :
    gizmo_shader(afs::root("resources/shaders/gizmo/gizmo.vs").c_str(),
                 afs::root("resources/shaders/gizmo/gizmo.fs").c_str()),
    isLocalSpace(false) {

  // load a flat shader here ?
  // the default raylib shader is flat though
  this->initial_click_info = Gizmo_InitialClickInfo{0};
  this->transform = Transform{
      .translation = vec3(),
      .scale = vec3(1.0),
  };
  this->gizmoType = Translate;
  this->is_hidden = true;

  this->models.reserve(MODELS_LEN);
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Arrow_X+.glb")); // ArrowX
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Arrow_Y+.glb")); // ArrowY
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Arrow_Z+.glb")); // ArrowY
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Plane_XY.glb")); // PlaneXY
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Plane_XZ.glb")); // PlaneXZ
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Plane_YZ.glb")); // PlaneYZ
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Ring_XY.glb")); // Ring_XY
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Ring_XZ.glb")); // Ring_XZ
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Ring_YZ.glb")); // Ring_YZ
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Scale_X+.glb")); // ArrowX
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Scale_Y+.glb")); // ArrowY
  this->models.emplace_back(
      afs::root("resources/models/gizmo/Scale_Z+.glb")); // ArrowY
  this->models.emplace_back(afs::root("resources/models/gizmo/Scale_All.glb"));
}

bool Gizmo::try_hold(Transform *transform, Camera &camera, vec2 mousePos,
                     Ray ray) {

  // There's an active selected object
  // Show & set gizmo position properly
  this->is_hidden = false;
  this->transform = Transform{}; // reset transform;
  this->transform.translation = transform->translation;

  // TODO: This should not be here, this should be in tick(), but

  // Try to check which arrow we're hitting
  //	Ray ray = GetMouseRay(mousePos, camera);

  if (!this->initial_click_info.exist) {
    // This is initial click on arrow/plane in gizmo, let's save the initial
    // clickInfo
    optional<Gizmo_GrabAxis> grabAxisOpt = this->grab_axis(ray);
    if (!grabAxisOpt.has_value()) { // no arrows were clicked
      return false;
    }
    auto grabAxis = grabAxisOpt.value();

    // Get a ray to plane intersection.
    optional<vec3> rayPlaneHit = this->ray_plane_intersection(
        ray, grabAxis.activeAxis, this->transform.translation, camera);

    if (!rayPlaneHit.has_value()) { // no planes were clicked
      return false;
    }

    this->initial_transform = this->last_transform = *transform;
    this->initial_click_info = Gizmo_InitialClickInfo{
        .exist = true,
        .activeAxis = grabAxis.activeAxis,
        .position = grabAxis.rayCollisionPosition,
        .firstRayPlaneHitPos = rayPlaneHit.value(),
        .initialSelfPos = this->transform.translation,
        .lastFrameRayPlaneHitPos = rayPlaneHit.value(),
        .initialMousePos = mousePos,
        .lastFrameMousePos = mousePos,
    };
    return true;
  }

  // This is no longer initial hit, but is a dragging movement
  optional<vec3> rayPlaneHit =
      this->ray_plane_intersection(ray, this->initial_click_info.activeAxis,
                                   this->transform.translation, camera);

  // Ignore ray-plane parallel cases
  if (rayPlaneHit.has_value()) {
    if (this->gizmoType == Translate) {
      vec3 newPos = this->handle_translate(this->initial_click_info.activeAxis,
                                           rayPlaneHit.value());
      transform->translation = newPos;
    } else if (this->gizmoType == Scale) {
      if (this->initial_click_info.activeAxis) {
        // uniform scale
        float delta =
            (this->initial_click_info.lastFrameMousePos.y - mousePos.y) / 10.0f;
        transform->scale = transform->scale + vec3(delta);
      } else {
        // unlike translate, we just return delta here
        vec3 delta = rayPlaneHit.value() -
                     this->initial_click_info.lastFrameRayPlaneHitPos;
        transform->scale = transform->scale + delta;
      }
    } else if (this->gizmoType == Rotate) {
      vec3 unitVecA =
          normalize(rayPlaneHit.value() - this->transform.translation);
      vec3 unitVecB =
          normalize(this->initial_click_info.lastFrameRayPlaneHitPos -
                    this->transform.translation);
      quat rot = glm::rotation(unitVecB, unitVecA);
      transform->rotation = rot * transform->rotation;
    }
    this->initial_click_info.lastFrameRayPlaneHitPos = rayPlaneHit.value();
    last_transform = *transform;
  }
  this->initial_click_info.lastFrameMousePos = mousePos;

  return true;
}

std::optional<std::tuple<entt::entity, Transform, Transform>>
Gizmo::handle_release() {
  this->initial_click_info = Gizmo_InitialClickInfo{0};
  this->is_dragging = false;

  if (this->last_moved_entity.has_value()) {
    auto moved =
        make_tuple(*last_moved_entity, initial_transform, last_transform);
    ;
    last_moved_entity = nullopt;
    return moved;
  }
  return std::nullopt;
}

optional<Gizmo_GrabAxis> Gizmo::grab_axis(Ray ray) {

  auto tray = ray.apply_transform_inversed(transform);
  if (this->gizmoType == Translate) {
    auto coll = tray.intersect(this->models[ArrowX].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = X};
    }
    coll = tray.intersect(this->models[ArrowY].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = Y};
    }
    coll = tray.intersect(this->models[ArrowZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = Z};
    }
    coll = tray.intersect(this->models[PlaneYZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = YZ};
    }
    coll = tray.intersect(this->models[PlaneXZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = XZ};
    }
    coll = tray.intersect(this->models[PlaneXY].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = XY};
    }
  } else if (this->gizmoType == Scale) {
    auto coll = tray.intersect(this->models[ScaleX].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = X};
    }
    coll = tray.intersect(this->models[ScaleY].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = Y};
    }
    coll = tray.intersect(this->models[ScaleZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = Z};
    }
    coll = tray.intersect(this->models[PlaneYZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = YZ};
    }
    coll = tray.intersect(this->models[PlaneXZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = XZ};
    }
    coll = tray.intersect(this->models[PlaneXY].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = XY};
    }
    coll = tray.intersect(this->models[ScaleAll].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = All};
    }
  } else if (this->gizmoType == Rotate) {
    auto coll = tray.intersect(this->models[RotationYZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = YZ};
    }
    coll = tray.intersect(this->models[RotationXZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = XZ};
    }
    coll = tray.intersect(this->models[RotationXY].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolve(coll.value()),
                            .activeAxis = XY};
    }
  }

  return nullopt;
}

optional<vec3> Gizmo::ray_plane_intersection(Ray ray,
                                             Gizmo_ActiveAxis activeAxis,
                                             vec3 planeCoord, Camera &camera) {

  vec3 activeAxisDir;
  float t = 0.0f;
  if (activeAxis == X || activeAxis == XZ) {
    activeAxisDir = vec3{1.0f, 0.0f, 0.0f};
    t = (planeCoord.y - ray.origin.y) / ray.dir.y;
  } else if (activeAxis == Y || activeAxis == XY) {
    activeAxisDir = vec3{0.0f, 1.0f, 0.0f};
    t = (planeCoord.z - ray.origin.z) / ray.dir.z;
  } else if (activeAxis == Z || activeAxis == YZ) {
    activeAxisDir = vec3{0.0f, 0.0f, 1.0f};
    t = (planeCoord.x - ray.origin.x) / ray.dir.x;
  } else if (activeAxis == All) {
    activeAxisDir = vec3{0.0f, 0.0f, 0.0f};
    t = distance(planeCoord, ray.origin);
  } else {
    return nullopt;
  }
  float isNearParallel = dot(ray.dir, activeAxisDir);

  if (isNearParallel < -0.99 || isNearParallel > 0.99) {
    return nullopt;
  }

  vec3 intersectionCoord = planeCoord;
  if (activeAxis == X) {
    intersectionCoord.x = ray.origin.x + t * ray.dir.x;
  } else if (activeAxis == Y) {
    intersectionCoord.y = ray.origin.y + t * ray.dir.y;
  } else if (activeAxis == Z) {
    intersectionCoord.z = ray.origin.z + t * ray.dir.z;
  } else if (activeAxis == XY) {
    intersectionCoord.x = ray.origin.x + t * ray.dir.x;
    intersectionCoord.y = ray.origin.y + t * ray.dir.y;
  } else if (activeAxis == YZ) {
    intersectionCoord.z = ray.origin.z + t * ray.dir.z;
    intersectionCoord.y = ray.origin.y + t * ray.dir.y;
  } else if (activeAxis == XZ) {
    intersectionCoord.x = ray.origin.x + t * ray.dir.x;
    intersectionCoord.z = ray.origin.z + t * ray.dir.z;
  } else if (activeAxis == All) {
    intersectionCoord = ray.origin + t * ray.dir;
  }

  return make_optional(intersectionCoord);
}

void Gizmo::render(Camera camera, vec3 lightPos) {
  if (this->is_hidden) {
    return;
  }

  gizmo_shader.use();
  gizmo_shader.setMat4("model", transform.get_model_matrix());
  gizmo_shader.setMat4("view", camera.get_view_matrix());
  gizmo_shader.setMat4("projection", camera.get_projection_matrix());
  gizmo_shader.setVec3("lightPos", lightPos);
  gizmo_shader.setVec3("viewPos", camera.Position);

  if (this->gizmoType == Translate) {
    gizmo_shader.setVec3("color", GREEN_X);
    this->models[ArrowX].draw(gizmo_shader);
    this->models[PlaneYZ].draw(gizmo_shader);

    gizmo_shader.setVec3("color", BLUE_Y);
    this->models[ArrowY].draw(gizmo_shader);
    this->models[PlaneXZ].draw(gizmo_shader);

    gizmo_shader.setVec3("color", RED_Z);
    this->models[ArrowZ].draw(gizmo_shader);
    this->models[PlaneXY].draw(gizmo_shader);
  } else if (this->gizmoType == Scale) {
    gizmo_shader.setVec3("color", GREEN_X);
    this->models[ScaleX].draw(gizmo_shader);
    this->models[PlaneYZ].draw(gizmo_shader);

    gizmo_shader.setVec3("color", BLUE_Y);
    this->models[ScaleY].draw(gizmo_shader);
    this->models[PlaneXZ].draw(gizmo_shader);

    gizmo_shader.setVec3("color", RED_Z);
    this->models[ScaleZ].draw(gizmo_shader);
    this->models[PlaneXY].draw(gizmo_shader);

    gizmo_shader.setVec3("color", YELLOW_ALL);
    this->models[ScaleAll].draw(gizmo_shader);

  } else if (this->gizmoType == Rotate) {
    gizmo_shader.setVec3("color", RED_Z);
    this->models[RotationXY].draw(gizmo_shader);

    gizmo_shader.setVec3("color", BLUE_Y);
    this->models[RotationXZ].draw(gizmo_shader);

    gizmo_shader.setVec3("color", GREEN_X);
    this->models[RotationYZ].draw(gizmo_shader);
  }
}

void Gizmo::change_mode(Gizmo_Type gizmoType) { this->gizmoType = gizmoType; }

void Gizmo::change_space(bool isLocalSpace) {
  this->isLocalSpace = isLocalSpace;
}

vec3 Gizmo::handle_translate(Gizmo_ActiveAxis activeAxis,
                             vec3 rayPlaneHitPoint) {
  vec3 initialRayPos = vec3();
  if (activeAxis == X) {
    initialRayPos.x = this->initial_click_info.firstRayPlaneHitPos.x -
                      this->initial_click_info.initialSelfPos.x;
  } else if (activeAxis == Y) {
    initialRayPos.y = this->initial_click_info.firstRayPlaneHitPos.y -
                      this->initial_click_info.initialSelfPos.y;
  } else if (activeAxis == Z) {
    initialRayPos.z = this->initial_click_info.firstRayPlaneHitPos.z -
                      this->initial_click_info.initialSelfPos.z;
  } else if (activeAxis == XY) {
    initialRayPos.x = this->initial_click_info.firstRayPlaneHitPos.x -
                      this->initial_click_info.initialSelfPos.x;
    initialRayPos.y = this->initial_click_info.firstRayPlaneHitPos.y -
                      this->initial_click_info.initialSelfPos.y;
  } else if (activeAxis == YZ) {
    initialRayPos.z = this->initial_click_info.firstRayPlaneHitPos.z -
                      this->initial_click_info.initialSelfPos.z;
    initialRayPos.y = this->initial_click_info.firstRayPlaneHitPos.y -
                      this->initial_click_info.initialSelfPos.y;
  } else if (activeAxis == XZ) {
    initialRayPos.x = this->initial_click_info.firstRayPlaneHitPos.x -
                      this->initial_click_info.initialSelfPos.x;
    initialRayPos.z = this->initial_click_info.firstRayPlaneHitPos.z -
                      this->initial_click_info.initialSelfPos.z;
  }

  return rayPlaneHitPoint - initialRayPos;
}

void Gizmo::show(Transform transform) {
  this->is_hidden = false;
  this->transform.translation = transform.translation;
}

void Gizmo::hide() { this->is_hidden = true; }

void Gizmo::tick(const Ray &mouse_ray, Camera &camera, vec2 mouse_pos,
                 entt::registry &world) {

  if (selected_entity) {
    auto transform = world.try_get<Transform>(*selected_entity);
    if (transform != nullptr) {
      if (is_dragging) {
        try_hold(transform, camera, mouse_pos, mouse_ray);
        last_moved_entity = selected_entity;
      }

      auto scale = glm::clamp(
          glm::distance(camera.Position, this->transform.translation) / 30.0f,
          0.1f, 3.0f);
      this->transform.scale = glm::vec3(scale);
    }
  }
}

bool Gizmo::handle_press(Ray &mouse_ray, Camera &camera, vec2 mouse_pos,
                         entt::registry &world) {

  // are we clicking the arrow/plane of the gizmo?
  if (selected_entity.has_value()) {
    // means gizmo is showing
    auto transform = world.get<Transform>(*selected_entity);
    if (try_hold(&transform, camera, mouse_pos, mouse_ray)) {
      is_dragging = true;
      // don't propagate this click
      return false;
    }
  }

  // Find some object to click into.
  selected_entity = nullopt;
  auto view = world.view<Transform, StaticMesh>();
  float dist = INFINITY;
  for (auto [entity, obj_transform, static_mesh]: view.each()) {
    auto ray = mouse_ray.apply_transform_inversed(obj_transform);
    for (auto &mesh: static_mesh.get_model()->meshes) {
      auto isect_t = ray.intersect(mesh.boundingBox);
      if (isect_t.has_value() && isect_t < dist) {
        selected_entity = entity;
        dist = *isect_t;
      }
    }
  }

  // nothing is clicked
  if (!selected_entity.has_value()) {
    // Clicks nothing, hide the gizmo
    selected_entity = nullopt;
    hide();
    return true; // propagate click
  }

  // something is selected
  auto &transform = world.get<Transform>(*selected_entity);
  show(transform);

  return false; // don't propagate click
}
} // namespace ale
