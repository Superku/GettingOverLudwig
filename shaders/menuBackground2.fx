const float4x4 matWorldViewProj; // World*view*projection matrix. 
const float4x4 matView; // World*view*projection matrix. 
const float4x4 matWorld; // World matrix. 
const float4 vecViewPos;  // The sun direction vector.
float4 vecSkill41;
float4 vecSkill45;
float4 vecTime;
float menuOpenPerc_var;

texture entSkin1;
sampler ColorMapSampler = sampler_state 
{ 
	Texture = <entSkin1>; 
	AddressU  = Wrap; 
	AddressV  = Wrap; 
	/*mipfilter = none;
	minfilter = none;
	magfilter = none;*/
}; 
 
texture highlights_tga_bmap;
sampler highlightMapSampler = sampler_state 
{ 
	Texture = <highlights_tga_bmap>; 
	AddressU  = Wrap; 
	AddressV  = Wrap; 
}; 

void DiffuseVS( 
in float4 InPos: POSITION, 
in float2 InTex: TEXCOORD0, 
in float3 InNormal: NORMAL, 
out float4 OutPos: POSITION,
out float4 OutTex: TEXCOORD0,
out float3 OutNormal: TEXCOORD1, 
out float2 OutLookup: TEXCOORD2)
{ 
	//InPos.xyz *= vecSkill41.w;
	InPos.z += 2;
	OutTex.z = floor(InPos.z/10.0);
	OutTex.w = 0;
	InPos.z *= 0.01;
	OutPos = mul(InPos, matWorldViewProj); 
	float4 worldPos = mul(InPos,matWorld);
	OutTex.xy = InPos.xy/float2(200,170)*0.5+0.5; //worldPos.xy*0.0025 * float2(1, 923/801.0); // 923x801 was the previous texture resolution
	OutTex.y = 1-OutTex.y;
	//OutTex.zw = InTex.xy;
	OutNormal = normalize(mul(InNormal, matWorld));
	float2 lookup = mul(OutNormal,matView).xy;
	OutLookup.xy = lookup*0.15+0.4; //*float3(1,0.25,1)
} 


// Pixel Shader: 
float4 DiffusePS( 
in float4 InTex: TEXCOORD0,
in float3 InNormal: TEXCOORD1,
in float2 Lookup: TEXCOORD2): COLOR 
{ 
	//float4 texData = tex2D(ColorMapSampler, InTex.xy);
	//return texData.a;
	/*float2 hexagonCenter;
	hexagonCenter.x = floor(InTex.x*8)/8.0;
	hexagonCenter.y = floor(InTex.y*8)/8.0;
	float4 hexData = tex2D(ColorMapSampler, hexagonCenter);*/
	//return hexData.a;
	//float fadeInOut = sin(vecTime.w*0.1)*1.5 + texData.a + 0.5 - floor(InTex.y*16)/16.0;
	//float fadeInOut = sin(vecTime.w*0.05)*1.5+0.5 - floor(-InTex.x*0 + InTex.y*50 + texData.a)/20.0;
	//texData.a += floor(InTex.y);
	float fadeInOut = menuOpenPerc_var*1.8-1 - InTex.z; //sin(vecTime.w*0.05 + InTex.x*0.1)*120+60- InTex.z;
	//fadeInOut = floor(fadeInOut*40);
	clip(fadeInOut);
	float normalFac = 1-saturate(fadeInOut*0.05);

	InNormal.xy *= normalFac;
	//Lookup.xy *= normalFac;
	//Lookup.xy += 0.4;
	InNormal = normalize(InNormal);

	float3 lightDir = -normalize(float3(1,-1,1));
	float diffuse = saturate( dot(lightDir, InNormal) )*0.2 + 0.9;
	float highlight = tex2D(highlightMapSampler,Lookup.xy);
	highlight = highlight*highlight;

	float4 yellow = float4(1,0.8,0,1);
	float4 orange = float4(1,0.5,0,1);
	float orangeFac = InTex.x*InTex.y;
	float4 Color = lerp(yellow, orange, orangeFac);

	float highlightWave = sin((InTex.x + InTex.y)*5 - vecTime.w*0.075)*0.5+0.5;
	highlightWave = smoothstep(0,1,highlightWave);
	highlightWave = pow(highlightWave,8);
	highlight *= highlightWave*0.5+0.5;
	float4 final = Color*diffuse + highlight*0.75;

	return final;
} 

technique DiffuseTechnique 
{ 
	pass P0 
	{ 
		ZWriteEnable = true;
		AlphaBlendEnable = false;
		VertexShader = compile vs_3_0 DiffuseVS(); 
		PixelShader  = compile ps_3_0 DiffusePS(); 
	} 
} 
