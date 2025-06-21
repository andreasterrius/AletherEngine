//
// Created by Alether on 1/7/2025.
//

module;

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <nfd.hpp>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <spdlog/spdlog.h>

export module editor:imgui_integration;
import graphics;
import input;
import data;
import serde;

using namespace ale::graphics;
using namespace ale::input;
using namespace ale::data;
using namespace glm;

export namespace ale::editor {

struct MyImguiStyle {
  float frame_rounding;
  vec2 frame_padding;
  vec2 item_spacing;
  vec2 window_padding;
  float scrollbar_size;

  std::string default_font_path;
};

class ImguiIntegration : WindowEventListener {
  bool has_init = false;

public:
  void start_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void end_frame() {
    GLboolean srgbWasEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_FRAMEBUFFER_SRGB);

    ImGui::Render();
    auto draw_data = ImGui::GetDrawData();
    if (draw_data != nullptr) {
      ImGui_ImplOpenGL3_RenderDrawData(draw_data);
    }

    auto &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();

      // Update and Render additional Platform Windows
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();

      glfwMakeContextCurrent(backup_current_context);
    }

    if (srgbWasEnabled) {
      glEnable(GL_FRAMEBUFFER_SRGB);
    } else {
      glDisable(GL_FRAMEBUFFER_SRGB);
    }
  }

  void load_style() {
    auto style_vec = afs::load(afs::root("resources/editor/style.json"));
    auto style_string = string(style_vec.data(), style_vec.size());

    try {
      auto bb = rfl::json::read<MyImguiStyle>(style_string).value();
    } catch (exception &e) {
      spdlog::error("Error loading style: {}", e.what());
    }

    auto style = rfl::json::read<MyImguiStyle>(style_string).value();

    ImGui::StyleColorsDark();
    auto &s = ImGui::GetStyle();
    s.FrameRounding = style.frame_rounding;
    s.FramePadding = ImVec2(style.frame_padding.x, style.frame_padding.y);
    s.ItemSpacing = ImVec2(style.item_spacing.x, style.item_spacing.y);
    s.WindowPadding = ImVec2(style.window_padding.x, style.window_padding.y);
    s.ScrollbarSize = style.scrollbar_size;

    if (!style.default_font_path.empty()) {
      auto &io = ImGui::GetIO();
      io.FontDefault = io.Fonts->AddFontFromFileTTF(
          afs::root(style.default_font_path).c_str(), 15.0f);
    }
  }

public:
  ImguiIntegration(Window *window, float content_scale) {
    has_init = true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // TODO: IO settings here
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

    // Second param install_callback=true will install
    // GLFW callbacks and chain to existing ones.
    ImGui_ImplGlfw_InitForOpenGL(window->get(), true);
    ImGui_ImplOpenGL3_Init();
    NFD_Init();

    ImGui::GetStyle().ScaleAllSizes(content_scale);
    ImGui::GetIO().FontGlobalScale = content_scale;

    load_style();
  }

  ImguiIntegration(const ImguiIntegration &other) = delete;

  ImguiIntegration(ImguiIntegration &&other) noexcept {
    has_init = other.has_init;
    other.has_init = false;
  }

  ImguiIntegration &operator=(ImguiIntegration other) {
    has_init = other.has_init;
    other.has_init = false;
    return *this;
  }

  ~ImguiIntegration() {
    if (has_init) {
      NFD_Quit();
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
    }
  }

public:
  void cursor_pos_callback(double xpos, double ypos, double xoffset,
                           double yoffset) override {}
  void framebuffer_size_callback(int width, int height) override {}
  void key_callback(int key, int scancode, int action, int mods) override {
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
      // load from file
      // afs::load(afs::root(""));
    }
  }
  void mouse_button_callback(int button, int action, int mods) override {}
  void scroll_callback(double x_offset, double y_offset) override {}
};
} // namespace ale::editor
