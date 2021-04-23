#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <bstring.h>
#include "softTexture.h"


softTexture::softTexture(void)
{
 name_ = strdup("");
 uvswap_ = 0;
 uAlternate_ = 0;
 vAlternate_ = 0;
 repeat_[0] = repeat_[1] = 1;
 transparency_ = 0;
 model_ = NULL;
 tex_ = NULL;
 loadFailed_ = 0;
}

void softTexture::setModel(softModel *model)
{
 model_ = model;
}

void softTexture::parse(tokenStream *tstream)
{
#define NEXTTOKEN() { if (!tstream->next(token,sizeof(token))) return; }
 char token[1024];
 enum { TEX_NAME, TEX_METHOD, TEX_REPEAT, TEX_UVSWAP, TEX_UALTERNATE,
	 TEX_VALTERNATE, TEX_TRANSPARENCY, OPEN_BRACE, CLOSE_BRACE };
 tokenStream::tokenVal table[] = {
		{ "name", TEX_NAME },
		{ "method", TEX_METHOD },
		{ "repeat", TEX_REPEAT },
		{ "uvswap", TEX_UVSWAP },
		{ "uAlternate", TEX_UALTERNATE },
		{ "vAlternate", TEX_VALTERNATE },
		{ "transparency", TEX_TRANSPARENCY },
		{ "{", OPEN_BRACE },
		{ "}", CLOSE_BRACE },
		{ NULL, -1 }
		};
 tokenStream::tokenVal method_table[] = {
		{ "UV", UV_METHOD },
		{ "XY", XY_METHOD },
		{ "XZ", XZ_METHOD },
		{ "YZ", YZ_METHOD },
		{ "SPHERICAL", SPHERICAL_METHOD },
		{ "CYLINDRICAL", CYLINDRICAL_METHOD },
		{ NULL, -1 }
		};
 int loop=1;
 if (!tstream)
	return;
 NEXTTOKEN()		/* texture ID in [] */
 NEXTTOKEN()		/* "{" */
 while (loop)
	{
	NEXTTOKEN()
	switch (tstream->lookup(token,table))
		{
		case CLOSE_BRACE: loop = 0; break;
		case OPEN_BRACE: tstream->skipBlock("{","}"); break;
		case TEX_NAME:	NEXTTOKEN()
				name_ = strdup(token);
				break;
		case TEX_METHOD: NEXTTOKEN()
				method_ = tstream->lookup(token,method_table);
				break;
		case TEX_REPEAT: repeat_[0] = tstream->nextFloat();
				repeat_[1] = tstream->nextFloat();
				break;
		case TEX_UVSWAP: uvswap_ = 1;
				break;
		case TEX_UALTERNATE: uAlternate_ = 1;
				break;
		case TEX_VALTERNATE: vAlternate_ = 1;
				break;
		case TEX_TRANSPARENCY: transparency_ = tstream->nextFloat();
				break;
		}
	}
}

pfTexture * softTexture::pfTex(void)
{
 if ((!tex_) && (!loadFailed_))
    {
    if ((model_) && (model_->findPfTex(name_)))
	tex_ = model_->findPfTex(name_);
    else
	{
	char filename[256], path[PF_MAXSTRING],
		*ext[] = { ".rgb", ".rgba", ".bw", ".sgi", NULL };
	int i;
	for (i=0; ext[i]; i++)
		{
		sprintf(filename,"%s%s",name_,ext[i]);
		if (pfFindFile(filename,path,R_OK))
			break;
		}
	if (ext[i])
		{
		printf("Loading texture %s\n",name_);
		tex_ = new pfTexture;
		tex_->loadFile(filename);
		if (model_)
			model_->cachePfTex(name_,tex_);
		}
	else
		{
		printf("softTexture::pfTex(): failed to find \"%s\"\n",name_);
		loadFailed_ = 1;
		}
	}
    }
 return tex_;
}

void softTexture::transform(pfVec2 & uv)
{
 uv[0] *= repeat_[0];
 uv[1] *= repeat_[1];
 if (uvswap_)
	{
	float tmp = uv[0];
	uv[0] = repeat_[1] - uv[1];	/* What the hell does Softimage think it's doing? */
	uv[1] = repeat_[0] - tmp;
	}
#if 1
/*  This is a quick hack for the rompiazza model; it works as long as u & v are in [0,1] */
 if (uAlternate_)
	uv[0] = 1.0f - uv[0];
 if (vAlternate_)
	uv[1] = 1.0f - uv[1];
#endif
}

pfVec2 softTexture::generateTexCoord(pfVec3 vert,pfBox boundBox)
{
 pfVec2 tc;
 tc.set(0,0);
 if (method_ == XY_METHOD)
	{
	float size;
	size = boundBox.max[PF_X] - boundBox.min[PF_X];
	if (size > 0.0f)
		tc[0] = (vert[PF_X] - boundBox.min[PF_X]) / size;
	size = boundBox.max[PF_Y] - boundBox.min[PF_Y];
	if (size > 0.0f)
		tc[1] = (vert[PF_Y] - boundBox.min[PF_Y]) / size;
	}
 else if (method_ == XZ_METHOD)
	{
	float size;
	size = boundBox.max[PF_X] - boundBox.min[PF_X];
	if (size > 0.0f)
		tc[0] = (vert[PF_X] - boundBox.min[PF_X]) / size;
	size = boundBox.max[PF_Z] - boundBox.min[PF_Z];
	if (size > 0.0f)
		tc[1] = (vert[PF_Z] - boundBox.min[PF_Z]) / size;
	}
 else if (method_ == YZ_METHOD)
	{
	float size;
	size = boundBox.max[PF_Z] - boundBox.min[PF_Z];
	if (size > 0.0f)
		tc[0] = (vert[PF_Z] - boundBox.min[PF_Z]) / size;
	size = boundBox.max[PF_Y] - boundBox.min[PF_Y];
	if (size > 0.0f)
		tc[1] = (vert[PF_Y] - boundBox.min[PF_Y]) / size;
	}
 else if (method_ == CYLINDRICAL_METHOD)
	{
	float size;
	pfVec3 dir;
	dir = vert - (boundBox.min + boundBox.max) / 2.0f;
	tc[0] = pfArcTan2(-dir[PF_Z],dir[PF_X]) / 360.0f;
	if (tc[0] < 0.0f)
		tc[0] += 1.0f;
	size = boundBox.max[PF_Y] - boundBox.min[PF_Y];
	if (size > 0.0f)
		tc[1] = (vert[PF_Y] - boundBox.min[PF_Y]) / size;
	}
 else if (method_ == SPHERICAL_METHOD)
	{
	pfVec3 dir;
	dir = vert - (boundBox.min + boundBox.max) / 2.0f;
	tc[0] = pfArcTan2(-dir[PF_Z],dir[PF_X]) / 360.0f;
	if (tc[0] < 0.0f)
		tc[0] += 1.0f;
	dir.normalize();
	tc[1] = (pfArcSin(dir[PF_Y]) + 90.0f) / 180.0f;
/*	tc[1] = fatan2(-dir[PF_Y],
			sqrtf(dir[PF_Z]*dir[PF_Z]+dir[PF_X]*dir[PF_X])) / M_PI
*/
	}
 else if (method_ == UV_METHOD)
	;
 else
	{
	printf("softTexture::generateTexCoord(): unsupported method for texture \"%s\"\n",
		name_);
	}
 return tc;
}
