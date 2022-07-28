#include "RSMInclude.hlsli"

struct Output {
    float4 world_position : SV_Target0;
    float4 normal : SV_Target1;
    float4 depth : SV_Target2;
    float4 flux : SV_Target3;
};

Output main(Pixel input_pixel) {
    
    Output output;

    output.world_position = float4(input_pixel.world_position, 1.0);
    output.normal = float4(normalize(input_pixel.normal), 1.0);
    output.depth = float4(input_pixel.depth, 1.0);
    
    float3 l = normalize(l_pov_world_position.xyz);
    float shading = max(0.0, dot(output.normal.xyz, l)) * light_intensity.x;
    
    output.flux = input_pixel.color * light_color;
    output.flux *= shading;
    output.flux.w = 1.0;
    
    return output;
}