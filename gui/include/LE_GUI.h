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
	public:
		MaterialEditor();
		void draw() override;
		void load_material(std::shared_ptr<LightEngine::DefaultMaterial> &default_material_ptr);
		void load_material(std::shared_ptr<LightEngine::PBRMaterial> &pbr_material_ptr);
	};

}