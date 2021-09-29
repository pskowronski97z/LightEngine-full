#include <LE_GUI.h>

constexpr int color_edit_flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs;

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

		if(ImGui::ColorEdit3("Specular color", default_parameters_.specular, color_edit_flags)) {
			default_material_ptr_->set_specular_color(default_parameters_.specular);
			update_default_material_ = true;
		}	
					
		if(ImGui::ColorEdit3("Diffuse color", default_parameters_.diffuse, color_edit_flags)) { 
			default_material_ptr_->set_diffuse_color(default_parameters_.diffuse);
			update_default_material_ = true;
		
		}
			
		if(ImGui::ColorEdit3("Ambient color", default_parameters_.ambient, color_edit_flags)) {
			default_material_ptr_->set_ambient_color(default_parameters_.ambient);
			update_default_material_ = true;
		}
			

		ImGui::Separator();
		ImGui::LabelText("","Levels");

		if(ImGui::SliderFloat("Specular", &default_parameters_.specular_level_, 0.0, 1.0, "%.2f")) {
			default_material_ptr_->set_specular_level(default_parameters_.specular_level_);
			update_default_material_ = true;
		}
			
		if(ImGui::SliderInt("Glossiness",&default_parameters_.glossiness_,1.0,500)) {
			default_material_ptr_->set_glossiness(default_parameters_.glossiness_);
			update_default_material_ = true;
		}

		if (ImGui::CollapsingHeader("Attenuation")) {

			if (ImGui::DragFloat("Quadratic", &default_parameters_.quadratic_att_, 0.01, 0, D3D11_FLOAT32_MAX, "%.2f")) {
				default_material_ptr_->set_quadratic_att(default_parameters_.quadratic_att_);
				update_default_material_ = true;
			}
				
			if (ImGui::DragFloat("Linear", &default_parameters_.linear_att_, 0.01, 0, D3D11_FLOAT32_MAX, "%.2f")) {
				default_material_ptr_->set_linear_att(default_parameters_.linear_att_);
				update_default_material_ = true;
			}
				
			if (ImGui::DragFloat("Constant", &default_parameters_.constant_att_, 0.01, 0, D3D11_FLOAT32_MAX, "%.2f")){
				default_material_ptr_->set_constant_att(default_parameters_.constant_att_);
				update_default_material_ = true;
			}
				
		}

		ImGui::Separator();

		ImGui::LabelText("", "Diffuse map");

		//TODO: Add functionality 
		if (ImGui::Button(button_labels_.at(0).c_str(), button_size_)) {
			// Handle with Asset Manager
		}

		ImGui::LabelText("", "Normal map");	

		//TODO: Add functionality 
		if (ImGui::Button(button_labels_.at(1).c_str(), button_size_)) {
			// Handle with Asset Manager
		}

		if (default_parameters_.use_normal_map_) {
			if (ImGui::Checkbox("Flip X", &flip_x_)) {
				//if(flip_x_)
					//flip tangent
				//else
					//unflip tangent
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("Flip Y", &flip_y_)) {
				//if(flip_x_)
					//flip bitangent
				//else
					//unflip bitangent
			}
		}

		if(update_default_material_) {
			default_material_ptr_->update();
			update_default_material_ = false;
		}
			
		
	}
	
	if (pbr_material_ptr_ != nullptr) {

		if(ImGui::ColorEdit3("Albedo", pbr_parameters_.albedo_, color_edit_flags)) {
			pbr_material_ptr_->set_albedo(pbr_parameters_.albedo_);
			update_pbr_material_ = true;
		}
			
		
		if (ImGui::SliderFloat("Roughness", &pbr_parameters_.roughness_, 0.01f, 1.0f, "%.2f")) {
			pbr_material_ptr_->set_roughness(pbr_parameters_.roughness_);
			update_pbr_material_ = true;
		}

		if (ImGui::SliderFloat("Metallic", &pbr_parameters_.metalness_, 0.0f, 1.0f, "%.2f")) {
			pbr_material_ptr_->set_metalness(pbr_parameters_.metalness_);
			update_pbr_material_ = true;
		}

		ImGui::Separator();

		ImGui::LabelText("", "Albedo map");
		ImGui::Button(button_labels_.at(0).c_str(),button_size_);

		ImGui::LabelText("", "Roughness map");
		ImGui::Button(button_labels_.at(1).c_str(),button_size_);

		ImGui::LabelText("", "Metalness map");
		ImGui::Button(button_labels_.at(2).c_str(),button_size_);

		ImGui::LabelText("", "Normal map");
		ImGui::Button(button_labels_.at(3).c_str(),button_size_);
		
		ImGui::LabelText("", "Ambient occlusion map");
		ImGui::Button(button_labels_.at(4).c_str(),button_size_);

		if (update_pbr_material_) {
			pbr_material_ptr_->update();
			update_pbr_material_ = false;
		}
	}
	

	ImGui::End();
}

void LightEngineUI::MaterialEditor::load_material(std::shared_ptr<LightEngine::DefaultMaterial> &default_material_ptr) {
	default_material_ptr_ = default_material_ptr;
	default_parameters_ = default_material_ptr_->get_all_parameters();
	material_name_ = default_material_ptr_->get_name();
	flip_x_ = default_parameters_.flip_tb_vectors_ & 0x00FF;
	flip_y_ = default_parameters_.flip_tb_vectors_ & 0xFF00;
	pbr_material_ptr_ = nullptr;
	button_labels_ = default_material_ptr_->get_maps_names();
}

void LightEngineUI::MaterialEditor::load_material(std::shared_ptr<LightEngine::PBRMaterial> &pbr_material_ptr) {
	pbr_material_ptr_ = pbr_material_ptr;
	pbr_parameters_ = pbr_material_ptr_->get_all_parameters();
	material_name_ = pbr_material_ptr_->get_name();	
	default_material_ptr_ = nullptr;
	button_labels_ = pbr_material_ptr_->get_maps_names();
}