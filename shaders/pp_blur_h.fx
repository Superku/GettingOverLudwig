
float4 vecViewPort;
float backgroundBlurScale_var;
float backgroundBlurScaleH_flt;
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
	//texCoord.x += 0.25;
	//return tex2D(postTex, texCoord);

	//float depth = tex2D(g_buffer_pos, texCoord).w;
	//float fac = 1+depth*0.001;
	//float scale = vecViewPort.z*backgroundBlurScale_var;//*2.0/fac;
	float scale = backgroundBlurScaleH_flt;
	for(i = 0; i < 13; i++) color += tex2D(postTex, texCoord + scale*float2(i-6,0))*gauss1D_weights2[i];
	//for(i = -1; i < 2; i++) color += tex2D(postTex, texCoord + scale*float2(i,0));
	//color *= 0.33333;
	//color += depth*0.0001;

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