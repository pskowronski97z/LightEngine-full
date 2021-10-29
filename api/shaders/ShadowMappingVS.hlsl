struct VS_INPUT {
    float3 position : VT3_POSITION; // Dane wejœciowe o formacie zadanym w deskryptorze D3D11_INPUT_ELEMENT_DESC
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
};

struct PS_INPUT {
    float4 position : SV_POSITION; // Dane wyjœciowe, musi siê znajdowaæ wœród nich wektor 4d oznaczony jako SV_POSITION (konwertuje interpolowane pozycje do przestrzeni ekranu)
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
    float3 normal : VT3_NORMAL;
    float3 tangent : VT3_TANGENT;
    float3 bitangent : VT3_BITANGENT;
    float3 world_position : VT3_POSITION;
    float3 gouraud_shading : VT3_COLOR1;
};

cbuffer LIGHT_CAMERA : register(b4) {
    matrix light_camera_matrix_;
    matrix light_projection_matrix_;
}


PS_INPUT main( VS_INPUT input_vertex) {
    
    PS_INPUT vs_output;
    
    vs_output.world_position = input_vertex.position;
    vs_output.position = mul(float4(input_vertex.position, 1.0f), light_camera_matrix_);
    vs_output.position = mul(vs_output.position, light_projection_matrix_);
    
    return vs_output;
}