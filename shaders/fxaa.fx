

/*============================================================================
FXAA QUALITY - TUNING KNOBS
============================================================================*/
#ifndef FXAA_QUALITY__EDGE_THRESHOLD
	//
	// The minimum amount of local contrast required to apply algorithm.
	//
	// 1/3 - too little
	// 1/4 - low quality
	// 1/6 - default
	// 1/8 - high quality
	// 1/16 - overkill
	//
	#define FXAA_QUALITY__EDGE_THRESHOLD (1.0/6.0)
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_QUALITY__EDGE_THRESHOLD_MIN
	//
	// Trims the algorithm from processing darks.
	//
	// 1/32 - visible limit
	// 1/16 - high quality
	// 1/12 - upper limit (default, the start of visible unfiltered edges)
	//
	#define FXAA_QUALITY__EDGE_THRESHOLD_MIN (1.0/12.0)
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_QUALITY__SUBPIX_CAP
	//
	// Insures fine detail is not completely removed.
	// This partly overrides FXAA_SUBPIX_TRIM.
	//
	// 3/4 - default amount of filtering
	// 7/8 - high amount of filtering
	// 1 - no capping of filtering
	//
	#define FXAA_QUALITY__SUBPIX_CAP (3.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#ifndef FXAA_QUALITY__SUBPIX_TRIM
	//
	// Controls removal of sub-pixel aliasing,
	//
	// 1/2 - low removal (sharper but more sub-pixel aliasing)
	// 1/3 - medium removal
	// 1/4 - default removal
	// 1/8 - high removal
	// 0 - complete removal (softer but less sub-pixel aliasing)
	//
	#define FXAA_QUALITY__SUBPIX_TRIM (1.0/4.0)
#endif


#define int2 float2
#define FxaaInt2 float2
#define FxaaFloat2 float2
#define FxaaFloat3 float3
#define FxaaFloat4 float4
#define FxaaDiscard clip(-1)
#define FxaaDot3(a, b) dot(a, b)
#define FxaaSat(x) saturate(x)
#define FxaaLerp(x,y,s) lerp(x,y,s)
#define FxaaTex sampler2D
#define FxaaTexTop(t, p) tex2Dlod(t, float4(p, 0.0, 0.0))
#define FxaaTexOff(t, p, o, r) tex2Dlod(t, float4(p + (o * r), 0, 0))


