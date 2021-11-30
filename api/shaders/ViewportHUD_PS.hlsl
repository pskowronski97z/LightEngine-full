struct PS_INPUT {
    float4 position : SV_POSITION; // Dane wyjœciowe, musi siê znajdowaæ wœród nich wektor 4d oznaczony jako SV_POSITION (konwertuje interpolowane pozycje do przestrzeni ekranu)
    float4 color : VT3_COLOR;
    float3 uvw : VT3_UVW;
};


SamplerState DEFAULT_SAMPLER : register(s0);
Texture2D SHADOW_MAP : register(t5);

float4 main(PS_INPUT input) : SV_TARGET{
    
    if (input.color[3] < 0){
        return SHADOW_MAP.Sample(DEFAULT_SAMPLER, input.uvw);
    }
    
	return input.color;
}