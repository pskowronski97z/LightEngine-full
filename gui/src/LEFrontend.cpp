#include <LEFrontend.h>

constexpr int color_edit_flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs;

LightEngineUI::Frontend::Window::Window(std::string name) : name_(std::move(name)) {
	flags_ = ImGuiWindowFlags_NoCollapse;
}

LightEngineUI::Frontend::MaterialEditor::MaterialEditor() : Window("Material Editor") {}

void LightEngineUI::Frontend::MaterialEditor::render() {

	ImGui::Begin(name_.c_str(), nullptr, flags_);
	
	if(view_ptr != nullptr)
		view_ptr->render(ImGui::GetWindowWidth());
	
	ImGui::End();
}

void LightEngineUI::Frontend::MaterialEditor::load_material(std::shared_ptr<LightEngine::DefaultMaterial> &default_material_ptr) {
	view_ptr = std::make_shared<LightEngineUI::Frontend::MaterialEditor::DefaultView>(default_material_ptr);	
}

void LightEngineUI::Frontend::MaterialEditor::load_material(std::shared_ptr<LightEngine::PBRMaterial> &pbr_material_ptr) {
	view_ptr = std::make_shared<LightEngineUI::Frontend::MaterialEditor::PBRView>(pbr_material_ptr);
}

LightEngineUI::Frontend::MaterialEditor::DefaultView::DefaultView(std::shared_ptr<LightEngine::DefaultMaterial> &material_ptr) : material_ptr_(material_ptr) {
	parameters_ = material_ptr_->get_all_parameters();
	material_name_ = material_ptr_->get_name();
	flip_x_ = parameters_.flip_tb_vectors_ & 0x00FF;
	flip_y_ = parameters_.flip_tb_vectors_ & 0xFF00;
	button_labels_ = material_ptr_->get_maps_names();
}

void LightEngineUI::Frontend::MaterialEditor::DefaultView::render(int window_width) {
	static ImVec2 button_size;
	static const ImVec2 spacing(0.0f, 5.0f);
	button_size.x = window_width;
	button_size.y = 20;

	ImGui::Dummy(spacing);
	ImGui::BulletText(material_name_.c_str());
	ImGui::Dummy(spacing);
	ImGui::Separator();

	if (ImGui::ColorEdit3("Specular color", parameters_.specular, color_edit_flags)) {
		material_ptr_->set_specular_color(parameters_.specular);
		material_ptr_->update();
	}

	if (ImGui::ColorEdit3("Diffuse color", parameters_.diffuse, color_edit_flags)) {
		material_ptr_->set_diffuse_color(parameters_.diffuse);
		material_ptr_->update();
	}

	if (ImGui::ColorEdit3("Ambient color", parameters_.ambient, color_edit_flags)) {
		material_ptr_->set_ambient_color(parameters_.ambient);
		material_ptr_->update();
	}

	ImGui::Separator();
	ImGui::LabelText("", "Levels");

	if (ImGui::SliderFloat("Specular", &parameters_.specular_level_, 0.0, 1.0, "%.2f")) {
		material_ptr_->set_specular_level(parameters_.specular_level_);
		material_ptr_->update();
	}

	if (ImGui::SliderInt("Glossiness", &parameters_.glossiness_, 1.0, 500)) {
		material_ptr_->set_glossiness(parameters_.glossiness_);
		material_ptr_->update();
	}

	if (ImGui::CollapsingHeader("Attenuation")) {

		if (ImGui::DragFloat("Quadratic", &parameters_.quadratic_att_, 0.01, 0, D3D11_FLOAT32_MAX, "%.2f")) {
			material_ptr_->set_quadratic_att(parameters_.quadratic_att_);
			material_ptr_->update();
		}

		if (ImGui::DragFloat("Linear", &parameters_.linear_att_, 0.01, 0, D3D11_FLOAT32_MAX, "%.2f")) {
			material_ptr_->set_linear_att(parameters_.linear_att_);
			material_ptr_->update();
		}

		if (ImGui::DragFloat("Constant", &parameters_.constant_att_, 0.01, 0, D3D11_FLOAT32_MAX, "%.2f")) {
			material_ptr_->set_constant_att(parameters_.constant_att_);
			material_ptr_->update();
		}

	}

	ImGui::Separator();

	ImGui::LabelText("", "Diffuse map");

	//TODO: Add functionality 
	if (ImGui::Button(button_labels_.at(0).c_str(), button_size)) {
		// Handle with Asset Manager
	}

	ImGui::LabelText("", "Normal map");

	//TODO: Add functionality 
	if (ImGui::Button(button_labels_.at(1).c_str(), button_size)) {
		// Handle with Asset Manager
	}

	if (parameters_.use_normal_map_) {
		if (ImGui::Checkbox("Flip X", &flip_x_)) {
			material_ptr_->flip_tangent();
			material_ptr_->update();
		}

		ImGui::SameLine();

		if (ImGui::Checkbox("Flip Y", &flip_y_)) {
			material_ptr_->flip_bitangent();
			material_ptr_->update();
		}
	}

}

LightEngineUI::Frontend::MaterialEditor::PBRView::PBRView(std::shared_ptr<LightEngine::PBRMaterial>& material_ptr) : material_ptr_(material_ptr) {
	parameters_ = material_ptr_->get_all_parameters();
	material_name_ = material_ptr_->get_name();	
	button_labels_ = material_ptr_->get_maps_names();
}

void LightEngineUI::Frontend::MaterialEditor::PBRView::render(int window_width) {

	static ImVec2 button_size;
	static const ImVec2 spacing(0.0f, 5.0f);
	button_size.x = window_width;
	button_size.y = 20;

	ImGui::Dummy(spacing);
	ImGui::BulletText(material_name_.c_str());
	ImGui::Dummy(spacing);
	ImGui::Separator();

	if (ImGui::ColorEdit3("Albedo", parameters_.albedo_, color_edit_flags)) {
		material_ptr_->set_albedo(parameters_.albedo_);
		material_ptr_->update();
	}

	if (ImGui::SliderFloat("Roughness", &parameters_.roughness_, 0.01f, 1.0f, "%.2f")) {
		material_ptr_->set_roughness(parameters_.roughness_);
		material_ptr_->update();
	}

	if (ImGui::SliderFloat("Metalness", &parameters_.metalness_, 0.0f, 1.0f, "%.2f")) {
		material_ptr_->set_metalness(parameters_.metalness_);
		material_ptr_->update();
	}

	ImGui::Separator();

	ImGui::LabelText("", "Albedo map");
	ImGui::Button(button_labels_.at(0).c_str(), button_size);
	
	ImGui::LabelText("", "Roughness map");
	ImGui::Button(button_labels_.at(1).c_str(), button_size);

	ImGui::LabelText("", "Metalness map");
	ImGui::Button(button_labels_.at(2).c_str(), button_size);

	ImGui::LabelText("", "Normal map");
	ImGui::Button(button_labels_.at(3).c_str(), button_size);

	ImGui::LabelText("", "Ambient occlusion map");
	ImGui::Button(button_labels_.at(4).c_str(), button_size);

}
