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

		LightEngine::VertexShader vs(core, shader_directory + L"CameraVS.cso");
		LightEngine::VertexShader hud_vs(core, shader_directory + L"ViewportHUD_VS.cso");
		LightEngine::VertexShader shadow_mapping_vs(core, shader_directory + L"ShadowMappingVS.cso");

		LightEngine::PixelShader blinn_ps(core, shader_directory + L"BlinnPS.cso");
		LightEngine::PixelShader phong_ps(core, shader_directory + L"PhongPS.cso");
		LightEngine::PixelShader gouraud_ps(core, shader_directory + L"GouraudPS.cso");
		LightEngine::PixelShader pbr_ps(core, shader_directory + L"PBRShader.cso");	
		LightEngine::PixelShader hud_ps(core, shader_directory + L"ViewportHUD_PS.cso");
		LightEngine::PixelShader shadow_mapping_ps(core, shader_directory + L"ShadowMappingPS.cso");
		LightEngine::PixelShader mc_gi(core, shader_directory + L"MonteCarloGI.cso");
		LightEngine::ComputeShader cs_(core, shader_directory + L"MonteCarloGI_CS.cso");
		//LightEngine::Texture2D texture_a(core, "B:\\CG Projects\\Tekstury\\PavingStones094_1K-PNG\\PavingStones094_1K_AmbientOcclusion.png");
		//LightEngine::Texture2D texture_b(core, "B:\\CG Projects\\Tekstury\\blue.jpg");

		static float blank[1000000];
		std::vector<float> test_1 = create_test_texture_3d(500, 500);
		std::vector<float> test_2 = create_test_texture_3d(250, 250);

		LightEngine::Texture3D texture_a(core, "CS Out", 500, 500, 3, test_1.data());
		LightEngine::Texture3D texture_b(core, "CS Out", 250, 250, 3, test_2.data());

		LightEngine::Texture2D output_texture(core, "CS Out", 500, 500, blank);
		LightEngine::ShaderResourceManager sr_manager(core);

		cs_.bind();
		sr_manager.bind_texture_buffer(texture_a, LightEngine::ShaderType::ComputeShader, 2u);
		sr_manager.bind_texture_buffer(texture_b, LightEngine::ShaderType::ComputeShader, 3u);
		sr_manager.bind_cs_unordered_access_buffer(output_texture, 0u);
		cs_.run();
		sr_manager.unbind_cs_unordered_access_buffer(0u);
		output_texture.generate_mip_maps();
		sr_manager.bind_texture_buffer(output_texture, LightEngine::ShaderType::PixelShader, 0u);

		std::shared_ptr<LightEngine::PixelShader> blinn_ps_ptr = std::make_shared<LightEngine::PixelShader>(blinn_ps);
		std::shared_ptr<LightEngine::PixelShader> phong_ps_ptr = std::make_shared<LightEngine::PixelShader>(phong_ps);
		std::shared_ptr<LightEngine::PixelShader> gouraud_ps_ptr = std::make_shared<LightEngine::PixelShader>(gouraud_ps);
		std::shared_ptr<LightEngine::PixelShader> pbr_ps_ptr = std::make_shared<LightEngine::PixelShader>(pbr_ps);
		std::shared_ptr<LightEngine::PixelShader> mc_gi_ptr = std::make_shared<LightEngine::PixelShader>(mc_gi);


		LightEngine::FPSCamera fps_camera(core);
		fps_camera.update();

		LightEngine::ArcballCamera arcball_camera(core, 50.0);
		LightEngine::ArcballCamera light_camera(core, 50.0);

		LightEngine::LightSource point_light(core, 1.0);
		LightEngine::LightSource direct_light(core, 0.0);
	
		LightEngine::Materials::BasicMaterial default_material(core, "Default00");
		LightEngine::Materials::PBRMaterial pbr_material(core, "PBR_Metallic_Roughness");
		LightEngine::Sampler sampler_nearest(core, LightEngine::Sampler::Filtering::NEAREST);
		LightEngine::Sampler sampler_bilinear(core, LightEngine::Sampler::Filtering::BILINEAR);
		LightEngine::Sampler sampler_trilinear(core, LightEngine::Sampler::Filtering::TRILINEAR);
		LightEngine::Sampler sampler_anisotropic(core, LightEngine::Sampler::Filtering::ANISOTROPIC);
		LightEngine::RenderableTexture shadow_map(core, "Shadow map", 512, 512);
		
		
		LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture> texture_browser_model;
		LightEngineUI::Backend::BrowserModel<LightEngine::Materials::BasicMaterial> bm_browser_model;
		std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Materials::BasicMaterial>> bm_browser_model_ptr = std::make_shared<LightEngineUI::Backend::BrowserModel<LightEngine::Materials::BasicMaterial>>(bm_browser_model);
		std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>> tbm_ptr = std::make_shared<LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>>(texture_browser_model);
		LightEngineUI::Frontend::TextureBrowser texture_browser(tbm_ptr,core);
		LightEngineUI::Frontend::MaterialEditor material_editor(tbm_ptr);
		LightEngineUI::Frontend::BasicMaterialsBrowser bm_browser(bm_browser_model_ptr,core);



		std::vector<LightEngine::Vertex3> border_vertices{
			{{-0.99,0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{0.99,0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{0.99,-0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{-0.99,-0.99,0.0},{0.4,0.4,0.4,1.0}},
			{{-0.99,0.99,0.0},{0.4,0.4,0.4,1.0}}};

		std::vector<LightEngine::Vertex3> shadow_map_display_vertices{
			{{-0.99,-0.99,0.0},{0.4,0.4,0.4,-1.0},{0,1,0}},
			{{-0.99,0.0,0.0},{0.4,0.4,0.4,-1.0},{0,0,0}},
			{{-0.4,-0.99,0.0},{0.4,0.4,0.4,-1.0},{1,1,0}},			
			{{-0.4,0.0,0.0},{0.4,0.4,0.4,-1.0},{1,0,0}}};

		std::vector<LightEngine::Vertex3> test_plane_vtx{
			{{25.0, 0.0, -25.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}},
			{{-25.0, 0.0, 25.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}},
			{{25.0, 0.0, 25.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}},
			{{-25.0, 0.0, -25.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}},
			{{-25.0, 0.0, 25.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}},
			{{25.0, 0.0, -25.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}}
		};

		LightEngine::Geometry<LightEngine::Vertex3> viewport_border(core, border_vertices, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, "border");
		LightEngine::Geometry<LightEngine::Vertex3> shadow_map_display(core, shadow_map_display_vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,"shadow_map_display");
		LightEngine::Geometry<LightEngine::Vertex3> test_plane(core, test_plane_vtx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, "test_plane");

		//scene.push_back(test_plane);

		direct_light.set_position(direct_light_position);
		default_material.assign_vertex_shader(std::make_shared<LightEngine::VertexShader>(vs));
		default_material.assign_pixel_shader(blinn_ps_ptr);

		std::shared_ptr<LightEngine::Materials::BasicMaterial> default_ptr = std::make_shared<LightEngine::Materials::BasicMaterial>(default_material);
		std::shared_ptr<LightEngine::Materials::PBRMaterial> pbr_ptr = std::make_shared<LightEngine::Materials::PBRMaterial>(pbr_material);	

		material_editor.load_material(default_ptr);

		default_ptr->assign_vertex_shader(std::make_shared<LightEngine::VertexShader>(vs));
		default_ptr->assign_pixel_shader(blinn_ps_ptr);

		point_light.bind_ps_buffer(1);
		point_light.bind_vs_buffer(1);
		direct_light.bind_ps_buffer(2);
		direct_light.bind_vs_buffer(2);

		sampler_anisotropic.bind(0);

		direct_light.update();
		
		//arcball_camera.modify_center(0,7.0f,0);
		arcball_camera.modify_horizontal_angle(-30.0f);
		arcball_camera.modify_vertical_angle(30.0f);
		arcball_camera.update_view_matrix();
		arcball_camera.update();
		
		arcball_camera.bind(0);

		//light_camera.modify_center(0.0,0.0,1.0);
		//light_camera.modify_horizontal_angle(-90.0f);
		light_camera.set_clipping_near(lc_near_z);
		light_camera.set_clipping_far(lc_far_z);
		light_camera.set_fov(lc_fov);
		light_camera.update_view_matrix();
		light_camera.update();
		
		light_camera.bind(4);

		//bind light-perspective camera

		int vp_width = 0;
		int vp_height = 0;
		

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
					core->setup_frame_buffer(window.get_width(),window.get_height(), false);
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

						ImGui::EndMainMenuBar();
					}					

					ImGui::Begin("Camera");

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


					ImGui::End();



					ImGui::Begin("Light Camera");

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

					ImGui::End();




					ImGui::Begin("Light Sources");

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

					ImGui::End();
						

					material_editor.render();
					texture_browser.render();
					bm_browser.render();

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
								light_camera.modify_vertical_angle(-cursor_x_delta * ORBITING_SENSITIVITY);
								light_camera.modify_horizontal_angle(-cursor_y_delta * ORBITING_SENSITIVITY);
								light_camera.update_view_matrix();
								update_camera = true;
							}

							if (AppWindow::IO::Mouse::middle_button_down()) {
								light_camera.pan_horizontal(-cursor_x_delta * PANNING_SENSITIVITY);
								light_camera.pan_vertical(cursor_y_delta * PANNING_SENSITIVITY);
								light_camera.update_view_matrix();
								update_camera = true;
							}

							if (wheel_d) {
								light_camera.modify_radius(-wheel_d * ZOOM_SENSITIVITY*0.1);
								light_camera.update_view_matrix();
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

					light_camera.update();
					update_camera = false;
				}

				if(update_light){
					point_light.update();
					direct_light.update();
					update_light = false;
				}

				if(update_material){
					default_ptr->update();
					//pbr_ptr->update();
					update_material = false;
				}

				if(update_viewport){		
					vp_width = window.get_width() + vp_const_width_offset;
					vp_height = window.get_height() + vp_const_height_offset;

					core->viewport_setup(vp_tl_x,vp_tl_y,vp_width, vp_height);
					arcball_camera.set_aspect_ratio((float)vp_width/(float)vp_height);
					arcball_camera.update_projection_matrix();
					light_camera.set_aspect_ratio((float)vp_width/(float)vp_height);
					light_camera.update_projection_matrix();
					fps_camera.set_aspect_ratio((float)vp_width/(float)vp_height);
					fps_camera.update_projection_matrix();

					arcball_camera.update();
					fps_camera.update();
					light_camera.update();
					update_viewport = false;
				}

				core->clear_frame_buffer(color);
				

				//default_ptr->bind();

				//scene[0].bind_topology();
				//scene[0].bind_vertex_buffer();
				//scene[0].draw(0);	

				

				for (int i = 0; i < scene.size(); i++) {
											
					default_ptr->bind();

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
