#pragma comment(lib, "gui")
#pragma comment(lib, "api")
#pragma comment(lib, "imgui_dx11")

#include <iostream>
#include <WinMain.h>
#include <LEException.h>
#include <LETexture.h>
#include <LEGeometry.h>
#include <LEMaterial.h>
#include <LECamera.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <GUI_variables.h>
#include <LEFrontend.h>
#include <LEBackend.h>
#include <../shaders/MC_Path_tracing/MC_Properties.hlsli>
#include <FilePaths.h>

constexpr int res_w = 1280;
constexpr int res_h = 720;

std::vector<float> create_test_texture_3d(int width, int height){

	std::vector<float> result(width*height*12, 0.0f);
		
	int indexes = width * height * 4;

	for(int i=0; i < indexes ; i += 4) 
		result[i] = 1.0f;
	
	for(int i=indexes; i < 2*indexes ; i += 4) 
		result[i + 1] = 1.0f;
	
	for(int i=2*indexes; i < 3*indexes ; i += 4) 
		result[i + 2] = 1.0f;
	
	return result;
}


int main(int argc, const char **argv) {

	AppWindow::Window window("LightEngine v1.0", res_w + 16, res_h + 39);
	//window.set_style(AppWindow::Style::MAXIMIZE);
	//window.set_style(AppWindow::Style::MINIMIZE);
	//window.set_style(AppWindow::Style::SIZEABLE);
	window.set_icon("LE.ico");

	float color[4]{0.4,0.4,0.41};
	MSG msg = { 0 };

	window.show_window();

	
	
	try {

		auto core = std::make_shared<LightEngine::Core>(window.get_handle(), window.get_width(), window.get_height());

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		(void)io;

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  
		

		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(window.get_handle());
		ImGui_ImplDX11_Init(core->get_device_ptr().Get(), core->get_context_ptr().Get());
		ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
		std::wstring shader_directory(COMPILED_SHADERS_DIR);

		
		//LightEngine::VertexShader hud_vs(core, shader_directory + L"ViewportHUD_VS.cso");
		//LightEngine::VertexShader shadow_mapping_vs(core, shader_directory + L"ShadowMappingVS.cso");
		//LightEngine::PixelShader blinn_ps(core, shader_directory + L"BlinnPS.cso");
		//LightEngine::PixelShader phong_ps(core, shader_directory + L"PhongPS.cso");
		//LightEngine::PixelShader gouraud_ps(core, shader_directory + L"GouraudPS.cso");
		//LightEngine::PixelShader pbr_ps(core, shader_directory + L"PBRShader.cso");	
		//LightEngine::PixelShader hud_ps(core, shader_directory + L"ViewportHUD_PS.cso");
		//LightEngine::PixelShader shadow_mapping_ps(core, shader_directory + L"ShadowMappingPS.cso");
		//LightEngine::PixelShader data_generate(core, shader_directory + L"DataGenerationPS.cso");

		LightEngine::VertexShader vs(core, shader_directory + L"CameraVS.cso");
		LightEngine::PixelShader pixel_data_generate(core, shader_directory + L"GeneratePixelDataPS.cso");
		LightEngine::PixelShader lambert_diffuse_ps(core, shader_directory + L"LambertDiffusePS.cso");
		LightEngine::ComputeShader cs_(core, shader_directory + L"RT_Shadows_CS.cso");
		//LightEngine::ComputeShader sampling_cs(core, shader_directory + L"GeometrySamplingCS.cso");
		LightEngine::ComputeShader lambert_cs(core, shader_directory + L"LambertDiffuseCS_2.cso");
		LightEngine::ComputeShader k_best_cs(core, shader_directory + L"LambertDiffuseCS_3.cso");

		static float blank[res_w * res_h * 4 ];
		ZeroMemory(blank, res_w * res_h * 4);
		//std::vector<float> test_1 = create_test_texture_3d(500, 500);
		//std::vector<float> test_2 = create_test_texture_3d(250, 250);

		static float general_3d_buffer[res_w * res_h * 4 * SAMPLES_COUNT];
		ZeroMemory(general_3d_buffer, res_w * res_h * 4 * SAMPLES_COUNT);

		LightEngine::Texture2D pixel_world_position(core, "Pixel world position", res_w, res_h, blank);
		LightEngine::Texture2D pixel_normal(core, "Pixel normal", res_w, res_h, blank);
		LightEngine::Texture2D pixel_tangent(core, "Pixel tangent", res_w, res_h, blank);
		LightEngine::Texture2D pixel_bitangent(core, "Pixel bitangent", res_w, res_h, blank);
		LightEngine::Texture2D pixel_uvw(core, "Pixel texture coords", res_w, res_h, blank);
		LightEngine::Texture2D pixel_color(core, "Pixel color", res_w, res_h, blank);
		LightEngine::Texture2D shadow_map(core, "Ray traced shadow map", res_w, res_h, blank);
		LightEngine::Texture3D general_3D_buffer(core, "3D Buffer", res_w, res_h, SAMPLES_COUNT, general_3d_buffer);
		LightEngine::Texture2D noise(core, NOISE_TX_PATH);

		
		
		core->add_texture_to_render(pixel_world_position, 0u);
		core->add_texture_to_render(pixel_normal, 1u);
		core->add_texture_to_render(pixel_tangent, 2u);
		core->add_texture_to_render(pixel_bitangent, 3u);
		core->add_texture_to_render(pixel_uvw, 4u);
		core->add_texture_to_render(pixel_color, 5u);

		LightEngine::ShaderResourceManager sr_manager(core);

		//sr_manager.bind_texture_buffer(noise, LightEngine::ShaderType::ComputeShader, 6u);
		//sr_manager.bind_texture_buffer(noise, LightEngine::ShaderType::PixelShader, 0u);
		//core->add_texture_to_render(world_position_data, 0u);
		//core->add_texture_to_render(normal_data, 1u);
		//sr_manager.bind_texture_buffer(texture_a, LightEngine::ShaderType::ComputeShader, 2u);
		//sr_manager.bind_texture_buffer(texture_b, LightEngine::ShaderType::ComputeShader, 3u);
		//sr_manager.bind_cs_unordered_access_buffer(output_texture, 0u);
		//cs_.run();
		//sr_manager.unbind_cs_unordered_access_buffer(0u);
		//output_texture.generate_mip_maps();
		//sr_manager.bind_texture_buffer(output_texture, LightEngine::ShaderType::PixelShader, 0u);
		//std::shared_ptr<LightEngine::PixelShader> blinn_ps_ptr = std::make_shared<LightEngine::PixelShader>(blinn_ps);
		//std::shared_ptr<LightEngine::PixelShader> phong_ps_ptr = std::make_shared<LightEngine::PixelShader>(phong_ps);
		//std::shared_ptr<LightEngine::PixelShader> gouraud_ps_ptr = std::make_shared<LightEngine::PixelShader>(gouraud_ps);
		//std::shared_ptr<LightEngine::PixelShader> pbr_ps_ptr = std::make_shared<LightEngine::PixelShader>(pbr_ps);
		//std::shared_ptr<LightEngine::PixelShader> mc_gi_ptr = std::make_shared<LightEngine::PixelShader>(mc_gi);


		LightEngine::FPSCamera fps_camera(core);
		fps_camera.update();

		LightEngine::ArcballCamera arcball_camera(core, 50.0);
		//LightEngine::ArcballCamera light_camera(core, 50.0);
		//LightEngine::LightSource point_light(core, 1.0);
		//LightEngine::LightSource direct_light(core, 0.0);	
		//LightEngine::Materials::BasicMaterial default_material(core, "Default00");
		//LightEngine::Materials::PBRMaterial pbr_material(core, "PBR_Metallic_Roughness");
		LightEngine::Sampler sampler_nearest(core, LightEngine::Sampler::Filtering::NEAREST);
		LightEngine::Sampler sampler_bilinear(core, LightEngine::Sampler::Filtering::BILINEAR);
		LightEngine::Sampler sampler_trilinear(core, LightEngine::Sampler::Filtering::TRILINEAR);
		LightEngine::Sampler sampler_anisotropic(core, LightEngine::Sampler::Filtering::ANISOTROPIC);

		const float **colors = new const float *[5];

		float white[3]{ 1.0, 1.0, 1.0 };
		float red[3]{ 1.0, 0.3, 0.3 };
		float green[3]{ 0.3, 1.0, 0.3 };
		float blue[3]{ 0.3, 0.3, 1.0 };
		float yellow[3]{ 0.3, 1.0, 1.0 };

		colors[0] = white;
		colors[1] = blue;
		colors[2] = red;
		colors[3] = green;
		colors[4] = yellow;
		
		std::vector<LightEngine::Geometry<LightEngine::Vertex3>> scene = LightEngine::Geometry<LightEngine::Vertex3>::load_from_obj(core, SCENE_FILE_PATH, colors, 5);
		
	
		// Objects merging
		int scene_vertex_count = 0;

		for (auto& obj : scene) 
			scene_vertex_count += obj.get_vertices_vector().size();
		
		std::vector<LightEngine::Vertex3> merged_vertices(scene_vertex_count);

		auto destination_itr = merged_vertices.begin();

		for (auto& obj : scene) {
			std::vector<LightEngine::Vertex3> obj_vertices(obj.get_vertices_vector());

			auto source_begin_itr = obj_vertices.begin();
			auto source_end_itr = obj_vertices.end();

			std::copy(source_begin_itr, source_end_itr, destination_itr);

			destination_itr += obj_vertices.size();
		}

		std::vector<LightEngine::Geometry<LightEngine::Vertex3>> tmp;
		tmp.push_back(scene[0]);

		uint32_t texture_width;

		uint32_t *lut = LightEngine::GeometryTools::generate_lookup_matrix(
			5.0,
			tmp,
			merged_vertices,
			texture_width);

		LightEngine::Texture2D lut_texture(core, "LUT", texture_width, 1, lut);

		sr_manager.bind_texture_buffer(lut_texture, LightEngine::ShaderType::ComputeShader, 8u);
		sr_manager.bind_texture_buffer(noise, LightEngine::ShaderType::ComputeShader, 7u);

		/*LightEngine::Geometry<LightEngine::Vertex3> icosphere = scene[0];

		std::vector<uint32_t> patch_triangles_indices = LightEngine::GeometryTools::calculate_neighbouring_geometry(
			3.0,
			scene.at(0).get_vertices_vector(),
			merged_vertices);


		std::vector<LightEngine::Vertex3> patch_vertices;

		for (int i = 1; i < patch_triangles_indices.size(); i++) {
			patch_vertices.push_back(merged_vertices.at(3 * patch_triangles_indices.at(i)));
			patch_vertices.push_back(merged_vertices.at(3 * patch_triangles_indices.at(i) + 1));
			patch_vertices.push_back(merged_vertices.at(3 * patch_triangles_indices.at(i) + 2));
		}

		LightEngine::Geometry<LightEngine::Vertex3> patch(core, patch_vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, "Patch");*/



		//scene.clear();
		//scene.push_back(patch);
		//scene.push_back(icosphere);

		//int32_t *lut = LightEngine::GeometryTools::generate_lookup_matrix(3, 968, 4, 2.0, scene, merged_vertices);

		//LightEngine::Texture3D lut_texture(core, "LUT", 968, 3, 4, lut);

		//sr_manager.bind_texture_buffer(lut_texture, LightEngine::ShaderType::ComputeShader, 8u);

		static float neighbour_contribution[res_w * res_h * 4 * 20];
		ZeroMemory(general_3d_buffer, res_w * res_h * 4 * 20);

		LightEngine::Texture3D gi_map(core, "GI map", res_w, res_h, 20, neighbour_contribution);

		/*std::vector<std::vector<int32_t>> look_up_table = LightEngine::GeometryTools::get_k_visible_neighbours(
			10,
			5.0,
			scene.at(1).get_vertices_vector(),
			merged_vertices
		);

		std::vector<LightEngine::Vertex3> visible_patch;

		for (auto& row : look_up_table) {
			for (auto& index : row) {
				if (index > 0) {
					visible_patch.push_back(merged_vertices.at(3 * index));
					visible_patch.push_back(merged_vertices.at(3 * index + 1));
					visible_patch.push_back(merged_vertices.at(3 * index + 2));
				}
			}
		}

		LightEngine::Geometry<LightEngine::Vertex3> patch(core, visible_patch, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, "");

		scene.clear();
		scene.push_back(patch);*/


		int triangle_count = merged_vertices.size() / 3;

		static std::vector<LightEngine::Vertex3> tris_v0s(0);
		static std::vector<LightEngine::Vertex3> tris_v1s(0);
		static std::vector<LightEngine::Vertex3> tris_v2s(0);

		tris_v0s.reserve(triangle_count);
		tris_v1s.reserve(triangle_count);
		tris_v2s.reserve(triangle_count);


		for (int i = 0; i < merged_vertices.size(); i += 3) {

			tris_v0s.push_back(merged_vertices[i]);
			tris_v1s.push_back(merged_vertices[i + 1]);
			tris_v2s.push_back(merged_vertices[i + 2]);

		}

		LightEngine::Texture2D v0s_tex = LightEngine::Texture2D::store_geometry(core, tris_v0s);
		LightEngine::Texture2D v1s_tex = LightEngine::Texture2D::store_geometry(core, tris_v1s);
		LightEngine::Texture2D v2s_tex = LightEngine::Texture2D::store_geometry(core, tris_v2s);

		sr_manager.bind_texture_buffer(v0s_tex, LightEngine::ShaderType::ComputeShader, 0u);
		sr_manager.bind_texture_buffer(v1s_tex, LightEngine::ShaderType::ComputeShader, 1u);
		sr_manager.bind_texture_buffer(v2s_tex, LightEngine::ShaderType::ComputeShader, 2u);
		
		

		/*direct_light.set_position(direct_light_position);
		default_material.assign_vertex_shader(std::make_shared<LightEngine::VertexShader>(vs));
		default_material.assign_pixel_shader(blinn_ps_ptr);

		std::shared_ptr<LightEngine::Materials::BasicMaterial> default_ptr = std::make_shared<LightEngine::Materials::BasicMaterial>(default_material);
		std::shared_ptr<LightEngine::Materials::PBRMaterial> pbr_ptr = std::make_shared<LightEngine::Materials::PBRMaterial>(pbr_material);	

		material_editor.load_material(default_ptr);

		/*default_ptr->assign_vertex_shader(std::make_shared<LightEngine::VertexShader>(vs));
		default_ptr->assign_pixel_shader(blinn_ps_ptr);

		point_light.bind_ps_buffer(1);
		point_light.bind_vs_buffer(1);
		direct_light.bind_ps_buffer(2);
		direct_light.bind_vs_buffer(2);*/

		sampler_anisotropic.bind(0);

		arcball_camera.modify_horizontal_angle(-30.0f);
		arcball_camera.modify_vertical_angle(30.0f);
		arcball_camera.update_view_matrix();
		arcball_camera.update();
		
		arcball_camera.bind(0);

		/*light_camera.modify_center(0.0, 0.0, 1.0);
		light_camera.modify_horizontal_angle(-90.0f);
		light_camera.set_clipping_near(lc_near_z);
		light_camera.set_clipping_far(lc_far_z);
		light_camera.set_fov(lc_fov);
		light_camera.update_view_matrix();
		light_camera.update();
		
		light_camera.bind(4);

		bind light-perspective camera*/

		LightEngine::Light p0;
		p0.position_.x = point_light_position[0];
		p0.position_.y = point_light_position[1];
		p0.position_.z = point_light_position[2];

		p0.color_.x = point_light_color[0];
		p0.color_.y = point_light_color[1];
		p0.color_.z = point_light_color[2];
		p0.color_.w = point_light_intensity;

		uint32_t cpu_data[4]{ 0,0,0,0 };

		LightEngine::CBuffer<LightEngine::Light> point_light_cbuff(core, "Point light 0", &p0, sizeof(p0));
		LightEngine::CBuffer<uint32_t> cpu_data_cbuff(core, "Cpu data", cpu_data, 16u);

		
		sr_manager.bind_constant_buffer(cpu_data_cbuff, LightEngine::ShaderType::PixelShader, 1u);

		int vp_width = 0;
		int vp_height = 0;
		
		vs.bind();

		while (WM_QUIT != msg.message) {
			
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame(); 
				
				//ImGui::DockSpaceOverViewport(0,ImGuiDockNodeFlags_PassthruCentralNode,0);
				
				if(window.was_resized()) {
				
					std::cout<<window.get_width()<<" x "<<window.get_height()<<std::endl;
					core->setup_frame_buffer(window.get_width(),window.get_height(), false);
					core->render_to_frame_buffer();


					arcball_camera.set_aspect_ratio((float)res_w/(float)res_h);
					arcball_camera.update_projection_matrix();
					fps_camera.set_aspect_ratio((float)res_w/(float)res_h);
					fps_camera.update_projection_matrix();

					arcball_camera.update();
					fps_camera.update();


					//update_viewport = true;

				}
				// GUI Scope
				{
					
					if (ImGui::BeginMainMenuBar()) {

						if (ImGui::BeginMenu("File")) {

							if (ImGui::MenuItem("Import")) {
								std::string path = AppWindow::open_file_dialog(model_file_filter, model_file_filter_size);
								if (path != "") {
									std::vector<LightEngine::Geometry<LightEngine::Vertex3>> object_set = LightEngine::Geometry<LightEngine::Vertex3>::load_from_obj(core, path, nullptr, 0);
									for (auto& object : object_set) {
										scene.push_back(object);
									}
								}
							}
							ImGui::EndMenu();					
						}									
						
						if (ImGui::BeginMenu("Settings")){
						
							if (ImGui::BeginMenu("Render quality")) {

								ImGui::LabelText("", "Texture filtering");

								if (ImGui::RadioButton("Nearest", &texture_filtering, 0))
									sampler_nearest.bind(0);

								if (ImGui::RadioButton("Bilinear", &texture_filtering, 1))
									sampler_bilinear.bind(0);

								if (ImGui::RadioButton("Trilinear", &texture_filtering, 2))
									sampler_trilinear.bind(0);

								if (ImGui::RadioButton("Anisotropic", &texture_filtering, 3))
									sampler_anisotropic.bind(0);

								ImGui::EndMenu();
							}

							if (ImGui::BeginMenu("Preferences")) {

								ImGui::LabelText("", "Viewport");

								if(ImGui::DragInt("Position X",&vp_tl_x,1.0,0,INT_MAX))
									update_viewport = true;
								
								if(ImGui::DragInt("Position Y",&vp_tl_y,1.0,0,INT_MAX))
									update_viewport = true;

								if(ImGui::DragInt("Width offset",&vp_const_width_offset,1.0,-INT_MAX,INT_MAX))
									update_viewport = true;

								if(ImGui::DragInt("Height offset",&vp_const_height_offset,1.0,-INT_MAX,INT_MAX))
									update_viewport = true;

								ImGui::EndMenu();
							}

							ImGui::EndMenu();
						}


						if (ImGui::BeginMenu("Camera")) {

							if (ImGui::RadioButton("FPS", &camera, 0)) {
								std::cout << "FPS camera selected" << std::endl;
								fps_camera.bind(0);
								update_camera = true;
							}

							ImGui::SameLine();

							if (ImGui::RadioButton("Arcball", &camera, 1)) {
								std::cout << "Arcball camera selected" << std::endl;
								arcball_camera.bind(0);
								update_camera = true;
							}

							if (camera) {
								if (ImGui::SliderInt("FOV", &arcball_fov, 0, 180)) {
									arcball_camera.set_fov((float)arcball_fov);
									arcball_camera.update_projection_matrix();
									update_camera = true;
								}
							}
							else {
								if (ImGui::SliderInt("FOV", &world_fov, 0, 180)) {
									fps_camera.set_fov((float)world_fov);
									fps_camera.update_projection_matrix();
									update_camera = true;
								}
							}


							if (ImGui::Button("Reset camera", ImVec2(150, 30))) {

								if (camera)
									arcball_camera.reset();
								else
									fps_camera.reset();
								update_camera = true;
							}

							ImGui::EndMenu();

						}


						if (ImGui::BeginMenu("Light Sources")) {

							if (ImGui::BeginMenu("Point light")) {
								if (ImGui::CollapsingHeader("Color")) {
									if (ImGui::ColorPicker3("Color", point_light_color)) {
										//point_light.set_color(point_light_color);

										p0.color_.x = point_light_color[0];
										p0.color_.y = point_light_color[1];
										p0.color_.z = point_light_color[2];

										update_light = true;
									}
								}

								if (ImGui::DragFloat3("Position", point_light_position, 0.1, -D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, "%.1f")) {
									//point_light.set_position(point_light_position);
									p0.position_.x = point_light_position[0];
									p0.position_.y = point_light_position[1];
									p0.position_.z = point_light_position[2];

									update_light = true;
								}

								if (ImGui::DragFloat("Intensity", &point_light_intensity, 10.0f, 1.0f, 1000.0f, "%.2f")) {
									//point_light.set_intensity(point_light_intensity);
									p0.color_.w = point_light_intensity;
									update_light = true;
								}

								ImGui::EndMenu();
							}

							if (ImGui::BeginMenu("Direct light")) {
								if (ImGui::ColorPicker3("Color", direct_light_color)) {
									//direct_light.set_color(direct_light_color);
									update_light = true;
								}

								if (ImGui::DragFloat("Intensity", &direct_light_intensity, 10.0f, 1.0f, 1000.0f, "%.2f")) {
									//direct_light.set_intensity(direct_light_intensity);
									update_light = true;
								}

								ImGui::EndMenu();
							}



							ImGui::EndMenu();
						}

						ImGui::EndMainMenuBar();
					}					


					/*ImGui::Begin("Light Camera");

					ImGui::Checkbox("Focus IO", &light_camera_focus);
					
					if (ImGui::SliderInt("FOV", &lc_fov, 1, 180)) {
						light_camera.set_fov((float)lc_fov);
						light_camera.update_projection_matrix();
						update_camera = true;
					}
					
					if (ImGui::SliderInt("Near Z", &lc_near_z, 1, 100)) {
						light_camera.set_clipping_near((float)lc_near_z);
						light_camera.update_projection_matrix();
						update_camera = true;
					}

					if (ImGui::SliderInt("Far Z", &lc_far_z, 1, 100)) {
						light_camera.set_clipping_far((float)lc_far_z);
						light_camera.update_projection_matrix();
						update_camera = true;
					}

					ImGui::End();*/					

					//material_editor.render();
					//texture_browser.render();
					//bm_browser.render();

					ImGui::Render();								
				}
							
				// IO Scope
				if(!ImGui::GetIO().WantCaptureMouse) {
					
					cursor_x_delta = AppWindow::IO::Mouse::get_position_x() - cursor_x_buffer;
					cursor_x_buffer = AppWindow::IO::Mouse::get_position_x();

					cursor_y_delta = AppWindow::IO::Mouse::get_position_y() - cursor_y_buffer;
					cursor_y_buffer = AppWindow::IO::Mouse::get_position_y();

					wheel_d = AppWindow::IO::Mouse::get_wheel_delta();
			

					if(camera) {
						if (light_camera_focus) {
							if (AppWindow::IO::Mouse::left_button_down()) {
								//light_camera.modify_vertical_angle(-cursor_x_delta * ORBITING_SENSITIVITY);
								//light_camera.modify_horizontal_angle(-cursor_y_delta * ORBITING_SENSITIVITY);
								//light_camera.update_view_matrix();
								update_camera = true;
							}

							if (AppWindow::IO::Mouse::middle_button_down()) {
								//light_camera.pan_horizontal(-cursor_x_delta * PANNING_SENSITIVITY);
								//light_camera.pan_vertical(cursor_y_delta * PANNING_SENSITIVITY);
								//light_camera.update_view_matrix();
								update_camera = true;
							}

							if (wheel_d) {
								//light_camera.modify_radius(-wheel_d * ZOOM_SENSITIVITY*0.1);
								//light_camera.update_view_matrix();
								update_camera = true;
							}


						}
						else {
							if (AppWindow::IO::Mouse::left_button_down()) {
								arcball_camera.modify_vertical_angle(-cursor_x_delta * ORBITING_SENSITIVITY);
								arcball_camera.modify_horizontal_angle(-cursor_y_delta * ORBITING_SENSITIVITY);
								arcball_camera.update_view_matrix();
								update_camera = true;
							}

							if (AppWindow::IO::Mouse::middle_button_down()) {
								arcball_camera.pan_horizontal(-cursor_x_delta * PANNING_SENSITIVITY);
								arcball_camera.pan_vertical(cursor_y_delta * PANNING_SENSITIVITY);
								arcball_camera.update_view_matrix();
								update_camera = true;
							}

							if (wheel_d) {
								arcball_camera.modify_radius(-wheel_d * ZOOM_SENSITIVITY);
								arcball_camera.update_view_matrix();
								update_camera = true;
							}
						}
					} else {

						if (AppWindow::IO::Mouse::left_button_down()) {
							fps_camera.modify_vertical_angle(-cursor_x_delta * FPS_SENSITIVITY);
							fps_camera.modify_horizontal_angle(-cursor_y_delta * FPS_SENSITIVITY);
							fps_camera.update_view_matrix();
							update_camera = true;
						}

						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::S)) {
							fps_camera.move_relative_z(1.0);
							fps_camera.update_view_matrix();
							update_camera = true;
						}
					
						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::W)) {
							fps_camera.move_relative_z(-1.0);
							fps_camera.update_view_matrix();
							update_camera = true;
						}

						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::A)) {
							fps_camera.move_relative_x(1.0);
							fps_camera.update_view_matrix();
							update_camera = true;
						}
					
						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::D)) {
							fps_camera.move_relative_x(-1.0);
							fps_camera.update_view_matrix();
							update_camera = true;
						}

						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::Q)) {
							fps_camera.move_y(1.0);
							fps_camera.update_view_matrix();
							update_camera = true;
						}
					
						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::E)) {
							fps_camera.move_y(-1.0);
							fps_camera.update_view_matrix();
							update_camera = true;
						}
					}				
				}

				if (update_camera) {

					if (camera == 0) 
						fps_camera.update();			
					else 
						arcball_camera.update();

					//light_camera.update();
					update_camera = false;
				}

				if(update_light){
					//point_light.update();
					//direct_light.update();
					point_light_cbuff.update(&p0, sizeof(p0));
					update_light = false;
				}

				if(update_material){
					//default_ptr->update();
					//pbr_ptr->update();
					update_material = false;
				}

				if(update_viewport){		
					vp_width = window.get_width() + vp_const_width_offset;
					vp_height = window.get_height() + vp_const_height_offset;

					core->viewport_setup(vp_tl_x,vp_tl_y,vp_width, vp_height);
					arcball_camera.set_aspect_ratio((float)vp_width/(float)vp_height);
					arcball_camera.update_projection_matrix();
					//light_camera.set_aspect_ratio((float)vp_width/(float)vp_height);
					//light_camera.update_projection_matrix();
					fps_camera.set_aspect_ratio((float)vp_width/(float)vp_height);
					fps_camera.update_projection_matrix();

					arcball_camera.update();
					fps_camera.update();
					//light_camera.update();
					update_viewport = false;
				}
			
				
				core->clear_frame_buffer(color);				
				core->flush_render_targets();
				core->render_to_textures();


				pixel_data_generate.bind();
				
				//tmp[0].bind_topology();
				//tmp[0].bind_vertex_buffer();

				// Run first pass to generate data
				//tmp[0].draw(0);

				
				for (uint32_t i = 0; i < scene.size(); i++) {

					cpu_data[0] = i;
					cpu_data_cbuff.update(cpu_data, 16u);

					scene[i].bind_topology();
					scene[i].bind_vertex_buffer();

					// Run first pass to generate data
					scene[i].draw(0);

				}

				core->render_to_frame_buffer();
				
				// Bind light source to cs
				sr_manager.bind_constant_buffer(point_light_cbuff, LightEngine::ShaderType::ComputeShader, 0u);
				// Bind g-buffer to cs
				sr_manager.bind_texture_buffer(pixel_world_position, LightEngine::ShaderType::ComputeShader, 3u);
				sr_manager.bind_texture_buffer(pixel_normal, LightEngine::ShaderType::ComputeShader, 4u);
				sr_manager.bind_texture_buffer(pixel_color, LightEngine::ShaderType::ComputeShader, 5u);
				sr_manager.bind_texture_buffer(pixel_tangent, LightEngine::ShaderType::ComputeShader, 6u);
				sr_manager.bind_texture_buffer(pixel_bitangent, LightEngine::ShaderType::ComputeShader, 9u);
				
				// Bind GI map uav to cs
				sr_manager.bind_cs_unordered_access_buffer(gi_map, 0u);
				
				// Run cs 
				k_best_cs.bind();
				k_best_cs.run(res_w, res_h, 1);
		
				// Unbind light source from CS
				sr_manager.unbind_constant_buffer(LightEngine::ShaderType::ComputeShader, 0u);
				// Unbind g-buffer
				sr_manager.unbind_texture_buffer(LightEngine::ShaderType::ComputeShader, 3u);
				sr_manager.unbind_texture_buffer(LightEngine::ShaderType::ComputeShader, 4u);
				sr_manager.unbind_texture_buffer(LightEngine::ShaderType::ComputeShader, 5u);
				sr_manager.unbind_texture_buffer(LightEngine::ShaderType::ComputeShader, 6u);
				sr_manager.unbind_texture_buffer(LightEngine::ShaderType::ComputeShader, 9u);


				// Unbind GI map from uav
				sr_manager.unbind_cs_unordered_access_buffer(0u);
				// Bind light source to PS
				
				sr_manager.bind_constant_buffer(point_light_cbuff, LightEngine::ShaderType::PixelShader, 0u);
				//Bind GI map to ps
				sr_manager.bind_texture_buffer(gi_map, LightEngine::ShaderType::PixelShader, 0u);
				lambert_diffuse_ps.bind();

				for (uint32_t i = 0; i < scene.size(); i++) {

					scene[i].bind_topology();
					scene[i].bind_vertex_buffer();

					scene[i].draw(0);

				}

				// Unbind GI Map from PS
				sr_manager.unbind_texture_buffer(LightEngine::ShaderType::PixelShader, 0u);
				// Unbind light sourve from PS
				sr_manager.unbind_constant_buffer(LightEngine::ShaderType::PixelShader, 0u);

				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());				
				
				core->present_frame();			
				
			}						
		}
	}
	catch (LightEngine::LECoreException &e) {
		std::cout << e.what();
		std::wcout << e.get_message();

		std::cin.get();
	}
	std::cin.get();
	return 0;
}
