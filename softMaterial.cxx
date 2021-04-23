#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <bstring.h>
#include <Performer/pr/pfMaterial.h>
#include "softMaterial.h"

enum { MATTYPE_CONSTANT, MATTYPE_FLAT, MATTYPE_LAMBERT, MATTYPE_PHONG,
	 MATTYPE_BLINN, MATTYPE_SHADOW };

softMaterial::softMaterial(void)
{
 id_ = -1;
 name_ = strdup("");
 type_ = MATTYPE_LAMBERT;
 ambient_.set(0,0,0);
 diffuse_.set(0,0,0);
 specular_.set(0,0,0);
 exponent_ = 0.0;
 transparency_ = 0.0;
 texture_ = NULL;
 model_ = NULL;
 gstate_ = NULL;
}

void softMaterial::setModel(softModel *model)
{
 model_ = model;
}

void softMaterial::parse(tokenStream *tstream)
{
#define NEXTTOKEN() { if (!tstream->next(token,sizeof(token))) return; }
 char token[1024];
 enum { MAT_NAME, MAT_AMBIENT, MAT_DIFFUSE, MAT_SPECULAR, MAT_EXPONENT,
	MAT_TRANSPARENCY, MAT_TEXTURE, MAT_TYPE, OPEN_BRACE, CLOSE_BRACE } ;
 tokenStream::tokenVal table[] = {
		{ "name", MAT_NAME },
		{ "ambient", MAT_AMBIENT },
		{ "diffuse", MAT_DIFFUSE },
		{ "specular", MAT_SPECULAR },
		{ "exponent", MAT_EXPONENT },
		{ "transparency", MAT_TRANSPARENCY },
		{ "texture", MAT_TEXTURE },
		{ "type", MAT_TYPE },
		{ "{", OPEN_BRACE },
		{ "}", CLOSE_BRACE },
		{ NULL, -1 }
		};
 int loop=1;
 if (!tstream)
	return;
 NEXTTOKEN()		/* material ID in [] */
 id_ = atoi(&token[1]);
 NEXTTOKEN()		/* "{" */
 while (loop)
	{
	NEXTTOKEN()
	switch (tstream->lookup(token,table))
		{
		case CLOSE_BRACE: loop = 0; break;
		case OPEN_BRACE: tstream->skipBlock("{","}"); break;
		case MAT_NAME:	NEXTTOKEN()
				name_ = strdup(token);
				break;
		case MAT_AMBIENT: tstream->getNextVec3(&ambient_);
				  break;
		case MAT_DIFFUSE: tstream->getNextVec3(&diffuse_);
				  break;
		case MAT_SPECULAR: tstream->getNextVec3(&specular_);
				   break;
		case MAT_EXPONENT: exponent_ = tstream->nextFloat();
				   break;
		case MAT_TRANSPARENCY: transparency_ = tstream->nextFloat();
					break;
		case MAT_TEXTURE: texture_ = new softTexture;
				  texture_->setModel(model_);
				  texture_->parse(tstream);
				  break;
		case MAT_TYPE:	parseType(tstream);
				break;
		}
	}
}

void softMaterial::parseType(tokenStream *tstream)
{
 char token[1024];
 tokenStream::tokenVal table[] = {
		{ "CONSTANT", MATTYPE_CONSTANT },
		{ "FLAT", MATTYPE_FLAT },
		{ "LAMBERT", MATTYPE_LAMBERT },
		{ "PHONG", MATTYPE_PHONG },
		{ "BLINN", MATTYPE_BLINN },
		{ "SHADOW", MATTYPE_SHADOW },
		{ NULL, -1 }
		};
 int val;
 NEXTTOKEN()
 val = tstream->lookup(token,table);
 if (val > -1)
	type_ = val;
 else
	printf("Error (softMaterial::parse): unknown type \"%s\"\n",token);
}

softTexture *softMaterial::texture(void)
{
 if ((!texture_) && (model_))
	texture_ = model_->texture();
 return texture_;
}

pfGeoState * softMaterial::pfGState(void)
{
 if (!gstate_)
	{
	pfMaterial *mtl;
	float alpha;
	softTexture *tex = texture_;
	if (!tex)
		tex = model_->texture();
	gstate_ = new pfGeoState;
	mtl = new pfMaterial;
	alpha = 1.0f - transparency_;
	if (tex)
		alpha *= (1.0f - tex->transparency());
	mtl->setSide(PFMTL_BOTH);
	mtl->setAlpha(alpha);
	if (type_ >= MATTYPE_PHONG)
		mtl->setShininess((exponent_*128.0f)/300.0f); /* Softimage's max is 300; GL's max is 128 */
	else
		mtl->setShininess(0.0f);
	mtl->setColor(PFMTL_AMBIENT, ambient_[0], ambient_[1], ambient_[2]);
	mtl->setColor(PFMTL_DIFFUSE, diffuse_[0], diffuse_[1], diffuse_[2]);
	mtl->setColor(PFMTL_SPECULAR, specular_[0],specular_[1],specular_[2]);
	mtl->setColorMode(PFMTL_BOTH,PFMTL_CMODE_OFF);
	gstate_->setAttr(PFSTATE_FRONTMTL,mtl);
/*	gstate_->setAttr(PFSTATE_BACKMTL,mtl); */
	gstate_->setMode(PFSTATE_ENLIGHTING,PF_ON);
	if (tex)
		{
		gstate_->setAttr(PFSTATE_TEXTURE,tex->pfTex());
		gstate_->setMode(PFSTATE_ENTEXTURE,PF_ON);
		}
	}
 return gstate_;
}


pfGeoState * softMaterial::defaultGState_=NULL;

pfGeoState * softMaterial::defaultpfGState(void)
{
 if (!defaultGState_)
	{
	pfMaterial *mtl = new pfMaterial;
	mtl->setAlpha(1.0);
	mtl->setShininess(0.0);
	mtl->setColor(PFMTL_AMBIENT, .2, .2, .2);
	mtl->setColor(PFMTL_DIFFUSE, 1.0, 1.0, 1.0);
	defaultGState_ = new pfGeoState;
	defaultGState_->setAttr(PFSTATE_FRONTMTL,mtl);
	defaultGState_->setAttr(PFSTATE_BACKMTL,mtl);
	defaultGState_->setMode(PFSTATE_ENLIGHTING,PF_ON);
	}
 return defaultGState_;
}
