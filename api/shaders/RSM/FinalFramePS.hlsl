#include "RSMInclude.hlsli"

Texture2D GI_MAP : register(t0);
Texture2D DEPTH_MAP : register(t1);

float4 main(Pixel inputPixel) : SV_TARGET {
    
    float4 l_pov_position = mul(float4(inputPixel.world_position, 1.0), l_pov_transform_matrix);
    
    l_pov_position = mul(l_pov_position, l_pov_projection_matrix);
    l_pov_position /= l_pov_position.w;
    
    float2 uv;
    
    uv.x = mad(l_pov_position.x, 0.5, 0.5);
    uv.y = mad(-l_pov_position.y, 0.5, 0.5);
   
    float sampled_depth = DEPTH_MAP.Sample(SAMPLER, uv); 
    float3 n = normalize(inputPixel.normal.xyz);
    float3 l = normalize(l_pov_world_position.xyz);   
    float shading = max(0.0, dot(n, l)) * light_intensity.x;
    float4 direct_diffuse = inputPixel.color * light_color;
    
    direct_diffuse *= shading;
 
    if (l_pov_position.z > (sampled_depth + 0.001))
        direct_diffuse *= 0.0;

    uint gi_map_width;
    uint gi_map_height;
    
    GI_MAP.GetDimensions(gi_map_width, gi_map_height);
    
    float x = SCREEN_WIDTH / float(gi_map_width);
    float y = SCREEN_HEIGHT / float(gi_map_height);
    
    x = inputPixel.position.x / x;
    y = inputPixel.position.y / y;
    
    float4 indirect_diffuse = float4(0.0, 0.0, 0.0, 0.0);
    uint x1 = floor(x);
    uint x2 = ceil(x);
    uint y1 = floor(y);
    uint y2 = ceil(y); 
    float x_value = x - x1;
    float y_value = y - y1;
    float4 q11 = GI_MAP.Load(uint3(x1, y1, 0));
    float4 q21 = GI_MAP.Load(uint3(x2, y1, 0));
    float4 q12 = GI_MAP.Load(uint3(x1, y2, 0));
    float4 q22 = GI_MAP.Load(uint3(x2, y2, 0));       
    float4 xy1 = lerp(q11, q21, x_value);
    float4 xy2 = lerp(q12, q22, x_value);
    
    indirect_diffuse = lerp(xy1, xy2, y_value);   
  
    return direct_diffuse + indirect_diffuse;
}