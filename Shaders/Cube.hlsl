

cbuffer CBPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gView;
	float4x4 gProj;
};

struct VSInput
{
	float3 pos : POSITION;
	float3 col : COLOR0;
};

struct PSInput
{
	float4 posH : SV_POSITION;
	float3 col : COLOR0;
};

PSInput VSMain(VSInput vin)
{
	PSInput o;
	float4 worldPos = mul(float4(vin.pos, 1.0f), gWorld);
	float4 viewPos = mul(worldPos, gView);
	o.posH = mul(viewPos, gProj);
	o.col = vin.col;
	return o;
}

float4 PSMain(PSInput pin) : SV_TARGET
{
	return float4(pin.col, 1.0f);
}