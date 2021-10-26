
float4 vecViewPort;
float backgroundBlurScale_var;
float backgroundBlurScaleV_flt;
float gauss1D_weights2[13]={0.036, 0.051, 0.067, 0.083, 0.098, 0.107, 0.110, 0.107, 0.098, 0.083, 0.067, 0.051, 0.036 };

texture TargetMap;

sampler postTex = sampler_state
{
	texture = (TargetMap);
	/*MinFilter = none;
	MagFilter = none;
	MipFilter = none;*/
	AddressU = Clamp;
	AddressV = Clamp;
};

float4 Blur_PS(float2 texCoord: TEXCOORD0) : COLOR
{
	int i,j;
	float4 color = 0;

//texCoord *= 2;
//return tex2D(postTex, texCoord);
//float scale = vecViewPort.z*backgroundBlurScale_var;
	float scale = backgroundBlurScaleV_flt;
	for(i = 0; i < 13; i++) color += tex2D(postTex, texCoord + scale*float2(0,i-6))*gauss1D_weights2[i];
	//for(i = -1; i < 2; i++) color += tex2D(postTex, texCoord + scale*float2(0,i));
	//color *= 0.33333;

	return color;
}

technique tech_00
{
	pass pass_00
	{
		VertexShader = null;
		PixelShader = compile ps_3_0 Blur_PS();
	}
}

technique fallback { pass one { } }