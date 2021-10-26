float4x4 matWorld;
float4x4 matWorldViewProj;
float4x4 matViewProj;
//float4 vecViewPos;
float4 vecTime;
float4 vecSkill41;
//bool PASS_SOLID;
float camera_top_var;
float camera_bottom_var;

texture background_blurred_bmap_bmap;
texture perlinNoise_tga_bmap;

sampler2D smpRefr = sampler_state
{
	Texture = <background_blurred_bmap_bmap>;
	
	AddressU		= Mirror;
	AddressV		= Mirror;
};

sampler2D NormalMapSampler = sampler_state
{
	Texture = <perlinNoise_tga_bmap>;
	AddressU		= Wrap;
	AddressV		= Wrap;
};

struct VS_OUT
{
	float4 Pos : POSITION;
	float4 Tex : TEXCOORD0;
	float3 ProjTex : TEXCOORD1;
};

VS_OUT Shader_VS(float4 inPos : POSITION, float2 inTex : TEXCOORD0)
{
	VS_OUT Out = (VS_OUT)0;
	
	float4 worldpos = mul(inPos, matWorld);
	float oldposY,fac,time = vecTime.w*0.3;

	Out.Tex.xy = inTex;
	Out.Tex.zw = worldpos.xy;

	oldposY = worldpos.y;
	float offset = sin(time+worldpos.x*0.01+worldpos.y*0.2)+1.25*sin(-time*0.67+worldpos.x*0.02); //; //*sin(time*0.3+worldpos.x/96.0);
	worldpos.y += 8*offset;
	Out.Tex.z = offset*0.1;
	Out.Pos = mul(worldpos, matViewProj);
	Out.ProjTex = Out.Pos.xyw;

	return Out;
}

float4 Shader_PS(float4 InTex : TEXCOORD0, float3 ProjTex : TEXCOORD1) : COLOR
{
	ProjTex.xy = ProjTex.xy/ProjTex.z;
	ProjTex.xy = ProjTex.xy*0.5 + 0.5;
	ProjTex.y = 1.0-ProjTex.y;

	float waterHeight = -4524;
	float height = camera_top_var-camera_bottom_var;
	float h2 = camera_top_var-waterHeight;
	float baseY = h2/height;
	ProjTex.y = baseY-(waterHeight-InTex.w)/height*2 - 0.2;
	//ProjTex.y = 1-ProjTex.y;
	
	float4 texNormalData = tex2D(NormalMapSampler,InTex.xy*3+float2(vecTime.w*0.0075,0));
	texNormalData += tex2D(NormalMapSampler,InTex.xy*1.7+vecTime.w*0.001);
	float3 texNormal = texNormalData-1;

	float waterLine = saturate(1-InTex.y*20);
	//waterLine *= saturate(InTex.y*400);
	//waterLine = smoothstep(0,1,waterLine);
	//waterLine = saturate(InTex.y*2);
	waterLine = waterLine*waterLine*waterLine;

	float4 Color = tex2D(smpRefr, ProjTex.xy+texNormal.xy*0.2); //  + float2(0,waterLine)*0
	//return Color - waterLine*waterLine*0.1;
	float4 tint = float4(0.7,0.8,1.0,1);
	float brightness = abs(InTex.y-0.5)+InTex.z;
	float4 final = Color+brightness*0.15+texNormal.x*texNormal.y + waterLine*0.5*float4(1,0.5,0.4,1);

	return final*tint;
}

technique tech_00
{
	pass pass_00
	{
		//CullMode = None;
		
		VertexShader = compile vs_3_0 Shader_VS();
		PixelShader = compile ps_3_0 Shader_PS();
	}
}
