cbuffer ConstBuffer
	: register(b0)
{
	matrix WVP : WORLDVIEWPROJECTION;
};

struct Vertex
{
	float4 Position : POSITION;
};

struct Pixel
{
	float4 Position : SV_POSITION;
};

Pixel VS(Vertex Arg)
{
	Pixel Result;
	Result.Position = mul(Arg.Position, transpose(WVP));
	return Result;
}

float4 PS(Pixel Arg)
	: SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
