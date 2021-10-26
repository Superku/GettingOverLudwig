
// Pixel Shader: 
float4 DiffusePS(in float2 InTex: TEXCOORD0): COLOR 
{ 
	//return InTex.x;
	return float4(1,1,1,0.0);
} 

technique DiffuseTechnique 
{ 
	pass P0 
	{ 
	//zWriteEnable = false;
	alphaBlendEnable = false;
		PixelShader = compile ps_3_0 DiffusePS(); 
	} 
} 

