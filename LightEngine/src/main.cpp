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


int main(int argc, const char** argv) {

	AppWindow::Window window("LightEngine v1.0", 1000, 640);
	window.set_style(AppWindow::Style::MAXIMIZE);
	//window.set_style(AppWindow::Style::MINIMIZE);
	//window.set_style(AppWindow::Style::SIZEABLE);
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

		LightEngine::VertexShader vs(core, shader_directory + L"CameraVS.cso");
		LightEngine::PixelShader diffuse_ps(core, shader_directory + L"DirectDiffusePS.cso");
		
		LightEngine::FPSCamera fps_camera(core);
		fps_camera.update();

		LightEngine::ArcballCamera arcball_camera(core, 50.0);

		LightEngine::LightSource point_light(core, 1.0);
		LightEngine::LightSource direct_light(core, 0.0);

		LightEngine::Materials::BasicMaterial default_material(core, "Default00");
		LightEngine::Materials::PBRMaterial pbr_material(core, "PBR_Metallic_Roughness");
		LightEngine::Sampler sampler_nearest(core, LightEngine::Sampler::Filtering::NEAREST);
		LightEngine::Sampler sampler_bilinear(core, LightEngine::Sampler::Filtering::BILINEAR);
		LightEngine::Sampler sampler_trilinear(core, LightEngine::Sampler::Filtering::TRILINEAR);
		LightEngine::Sampler sampler_anisotropic(core, LightEngine::Sampler::Filtering::ANISOTROPIC);
		
		direct_light.set_position(direct_light_position);
		
		point_light.bind_ps_buffer(1);
		point_light.bind_vs_buffer(1);
		direct_light.bind_ps_buffer(2);
		direct_light.bind_vs_buffer(2);

		sampler_anisotropic.bind(0);

		point_light.update();

		//arcball_camera.modify_center(0,7.0f,0);
		arcball_camera.modify_horizontal_angle(-30.0f);
		arcball_camera.modify_vertical_angle(30.0f);
		arcball_camera.update_view_matrix();
		arcball_camera.update();

		arcball_camera.bind(0);

		

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

				ImGui::DockSpaceOverViewport(0, ImGuiDockNodeFlags_PassthruCentralNode, 0);

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
									std::vector<LightEngine::Geometry<LightEngine::Vertex3>> object_set = LightEngine::Geometry<LightEngine::Vertex3>::load_from_obj(core, path, nullptr, 0);
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
						arcball_camera.update();


					update_camera = false;
				}

				if (update_light) {
					point_light.update();
					direct_light.update();
					update_light = false;
				}


				if (update_viewport) {
					vp_width = window.get_width();
					vp_height = window.get_height();

					core->viewport_setup(0, 0, vp_width, vp_height);
					arcball_camera.set_aspect_ratio((float)vp_width / (float)vp_height);
					arcball_camera.update_projection_matrix();
					fps_camera.set_aspect_ratio((float)vp_width / (float)vp_height);
					fps_camera.update_projection_matrix();

					arcball_camera.update();
					fps_camera.update();
					update_viewport = false;
				}

				core->clear_frame_buffer(color);

				for (int i = 0; i < scene.size(); i++) {

					vs.bind();
					diffuse_ps.bind();

					scene[i].bind_vertex_buffer();
					scene[i].bind_topology();
					scene[i].draw(0);
				}

				

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