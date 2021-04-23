#ifndef _softPrimitive_h_
#define _softPrimitive_h_

#include <Performer/pf/pfNode.h>
#include "tokenStream.h"
#include "softModel.h"

class softPrimitive
	{
	public:
	 softPrimitive(void) { model_ = NULL; }
	 virtual void parse(tokenStream *)=0;
	 virtual pfNode *pfGraph(void)=0;
	 virtual void setModel(softModel *m) { model_ = m; }
	 virtual class softModel *model(void) { return model_; }
	protected:
	 softModel *model_;
	};

#endif
