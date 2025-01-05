
#ifndef ALETHERENGINE_STATIC_MESH_H
#define ALETHERENGINE_STATIC_MESH_H

#include <memory>

#include "../sdf_model.h"
#include "../sdf_model_packed.h"
#include "model.h"

using namespace std;
using ShadowEntry = pair<shared_ptr<SdfModelPacked>, unsigned int>;

namespace ale {
class StaticMesh {
 private:
  shared_ptr<Model> model;
  optional<ShadowEntry> sdf_shadow;

 public:
  StaticMesh(shared_ptr<Model> model, optional<ShadowEntry> sdf_shadow);

  shared_ptr<Model> get_model();
  optional<ShadowEntry> &get_sdf_shadow();
};
};  // namespace ale

#endif