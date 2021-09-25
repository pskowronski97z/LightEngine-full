struct PS_INPUT {
    float4 position : SV_POSITION; 
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 gouraud_shading : VT3_COLOR1;
};

struct LIGHT_SOURCE {
    float4 coordinates;
    float4 color;
};

Texture2D DIFFUSE_MAP : register(t0);
Texture2D NORMAL_MAP : register(t1);
SamplerState DEFAULT_SAMPLER : register(s0);

cbuffer CAMERA : register(b0) {
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
    float quadratic_att;
    float linear_att;
    float constant_att;
    float specular_level;
    int glossiness;
    bool use_diffuse_map_;
    bool use_normal_map_;
    int flip_tb_vectors_;
    float4 specular;
    float4 diffuse;
    float4 ambient;
};

float3 get_mapped_normal(PS_INPUT pixel) {
    
    float4 normal_sample = NORMAL_MAP.Sample(DEFAULT_SAMPLER, float2(pixel.uvw[0], pixel.uvw[1]));
    
    float scale_t = 1.0 - asfloat(0xFF & flip_tb_vectors_);
    float scale_b = 1.0 - asfloat(0xFF & (flip_tb_vectors_ >> 8));
      
    float3x3 tbn_matrix = float3x3(scale_t * normalize(pixel.tangent), scale_b * normalize(pixel.bitangent), normalize(pixel.normal));
    
    normal_sample = normalize(normal_sample * 2.0 - 1.0);
    pixel.normal = mul(normal_sample, tbn_matrix);
    
    return normalize(pixel.normal);
}

float get_attenuation(float distance) {
    float attenuation = quadratic_att * distance * distance + linear_att * distance + constant_att;
    attenuation = attenuation * attenuation;
    return rsqrt(attenuation);
}