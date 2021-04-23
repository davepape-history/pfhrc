#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <bstring.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include "softFace.h"
#include "softMaterial.h"

#define UNSUPPORTED	0
#define LINEAR		1


softFace::softFace(void)
{
 numVerts_ = 0;
 vertex_ = NULL;
 type_ = LINEAR;
 model_ = NULL;
}

void softFace::parse(tokenStream *tstream)
{
#define NEXTTOKEN() { if (!tstream->next(token,sizeof(token))) return; }
 char token[1024];
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
	if (!strcmp(token,"}"))
		loop = 0;
	else if (!strcmp(token,"type"))
		parseType(tstream);
	else if (!strcmp(token,"tension"))
		parseTension(tstream);
	else if (!strcmp(token,"step"))
		parseStep(tstream);
	else if (!strcmp(token,"nbKeys"))
		parseNbKeys(tstream);
	else if (!strcmp(token,"controlPoints"))
		parseControlPoints(tstream);
	else if (!strcmp(token,"{"))
		tstream->skipBlock("{","}");
	}
}

void softFace::parseType(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()
 if (!strcasecmp(token,"LINEAR"))
	type_ = LINEAR;
 else
	type_ = UNSUPPORTED;
}

void softFace::parseTension(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()
}

void softFace::parseStep(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()
}

void softFace::parseNbKeys(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()
 numVerts_ = atoi(token);
}

void softFace::parseControlPoints(tokenStream *tstream)
{
 char token[1024];
 vertex_ = (pfVec3 *) pfMalloc(numVerts_ * sizeof(pfVec3),pfGetSharedArena());
 NEXTTOKEN()		/* Should be "{" */
 NEXTTOKEN()		/* Vertex id in [] */
 while (strcmp(token,"}"))
	{
	int id;
	id = atoi(&token[1]);
	NEXTTOKEN()		/* "position" */
	NEXTTOKEN()
	vertex_[id][0] = atof(token);
	NEXTTOKEN()
	vertex_[id][1] = atof(token);
	NEXTTOKEN()
	vertex_[id][2] = atof(token);
	NEXTTOKEN()		/* Vertex id in [] */
	}
 boundBox_.makeEmpty();
 boundBox_.around(vertex_,numVerts_);
 center_ = (boundBox_.min + boundBox_.max) / 2.0f;
}


pfNode * softFace::pfGraph(void)
{
 pfGeode *geode = new pfGeode;
 pfGeoSet *gset = new pfGeoSet;
 softTexture *texture=NULL;
 int j,texMethod=softTexture::UV_METHOD;
 int *lengths = (int *) pfMalloc(sizeof(int),pfGetSharedArena());
 pfVec3 *verts = (pfVec3 *) pfMalloc(numVerts_*sizeof(pfVec3),pfGetSharedArena());
 pfVec2 *texc = (pfVec2 *) pfMalloc(numVerts_*sizeof(pfVec2),pfGetSharedArena());
 softMaterial *mat=NULL;
 gset->setNumPrims(1);
 lengths[0] = numVerts_;
 gset->setPrimLengths(lengths);
 if (model_)
	mat = model_->findMaterial(0);
 if (mat)
	gset->setGState(mat->pfGState());
 else
	{
	char *name = "";
	if (model_) name = model_->name();
		printf("Warning: softFace::pfGraph (%s): failed to find "
			"material %d\n",name,0);
	gset->setGState(softMaterial::defaultpfGState());
	}
 if (mat)
	texture = mat->texture();
 if (texture)
	texMethod = texture->method();
/* PFGS_POLYS leads to a core dump when setting the Isect mask, so this
   triangulates the polygons instead.  Assumes convex polygons, of course. */
 { int leftI,rightI;
   gset->setPrimType(PFGS_TRISTRIPS);
   for (j=0, rightI=0,leftI=numVerts_-1; j<numVerts_; )
	{
	verts[j] = vertex_[leftI];
	texc[j] = generateTexCoord(texMethod,verts[j]);
	if (texture)
		texture->transform(texc[j]);
	j++; leftI--;
	if (j < numVerts_)
		{
		verts[j] = vertex_[rightI];
		texc[j] = generateTexCoord(texMethod,verts[j]);
		if (texture)
			texture->transform(texc[j]);
		j++; rightI++;
		}
	}
 }
 gset->setAttr(PFGS_COORD3,PFGS_PER_VERTEX,verts,NULL);
 if (texture)
	gset->setAttr(PFGS_TEXCOORD2,PFGS_PER_VERTEX,texc,NULL);
 geode->addGSet(gset);
 return geode;
}

pfVec2 softFace::generateTexCoord(int method,pfVec3 vert)
{
 pfVec2 tc;
 tc.set(0,0);
 if (method == softTexture::XY_METHOD)
	{
	float size;
	size = boundBox_.max[PF_X] - boundBox_.min[PF_X];
	if (size > 0.0f)
		tc[0] = (vert[PF_X] - boundBox_.min[PF_X]) / size;
	size = boundBox_.max[PF_Y] - boundBox_.min[PF_Y];
	if (size > 0.0f)
		tc[1] = (vert[PF_Y] - boundBox_.min[PF_Y]) / size;
	}
 else if (method == softTexture::XZ_METHOD)
	{
	float size;
	size = boundBox_.max[PF_X] - boundBox_.min[PF_X];
	if (size > 0.0f)
		tc[0] = (vert[PF_X] - boundBox_.min[PF_X]) / size;
	size = boundBox_.max[PF_Z] - boundBox_.min[PF_Z];
	if (size > 0.0f)
		tc[1] = (vert[PF_Z] - boundBox_.min[PF_Z]) / size;
	}
 else if (method == softTexture::YZ_METHOD)
	{
	float size;
	size = boundBox_.max[PF_Z] - boundBox_.min[PF_Z];
	if (size > 0.0f)
		tc[0] = (vert[PF_Z] - boundBox_.min[PF_Z]) / size;
	size = boundBox_.max[PF_Y] - boundBox_.min[PF_Y];
	if (size > 0.0f)
		tc[1] = (vert[PF_Y] - boundBox_.min[PF_Y]) / size;
	}
 else if (method == softTexture::CYLINDRICAL_METHOD)
	{
	float size;
	pfVec3 dir;
	dir = vert - center_;
	tc[0] = pfArcTan2(-dir[PF_Z],dir[PF_X]) / 360.0f;
	if (tc[0] < 0.0f)
		tc[0] += 1.0f;
	size = boundBox_.max[PF_Y] - boundBox_.min[PF_Y];
	if (size > 0.0f)
		tc[1] = (vert[PF_Y] - boundBox_.min[PF_Y]) / size;
	}
 else if (method == softTexture::SPHERICAL_METHOD)
	{
	pfVec3 dir;
	dir = vert - center_;
	tc[0] = pfArcTan2(-dir[PF_Z],dir[PF_X]) / 360.0f;
	if (tc[0] < 0.0f)
		tc[0] += 1.0f;
	dir.normalize();
	tc[1] = (pfArcSin(dir[PF_Y]) + 90.0f) / 180.0f;
/*	tc[1] = fatan2(-dir[PF_Y],
			sqrtf(dir[PF_Z]*dir[PF_Z]+dir[PF_X]*dir[PF_X])) / M_PI
*/
	}
 else
	{
	printf("softFace::generateTexCoord(): unsupported method %d\n",method);
	}
 return tc;
}
