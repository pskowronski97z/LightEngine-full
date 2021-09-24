struct VS_INPUT {
    float3 position : VT3_POSITION; // Dane wej�ciowe o formacie zadanym w deskryptorze D3D11_INPUT_ELEMENT_DESC
    float4 color : VT3_COLOR;
};

struct PS_INPUT {
    float4 position : SV_POSITION; // Dane wyj�ciowe, musi si� znajdowa� w�r�d nich wektor 4d oznaczony jako SV_POSITION (konwertuje interpolowane pozycje do przestrzeni ekranu)
    float4 color : VT3_COLOR;
};

PS_INPUT main( VS_INPUT input) {
    PS_INPUT result;
    result.color = input.color;
    result.position = float4(input.position, 1.0);
	
    return result;
}