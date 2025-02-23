//
// Created by Alether on 1/7/2025.
//

#include "imgui_integration.h"

//clang-format off
#include <glad/glad.h>
//clang-format off
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
//clang-format on
#include <iostream>

void ale::ImguiIntegration::start_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ale::ImguiIntegration::end_frame() {
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
}

ale::ImguiIntegration::ImguiIntegration(GLFWwindow *raw, float content_scale) {
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
  ImGui_ImplGlfw_InitForOpenGL(raw, true);
  ImGui_ImplOpenGL3_Init();

  ImGui::GetStyle().ScaleAllSizes(content_scale);
  ImGui::GetIO().FontGlobalScale = content_scale;
}

ale::ImguiIntegration::~ImguiIntegration() {
  if (has_init) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
}