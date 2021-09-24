struct PS_INPUT {
    float4 position : SV_POSITION; // Dane wyjœciowe, musi siê znajdowaæ wœród nich wektor 4d oznaczony jako SV_POSITION (konwertuje interpolowane pozycje do przestrzeni ekranu)
    float4 color : VT3_COLOR;
};


float4 main(PS_INPUT input) : SV_TARGET{
	return input.color;
}