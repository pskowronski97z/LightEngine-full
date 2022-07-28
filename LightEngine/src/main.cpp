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
#include <FilePaths.h>
#include <GI_Tools.h>
#define USE_RSM

int main(int argc, const char** argv) {

	AppWindow::Window window("LightEngine v1.0", 1000, 640);
	window.set_style(AppWindow::Style::MAXIMIZE);
	//window.set_style(AppWindow::Style::MINIMIZE);
	window.set_style(AppWindow::Style::SIZEABLE);
	window.set_icon("LE.ico");

	float color[4]{ 0.15,0.15,0.15 };
	MSG msg = { 0 };

	window.show_window();

	std::vector<LightEngine::Geometry<LightEngine::Vertex3>> scene;

	try {

		auto core = std::make_shared<LightEngine::Core>(window.get_handle(), window.get_width(), window.get_height());

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(window.get_handle());
		ImGui_ImplDX11_Init(core->get_device_ptr().Get(), core->get_context_ptr().Get());
		ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
		std::wstring shader_directory(COMPILED_SHADERS_DIR);

		LightEngine::VertexShader vs(core, shader_directory + L"BasicVS.cso");
		LightEngine::PixelShader frame_ps(core, shader_directory + L"FinalFramePS.cso");
		LightEngine::PixelShader low_res_ps(core, shader_directory + L"LowResExtPS.cso");
		LightEngine::PixelShader gbuffer_ps(core, shader_directory + L"GBufferPS.cso");
		
		LightEngine::FPSCamera fps_camera(core);
		fps_camera.update();

		LightEngine::ArcballCamera direct_light_pov(core, 50.0);

		LightEngine::LightSource point_light(core, 1.0);
		LightEngine::LightSource direct_light(core, 0.0);

		LightEngine::Materials::BasicMaterial default_material(core, "Default00");
		LightEngine::Materials::PBRMaterial pbr_material(core, "PBR_Metallic_Roughness");
		LightEngine::Sampler sampler_nearest(core, LightEngine::Sampler::Filtering::NEAREST);
		LightEngine::Sampler sampler_bilinear(core, LightEngine::Sampler::Filtering::BILINEAR);
		LightEngine::Sampler sampler_trilinear(core, LightEngine::Sampler::Filtering::TRILINEAR);
		LightEngine::Sampler sampler_anisotropic(core, LightEngine::Sampler::Filtering::ANISOTROPIC);
		
		
		//point_light.bind_ps_buffer(1);
		//point_light.bind_vs_buffer(1);
		//point_light.update();

		
		//direct_light.bind_vs_buffer(2);

		sampler_anisotropic.bind(0);

		direct_light_pov.set_orthographic_projection();
		direct_light_pov.modify_horizontal_angle(-30.0f);
		direct_light_pov.modify_vertical_angle(30.0f);
		direct_light_pov.update_view_matrix();
		direct_light_pov.update();

		//direct_light_pov.bind(0);
		//direct_light_pov.bind(1);

		LightEngine::ShaderResourceManager shader_resource_manager(core);

		#ifdef USE_RSM

		const int gbuffer_w = 512;
		const int gbuffer_h = 512;
		const int low_res_map_w = 492;
		const int low_res_map_h = 301;

		static float zeros[4 * gbuffer_w * gbuffer_h];
		ZeroMemory(zeros, 4 * gbuffer_w * gbuffer_h);

		LightEngine::Texture2D gbuffer_world_position(core, "GBuffer world position", gbuffer_w, gbuffer_h, zeros);
		LightEngine::Texture2D gbuffer_normal(core, "GBuffer normal", gbuffer_w, gbuffer_h, zeros);
		LightEngine::Texture2D gbuffer_depth(core, "GBuffer depth", gbuffer_w, gbuffer_h, zeros);
		LightEngine::Texture2D gbuffer_flux(core, "GBuffer flux", gbuffer_w, gbuffer_h, zeros);
		LightEngine::Texture2D low_res_map(core, "Low resolution GI map", low_res_map_w, low_res_map_h, zeros);

		std::vector<const LightEngine::AbstractTexture*> gbuffer(4u);
		std::vector<const LightEngine::AbstractTexture*> gi_map(1u);

		gbuffer[0] = &gbuffer_world_position;
		gbuffer[1] = &gbuffer_normal;
		gbuffer[2] = &gbuffer_depth;
		gbuffer[3] = &gbuffer_flux;
		gi_map[0] = &low_res_map;

		direct_light_pov.set_aspect_ratio((float)gbuffer_w / (float)gbuffer_h);
		direct_light_pov.update_projection_matrix();
		direct_light_pov.update();

		//core->add_texture_to_render(gbuffer_world_position, 0u);
		//core->add_texture_to_render(gbuffer_normal, 1u);
		//core->add_texture_to_render(gbuffer_depth, 2u);
		//core->add_texture_to_render(gbuffer_flux, 3u);

		// Generating the sampling pattern
		std::vector<float> rsm_sampling_pattern(GI::RSM::generate_sampling_pattern(10000u));
		LightEngine::Texture2D rsm_sampling_pattern_tx(core, "RSM sampling pattern", 10000u, 1u, rsm_sampling_pattern.data());
		shader_resource_manager.bind_texture_buffer(rsm_sampling_pattern_tx, LightEngine::ShaderType::PixelShader, 3u);
		
		#endif

		while (WM_QUIT != msg.message) {

			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {

				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				//ImGui::DockSpaceOverViewport(0, ImGuiDockNodeFlags_PassthruCentralNode, 0);

				if (window.was_resized()) {

					std::cout << window.get_width() << " x " << window.get_height() << std::endl;
					core->setup_frame_buffer(window.get_width(), window.get_height(), false);
					core->render_to_frame_buffer();
					update_viewport = true;

				}
				// GUI Scope
				{

					if (ImGui::BeginMainMenuBar()) {

						if (ImGui::BeginMenu("File")) {

							if (ImGui::MenuItem("Import")) {
								std::string path = AppWindow::open_file_dialog(model_file_filter, model_file_filter_size);
								if (path != "") {
									const float **colors = new const float*[4]; 
									colors[0] = new float[3]{ 1.0, 1.0, 1.0 };
									colors[1] = new float[3]{ 1.0, 1.0, 0.4 };
									colors[2] = new float[3]{ 0.2, 0.6, 1.0 };
									colors[3] = new float[3]{ 1.0, 0.5, 0.5 };

									std::vector<LightEngine::Geometry<LightEngine::Vertex3>> object_set = LightEngine::Geometry<LightEngine::Vertex3>::load_from_obj(core, path, colors, 4);
									for (auto& object : object_set) {
										scene.push_back(object);
									}
								}
							}
							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Settings")) {

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

					if (ImGui::RadioButton("Direct light POV", &camera, 1)) {
						std::cout << "Direct light POV selected" << std::endl;
						direct_light_pov.bind(0);
						update_camera = true;
					}

					if (camera) {
						if (ImGui::SliderInt("Scaling", &orthographic_scaling, 1, 100)) {
							direct_light_pov.set_orthographic_scaling((float)orthographic_scaling);
							direct_light_pov.update_projection_matrix();
							update_camera = true;
						}

						if (ImGui::SliderInt("Far Z", &lc_far_z, lc_near_z + 1, 500)) {
							direct_light_pov.set_clipping_far((float)lc_far_z);
							direct_light_pov.update_projection_matrix();
							update_camera = true;
						}

						if (ImGui::SliderInt("Near Z", &lc_near_z, 0, lc_far_z - 1)) {
							direct_light_pov.set_clipping_near((float)lc_near_z);
							direct_light_pov.update_projection_matrix();
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
							direct_light_pov.reset();
						else
							fps_camera.reset();
						update_camera = true;
					}


					ImGui::End();


					ImGui::Begin("Light Sources");

					/*if (ImGui::BeginMenu("Point light")) {
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
					*/
					//if (ImGui::BeginMenu("Direct light")) {
						if (ImGui::ColorPicker3("Color", direct_light_color)) {
							direct_light.set_color(direct_light_color);
							update_light = true;
						}

						if (ImGui::DragFloat("Intensity", &direct_light_intensity, 0.05f, 0.0f, 100.0f, "%.2f")) {
							direct_light.set_intensity(direct_light_intensity);
							update_light = true;
						}

						//ImGui::EndMenu();
					//}

					ImGui::End();
					ImGui::Render();
				}

				// IO Scope
				if (!ImGui::GetIO().WantCaptureMouse) {

					cursor_x_delta = AppWindow::IO::Mouse::get_position_x() - cursor_x_buffer;
					cursor_x_buffer = AppWindow::IO::Mouse::get_position_x();

					cursor_y_delta = AppWindow::IO::Mouse::get_position_y() - cursor_y_buffer;
					cursor_y_buffer = AppWindow::IO::Mouse::get_position_y();

					wheel_d = AppWindow::IO::Mouse::get_wheel_delta();


					if (camera) {

						if (AppWindow::IO::Mouse::left_button_down()) {
							direct_light_pov.modify_vertical_angle(-cursor_x_delta * ORBITING_SENSITIVITY);
							direct_light_pov.modify_horizontal_angle(-cursor_y_delta * ORBITING_SENSITIVITY);
							direct_light_pov.update_view_matrix();
							update_camera = true;
						}

						if (AppWindow::IO::Mouse::middle_button_down()) {
							direct_light_pov.pan_horizontal(-cursor_x_delta * PANNING_SENSITIVITY);
							direct_light_pov.pan_vertical(cursor_y_delta * PANNING_SENSITIVITY);
							direct_light_pov.update_view_matrix();
							update_camera = true;
						}

						if (wheel_d) {
							direct_light_pov.modify_radius(-wheel_d * ZOOM_SENSITIVITY);
							direct_light_pov.update_view_matrix();
							update_camera = true;
						}

					}
					else {

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
						direct_light_pov.update();


					update_camera = false;
				}

				if (update_light) {
					point_light.update();
					direct_light.update();
					update_light = false;
				}


				if (update_viewport) {

					int vp_width = window.get_width();
					int vp_height = window.get_height();

					core->viewport_setup(0, 0, vp_width, vp_height);
					
					fps_camera.set_aspect_ratio((float)vp_width / (float)vp_height);
					fps_camera.update_projection_matrix();			
					fps_camera.update();
					update_viewport = false;
				}

				core->clear_frame_buffer(color);
				
				

				#ifdef USE_RSM
				{
					for (auto& tx : gbuffer)
						tx->clear();

					for (auto& tx : gi_map)
						tx->clear();

					low_res_map.clear();

					// Baking the G-Buffer

					vs.bind();
					direct_light_pov.bind_vs_buffer(0u);
				
					gbuffer_ps.bind();
					direct_light_pov.bind_ps_buffer(0u);
					direct_light.bind_ps_buffer(1u);
					
					core->render_to_textures(gbuffer, 4u);

					core->viewport_setup(0, 0, gbuffer_w, gbuffer_h);

					for (int i = 0; i < scene.size(); i++) {
						scene[i].bind_vertex_buffer();
						scene[i].bind_topology();
						scene[i].draw(0);
					}

					// Rendering the GI map

					core->render_to_textures(gi_map, 1u);
					low_res_ps.bind();
					direct_light_pov.bind_ps_buffer(0);
					fps_camera.bind_vs_buffer(0);

					gbuffer_world_position.generate_mip_maps();
					gbuffer_normal.generate_mip_maps();
					gbuffer_flux.generate_mip_maps();

					shader_resource_manager.bind_texture_buffer(gbuffer_world_position, LightEngine::ShaderType::PixelShader, 0u);
					shader_resource_manager.bind_texture_buffer(gbuffer_normal, LightEngine::ShaderType::PixelShader, 1u);
					shader_resource_manager.bind_texture_buffer(gbuffer_flux, LightEngine::ShaderType::PixelShader, 2u);

					core->viewport_setup(0, 0, low_res_map_w, low_res_map_h);

					for (int i = 0; i < scene.size(); i++) {
						scene[i].bind_vertex_buffer();
						scene[i].bind_topology();
						scene[i].draw(0);
					}

					shader_resource_manager.unbind_texture_buffer(LightEngine::ShaderType::PixelShader, 0u);
					shader_resource_manager.unbind_texture_buffer(LightEngine::ShaderType::PixelShader, 1u);
					shader_resource_manager.unbind_texture_buffer(LightEngine::ShaderType::PixelShader, 2u);
				
					// Rendering the frame
					core->render_to_frame_buffer();
					
					if (camera) {
						direct_light_pov.bind_vs_buffer(0);
						core->viewport_setup(0, 0, gbuffer_w, gbuffer_h);
					}
					else {
						fps_camera.bind_vs_buffer(0);
						core->viewport_setup(0, 0, window.get_width(), window.get_height());
					}
								
					frame_ps.bind();
					// direct_light_pov.bind_ps_buffer(0);

					gbuffer_depth.generate_mip_maps();
					shader_resource_manager.bind_texture_buffer(low_res_map, LightEngine::ShaderType::PixelShader, 0u);
					shader_resource_manager.bind_texture_buffer(gbuffer_depth, LightEngine::ShaderType::PixelShader, 1u);
								
					for (int i = 0; i < scene.size(); i++) {
						scene[i].bind_vertex_buffer();
						scene[i].bind_topology();
						scene[i].draw(0);
					}

					shader_resource_manager.unbind_texture_buffer(LightEngine::ShaderType::PixelShader, 0u);
					shader_resource_manager.unbind_texture_buffer(LightEngine::ShaderType::PixelShader, 1u);
								
				}
				#endif 

				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				core->present_frame();
			}
		}
	}
	catch (LightEngine::LECoreException& e) {
		std::cout << e.what();
		std::wcout << e.get_message();

		std::cin.get();
	}
	std::cin.get();
	return 0;
}