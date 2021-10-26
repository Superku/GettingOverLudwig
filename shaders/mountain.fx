const float4x4 matWorldViewProj;
const float4x4 matWorld;
const float4x4 matView;
const float fAmbient;
const float4 vecSunDir;
const float4 vecViewDir;
const float4 vecSkill41;
const float4 vecSkill45;

texture highlights_tga_bmap;
sampler HighlightMapSampler = sampler_state 
{ 
   Texture = <highlights_tga_bmap>; 
   AddressU  = Clamp; 
   AddressV  = Clamp; 
}; 
    
void DiffuseVS( 
   in float4 InPos: POSITION, 
   in float3 InNormal: NORMAL, 
   in float2 InTex: TEXCOORD0, 
   out float4 OutPos: POSITION, 
   out float2 OutTex: TEXCOORD0, 
   out float3 OutNormal: TEXCOORD1,
   out float2 OutLookup: TEXCOORD2,
   out float2 OutData: TEXCOORD3) 
{ 
   OutPos = mul(InPos, matWorldViewProj); 
   OutNormal = normalize(mul(InNormal, matWorld));
   OutTex.xy = InTex;
   OutLookup = OutNormal*0.35+0.5;
   OutData.x = OutPos.y*0.5+0.5;
   OutData.y = mul(InPos, matWorld).z/8192+vecSkill45.x;
} 
    
float4 DiffusePS( 
   in float4 InTex: TEXCOORD0, 
   in float3 InNormal: TEXCOORD1,
   in float2 InLookup: TEXCOORD2,
   in float2 InData: TEXCOORD3): COLOR 
{ 
	InNormal = normalize(InNormal);
	float3 InSunDir = -normalize(float3(4.75,-8,3));
	float Diffuse = 0.75+0.2*saturate(dot(InSunDir, InNormal)); 
	
	float Highlight = tex2D(HighlightMapSampler, InLookup);  //InTex // InLookup
   float4 Color = float4(0.55,0.5,0.6,1);
   
   float4 final = Color*(Diffuse+Highlight*Highlight*0.75)-0.085;
   float4 sky = lerp(0.25,float4(0.5,0.05,0.025,1),InData.x);
   final.rgb = lerp(final.rgb,sky,InData.y);

   final *= (1+fAmbient);
   return  final; 
} 
 

technique DiffuseTechnique 
{ 
   pass P0 
   { 
      zWriteEnable = true;
      alphaBlendEnable = false;
      VertexShader = compile vs_3_0 DiffuseVS(); 
      PixelShader  = compile ps_3_0 DiffusePS(); 
   } 
}
