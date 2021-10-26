////////////////////////////////
// spearParticles.c
////////////////////////////////

BMAP* sphere_tga = "sphere.tga";
BMAP* grad256px_tga = "grad256px.tga";
BMAP* cloudSpr1_tga = "cloudSpr1.tga"; // keywords: smoke, cloud
BMAP* cloudSpr2_tga = "cloudSpr2.tga";
BMAP* star1_tga = "star1.tga";
BMAP* star2_tga = "star2.tga";

void p_spearImpactStreaks_fade(PARTICLE* p)
{
	p->x += p->vel_x*p->skill_b*time_step;
	p->z += p->vel_z*p->skill_b*time_step;
	vec_scale(p->vel_x,1-0.33*time_step);
	p->size = p->skill_a*minv(p->lifespan/3.0,1);
}

void p_spearImpactStreaks(PARTICLE* p)
{
	vec_randomize2(p->skill_x,0,2);
	p->skill_y = 0;
	vec_add(p->vel_x,p->skill_x);
	vec_normalize(p->vel_x,50+random(30));
	vec_set(p->blue,vector(128,180+random(10),200+random(20)));
	vec_scale(p->blue,0.9+random(0.1));
	vec_lerp(p->blue,p->blue,COLOR_WHITE,random(0.5));
	set(p, STREAK); //
	p->bmap = sphere_tga;
	p->alpha = 100;
	p->skill_a = p->size = 0.65*(3+random(2));
	p->skill_b = 0.3+random(0.4);
	p->lifespan = 2.8+random(1);
	p->event = p_spearImpactStreaks_fade;
}

void p_spearImpactGlow_fade(PARTICLE* p)
{
	p->skill_y = p->lifespan/2.0;
	p->alpha = p->skill_b*p->skill_y*p->skill_y;
}

void p_spearImpactGlow(PARTICLE* p) // rather a flash instead of a glow
{
	p->y = -1024; // in FG but not in front of menu
	vec_set(p->blue,vector(106,170+random(50),220+random(30)));
	vec_scale(p->blue,0.8+random(0.2));
	p->bmap = grad256px_tga;
	p->alpha = p->skill_b = p->vel_y*(25+random(10));
	p->skill_a = p->size = p->vel_x*(128+random(64));
	p->lifespan = 2;
	p->event = p_spearImpactGlow_fade;
}

void p_spearImpactSmoke_fade(PARTICLE* p)
{
	vec_scale(p->vel_x,1-0.25*time_step);
	p->size = p->skill_a*minv(p->lifespan/6.0,1);
}

void p_spearImpactSmoke(PARTICLE* p)
{
    p->y -= 256;
	p->skill_a = -p->vel_z;
	p->skill_b = p->vel_x;
	p->skill_c = (1+random(4))*(1-2*(random(2) > 1));
	p->vel_x += p->skill_c*p->skill_a;
	p->vel_z += p->skill_c*p->skill_b;
	vec_randomize2(p->skill_x,0,1);
	vec_add(p->vel_x,p->skill_x);
	//p->vel_y = 0;
	vec_normalize(p->vel_x,(2+random(10))*3);
	vec_fill(p->blue,120+random(28));
	set(p, MOVE);
	p->bmap = cloudSpr1_tga;
	if(random(2) > 1) p->bmap = cloudSpr2_tga;
	p->alpha = 100;
	p->skill_a = p->size = 2.5*(8+random(8));
	p->lifespan = 12+random(6);
	p->event = p_spearImpactSmoke_fade;
}

void p_characterHitStars_fade(PARTICLE* p)
{
	vec_scale(p->vel_x,1-0.3*time_step);
	p->size = p->skill_a*minv(p->lifespan/4.0,1);
}

void p_characterHitStars(PARTICLE* p)
{
	p->y -= 256;
	vec_randomize2(p->vel_x,5, 80);
	vec_fill(p->blue,160+random(92));
	set(p, MOVE); //
	p->bmap = star1_tga;
	if(random(2) > 1) p->bmap = star2_tga;
	p->alpha = 100;
	p->skill_a = p->size = (7+random(10))*2;
	p->lifespan = 5+random(3);
	p->event = p_characterHitStars_fade;
}
