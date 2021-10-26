texture TargetMap;
float4 vecViewPort;

sampler2D smpScreen = sampler_state
{
	Texture = <TargetMap>;
	
	MinFilter	= linear;
	MagFilter	= linear;
	MipFilter	= linear;
	
	AddressU		= Clamp;
	AddressV		= Clamp;
};

float4 pp_luma(float2 inTex : TEXCOORD0) : COLOR
{
	float4 color;
	color.rgb = tex2D(smpScreen, inTex).rgb;
//	color.a = dot(color.rgb, float3(0.299, 0.587, 0.114));
	color.a = dot(sqrt(color.rgb), float3(0.299, 0.587, 0.114));
	
	return color;
}

technique tech_00
{
	pass pass_00
	{
		PixelShader = compile ps_3_0 pp_luma();
	}
}
