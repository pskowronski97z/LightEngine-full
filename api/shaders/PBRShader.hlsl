struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 gouraud_shading : VT3_COLOR1;
    float4 light_space_position : VT3_COLOR2;
};

struct LIGHT_SOURCE {
    float4 coordinates;
    float4 color;
    float intensity;
};

cbuffer CAMERA : register(b0) {
    float4 camera_data_;
    matrix camera_matrix_;
    matrix projection_matrix_;
}

cbuffer POINT_LIGHT : register(b1) {
    float4 coordinates_pt;
    float4 color_pt;
    float4 additional_pt;
};

cbuffer DIRECT_LIGHT : register(b2) {
    float4 coordinates_dir;
    float4 color_dir;
    float4 additional_dir;
};

cbuffer MATERIAL : register(b3) {
    float4 albedo;
    float roughness;
    float metalness;
    float ambient_occlusion;
    bool use_albedo_map;
    bool use_roughness_map;
    bool use_metalness_map;
    bool use_normal_map;
    bool use_ao_map;
};

cbuffer LIGHT_CAMERA : register(b4) {
    float4 light_camera_data_;
    matrix light_camera_matrix_;
    matrix light_projection_matrix_;
}

Texture2D ALBEDO_MAP : register(t0);
Texture2D ROUGHNESS_MAP : register(t1);
Texture2D METALNESS_MAP : register(t2);
Texture2D NORMAL_MAP : register(t3);
Texture2D AO_MAP : register(t4);
Texture2D SHADOW_MAP : register(t5);



SamplerState DEFAULT_SAMPLER : register(s0);

static const float PI = 3.14159265f;

float schlick_ggx_geometry(float3 n, float3 v, float a) {
    float a_plus_one = a + 1.0f;
    float n_dot_v = max(0.0f, dot(n, v));
    float k = (a_plus_one * a_plus_one) / 8.0f; 
    return 1.0f / ((n_dot_v * (1.0f - k)) + k);   
}

float trowbridge_reitz_ggx_ndf(float3 n, float3 h, float a) {   
    float a_2 = a * a;
    float n_dot_h = max(dot(n, h), 0.0f);
    float n_dot_h_2 = n_dot_h * n_dot_h;
    float denominator = (n_dot_h_2 * (a_2 - 1.0f)) + 1.0f;
    denominator = PI * denominator * denominator; 
    return a_2 / denominator;
}

float3 schlick_fresnel(float3 h, float3 v, float3 fragment_color, float metallic) {
    float3 f0 = lerp(0.04, fragment_color, metallic);
    return f0 + ((1.0 - f0) * pow(saturate(1.0f - max(0.0f, dot(h, v))), 5.0f));
}

float3 cook_torrance(float3 n, float3 l, float3 v, float3 h, float3 fragment_color, float roughness, float metalness) {
    float a = roughness * roughness;
    float ndf = trowbridge_reitz_ggx_ndf(n, h, a);
    float geo = schlick_ggx_geometry(n, v, a) * schlick_ggx_geometry(n, l, a); // Using Schlick GGX for Smith's geometry calculation
    float3 fresnel = schlick_fresnel(h, v, fragment_color, metalness);
    float3 kD = 1.0f - fresnel;
    
    kD *= (1.0f - metalness);
    
    return (kD * fragment_color / PI) + ((ndf * geo * fresnel) / 4.0f);
}

float3 get_reflectance_pt(PS_INPUT pixel, LIGHT_SOURCE point_light, float roughness, float metalness) {   
    // Data preprocessing
    float3 v = pixel.world_position;
    float3 l = point_light.coordinates - v;
    float d = length(l);
    
    v = -normalize(v);
    l = normalize(l);
    
    float3 h = normalize(l + v);
    float3 n = normalize(pixel.normal);   
    float3 radiance = point_light.intensity * point_light.color * (1.0f / (d * d));
    float3 brdf_value = cook_torrance(n, l, v, h, pixel.color, roughness, metalness);
    
    return brdf_value * radiance * max(0.0f, dot(l, n));
}

float3 get_reflectance_dir(PS_INPUT pixel, LIGHT_SOURCE direct_light, float roughness, float metalness) {
    // Data preprocessing
    float3 v = pixel.world_position;  
    float3 l = -direct_light.coordinates;
    
    v = -normalize(v);
    l = normalize(l);
    
    float3 h = normalize(l + v);
    float3 n = normalize(pixel.normal);
    
    float3 radiance = direct_light.intensity * direct_light.color; 
    float3 brdf_value = cook_torrance(n, l, v, h, pixel.color, roughness, metalness);
    return brdf_value * radiance * max(0.0f, dot(l, n));
}

float3 get_mapped_normal(PS_INPUT pixel) { 
    float4 normal_sample = NORMAL_MAP.Sample(DEFAULT_SAMPLER, float2(pixel.uvw[0], pixel.uvw[1]));      
    float3x3 tbn_matrix = float3x3(normalize(pixel.tangent), normalize(pixel.bitangent), normalize(pixel.normal));
    
    normal_sample = normalize(normal_sample * 2.0 - 1.0);
    pixel.normal = mul(normal_sample, tbn_matrix);
    
    return normalize(pixel.normal);
}

float get_shadowing_value(float4 light_space_position) {
    
    light_space_position /= light_space_position[3];
    
    float u = light_space_position[0] * 0.5 + 0.5;
    float v = 1.0 - light_space_position[1] * 0.5 + 0.5;
    
    float sampled_z = SHADOW_MAP.Sample(DEFAULT_SAMPLER, float2(u, v));
    
    return sampled_z + 0.002 < light_space_position[2] ? 0 : 1;
    
}

float4 main(PS_INPUT input) : SV_TARGET {
    
    LIGHT_SOURCE point_light;
    point_light.color = color_pt;
    point_light.coordinates = mul(coordinates_pt, camera_matrix_);
    point_light.intensity = additional_pt[0];// * get_shadowing_value(input.light_space_position);
    
    LIGHT_SOURCE direct_light;
    direct_light.color = color_dir;
    direct_light.coordinates = coordinates_dir;
    direct_light.intensity = additional_dir[0];// * get_shadowing_value(input.light_space_position);
    
    input.world_position = mul(float4(input.world_position, 1.0), camera_matrix_);
    
    float used_roughness = 0.0;
    float used_metalness = 0.0;
    float3 used_ao = 1.0;
    
    if(use_albedo_map)
        input.color = ALBEDO_MAP.Sample(DEFAULT_SAMPLER, float2(input.uvw[0], input.uvw[1]));
    else
        input.color = albedo;
    
    if (use_roughness_map)
        used_roughness = ROUGHNESS_MAP.Sample(DEFAULT_SAMPLER, float2(input.uvw[0], input.uvw[1]));
    else
        used_roughness = roughness;
    
    if(use_metalness_map)
        used_metalness = METALNESS_MAP.Sample(DEFAULT_SAMPLER, float2(input.uvw[0], input.uvw[1]));
    else
        used_metalness = metalness;
    
    if (use_ao_map)
        used_ao = AO_MAP.Sample(DEFAULT_SAMPLER, float2(input.uvw[0], input.uvw[1]));
    else
        used_ao = 1.0;
    
    if(use_normal_map)
        input.normal = get_mapped_normal(input);
    
    float3 lighting = get_reflectance_pt(input, point_light, used_roughness, used_metalness) * used_ao;
    lighting = lighting / (lighting + 1.0f); // Conversion to HDR
    lighting = pow(lighting, 0.8f); // Gamma correction
    
    return float4(lighting, 1.0f);
}