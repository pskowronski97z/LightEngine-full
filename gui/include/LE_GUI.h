#pragma once
#include <string>
#include <memory>
#include <ImGui/imgui.h>
#include <LEMaterial.h>

namespace LightEngineUI {

	class UIWindow {
	protected:
		std::string name_;
		ImGuiWindowFlags flags_;
	public:
		UIWindow(std::string name);
		virtual void draw() = 0;
	};

	class MaterialEditor : public UIWindow {
	private:
		std::shared_ptr<LightEngine::DefaultMaterial> default_material_ptr_ = nullptr;
		std::shared_ptr<LightEngine::PBRMaterial> pbr_material_ptr_ = nullptr;
		LightEngine::DefaultParameters default_parameters_;
		LightEngine::PBRParameters pbr_parameters_;
		std::string material_name_;
		bool flip_x_ = false;
		bool flip_y_ = false;
		bool update_default_material_ = false;
		bool update_pbr_material_ = false;
		ImVec2 button_size_;
		std::vector<std::string> button_labels_;
	public:
		MaterialEditor();
		void draw() override;
		void load_material(std::shared_ptr<LightEngine::DefaultMaterial> &default_material_ptr);
		void load_material(std::shared_ptr<LightEngine::PBRMaterial> &pbr_material_ptr);
	};

}