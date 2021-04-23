#ifndef _softTexture_h_
#define _softTexture_h_

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfTexture.h>
#include "tokenStream.h"
#include "softModel.h"

class softTexture
	{
	public:
	 softTexture(void);
	 void parse(tokenStream *);
	 pfTexture *pfTex(void);
	 void setModel(softModel *);
	 class softModel *model(void) { return model_; }
	 int method(void) { return method_; }
	 char *name(void) { return name_; }
	 float transparency(void) { return transparency_; }
	 void transform(pfVec2 & uv);
	 pfVec2 generateTexCoord(pfVec3 vert,pfBox boundBox);
	 enum { UV_METHOD, XY_METHOD, XZ_METHOD, YZ_METHOD, SPHERICAL_METHOD,
		CYLINDRICAL_METHOD };
	private:
	 char *name_;
	 int method_;
	 pfVec2 repeat_;
	 int uvswap_;
	 int uAlternate_, vAlternate_;
	 float transparency_;
	 softModel *model_;
	 pfTexture *tex_;
	 int loadFailed_;
	};

#endif
