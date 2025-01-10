//
// Created by alether on 9/26/23.
//
#include "gizmo.h"
#include "../camera.h"
#include "../data/model.h"
#include "../data/ray.h"
#include "../data/transform.h"
#include "../file_system.h"
#include <glm/glm.hpp>

using afs = ale::FileSystem;

ale::Gizmo::Gizmo()
    : basicColorShader(afs::root("src/gizmo/basic_color_shader.vs").c_str(),
                       afs::root("src/gizmo/basic_color_shader.fs").c_str()) {

  // load a flat shader here ?
  // the default raylib shader is flat though
  this->initialClickInfo = Gizmo_InitialClickInfo{0};
  this->transform = Transform{
      .translation = vec3(),
      .scale = vec3(1.0),
  };
  this->gizmoType = Translate;
  this->isHidden = true;

  this->models.reserve(MODELS_LEN);
  this->models.emplace_back(
      afs::root("resources/gizmo/Arrow_X+.glb")); // ArrowX
  this->models.emplace_back(
      afs::root("resources/gizmo/Arrow_Y+.glb")); // ArrowY
  this->models.emplace_back(
      afs::root("resources/gizmo/Arrow_Z+.glb")); // ArrowY
  this->models.emplace_back(
      afs::root("resources/gizmo/Plane_XY.glb")); // PlaneXY
  this->models.emplace_back(
      afs::root("resources/gizmo/Plane_XZ.glb")); // PlaneXZ
  this->models.emplace_back(
      afs::root("resources/gizmo/Plane_YZ.glb")); // PlaneYZ
  this->models.emplace_back(
      afs::root("resources/gizmo/Ring_XY.glb")); // Ring_XY
  this->models.emplace_back(
      afs::root("resources/gizmo/Ring_XZ.glb")); // Ring_XZ
  this->models.emplace_back(
      afs::root("resources/gizmo/Ring_YZ.glb")); // Ring_YZ
}

bool Gizmo::tryHold(Transform *objTransform, Ray ray, Camera camera) {
  // There's no currently active selected object
  if (objTransform == NULL) {
    this->isHidden = true;
    return false;
  }

  // There's an active selected object
  // Show & set gizmo position properly
  this->isHidden = false;
  this->transform.translation = objTransform->translation;

  // TODO: This should not be here, this should be in tick(), but
  // dependentPosition will be weakly owned. Scale the size depending on the
  // size
  float scale = glm::distance(camera.Position, objTransform->translation) /
                Gizmo_MaximumDistanceScale;
  scale = glm::clamp(scale, 0.25f, 1.0f);
  this->transform.scale = vec3(scale);
  this->scaleAll();

  // Try to check which arrow we're hitting
  //	Ray ray = GetMouseRay(mousePos, camera);

  if (!this->initialClickInfo.exist) {
    // This is initial click on arrow/plane in gizmo, let's save the initial
    // clickInfo
    optional<Gizmo_GrabAxis> grabAxisOpt = this->grabAxis(ray);
    if (!grabAxisOpt.has_value()) { // no arrows were clicked
      return false;
    }
    auto grabAxis = grabAxisOpt.value();

    // Get a ray to plane intersection.
    optional<vec3> rayPlaneHit = this->rayPlaneIntersection(
        ray, grabAxis.activeAxis, this->transform.translation);
    if (!rayPlaneHit.has_value()) {
      return false;
    }

    this->initialClickInfo = Gizmo_InitialClickInfo{
        .exist = true,
        .activeAxis = grabAxis.activeAxis,
        .position = grabAxis.rayCollisionPosition,
        .firstRayPlaneHitPos = rayPlaneHit.value(),
        .initialSelfPos = this->transform.translation,
        .lastFrameRayPlaneHitPos = rayPlaneHit.value(),
    };
    return true;
  }

  // This is no longer initial hit, but is a dragging movement
  optional<vec3> rayPlaneHit = this->rayPlaneIntersection(
      ray, this->initialClickInfo.activeAxis, this->transform.translation);

  // Ignore ray-plane parallel cases
  if (rayPlaneHit.has_value()) {
    if (this->gizmoType == Translate) {
      vec3 newPos = this->handleTranslate(this->initialClickInfo.activeAxis,
                                          rayPlaneHit.value());
      this->transform.translation = newPos;
      objTransform->translation = newPos;
    } else if (this->gizmoType == Scale) {
      // unlike translate, we just return delta here
      vec3 delta =
          rayPlaneHit.value() - this->initialClickInfo.lastFrameRayPlaneHitPos;
      objTransform->scale = objTransform->scale + delta;
    } else if (this->gizmoType == Rotate) {
      //			vec3 unitVecA = rayPlaneHit.value() -
      // this->position; 			vec3 unitVecB =
      // this->initialClickInfo.lastFrameRayPlaneHitPos - this->position;
      //
      //			vec4 delta = QuaternionFromvec3Tovec3(unitVecB,
      // unitVecA);
      //			//not commutative, multiply order matters!
      //			// world rotation
      //			transform->rotation = QuaternionMultiply(delta,
      // transform->rotation);
      //			// local rotation :
      // QuaternionMultiply(transform->rotation, delta);
    }
    this->initialClickInfo.lastFrameRayPlaneHitPos = rayPlaneHit.value();
  }

  return true;
}

void Gizmo::release() { this->initialClickInfo = Gizmo_InitialClickInfo{0}; }

