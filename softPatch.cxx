#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <bstring.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include "softPatch.h"
#include "softMaterial.h"

enum _patchType
	{
	PATCH_LINEAR,
	PATCH_BSPLINE,
	PATCH_UNKNOWN
	};

softPatch::softPatch(void)
{
 utype_ = PATCH_LINEAR;
 vtype_ = PATCH_LINEAR;
 upoint_ = 0;
 vpoint_ = 0;
 uclose_ = 0;
 vclose_ = 0;
 controlPoint_ = NULL;
 model_ = NULL;
}

void softPatch::parse(tokenStream *tstream)
{
#define NEXTTOKEN() { if (!tstream->next(token,sizeof(token))) return; }
 char token[1024];
 enum { PATCH_UTYPE, PATCH_UPOINT, PATCH_VTYPE, PATCH_VPOINT, PATCH_CONTROLPOINTS,
	 PATCH_UCLOSE, PATCH_VCLOSE, PATCH_OTHER, OPEN_BRACE, CLOSE_BRACE };
 tokenStream::tokenVal table[] = {
		{ "utype", PATCH_UTYPE },
		{ "upoint", PATCH_UPOINT },
		{ "uclose", PATCH_UCLOSE },
		{ "vtype", PATCH_VTYPE },
		{ "vpoint", PATCH_VPOINT },
		{ "vclose", PATCH_VCLOSE },
		{ "controlPoints", PATCH_CONTROLPOINTS },
		{ "utension", PATCH_OTHER },
		{ "ustep", PATCH_OTHER },
		{ "ucurve", PATCH_OTHER },
		{ "vtension", PATCH_OTHER },
		{ "vstep", PATCH_OTHER },
		{ "vcurve", PATCH_OTHER },
		{ "{", OPEN_BRACE },
		{ "}", CLOSE_BRACE },
		{ NULL, -1 }
		};
 tokenStream::tokenVal type_table[] = {
		{ "LINEAR", PATCH_LINEAR },
		{ "BSPLINE", PATCH_BSPLINE },
		{ NULL, -1 }
		};
 int loop=1;
 if (!tstream)
	return;
 do
	{
	NEXTTOKEN()
	}  while (strcmp(token,"{"));
 while (loop)
	{
	NEXTTOKEN()
	switch (tstream->lookup(token,table))
		{
		case CLOSE_BRACE: loop = 0; break;
		case OPEN_BRACE: tstream->skipBlock("{","}"); break;
		case PATCH_UTYPE:
				NEXTTOKEN()
				utype_ = tstream->lookup(token,type_table);
				break;
		case PATCH_UPOINT:
				NEXTTOKEN()
				upoint_ = atoi(token);
				break;
		case PATCH_UCLOSE:
				uclose_ = 1;
				break;
		case PATCH_VTYPE:
				NEXTTOKEN()
				vtype_ = tstream->lookup(token,type_table);
				break;
		case PATCH_VPOINT:
				NEXTTOKEN()
				vpoint_ = atoi(token);
				break;
		case PATCH_VCLOSE:
				vclose_ = 1;
				break;
		case PATCH_CONTROLPOINTS:
				parseControlPoints(tstream);
				break;
		case PATCH_OTHER: NEXTTOKEN() break;
		}
	}
}

void softPatch::parseControlPoints(tokenStream *tstream)
{
 char token[1024];
 if ((upoint_ <= 0) || (vpoint_ <= 0))
	{
	printf("Warning: softPatch::parseControlPoints: bad number of points "
		"(upoint=%d, vpoint=%d)\n",upoint_,vpoint_);
	return;
	}
 if (controlPoint_)
	pfFree(controlPoint_);
 controlPoint_ = (pfVec3 *) pfMalloc(upoint_ * vpoint_ * sizeof(pfVec3),pfGetSharedArena());
 NEXTTOKEN()		/* Should be "{" */
 NEXTTOKEN()		/* Control point id in [] */
 while (strcmp(token,"}"))
	{
	int u,v;
	sscanf(token,"[%d,%d]",&u,&v);
	NEXTTOKEN()		/* "position" */
	NEXTTOKEN()
	controlPoint_[u+v*upoint_][0] = atof(token);
	NEXTTOKEN()
	controlPoint_[u+v*upoint_][1] = atof(token);
	NEXTTOKEN()
	controlPoint_[u+v*upoint_][2] = atof(token);
	NEXTTOKEN()		/* Vertex id in [] */
	}
 boundBox_.makeEmpty();
 boundBox_.around(controlPoint_, upoint_ * vpoint_);
}


