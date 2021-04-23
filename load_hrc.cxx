#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "softModel.h"



extern "C"
pfNode * pfdLoadFile_hrc(char *file)
{
 char path[PF_MAXSTRING];
 if ((file) && (pfFindFile(file,path,R_OK)))
	{
	pfNode *node;
	softModel *model = new softModel;
	tokenStream *tstream;
	tstream = new tokenStream(path);
	model->parse(tstream);
	node = model->pfGraph();
	delete model;
	delete tstream;
	return node;
	}
 else
	return NULL;
}