void Gizmo::scaleAll() {
  //	this->models[ArrowX].d.transform = MatrixScale(this->scale, this->scale,
  // this->scale); 	this->models[ArrowY].d.transform =
  // MatrixScale(this->scale, this->scale, this->scale);
  // this->models[ArrowZ].d.transform = MatrixScale(this->scale, this->scale,
  // this->scale); 	this->models[PlaneXY].d.transform =
  // MatrixScale(this->scale, this->scale, this->scale);
  // this->models[PlaneXZ].d.transform = MatrixScale(this->scale, this->scale,
  // this->scale);
  //	this->models[PlaneYZ].d.transform = MatrixScale(this->scale,
  // this->scale, this->scale); 	this->models[RotationXY].d.transform =
  // MatrixScale(this->scale, this->scale, this->scale);
  //	this->models[RotationXZ].d.transform = MatrixScale(this->scale,
  // this->scale, this->scale); 	this->models[RotationYZ].d.transform =
  // MatrixScale(this->scale, this->scale, this->scale);
}

optional<Gizmo_GrabAxis> Gizmo::grabAxis(Ray ray) {

  if (this->gizmoType == Translate || this->gizmoType == Scale) {
    auto coll =
        ray.tryIntersect(transform, this->models[ArrowX].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = X};
    }
    coll =
        ray.tryIntersect(transform, this->models[ArrowY].meshes[0].boundingBox);
    ;
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = Y};
    }
    coll =
        ray.tryIntersect(transform, this->models[ArrowZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = Z};
    }
    coll = ray.tryIntersect(transform,
                            this->models[PlaneYZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = YZ};
    }
    coll = ray.tryIntersect(transform,
                            this->models[PlaneXZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = XZ};
    }
    coll = ray.tryIntersect(transform,
                            this->models[PlaneXY].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = XY};
    }
  } else if (this->gizmoType == Rotate) {
    auto coll = ray.tryIntersect(
        transform, this->models[RotationYZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = YZ};
    }
    coll = ray.tryIntersect(transform,
                            this->models[RotationXZ].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = XZ};
    }
    coll = ray.tryIntersect(transform,
                            this->models[RotationXY].meshes[0].boundingBox);
    if (coll.has_value()) {
      return Gizmo_GrabAxis{.rayCollisionPosition = ray.resolveT(coll.value()),
                            .activeAxis = XY};
    }
  }

  return nullopt;
}

optional<vec3> Gizmo::rayPlaneIntersection(Ray ray, Gizmo_ActiveAxis activeAxis,
                                           vec3 planeCoord) {

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
  }

  return make_optional(intersectionCoord);
}

void Gizmo::render(Camera camera, vec3 lightPos, vec2 screenSize) {
  if (this->isHidden) {
    return;
  }

  basicColorShader.use();
  basicColorShader.setMat4("model", transform.getModelMatrix());
  basicColorShader.setMat4("view", camera.GetViewMatrix());
  basicColorShader.setMat4(
      "projection", camera.GetProjectionMatrix(screenSize.x, screenSize.y));
  basicColorShader.setVec3("lightPos", lightPos);
  basicColorShader.setVec3("viewPos", camera.Position);

  // Must be called inside BeginMode3D render
  // TODO: need to set model mat here
  if (this->gizmoType == Translate || this->gizmoType == Scale) {
    // TODO: need to pass in color as well
    this->models[ArrowX].draw(basicColorShader);
    this->models[ArrowY].draw(basicColorShader);
    this->models[ArrowZ].draw(basicColorShader);

    this->models[PlaneXY].draw(basicColorShader);
    this->models[PlaneXZ].draw(basicColorShader);
    this->models[PlaneYZ].draw(basicColorShader);
  } else if (this->gizmoType == Rotate) {
    this->models[RotationXY].draw(basicColorShader);
    this->models[RotationXZ].draw(basicColorShader);
    this->models[RotationYZ].draw(basicColorShader);
  }
}

vec3 Gizmo::handleTranslate(Gizmo_ActiveAxis activeAxis,
                            vec3 rayPlaneHitPoint) {
  vec3 initialRayPos = vec3();
  if (activeAxis == X) {
    initialRayPos.x = this->initialClickInfo.firstRayPlaneHitPos.x -
                      this->initialClickInfo.initialSelfPos.x;
  } else if (activeAxis == Y) {
    initialRayPos.y = this->initialClickInfo.firstRayPlaneHitPos.y -
                      this->initialClickInfo.initialSelfPos.y;
  } else if (activeAxis == Z) {
    initialRayPos.z = this->initialClickInfo.firstRayPlaneHitPos.z -
                      this->initialClickInfo.initialSelfPos.z;
  } else if (activeAxis == XY) {
    initialRayPos.x = this->initialClickInfo.firstRayPlaneHitPos.x -
                      this->initialClickInfo.initialSelfPos.x;
    initialRayPos.y = this->initialClickInfo.firstRayPlaneHitPos.y -
                      this->initialClickInfo.initialSelfPos.y;
  } else if (activeAxis == YZ) {
    initialRayPos.z = this->initialClickInfo.firstRayPlaneHitPos.z -
                      this->initialClickInfo.initialSelfPos.z;
    initialRayPos.y = this->initialClickInfo.firstRayPlaneHitPos.y -
                      this->initialClickInfo.initialSelfPos.y;
  } else if (activeAxis == XZ) {
    initialRayPos.x = this->initialClickInfo.firstRayPlaneHitPos.x -
                      this->initialClickInfo.initialSelfPos.x;
    initialRayPos.z = this->initialClickInfo.firstRayPlaneHitPos.z -
                      this->initialClickInfo.initialSelfPos.z;
  }

  return rayPlaneHitPoint - initialRayPos;
}

void Gizmo::hide() { this->isHidden = true; }
