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

struct PS_OUTPUT {
   float4 world_postion : SV_Target0;
   float4 normal : SV_Target1;
};

PS_OUTPUT main(PS_INPUT ps_input) {
    
    PS_OUTPUT ps_output;
    ps_output.world_postion = float4(ps_input.world_position, 1.0);
    ps_output.normal = float4(ps_input.normal, 1.0);
    
    return ps_output;
}