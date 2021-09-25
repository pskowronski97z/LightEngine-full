#include <LE_GUI.h>

LightEngineUI::UIWindow::UIWindow(std::string name) : name_(std::move(name)) {
	flags_ = ImGuiWindowFlags_NoCollapse;
}

LightEngineUI::MaterialEditor::MaterialEditor() : UIWindow("Material Editor"), material_name_("No material loaded"), button_size_(ImVec2(20,20)) {}

void LightEngineUI::MaterialEditor::draw() {

	ImGui::Begin(name_.c_str(), nullptr, flags_);

	button_size_.x = ImGui::GetWindowWidth() - 15;

	ImGui::BulletText(material_name_.c_str());

	ImGui::Separator();

	if (default_material_ptr_ != nullptr) {

		is_diffuse_map_used_ = default_parameters_.use_diffuse_map_;
		is_normal_map_used_ = default_parameters_.use_normal_map_;
		constexpr int color_edit_flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs;

		if(ImGui::ColorEdit3("Specular color", default_parameters_.specular, color_edit_flags)) 		
			update_default_material_ = true;
		
		if(ImGui::ColorEdit3("Diffuse color", default_parameters_.diffuse, color_edit_flags)) 		
			update_default_material_ = true;

		if (is_diffuse_map_used_) {
			ImGui::SameLine();
			if (ImGui::Checkbox("Use map", &is_diffuse_map_used_)) {
				if (is_diffuse_map_used_)
					default_parameters_.use_diffuse_map_ = TRUE;
				else
					default_parameters_.use_diffuse_map_ = FALSE;
				update_default_material_ = true;
			}
		}
	
		if (ImGui::Button("Diffuse map", button_size_)) {
			// Handle with Asset Manager
		}
		
		if(ImGui::ColorEdit3("Ambient color", default_parameters_.ambient, color_edit_flags)) 		
			update_default_material_ = true;

		ImGui::Separator();
		ImGui::LabelText("","Levels");

		if(ImGui::DragFloat("Specular", &default_parameters_.specular_level_, 0.01, 0.0, 1.0, "%.2f"))
			update_default_material_ = true;

		if(ImGui::DragInt("Glossiness",&default_parameters_.glossiness_,1.0,1,500))
			update_default_material_ = true;

		ImGui::LabelText("","Attenuation");

		if(ImGui::DragFloat("Quadratic", &default_parameters_.quadratic_att_, 0.001, 0, D3D11_FLOAT32_MAX, "%.4f"))
			update_default_material_ = true;

		if(ImGui::DragFloat("Linear", &default_parameters_.linear_att_, 0.001, 0, D3D11_FLOAT32_MAX, "%.4f"))
			update_default_material_ = true;

		if(ImGui::DragFloat("Constant", &default_parameters_.constant_att_, 0.001, 0, D3D11_FLOAT32_MAX, "%.4f"))
			update_default_material_ = true;

		ImGui::Separator();

		

		ImGui::LabelText("", "Normal mapping");
		if (is_normal_map_used_) {
			if (ImGui::Checkbox("Enable", &is_normal_map_used_)) {
				if (is_normal_map_used_)
					default_parameters_.use_normal_map_ = TRUE;
				else
					default_parameters_.use_normal_map_ = FALSE;
				update_default_material_ = true;
			}
		}

		if (ImGui::Button("Normal map", button_size_)) {
			// Handle with Asset Manager
		}

		if (is_normal_map_used_) {
			if (ImGui::Checkbox("Flip X", &flip_x_)) {
				
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Flip Y", &flip_y_)) {
				
			}
		}

		if(update_default_material_){
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