pfNode * softPatch::pfGraph(void)
{
 pfGeode *geode = new pfGeode;
 pfGeoSet *gset = new pfGeoSet;
 int numPrims, *lengths;
 ushort *ilist;
 pfVec3 *norms;
 pfVec2 *texc;
 softMaterial *mat=NULL;
 softTexture *texture=NULL;
 int u,v,index;
 if (!controlPoint_)
	{
	char *name = "";
	if (model_) name = model_->name();
	printf("ERROR: softPatch::pfGraph (%s): no control points\n",name);
	return NULL;
	}
 gset->setPrimType(PFGS_TRISTRIPS);
 if (vclose_)
	numPrims = vpoint_;
 else
	numPrims = vpoint_-1;
 gset->setNumPrims(numPrims);
 lengths = (int *) pfMalloc(numPrims*sizeof(int),pfGetSharedArena());
 for (v=0; v<numPrims; v++)
	lengths[v] = upoint_ * 2;
 gset->setPrimLengths(lengths);
 if (model_)
	mat = model_->findMaterial(0);
 if (mat)
	gset->setGState(mat->pfGState());
 else
	{
	char *name = "";
	if (model_) name = model_->name();
	printf("Warning: softPatch::pfGraph (%s): failed to find material 0\n",name);
	gset->setGState(softMaterial::defaultpfGState());
	}
 if (vclose_)
	ilist = (ushort *) pfMalloc((2 * upoint_) * vpoint_ * sizeof(ushort),
				pfGetSharedArena());
 else
	ilist = (ushort *) pfMalloc((2 * upoint_) * (vpoint_ - 1) * sizeof(ushort),
				pfGetSharedArena());
 for (v = 0, index = 0; v < vpoint_-1; v++)
	for (u = 0; u < upoint_; u++)
		{
		ilist[index++] = u + v * upoint_;
		ilist[index++] = u + v * upoint_ + upoint_;
		}
 if (vclose_)
	for (u = 0; u < upoint_; u++)
		{
		ilist[index++] = u + (vpoint_-1) * upoint_;
		ilist[index++] = u;
		}
 gset->setAttr(PFGS_COORD3,PFGS_PER_VERTEX,controlPoint_,ilist);
 norms = (pfVec3 *) pfMalloc(upoint_ * vpoint_ * sizeof(pfVec3), pfGetSharedArena());
 computeNormals(norms);
 gset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, norms, ilist);
 if (mat)
	texture = mat->texture();
 if (!texture)
	texture = model_->texture();
 if (texture)
	{
	computeTexCoords(texture,&texc,&ilist);
	gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, texc, ilist);
	}
 geode->addGSet(gset);
 return geode;
}

void softPatch::computeNormals(pfVec3 *norm)
{
 int u,v,index;
 pfVec3 a,b,crossprod;
 for (u=0; u < upoint_; u++)
	for (v=0; v < vpoint_; v++)
		{
		index = u + v * upoint_;
		norm[index].set(0,0,0);
		if ((u < upoint_ - 1) && (v < vpoint_ - 1))
			{
			a = controlPoint_[index+1] - controlPoint_[index];
			b = controlPoint_[index+upoint_] - controlPoint_[index];
			crossprod.cross(a,b);
			norm[index] += crossprod;
			}
		if ((u) && (v < vpoint_ - 1))
			{
			a = controlPoint_[index+upoint_] - controlPoint_[index];
			b = controlPoint_[index-1] - controlPoint_[index];
			crossprod.cross(a,b);
			norm[index] += crossprod;
			}
		if ((u) && (v))
			{
			a = controlPoint_[index-1] - controlPoint_[index];
			b = controlPoint_[index-upoint_] - controlPoint_[index];
			crossprod.cross(a,b);
			norm[index] += crossprod;
			}
		if ((u < upoint_ - 1) && (v))
			{
			a = controlPoint_[index-upoint_] - controlPoint_[index];
			b = controlPoint_[index+1] - controlPoint_[index];
			crossprod.cross(a,b);
			norm[index] += crossprod;
			}
		norm[index].normalize();
		norm[index] *= -1.0f;
		}
}

void softPatch::computeTexCoords(softTexture *texture,pfVec2 **texc,ushort **ilist)
{
 int index,u,v,udim=upoint_,vdim=vpoint_;
 if (vclose_)
	vdim = vpoint_ + 1;
 *texc = (pfVec2 *) pfMalloc(udim * vdim * sizeof(pfVec2),pfGetSharedArena());
 if (texture->method() == softTexture::UV_METHOD)
	{
	for (v = 0; v < vdim; v++)
	    for (u = 0; u < udim; u++)
		{
		index = u + v * udim;
		(*texc)[index][0] = ((float)u)/upoint_;
		(*texc)[index][1] = ((float)v)/vpoint_;
		texture->transform((*texc)[index]);
		}
	}
 else
	{
	for (v = 0; v < vdim; v++)
	    for (u = 0; u < udim; u++)
		{
		index = u + v * udim;
		(*texc)[index] = texture->generateTexCoord(
						controlPoint_[u + (v%vpoint_)*vpoint_],
						boundBox_);
		texture->transform((*texc)[index]);
		}
	}
 *ilist = (ushort *) pfMalloc((2 * udim) * (vdim - 1) * sizeof(ushort),
				pfGetSharedArena());
 for (v = 0, index = 0; v < vdim - 1; v++)
	for (u = 0; u < udim; u++)
		{
		(*ilist)[index++] = u + v * udim;
		(*ilist)[index++] = u + (v+1) * udim;
		}
}
