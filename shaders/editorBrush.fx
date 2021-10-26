const float4x4 matWorldViewProj;
const float4x4 matWorld;
const float fAmbient;
const float4 vecSunDir;

texture entSkin1;

sampler ColorMapSampler = sampler_state 
{ 
   Texture = <entSkin1>; 
   AddressU  = Clamp; 
   AddressV  = Clamp; 
}; 
    
void DiffuseVS( 
   in float4 InPos: POSITION, 
   in float3 InNormal: NORMAL, 
   in float2 InTex: TEXCOORD0, 
   out float4 OutPos: POSITION, 
   out float2 OutTex: TEXCOORD0) 
{ 
   OutPos = mul(InPos, matWorldViewProj); 
   OutTex = InTex; 
} 
    
float4 DiffusePS( 
   in float2 InTex: TEXCOORD0): COLOR 
{ 
   //float4 Color = tex2D(ColorMapSampler, InTex);
   float len = length(InTex.xy-0.5);
   float4 final = 1; //-len;
   clip(1-len*2);
   final.a = 0.3;
  
   return final; 
} 
 

technique DiffuseTechnique 
{ 
   pass P0 
   { 
   zWriteEnable = false;
   alphaBlendEnable = true;
      VertexShader = compile vs_3_0 DiffuseVS(); 
      PixelShader  = compile ps_3_0 DiffusePS(); 
   } 
}
