
#ifndef ALETHERENGINE_STATIC_MESH_H
#define ALETHERENGINE_STATIC_MESH_H

#include <memory>

#include "../sdf_model.h"
#include "../sdf_model_packed.h"
#include "model.h"

namespace ale {
class StaticMesh {
 private:
  std::shared_ptr<Model> model;
  std::shared_ptr<SdfModel> sdf_model;
  std::shared_ptr<SdfModelPacked> sdf_model_packed;

 public:
  StaticMesh(std::shared_ptr<Model> model, std::shared_ptr<SdfModel> sdf_model,
             std::shared_ptr<SdfModelPacked> sdf_model_packed);

  std::shared_ptr<Model> &get_model();
};
};  // namespace ale

#endif