float4 FxaaPixelShader(
// {xy} = center of pixel
float2 pos,
// {xyzw} = not used on FXAA3 Quality
float4 posPos,       
// {rgb_} = color in linear or perceptual color space
// {___a} = luma in perceptual color space (not linear) 
FxaaTex tex,
// This must be from a constant/uniform.
// {x_} = 1.0/screenWidthInPixels
// {_y} = 1.0/screenHeightInPixels
float2 rcpFrame,
// {xyzw} = not used on FXAA3 Quality
float4 rcpFrameOpt 
) {   
	/*--------------------------------------------------------------------------*/
	float lumaN = FxaaTexOff(tex, pos.xy, FxaaInt2(0, -1), rcpFrame.xy).w;
	float lumaW = FxaaTexOff(tex, pos.xy, FxaaInt2(-1, 0), rcpFrame.xy).w;
	float4 rgbyM = FxaaTexTop(tex, pos.xy);
	float lumaE = FxaaTexOff(tex, pos.xy, FxaaInt2( 1, 0), rcpFrame.xy).w;
	float lumaS = FxaaTexOff(tex, pos.xy, FxaaInt2( 0, 1), rcpFrame.xy).w;
	float lumaM = rgbyM.w;
	/*--------------------------------------------------------------------------*/
	float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
	float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
	float range = rangeMax - rangeMin;
	/*--------------------------------------------------------------------------*/
	if(range < max(FXAA_QUALITY__EDGE_THRESHOLD_MIN, rangeMax * FXAA_QUALITY__EDGE_THRESHOLD))
	return rgbyM;
	/*--------------------------------------------------------------------------*/
	float lumaNW = FxaaTexOff(tex, pos.xy, FxaaInt2(-1,-1), rcpFrame.xy).w;
	float lumaNE = FxaaTexOff(tex, pos.xy, FxaaInt2( 1,-1), rcpFrame.xy).w;
	float lumaSW = FxaaTexOff(tex, pos.xy, FxaaInt2(-1, 1), rcpFrame.xy).w;
	float lumaSE = FxaaTexOff(tex, pos.xy, FxaaInt2( 1, 1), rcpFrame.xy).w;
	/*--------------------------------------------------------------------------*/
	#define FXAA_QUALITY__SUBPIX_TRIM_SCALE  (1.0/(1.0 - FXAA_QUALITY__SUBPIX_TRIM))
	/*--------------------------------------------------------------------------*/
	float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
	float rangeL = abs(lumaL - lumaM);
	float blendL = FxaaSat((rangeL / range) - FXAA_QUALITY__SUBPIX_TRIM) * FXAA_QUALITY__SUBPIX_TRIM_SCALE; 
	blendL = min(FXAA_QUALITY__SUBPIX_CAP, blendL);
	/*--------------------------------------------------------------------------*/
	float edgeVert = 
	abs(lumaNW + (-2.0 * lumaN) + lumaNE) +
	2.0 * abs(lumaW  + (-2.0 * lumaM) + lumaE ) +
	abs(lumaSW + (-2.0 * lumaS) + lumaSE);
	float edgeHorz = 
	abs(lumaNW + (-2.0 * lumaW) + lumaSW) +
	2.0 * abs(lumaN  + (-2.0 * lumaM) + lumaS ) +
	abs(lumaNE + (-2.0 * lumaE) + lumaSE);
	bool horzSpan = edgeHorz >= edgeVert;
	/*--------------------------------------------------------------------------*/
	float lengthSign = horzSpan ? -rcpFrame.y : -rcpFrame.x;
	if(!horzSpan) lumaN = lumaW;
	if(!horzSpan) lumaS = lumaE;
	float gradientN = abs(lumaN - lumaM);
	float gradientS = abs(lumaS - lumaM);
	lumaN = (lumaN + lumaM) * 0.5;
	lumaS = (lumaS + lumaM) * 0.5;
	/*--------------------------------------------------------------------------*/
	bool pairN = gradientN >= gradientS;
	if(!pairN) lumaN = lumaS;
	if(!pairN) gradientN = gradientS;
	if(!pairN) lengthSign *= -1.0;
	float2 posN;
	posN.x = pos.x + (horzSpan ? 0.0 : lengthSign * 0.5);
	posN.y = pos.y + (horzSpan ? lengthSign * 0.5 : 0.0);
	/*--------------------------------------------------------------------------*/
	#define FXAA_SEARCH_STEPS     6
	#define FXAA_SEARCH_THRESHOLD (1.0/4.0)
	/*--------------------------------------------------------------------------*/
	gradientN *= FXAA_SEARCH_THRESHOLD;
	/*--------------------------------------------------------------------------*/
	float2 posP = posN;
	float2 offNP = horzSpan ? 
	FxaaFloat2(rcpFrame.x, 0.0) :
	FxaaFloat2(0.0f, rcpFrame.y); 
	float lumaEndN;
	float lumaEndP;
	bool doneN = false;
	bool doneP = false;
	posN += offNP * (-1.5);
	posP += offNP * ( 1.5);
	for(int i = 0; i < FXAA_SEARCH_STEPS; i++) {
		lumaEndN = FxaaTexTop(tex, posN.xy).w;
		lumaEndP = FxaaTexTop(tex, posP.xy).w;
		bool doneN2 = abs(lumaEndN - lumaN) >= gradientN;
		bool doneP2 = abs(lumaEndP - lumaN) >= gradientN;
		if(doneN2 && !doneN) posN += offNP;
		if(doneP2 && !doneP) posP -= offNP;
		if(doneN2 && doneP2) break;
		doneN = doneN2;
		doneP = doneP2;
		if(!doneN) posN -= offNP * 2.0;
	if(!doneP) posP += offNP * 2.0; }
	/*--------------------------------------------------------------------------*/
	float dstN = horzSpan ? pos.x - posN.x : pos.y - posN.y;
	float dstP = horzSpan ? posP.x - pos.x : posP.y - pos.y;
	/*--------------------------------------------------------------------------*/
	bool directionN = dstN < dstP;
	lumaEndN = directionN ? lumaEndN : lumaEndP;
	/*--------------------------------------------------------------------------*/
	if(((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0)) 
	lengthSign = 0.0;
	/*--------------------------------------------------------------------------*/
	float spanLength = (dstP + dstN);
	dstN = directionN ? dstN : dstP;
	float subPixelOffset = 0.5 + (dstN * (-1.0/spanLength));
	subPixelOffset += blendL * (1.0/8.0);
	subPixelOffset *= lengthSign;
	float3 rgbF = FxaaTexTop(tex, FxaaFloat2(
	pos.x + (horzSpan ? 0.0 : subPixelOffset),
	pos.y + (horzSpan ? subPixelOffset : 0.0))).xyz;
	/*--------------------------------------------------------------------------*/
	lumaL *= lumaL;
	float lumaF = dot(rgbF, float3(0.299, 0.587, 0.114)) + (1.0/(65536.0*256.0));
	float lumaB = FxaaLerp(lumaF, lumaL, blendL);
	float scale = min(4.0, lumaB/lumaF);
	rgbF *= scale;
	return float4(rgbF, lumaM); 
}

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

float4 pp_aa(float2 inTex : TEXCOORD0) : COLOR
{
	//vecViewPort.zw = 1.0/2048.0;
	return FxaaPixelShader(inTex, float4(inTex-0.5*vecViewPort.zw, inTex+0.5*vecViewPort.zw), smpScreen, vecViewPort.zw, float4(vecViewPort.zw*2.0, vecViewPort.zw*0.5));
}

technique tech_00
{
	pass pass_00
	{
		PixelShader = compile ps_3_0 pp_aa();
	}
}
