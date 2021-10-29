cbuffer CAMERA : register(b0) {
    matrix camera_matrix_;
    matrix projection_matrix_;
}

cbuffer POINT_LIGHT : register(b1) {
    float4 coordinates_pt;
    float4 color_pt;
};

cbuffer DIRECT_LIGHT : register(b2) {
    float4 coordinates_dir;
    float4 color_dir;
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


struct VS_INPUT {
    float3 position : VT3_POSITION; // Dane wejœciowe o formacie zadanym w deskryptorze D3D11_INPUT_ELEMENT_DESC
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
};

struct PS_INPUT {
    float4 position : SV_POSITION;  // Dane wyjœciowe, musi siê znajdowaæ wœród nich wektor 4d oznaczony jako SV_POSITION (konwertuje interpolowane pozycje do przestrzeni ekranu)
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 gouraud_shading : VT3_COLOR1;
};

float get_attenuation(float distance) {
    float attenuation = quadratic_att * distance * distance + linear_att * distance + 1.0f;
    attenuation = attenuation * attenuation;
    return rsqrt(attenuation);
}

float3 gouraud_pt(float3 light_position, float4 light_color, float3 vertex_position, float3 vertex_normal) {
    float3 l = light_position - vertex_position;
    return get_attenuation(length(l)) * max(0.0, dot(normalize(l), vertex_normal)) * light_color;
}

float3 gouraud_dir(float3 light_direction, float4 light_color, float3 vertex_normal) {
    return max(0.0, dot(normalize(-light_direction), vertex_normal)) * light_color;
}

PS_INPUT main(VS_INPUT input) {
	
    PS_INPUT result;
    
    // ----------------- DEBUG ---------------------
    //input.position = mul(float4(input.position, 1.0f), rotate_y_);
    //input.normal = mul(float4(input.normal, 1.0f), rotate_y_);
    
    //input.position = mul(float4(input.position, 1.0f), rotate_x_);
    //result.normal = mul(float4(input.normal, 1.0f), rotate_x_);
    // ---------------------------------------------
    
    result.position = mul(float4(input.position, 1.0f), camera_matrix_);
    
    result.world_position = result.position;  
    result.position = mul(result.position, projection_matrix_);  
    result.normal = mul(float4(input.normal, 0.0f), camera_matrix_);    
    result.tangent = mul(float4(input.tangent, 0.0f), camera_matrix_);
    result.bitangent = mul(float4(input.bitangent, 0.0f), camera_matrix_);
    result.color = input.color;
    
    result.gouraud_shading = gouraud_pt(mul(coordinates_pt, camera_matrix_), color_pt, result.world_position, result.normal);
    result.gouraud_shading += gouraud_dir(mul(coordinates_dir, camera_matrix_), color_dir, result.normal);
    result.gouraud_shading = saturate(result.gouraud_shading);
    result.uvw = input.uvw;
    
    return result;
}