//////////////////////////////
// materials - dev version
//////////////////////////////

BMAP *autumnLeavesMask_tga = "autumnLeavesMask.tga";
BMAP *perlinNoise_tga = "perlinNoise.tga";
BMAP* highlights_tga = "highlights.tga"; // this texture came with an old free Gamestudio sample project (for a chrome effect)

/*

	NOTE !!!
	Some shaders were loosely based on shaders I made previously (that is before the game jam).
	This is how I write all my shaders though (thousands over the years), I take an old shader file,
	copy paste it and then change it around to make it look how I want it to be like.
	Back then I used this shader file for everything, from the official tutorial: http://www.conitec.net/shaders/shader_work2.htm
	Funnily enough the names for the technique ("DiffusePS") and other things from that example code can be found in my shaders to this day
	(around 14 years later, not quite sure when I started writing shaders).
	This is fine in my book, as engines nowadays usually come with all kinds of materials (shaders) anyway.
	Shaders made in material editors via nodes come with basic premade effect code themselves, otherwise it just wouldn't even work.

*/

MATERIAL* characterHoney_mat =
{
	effect = "characterHoney.fxo";
	flags = AUTORELOAD;
}

MATERIAL* menuPage_mat =
{
	effect = "menuPage.fxo";
	flags = AUTORELOAD;
}

MATERIAL* menuBackground_mat =
{
	effect = "menuBackground.fxo";
	flags = AUTORELOAD;
}

MATERIAL* menuBackground2_mat =
{
	effect = "menuBackground2.fxo";
	flags = AUTORELOAD;
}

MATERIAL* titleScreen_mat =
{
	effect = "titleScreen.fxo";
	flags = AUTORELOAD;
}

MATERIAL* geoDefault_mat =
{
	effect = "geoWLeaves.fxo"; // to do: without leaves, better performance
	flags = AUTORELOAD;
}

MATERIAL* geoWLeaves_mat =
{
	effect = "geoWLeaves.fxo";
	flags = AUTORELOAD;
}

MATERIAL* backgroundPlane_mat = 
{
	effect = "backgroundPlane.fxo";
	flags = AUTORELOAD;
}

MATERIAL* editorBrush_mat =
{
	effect = "editorBrush.fxo";
	flags = AUTORELOAD;
}

MATERIAL* water_mat =
{
	effect = "water.fxo";
	flags = AUTORELOAD;
}

MATERIAL* mat_alpha_fill =
{
	effect = "alpha_fill.fxo";
	flags = AUTORELOAD;
}

MATERIAL* particlesFGquad_mat =
{
	effect = "particlesFGquad.fxo";
	flags = AUTORELOAD;
}

MATERIAL* mountain_mat =
{
	effect = "mountain.fxo";
	flags = AUTORELOAD;
}

MATERIAL* leavesBG_mat =
{
	effect = "leavesBG.fxo";
	flags = AUTORELOAD;
}

MATERIAL* candleLight_mat =
{
	effect = "candleLight.fxo";
	flags = AUTORELOAD;
}

MATERIAL* pp_blur_h_mat =
{
	effect = "pp_blur_h.fxo";
	flags = AUTORELOAD;
}

MATERIAL* pp_blur_v_mat =
{
	effect = "pp_blur_v.fxo";
	flags = AUTORELOAD;
}

MATERIAL* pp_fxaa =
{
	effect = "fxaa.fxo";
	flags = AUTORELOAD;
}

MATERIAL* pp_fxaa_luma =
{
	effect = "fxaa_luma.fxo";
	flags = AUTORELOAD;
}

//skill1: vecSkill41 0
//skill2: vecSkill42 0
//skill3: vecSkill43 0
//skill4: vecSkill44 0
//skill5: vecSkill45 0
//skill6: vecSkill46 0
//skill7: vecSkill47 0
//skill8: vecSkill48 0
action geoModelSetup() // unused... doesn't matter
{
	set(my, PASSABLE | UNLIT);
}

void backgroundObjectAdd(ENTITY* ent);
//skill1: xFac 0
//skill2: zFac 0
//skill3: fogAdjust 0
action backgroundObject()
{
	set(my, PASSABLE | UNLIT);
	if(!my.skill1) my.skill1 = 1;
	if(!my.skill2) my.skill2 = 1;
	my.skill45 = floatv(my.skill3);
	my.skill60 = my.x;
	my.skill62 = my.z;
	backgroundObjectAdd(my);
	//if(entityContainer.entBackgroundObjectCount < BACKGROUND_OBJECTS_MAX) entBackgroundObjects[entityContainer.entBackgroundObjectCount++] = me;
}

action candleSetup()
{
	set(my, PASSABLE | UNLIT | TRANSLUCENT);
	my.material = geoDefault_mat;
	ent_mtlset(my, candleLight_mat, 2);
	my.alpha = 100;
	my.albedo = 100;
}
