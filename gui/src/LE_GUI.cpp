#include <LE_GUI.h>

LightEngineUI::UIWindow::UIWindow(std::string name) : name_(std::move(name)) {
	flags_ = ImGuiWindowFlags_NoCollapse;
}

LightEngineUI::MaterialEditor::MaterialEditor() : UIWindow("Material Editor"), material_name_("No material loaded") {}

void LightEngineUI::MaterialEditor::draw() {
	ImGui::Begin(name_.c_str(), nullptr, flags_);

	ImGui::LabelText("", material_name_.c_str());

	ImGui::Separator();

	

	if (default_material_ptr_ != nullptr) {

		bool update_default_material = false;
		bool is_diffuse_map_used = default_parameters_.use_diffuse_map_;
		constexpr int color_edit_flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs;

		if(ImGui::ColorEdit3("Specular color", default_parameters_.specular, color_edit_flags)) 		
			update_default_material = true;
		
		if(ImGui::ColorEdit3("Diffuse color", default_parameters_.diffuse, color_edit_flags)) 		
			update_default_material = true;

		if (is_diffuse_map_used) {
			ImGui::SameLine();
			if (ImGui::Checkbox("Use map", &is_diffuse_map_used)) {
				if (is_diffuse_map_used)
					default_parameters_.use_diffuse_map_ = TRUE;
				else
					default_parameters_.use_diffuse_map_ = FALSE;
				update_default_material = true;
			}
		}
		
		
		if (ImGui::Button("Load diffuse map",ImVec2(ImGui::GetWindowWidth(),20))) {
			
		}

		if(ImGui::ColorEdit3("Ambient color", default_parameters_.ambient, color_edit_flags)) 		
			update_default_material = true;


		if(update_default_material){
			default_material_ptr_->set_all_parameters(default_parameters_);
			default_material_ptr_->update();
		}
	}
	

	ImGui::End();
}

void LightEngineUI::MaterialEditor::load_material(std::shared_ptr<LightEngine::DefaultMaterial> &default_material_ptr) {
	default_material_ptr_ = default_material_ptr;
	default_parameters_ = default_material_ptr_->get_all_parameters();
	material_name_ = default_material_ptr_->get_name();
	pbr_material_ptr_ = nullptr;
	pbr_parameters_ = {};
}
