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
#include <LE_GUI.h>

//Path to a directory where all compiled HLSL shaders (from "shaders" directory) are placed
constexpr auto COMPILED_SHADERS_DIR = L"B:\\source\\repos\\LightEngine v2\\LightEngine-full\\bin\\x64\\Debug\\compiled shaders\\";

int main(int argc, const char **argv) {

	AppWindow::Window window("LightEngine v1.0", 1600, 900);
	window.set_style(AppWindow::Style::MAXIMIZE);
	window.set_style(AppWindow::Style::MINIMIZE);
	window.set_style(AppWindow::Style::SIZEABLE);
	window.set_icon("LE.ico");

	float color[4]{0.15,0.15,0.15};
	MSG msg = { 0 };

	window.show_window();

	std::vector<LightEngine::Geometry<LightEngine::Vertex3>> scene;
	
	try {

		auto core = std::make_shared<LightEngine::Core>(window.get_handle(), window.get_width(), window.get_height());

		core->clear_back_buffer(color);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		(void)io;

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  
		

		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(window.get_handle());
		ImGui_ImplDX11_Init(core->get_device_ptr().Get(), core->get_context_ptr().Get());
	
		
		
		std::wstring shader_directory(COMPILED_SHADERS_DIR);

		LightEngine::VertexShader vs(core, shader_directory + L"CameraVS.cso");
		LightEngine::PixelShader blinn_ps(core, shader_directory + L"BlinnPS.cso");
		LightEngine::PixelShader phong_ps(core, shader_directory + L"PhongPS.cso");
		LightEngine::PixelShader gouraud_ps(core, shader_directory + L"GouraudPS.cso");
		LightEngine::PixelShader pbr_ps(core, shader_directory + L"PBRShader.cso");
		LightEngine::VertexShader hud_vs(core, shader_directory + L"ViewportHUD_VS.cso");
		LightEngine::PixelShader hud_ps(core, shader_directory + L"ViewportHUD_PS.cso");

		std::shared_ptr<LightEngine::PixelShader> blinn_ps_ptr = std::make_shared<LightEngine::PixelShader>(blinn_ps);
		std::shared_ptr<LightEngine::PixelShader> phong_ps_ptr = std::make_shared<LightEngine::PixelShader>(phong_ps);
		std::shared_ptr<LightEngine::PixelShader> gouraud_ps_ptr = std::make_shared<LightEngine::PixelShader>(gouraud_ps);
		std::shared_ptr<LightEngine::PixelShader> pbr_ps_ptr = std::make_shared<LightEngine::PixelShader>(pbr_ps);

		LightEngine::FPSCamera fps_camera(core);
		LightEngine::ArcballCamera arcball_camera(core, 50.0);
		LightEngine::LightSource point_light(core, 1.0);
		LightEngine::LightSource direct_light(core, 0.0);
		LightEngine::DEBUG_VSTransform transform(core);
		LightEngine::DefaultMaterial default_material(core, "Default00");
		LightEngine::PBRMaterial pbr_material(core, "PBR_MR_00");
		LightEngine::Sampler sampler_nearest(core, LightEngine::Sampler::Filtering::NEAREST);
		LightEngine::Sampler sampler_bilinear(core, LightEngine::Sampler::Filtering::BILINEAR);
		LightEngine::Sampler sampler_trilinear(core, LightEngine::Sampler::Filtering::TRILINEAR);
		LightEngine::Sampler sampler_anisotropic(core, LightEngine::Sampler::Filtering::ANISOTROPIC);
		LightEngineUI::MaterialEditor material_editor;

		

		std::vector<LightEngine::Vertex3> border_vertices{
			{{-0.99,0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{0.99,0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{0.99,-0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{-0.99,-0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{-0.99,0.99,0.0},{0.4,0.4,0.4,1.0}}};

		LightEngine::Geometry<LightEngine::Vertex3> viewport_border(core,border_vertices,D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,"border");

		

		direct_light.set_position(direct_light_position);
		default_material.assign_vertex_shader(std::make_shared<LightEngine::VertexShader>(vs));
		default_material.assign_pixel_shader(blinn_ps_ptr);

		std::shared_ptr<LightEngine::DefaultMaterial> default_ptr = std::make_shared<LightEngine::DefaultMaterial>(default_material);

		material_editor.load_material(default_ptr);

		pbr_material.assign_vertex_shader(std::make_shared<LightEngine::VertexShader>(vs));
		pbr_material.assign_pixel_shader(pbr_ps_ptr);

		point_light.bind_ps_buffer(1);
		point_light.bind_vs_buffer(1);
		direct_light.bind_ps_buffer(2);
		direct_light.bind_vs_buffer(2);

		sampler_anisotropic.bind(0);

		direct_light.update();
		
		arcball_camera.update();
		arcball_camera.set_active(0);

	
		while (WM_QUIT != msg.message) {
			
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				
				ImGui::DockSpaceOverViewport(0,ImGuiDockNodeFlags_PassthruCentralNode,0);

				if(window.was_resized()) {
				
					std::cout<<window.get_width()<<" x "<<window.get_height()<<std::endl;
					core->update_frame_buffer(window.get_width(),window.get_height());
					update_viewport = true;

				}
				// GUI Scope
				{
					
					

					if (ImGui::BeginMainMenuBar()) {

						if (ImGui::BeginMenu("File")) {

							if (ImGui::MenuItem("Import")) {
								std::string path = AppWindow::open_file_dialog(model_file_filter, model_file_filter_size);
								if (path != "") {
									std::vector<LightEngine::Geometry<LightEngine::Vertex3>> object_set = LightEngine::Geometry<LightEngine::Vertex3>::load_from_obj(core, path);
									for(auto &object : object_set) {
										scene.push_back(object);
									}							
								}
							}
							ImGui::EndMenu();					
						}

						if (ImGui::BeginMenu("Camera")) {

							if (ImGui::RadioButton("FPS", &camera, 0)) {
								std::cout << "FPS camera selected" << std::endl;
								fps_camera.set_active(0);
								update_camera = true;
							}							

							ImGui::SameLine();

							if (ImGui::RadioButton("Arcball", &camera, 1)) {
								std::cout << "Arcball camera selected" << std::endl;
								arcball_camera.set_active(0);
								update_camera = true;
							}

							if (camera) {
								if (ImGui::SliderInt("FOV", &arcball_fov, 0, 180)) {
									arcball_camera.set_fov((float)arcball_fov);
									update_camera = true;
								}
							}
							else {
								if (ImGui::SliderInt("FOV", &world_fov, 0, 180)) {
									fps_camera.set_fov((float)world_fov);
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
										point_light.set_color(point_light_color);
										update_light = true;
									}
								}

								if (ImGui::DragFloat3("Position", point_light_position, 0.1, -D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, "%.1f")) {
									point_light.set_position(point_light_position);
									update_light = true;
								}

								if (ImGui::DragFloat("Intensity", &point_light_intensity, 10.0f, 1.0f, 1000.0f, "%.2f")) {
									point_light.set_intensity(point_light_intensity);
									update_light = true;
								}

								ImGui::EndMenu();
							}

							if (ImGui::BeginMenu("Direct light")) {
								if (ImGui::ColorPicker3("Color", direct_light_color)) {
									direct_light.set_color(direct_light_color);
									update_light = true;
								}

								if (ImGui::DragFloat("Intensity", &direct_light_intensity, 10.0f, 1.0f, 1000.0f, "%.2f")) {
									direct_light.set_intensity(direct_light_intensity);
									update_light = true;
								}
								
								ImGui::EndMenu();
							}

							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Material Editor")) {
							if (ImGui::BeginMenu(default_material.get_name().c_str())) {

								ImGui::LabelText("", "Shading mode");

								if (ImGui::RadioButton("Blinn", &shading, 0)) {
									std::cout << "Blinn shading selected" << std::endl;
									default_material.assign_pixel_shader(blinn_ps_ptr);
									update_material = true;
									
								}

								ImGui::SameLine();

								if (ImGui::RadioButton("Phong", &shading, 1)) {
									std::cout << "Phong shading selected" << std::endl;
									default_material.assign_pixel_shader(phong_ps_ptr);
									update_material = true;
									
								}

								ImGui::SameLine();

								if (ImGui::RadioButton("Gouraud", &shading, 2)) {
									std::cout << "Gouraud shading selected" << std::endl;
									default_material.assign_pixel_shader(gouraud_ps_ptr);
									update_material = true;
									
								}

								ImGui::Separator();

								if (ImGui::BeginMenu("Specular color")) {
									if (ImGui::ColorPicker3("Specular", specular_color)) {
										default_material.set_specular_color(specular_color);
										update_material = true;
									}
									ImGui::EndMenu();
								}

								if (ImGui::BeginMenu("Diffuse color")) {
									if (ImGui::ColorPicker3("Diffuse", diffuse_color)) {
										default_material.set_diffuse_color(diffuse_color);
										update_material = true;
									}

									ImGui::Separator();

									if (default_material.is_diffuse_map_assigned()) {
										if (ImGui::Checkbox("Use map", &is_diffuse_map_used)) {
											if(is_diffuse_map_used)
												default_material.select_diffuse_map_usage();
											else
												default_material.unselect_diffuse_map_usage();
											update_material = true;
										}
									}

									if(ImGui::Button(load_dmap_btn_lbl.c_str(), ImVec2(270,30))){
										std::string path = AppWindow::open_file_dialog(image_file_filter, image_file_filter_size);
										if (path != "") {
											LightEngine::Texture temp(core, path);
											default_material.assign_diffuse_map(std::make_shared<LightEngine::Texture>(temp));
											default_material.select_diffuse_map_usage();
											load_dmap_btn_lbl = temp.get_name();
											is_diffuse_map_used = true;
											update_material = true;
										}
									}								
									ImGui::EndMenu();								
								}

								if (ImGui::BeginMenu("Ambient color")) {
									if (ImGui::ColorPicker3("Ambient", ambient_color)) {
										default_material.set_ambient_color(ambient_color);
										update_material = true;
									}
									ImGui::EndMenu();
								}

								ImGui::Separator();							

								if (ImGui::DragInt("Glossiness", &glossiness, 1, 1, 500, "%3d")) {
									default_material.set_glossiness(glossiness);
									update_material = true;
								}

								if (ImGui::DragFloat3("Attenuation", attenuation, 0.001, 0, D3D11_FLOAT32_MAX, "%.4f")) {
									default_material.set_quadratic_att(attenuation[0]);
									default_material.set_linear_att(attenuation[1]);
									default_material.set_constant_att(attenuation[2]);
									update_material = true;
								}

								ImGui::Separator();
								ImGui::LabelText("", "Normal mapping");
									if (default_material.is_normal_map_assigned()) {
										if (ImGui::Checkbox("Enable", &is_normal_map_used)) {
											if(is_normal_map_used)
												default_material.select_normal_map_usage();
											else
												default_material.unselect_normal_map_usage();
											update_material = true;
										}
									}

									if(ImGui::Button(load_nmap_btn_lbl.c_str(), ImVec2(300,30))){
										std::string path = AppWindow::open_file_dialog(image_file_filter, image_file_filter_size);
										if (path != "") {
											LightEngine::Texture temp(core, path);
											default_material.assign_normal_map(std::make_shared<LightEngine::Texture>(temp));
											default_material.select_normal_map_usage();
											load_nmap_btn_lbl = temp.get_name();
											is_normal_map_used = true;
											update_material = true;
										}
									}

									if (default_material.is_normal_map_assigned()) {
										if (ImGui::Checkbox("Flip X", &flip_x)) {
											default_material.flip_tangent();
											update_material = true;
										}
										ImGui::SameLine();
										if (ImGui::Checkbox("Flip Y", &flip_y)) {
											default_material.flip_bitangent();
											update_material = true;
										}
									}

								ImGui::EndMenu();
							}
							
							if (ImGui::BeginMenu(pbr_material.get_name().c_str())){
							
								if (ImGui::BeginMenu("Albedo")) {
									if (ImGui::ColorPicker3("Albedo", albedo)) {
										pbr_material.set_albedo(albedo);
										update_material = true;
									}

									if (ImGui::Button(pbr_albedo_btn.c_str(), ImVec2(270, 30))) {
										std::string path = AppWindow::open_file_dialog(image_file_filter, image_file_filter_size);
										if (path != "") {
											LightEngine::Texture temp(core, path);
											pbr_material.assign_albedo_map(std::make_shared<LightEngine::Texture>(temp));
											pbr_albedo_btn = temp.get_name();
											update_material = true;
										}
									}

									ImGui::EndMenu();
								}

								

								if (ImGui::DragFloat("Roughness", &roughness, 0.005f, 0.0f, 1.0f, "%.2f")) {
									pbr_material.set_roughness(roughness);
									update_material = true;
								}

								if (ImGui::Button(pbr_roughness_btn.c_str(), ImVec2(270, 30))) {
									std::string path = AppWindow::open_file_dialog(image_file_filter, image_file_filter_size);
									if (path != "") {
										LightEngine::Texture temp(core, path);
										pbr_material.assign_roughness_map(std::make_shared<LightEngine::Texture>(temp));
										pbr_roughness_btn = temp.get_name();
										update_material = true;
									}
								}
								
								if (ImGui::DragFloat("Metallic", &metallic, 0.005f, 0.0f, 1.0f, "%.2f")) {
									pbr_material.set_metallic(metallic);
									update_material = true;
								}

								if (ImGui::Button(pbr_metalness_btn.c_str(), ImVec2(270, 30))) {
									std::string path = AppWindow::open_file_dialog(image_file_filter, image_file_filter_size);
									if (path != "") {
										LightEngine::Texture temp(core, path);
										pbr_material.assign_metalness_map(std::make_shared<LightEngine::Texture>(temp));
										pbr_metalness_btn = temp.get_name();
										update_material = true;
									}
								}

								if (ImGui::Button(pbr_normal_btn.c_str(), ImVec2(270, 30))) {
									std::string path = AppWindow::open_file_dialog(image_file_filter, image_file_filter_size);
									if (path != "") {
										LightEngine::Texture temp(core, path);
										pbr_material.assign_normal_map(std::make_shared<LightEngine::Texture>(temp));
										pbr_normal_btn = temp.get_name();
										update_material = true;
									}
								}

								if (ImGui::Button(pbr_ao_btn.c_str(), ImVec2(270, 30))) {
									std::string path = AppWindow::open_file_dialog(image_file_filter, image_file_filter_size);
									if (path != "") {
										LightEngine::Texture temp(core, path);
										pbr_material.assign_ao_map(std::make_shared<LightEngine::Texture>(temp));
										pbr_ao_btn = temp.get_name();
										update_material = true;
									}
								}

								ImGui::EndMenu();
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

						ImGui::EndMainMenuBar();
					}					


					material_editor.draw();

					ImGui::Begin("Material ",nullptr);

					ImGui::End();
					


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
						
						if (AppWindow::IO::Mouse::left_button_down()) {
							arcball_camera.modify_vertical_angle(-cursor_x_delta * ORBITING_SENSITIVITY);
							arcball_camera.modify_horizontal_angle(-cursor_y_delta * ORBITING_SENSITIVITY);
							update_camera = true;
						}

						if (AppWindow::IO::Mouse::middle_button_down()) {
							arcball_camera.pan_horizontal(-cursor_x_delta * PANNING_SENSITIVITY);
							arcball_camera.pan_vertical(cursor_y_delta * PANNING_SENSITIVITY);
							update_camera = true;
						}

						if (wheel_d) {
							arcball_camera.modify_radius(-wheel_d * ZOOM_SENSITIVITY);
							update_camera = true;
						}
					} else {

						if (AppWindow::IO::Mouse::left_button_down()) {
							fps_camera.modify_vertical_angle(-cursor_x_delta * FPS_SENSITIVITY);
							fps_camera.modify_horizontal_angle(-cursor_y_delta * FPS_SENSITIVITY);
							update_camera = true;
						}

						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::S)) {
							fps_camera.move_relative_z(1.0);
							update_camera = true;
						}
					
						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::W)) {
							fps_camera.move_relative_z(-1.0);
							update_camera = true;
						}

						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::A)) {
							fps_camera.move_relative_x(1.0);
							update_camera = true;
						}
					
						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::D)) {
							fps_camera.move_relative_x(-1.0);
							update_camera = true;
						}

						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::Q)) {
							fps_camera.move_y(1.0);
							update_camera = true;
						}
					
						if (AppWindow::IO::Keyboard::key_down(AppWindow::IO::Key::E)) {
							fps_camera.move_y(-1.0);
							update_camera = true;
						}
					}				
				}

				if (update_camera) {

					if (camera == 0) 
						fps_camera.update();			
					else 
						arcball_camera.update();

					update_camera = false;
				}

				if(update_light){
					point_light.update();
					direct_light.update();
					update_light = false;
				}

				if(update_material){
					//default_material.update();
					pbr_material.update();
					update_material = false;
				}

				if(update_viewport){		
					int vp_width = window.get_width() + vp_const_width_offset;
					int vp_height = window.get_height() + vp_const_height_offset;

					core->viewport_setup(vp_tl_x,vp_tl_y,vp_width, vp_height);
					arcball_camera.set_scaling(vp_width, vp_height);
					fps_camera.set_scaling(vp_width, vp_height);
					arcball_camera.update();
					fps_camera.update();
					update_viewport = false;
				}

				core->clear_back_buffer(color);

				//default_material.bind();
				pbr_material.bind();

				for (int i = 0; i < scene.size(); i++) {
					scene[i].bind_topology();
					scene[i].bind_vertex_buffer();
					scene[i].draw(0);
				}

				hud_vs.bind();
				hud_ps.bind();

				viewport_border.bind_topology();
				viewport_border.bind_vertex_buffer();
				viewport_border.draw(0);

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