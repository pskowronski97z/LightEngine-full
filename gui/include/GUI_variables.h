#pragma once

#define ORBITING_SENSITIVITY 0.5
#define FPS_SENSITIVITY 0.2
#define PANNING_SENSITIVITY 0.04
#define ZOOM_SENSITIVITY 5

constexpr auto model_file_filter_size = 1;
constexpr auto image_file_filter_size = 11;

COMDLG_FILTERSPEC model_file_filter[model_file_filter_size] = { {L"Wavefront OBJ", L"*.obj"} };
COMDLG_FILTERSPEC image_file_filter[image_file_filter_size] = {
		{L"Windows bitmaps", L"*.bmp; *.dib"},
		{L"JPEG files", L"*.jpeg; *.jpg; *.jpe"},
		{L"JPEG 2000 files", L"*.jp2"},
		{L"Portable Network Graphics", L"*.png"},
		{L"WebP", L"*.webp"},
		{L"Portable image format", L"*.pbm; *.pgm; *.ppm; *.pxm; *.pnm"},
		{L"PFM files", L"*.pfm"},
		{L"Sun rasters", L"*.sr; *.ras"},
		{L"TIFF files", L"*.tiff; *.tif"},
		{L"OpenEXR Image files", L"*.exr"},
		{L"Radiance HDR", L"*.hdr, *.pic"}
};

float specular_color[]{ 1.0, 1.0, 1.0 };
float diffuse_color[]{ 1.0, 1.0, 1.0 };
float ambient_color[]{ 0.0, 0.0, 0.0 };
float albedo[]{ 0.0, 0.0, 0.0 };
float point_light_color[]{ 1.0, 1.0, 1.0 };
float point_light_position[]{ 0, 0, -0.8 };
float point_light_intensity = 1.0;
float direct_light_color[]{ 1.0, 1.0, 1.0 };
float direct_light_position[]{ 0, 0, 1.0 };
float direct_light_intensity = 1.0;
float attenuation[]{0.3, 0.3, 1.0};
float roughness = 0.0;
float metallic = 0.0;


int glossiness = 400;
int world_fov = 45;
int arcball_fov = 45;
int horizontal_angle = 0;
int vertical_angle = 0;
int camera = 1;
int shading = 0;
int cursor_x_buffer = 0;
int cursor_x_delta = 0;
int cursor_y_buffer = 0;
int cursor_y_delta = 0;
int wheel_d = 0;
int texture_filtering = 3;
int vp_tl_x = 269;
int vp_tl_y = 18;
int vp_const_width_offset = -519;
int vp_const_height_offset = -277;

bool update_camera = false;
bool update_material = false;
bool update_light = false;
bool update_viewport = false;
bool toogle_diffuse = false;
bool toogle_ambient = false;
bool is_diffuse_map_used = false;
bool is_normal_map_used = false;
bool flip_x = false;
bool flip_y = false;


std::string load_dmap_btn_lbl = "Load diffuse map";
std::string load_nmap_btn_lbl = "Load normal map";

std::string pbr_albedo_btn = "Load albedo map";
std::string pbr_roughness_btn = "Load roughness map";
std::string pbr_metalness_btn = "Load metalness map";
std::string pbr_normal_btn = "Load normal map";
std::string pbr_ao_btn = "Load ambient occlusion map";