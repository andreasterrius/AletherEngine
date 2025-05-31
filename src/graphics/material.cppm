module;
#include <glm/glm.hpp>
#include <memory>
#include <src/graphics/texture.h>

export module material;

export namespace ale {
struct BasicMaterial {
public:
  glm::vec3 diffuse_color = glm::vec3(1.0f);
  float specular_color = 1.0f;
  std::shared_ptr<Texture> diffuse_texture = nullptr;
  std::shared_ptr<Texture> specular_texture = nullptr;
  // std::shared_ptr<Texture> roughness_texture;
  // std::shared_ptr<Texture> metalness_texture;
  // std::shared_ptr<Texture> normal_texture;
  // std::shared_ptr<Texture> ao_texture;

  void add_diffuse(std::shared_ptr<Texture> texture) {
    this->diffuse_texture = texture;
    this->diffuse_color = glm::vec3(0.0f);
  }

  void add_specular(std::shared_ptr<Texture> texture) {
    this->specular_texture = texture;
    this->specular_color = 0.0f;
  }
};

struct PBRMaterial {
public:
  glm::vec3 albedo = glm::vec3(1.0f);
  std::shared_ptr<Texture> albedo_texture = nullptr;

  glm::vec3 normal = glm::vec3(0.0f);
  std::shared_ptr<Texture> normal_texture = nullptr;

  float metallic = 0.0f;
  std::shared_ptr<Texture> metallic_texture = nullptr;

  float roughness = 0.0f;
  std::shared_ptr<Texture> roughness_texture = nullptr;

  float ao = 0.0f;
  std::shared_ptr<Texture> ao_texture = nullptr;
};

}; // namespace ale
