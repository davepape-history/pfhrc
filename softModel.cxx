#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <bstring.h>
#include <Performer/pf/pfSCS.h>
#include "softModel.h"
#include "softMesh.h"
#include "softFace.h"
#include "softPatch.h"
#include "softSpline.h"
#include "softMaterial.h"

softModel::softModel(void)
{
 name_ = strdup("");
 scaling_.set(1,1,1);
 rotation_.set(0,0,0);
 translation_.set(0,0,0);
 primitives_ = new pfList;
 materials_ = new pfList;
 children_ = new pfList;
 parent_ = NULL;
 texture_ = NULL;
 pftexCache_ = new pfList;
}

void softModel::setParent(softModel *parent)
{
 parent_ = parent;
}

void softModel::parse(tokenStream *tstream)
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
	else if (!strcmp(token,"name"))
		{
		NEXTTOKEN()
		name_ = strdup(token);
		}
	else if (!strcmp(token,"scaling"))
		{
		NEXTTOKEN()
		scaling_[0] = atof(token);
		NEXTTOKEN()
		scaling_[1] = atof(token);
		NEXTTOKEN()
		scaling_[2] = atof(token);
		}
	else if (!strcmp(token,"rotation"))
		{
		NEXTTOKEN()
		rotation_[0] = atof(token);
		NEXTTOKEN()
		rotation_[1] = atof(token);
		NEXTTOKEN()
		rotation_[2] = atof(token);
		}
	else if (!strcmp(token,"translation"))
		{
		NEXTTOKEN()
		translation_[0] = atof(token);
		NEXTTOKEN()
		translation_[1] = atof(token);
		NEXTTOKEN()
		translation_[2] = atof(token);
		}
	else if (!strcmp(token,"model"))
		{
		softModel *child = new softModel;
		child->setParent(this);
		child->parse(tstream);
		children_->add(child);
		}
	else if (!strcmp(token,"mesh"))
		{
		softMesh *mesh = new softMesh;
		mesh->setModel(this);
		mesh->parse(tstream);
		primitives_->add(mesh);
		}
	else if (!strcmp(token,"face"))
		{
		softFace *face = new softFace;
		face->setModel(this);
		face->parse(tstream);
		primitives_->add(face);
		}
	else if (!strcmp(token,"patch"))
		{
		softPatch *patch = new softPatch;
		patch->setModel(this);
		patch->parse(tstream);
		primitives_->add(patch);
		}
	else if (!strcmp(token,"spline"))
		{
		softSpline *spline = new softSpline;
		spline->setModel(this);
		spline->parse(tstream);
		primitives_->add(spline);
		}
	else if (!strcmp(token,"material"))
		{
		softMaterial *material = new softMaterial;
		material->setModel(this);
		material->parse(tstream);
		materials_->add(material);
		}
	else if (!strcmp(token,"texture"))
		{
		texture_ = new softTexture;
		texture_->setModel(this);
		texture_->parse(tstream);
		}
	else if (!strcmp(token,"{"))
		tstream->skipBlock("{","}");
	else
		{
		char message[512];
		sprintf(message,"Unrecognized keyword \"%s\"",token);
		pfNotify(PFNFY_NOTICE, PFNFY_INTERNAL, message);
		}
	}
}

pfNode * softModel::pfGraph(void)
{
 pfSCS *scs;
 pfMatrix scsMat;
 int i;
 scsMat.makeTrans(translation_[0],translation_[1],translation_[2]);
 scsMat.preRot(rotation_[2],0.0,0.0,1.0,scsMat);
 scsMat.preRot(rotation_[0],1.0,0.0,0.0,scsMat);
 scsMat.preRot(rotation_[1],0.0,1.0,0.0,scsMat);
 scsMat.preScale(scaling_[0],scaling_[1],scaling_[2],scsMat);
 scs = new pfSCS(scsMat);
 for (i=0; i < children_->getNum(); i++)
	{
	softModel *child = (softModel *) children_->get(i);
	scs->addChild(child->pfGraph());
	}
 for (i=0; i < primitives_->getNum(); i++)
	{
	softPrimitive *prim = (softPrimitive *) primitives_->get(i);
	scs->addChild(prim->pfGraph());
	}
 return scs;
}

softMaterial * softModel::findMaterial(int id)
{
 int i;
 for (i=0; i < materials_->getNum(); i++)
	{
	softMaterial *mat = (softMaterial *) materials_->get(i);
	if (mat->ID() == id)
		return mat;
	}
 if (parent_)
	return parent_->findMaterial(id);
 return NULL;
}

struct _tex
	{
	char *name;
	pfTexture *tex;
	};

void softModel::cachePfTex(char *name,pfTexture *tex)
{
 if (parent_)
	parent_->cachePfTex(name,tex);
 else
	{
	struct _tex *entry = new struct _tex;
	entry->name = strdup(name);
	entry->tex = tex;
	pftexCache_->add(entry);
	}
}

pfTexture * softModel::findPfTex(char *name)
{
 if ((parent_) && (parent_->findPfTex(name)))
	return parent_->findPfTex(name);
 else
	{
	int i;
	for (i=0; i < pftexCache_->getNum(); i++)
		{
		struct _tex *entry = (struct _tex *) pftexCache_->get(i);
		if (!strcmp(name,entry->name))
			return entry->tex;
		}
	}
 return NULL;
}
