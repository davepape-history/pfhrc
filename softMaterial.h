#ifndef _softMaterial_h_
#define _softMaterial_h_

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoState.h>
#include "tokenStream.h"
#include "softModel.h"
#include "softTexture.h"

class softMaterial
	{
	public:
	 softMaterial(void);
	 void parse(tokenStream *);
	 pfGeoState *pfGState(void);
	 void setModel(softModel *);
	 class softModel *model(void) { return model_; }
	 int ID(void) { return id_; };
	 softTexture *texture(void);
	 static pfGeoState *defaultpfGState(void);
	private:
	 int id_;
	 char *name_;
	 int type_;
	 pfVec3 ambient_;
	 pfVec3 diffuse_;
	 pfVec3 specular_;
	 float exponent_;
	 float transparency_;
	 softTexture *texture_;
	 softModel *model_;
	 pfGeoState *gstate_;
	 void parseType(tokenStream *);
	 static pfGeoState *defaultGState_;
	};

#endif
