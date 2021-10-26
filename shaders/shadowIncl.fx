
#define USE_PCF // use percentage closer filtering
static const float fDark = 0.65;
static const float fBright = 1.0;
static const float fDepthOffset = 0.995;
static const float fPCF = 0.00075;
const float4x4 matEffect1;   // Precalculated texture projection matrix
const float3 kuSunDir;	// Sun direction vector.

//texture depth_view_bmap_bmap;
texture shadowmap_bmap;

sampler DepthSampler = sampler_state
{
	Texture = <shadowmap_bmap>;
	minfilter = none;
	magfilter = none;
	mipfilter = none;
};
// distance comparison function

float fDist(float4 DepthCoord,float fDepth)
{
	return tex2Dproj(DepthSampler,DepthCoord).r < (fDepth*fDepthOffset)? fDark : fBright;
}

#ifdef USE_PCF
	static const float4 fTaps_PCF[9] = {
		{-1.0,-1.0, 0.0, 0.0},
		{-1.0, 0.0, 0.0, 0.0},
		{-1.0, 1.0, 0.0, 0.0},
		{ 0.0,-1.0, 0.0, 0.0},
		{ 0.0, 0.0, 0.0, 0.0},
		{ 0.0, 1.0, 0.0, 0.0},
		{ 1.0,-1.0, 0.0, 0.0},
		{ 1.0, 0.0, 0.0, 0.0},
	{ 1.0, 1.0, 0.0, 0.0}};
#endif

float DoKuShadow(float4 InDepth)
{
	float fShadow = 1;
	//if(vecSkill45.y < 0.1)
		fShadow = 0;
		#ifdef USE_PCF
			for (int i=0; i < 9; i++)
			{
				float4 fTap = InDepth + fPCF*fTaps_PCF[i];
				fShadow += fDist(fTap,InDepth.z)/9.0;
			}
			#else
			fShadow = fDist(InDepth,InDepth.z);
		#endif
	return fShadow;
}


float4 DoKuDepth(float4 InPos)
{
	return mul( mul(InPos,matWorld), matEffect1 );
}

float4 DoKuDepthFromWorldPos(float4 worldPos)
{
	return mul( worldPos, matEffect1 );
}

