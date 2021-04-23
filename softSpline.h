#ifndef _softSpline_h_
#define _softSpline_h_

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>
#include <Performer/pf/pfNode.h>
#include "tokenStream.h"
#include "softModel.h"
#include "softPrimitive.h"

class softSpline : public softPrimitive
	{
	public:
	 softSpline(void);
	 void parse(tokenStream *);
	 pfNode *pfGraph(void);
	private:
	 void parseControlPoints(tokenStream *);
	 int type_;
	 int nbKeys_;
	 pfVec3 *controlPoint_;
	 pfBox boundBox_;
	};

#endif
