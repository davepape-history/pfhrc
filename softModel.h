#ifndef _softModel_h_
#define _softModel_h_

class softModel;

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfList.h>
#include <Performer/pf/pfNode.h>
#include "tokenStream.h"
#include "softPrimitive.h"

class softModel
	{
	public:
	 softModel(void);
	 void parse(tokenStream *);
	 pfNode *pfGraph(void);
	 class softMaterial * findMaterial(int id);
	 void setParent(class softModel *);
	 class softModel *parent(void) { return parent_; }
	 char *name(void) { return name_; }
	 class softTexture * texture(void) { return texture_; }
	 pfTexture * findPfTex(char *name);
	 void cachePfTex(char *name,pfTexture *tex);
	private:
	 char *name_;
	 pfVec3 scaling_,rotation_,translation_;
	 pfList *primitives_;
	 pfList *materials_;
	 pfList *children_;
	 class softModel *parent_;
	 class softTexture *texture_;
	 pfList *pftexCache_;
	};

#endif
