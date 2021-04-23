#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <bstring.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include "softSpline.h"
#include "softMaterial.h"

enum _splineType
	{
	SPLINE_LINEAR,
	SPLINE_BSPLINE,
	SPLINE_UNKNOWN
	};

softSpline::softSpline(void)
{
 type_ = SPLINE_LINEAR;
 nbKeys_ = 0;
 controlPoint_ = NULL;
 model_ = NULL;
}

void softSpline::parse(tokenStream *tstream)
{
#define NEXTTOKEN() { if (!tstream->next(token,sizeof(token))) return; }
 char token[1024];
 enum { SPLINE_TYPE, SPLINE_NBKEYS, SPLINE_CONTROLPOINTS,
	 SPLINE_OTHER, OPEN_BRACE, CLOSE_BRACE };
 tokenStream::tokenVal table[] = {
		{ "type", SPLINE_TYPE },
		{ "nbKeys", SPLINE_NBKEYS },
		{ "controlPoints", SPLINE_CONTROLPOINTS },
		{ "tension", SPLINE_OTHER },
		{ "step", SPLINE_OTHER },
		{ "{", OPEN_BRACE },
		{ "}", CLOSE_BRACE },
		{ NULL, -1 }
		};
 tokenStream::tokenVal type_table[] = {
		{ "LINEAR", SPLINE_LINEAR },
		{ "BSPLINE", SPLINE_BSPLINE },
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
		case SPLINE_TYPE:
				NEXTTOKEN()
				type_ = tstream->lookup(token,type_table);
				break;
		case SPLINE_NBKEYS:
				NEXTTOKEN()
				nbKeys_ = atoi(token);
				break;
		case SPLINE_CONTROLPOINTS:
				parseControlPoints(tstream);
				break;
		case SPLINE_OTHER: NEXTTOKEN() break;
		}
	}
}

void softSpline::parseControlPoints(tokenStream *tstream)
{
 char token[1024];
 if (nbKeys_ <= 0)
	{
	printf("Warning: softSpline::parseControlPoints: bad number of control points "
		"(nbKeys=%d)\n",nbKeys_);
	return;
	}
 if (controlPoint_)
	pfFree(controlPoint_);
 controlPoint_ = (pfVec3 *) pfMalloc(nbKeys_ * sizeof(pfVec3),pfGetSharedArena());
 NEXTTOKEN()		/* Should be "{" */
 NEXTTOKEN()		/* Control point id in [] */
 while (strcmp(token,"}"))
	{
	int u;
	sscanf(token,"[%d]",&u);
	NEXTTOKEN()		/* "position" */
	NEXTTOKEN()
	controlPoint_[u][0] = atof(token);
	NEXTTOKEN()
	controlPoint_[u][1] = atof(token);
	NEXTTOKEN()
	controlPoint_[u][2] = atof(token);
	NEXTTOKEN()		/* Vertex id in [] */
	}
 boundBox_.makeEmpty();
 boundBox_.around(controlPoint_, nbKeys_);
}


pfNode * softSpline::pfGraph(void)
{
 pfGeode *geode = new pfGeode;
 pfGeoSet *gset = new pfGeoSet;
 int *lengths;
 softMaterial *mat=NULL;
 if (!controlPoint_)
	{
	char *name = "";
	if (model_) name = model_->name();
	printf("ERROR: softSpline::pfGraph (%s): no control points\n",name);
	return NULL;
	}
 gset->setPrimType(PFGS_LINESTRIPS);
 gset->setNumPrims(1);
 lengths = (int *) pfMalloc(sizeof(int),pfGetSharedArena());
 lengths[0] = nbKeys_;
 gset->setPrimLengths(lengths);
 if (model_)
	mat = model_->findMaterial(0);
 if (mat)
	gset->setGState(mat->pfGState());
 else
	{
	char *name = "";
	if (model_) name = model_->name();
	printf("Warning: softSpline::pfGraph (%s): failed to find material 0\n",name);
	gset->setGState(softMaterial::defaultpfGState());
	}
 gset->setAttr(PFGS_COORD3,PFGS_PER_VERTEX,controlPoint_,NULL);
 geode->addGSet(gset);
 return geode;
}
