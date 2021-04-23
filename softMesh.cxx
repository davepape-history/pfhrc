#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <bstring.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include "softMesh.h"
#include "softMaterial.h"

struct _polylist
	{
	int numPolys;
	struct _polygon *list;
	};

struct _polygon
	{
	int numNodes;
	int *vertexID;
	pfVec3 *normal;
	pfVec2 *uvTexture;
	int materialID;
	};


softMesh::softMesh(void)
{
 numVerts_ = 0;
 vertex_ = NULL;
 polygons_ = new pfList;
 model_ = NULL;
}

void softMesh::parse(tokenStream *tstream)
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
	else if (!strcmp(token,"flag"))
		parseFlag(tstream);
	else if (!strcmp(token,"discontinuity"))
		parseDiscontinuity(tstream);
	else if (!strcmp(token,"vertices"))
		parseVertices(tstream);
	else if (!strcmp(token,"polygons"))
		parsePolygons(tstream);
	else if (!strcmp(token,"edges"))
		parseEdges(tstream);
	else if (!strcmp(token,"{"))
		tstream->skipBlock("{","}");
	}
}

void softMesh::parseFlag(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()
 if (!strcmp(token,"("))
	tstream->skipBlock("(",")");
}

void softMesh::parseDiscontinuity(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()
}

void softMesh::parseVertices(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()		/* Number of vertices */
 numVerts_ = atoi(token);
 vertex_ = (pfVec3 *) pfMalloc(numVerts_ * sizeof(pfVec3));
 NEXTTOKEN()		/* Should be "{" */
 NEXTTOKEN()		/* Vertex id in [] */
 while (strcmp(token,"}"))
	{
	if (token[0] == '[')
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
	else if (!strcmp(token,"flag"))
		{
		NEXTTOKEN()
		if (!strcmp(token,"("))
			tstream->skipBlock("(",")");
		NEXTTOKEN()
		}
	}
 boundBox_.makeEmpty();
 boundBox_.around(vertex_,numVerts_);
}

void softMesh::parsePolygons(tokenStream *tstream)
{
 char token[1024];
 struct _polygon *p=NULL;
 struct _polylist *polylist;
 int loop=1, num;
 NEXTTOKEN()		/* Number of polygons */
 num = atoi(token);
 polylist = (struct _polylist *) pfMalloc(sizeof(struct _polylist));
 polylist->numPolys = num;
 polylist->list = (struct _polygon *) pfMalloc(num * sizeof(struct _polygon));
 polygons_->add(polylist);
 NEXTTOKEN()		/* Should be "{" */
 while (loop)
	{
	NEXTTOKEN()
	if (!strcmp(token,"}"))
		loop = 0;
	else if (token[0] == '[')
		{
		p = &(polylist->list[atoi(token+1)]);
		parseNextPolygon(tstream,p);
		}
	else if (!strcmp(token,"material"))
		{
		NEXTTOKEN()		/* material number */
		if (p)
			p->materialID = atoi(token);
		}
	else if (!strcmp(token,"flag"))
		{
		NEXTTOKEN()		/* "(" */
		tstream->skipBlock("(",")");
		}
	else
		printf("softMesh::parsePolygons: unknown token \"%s\"\n",
			token);
	}
}

void softMesh::parseNextPolygon(tokenStream *tstream,struct _polygon *p)
{
 char token[1024];
 int nodeID, numNodes;
 NEXTTOKEN()		/* "nodes" */
 NEXTTOKEN()		/* number of nodes */
 p->numNodes = numNodes = atoi(token);
 p->vertexID = (int *) pfMalloc(numNodes * sizeof(int));
 p->normal = (pfVec3 *) pfMalloc(numNodes * sizeof(pfVec3));
 p->uvTexture = (pfVec2 *) pfMalloc(numNodes * sizeof(pfVec2));
 NEXTTOKEN()		/* "{" */
 NEXTTOKEN()		/* nodeID in [] */
 while (strcmp(token,"}"))
	{
	nodeID = atoi(&token[1]);
	NEXTTOKEN()	/* "vertex" */
	NEXTTOKEN()	/* vertex ID */
	p->vertexID[nodeID] = atoi(token);
	NEXTTOKEN()	/* "normal" */
	NEXTTOKEN()
	p->normal[nodeID][0] = atof(token);
	NEXTTOKEN()
	p->normal[nodeID][1] = atof(token);
	NEXTTOKEN()
	p->normal[nodeID][2] = atof(token);
	NEXTTOKEN()	/* "uvTexture" */
	NEXTTOKEN()
	p->uvTexture[nodeID][0] = atof(token);
	NEXTTOKEN()
	p->uvTexture[nodeID][1] = atof(token);
	NEXTTOKEN()	/* should be next node ID or "}" */
	}
}

void softMesh::parseEdges(tokenStream *tstream)
{
 char token[1024];
 NEXTTOKEN()		/* Number of edges */
 NEXTTOKEN()		/* "{" */
 tstream->skipBlock("{","}");
}



pfNode * softMesh::pfGraph(void)
{
 pfGroup * group = new pfGroup;
 int p;
 for (p=0; p < polygons_->getNum(); p++)
	buildPolygons((struct _polylist *) polygons_->get(p),group);
 return group;
}


void softMesh::buildPolygons(struct _polylist *polylist,pfGroup * parent)
{
 int i;
 for (i=1; i < polylist->numPolys; i++)
	if (polylist->list[i].materialID != polylist->list[0].materialID)
		{
		buildPolygonsInSeparateGeosets(polylist,parent);
		return;
		}
 buildPolygonsInOneGeoset(polylist,parent);
}


void softMesh::buildPolygonsInSeparateGeosets(struct _polylist *polylist,pfGroup *parent)
{
 pfGeode * geode = new pfGeode;
 softTexture *texture=NULL;
 int i,j,polyCount=0;
 parent->addChild(geode);
 for (i=0; i < polylist->numPolys; i++, polyCount++)
	{
	struct _polygon *p = &polylist->list[i];
	int *lengths = (int *) pfMalloc(sizeof(int),pfGetSharedArena());
	pfVec3 *verts = (pfVec3 *) pfMalloc(p->numNodes*sizeof(pfVec3),
						pfGetSharedArena());
	pfVec3 *norms = (pfVec3 *) pfMalloc(p->numNodes*sizeof(pfVec3),
						pfGetSharedArena());
	pfVec2 *texc;
	softMaterial *mat;
	pfGeoSet *gset = new pfGeoSet;
 	gset->setNumPrims(1);
	lengths[0] = p->numNodes;
	gset->setPrimLengths(lengths);
	mat = model_->findMaterial(p->materialID);
	if (mat)
		gset->setGState(mat->pfGState());
	else
		{
		char *name = "";
		if (model_) name = model_->name();
		printf("Warning: softMesh::pfGraph (%s): failed to find "
			"material %d\n",name,p->materialID);
		gset->setGState(softMaterial::defaultpfGState());
		}
	if (mat)
		texture = mat->texture();
	if (!texture)
		texture = model_->texture();
	if (texture)
		texc = (pfVec2 *) pfMalloc(p->numNodes*sizeof(pfVec2),pfGetSharedArena());
#if 0
/* See below (in #else clause) */
	if (p->numNodes == 3)
		gset->setPrimType(PFGS_TRIS);
	else if (p->numNodes == 4)
		gset->setPrimType(PFGS_QUADS);
	else
		gset->setPrimType(PFGS_POLYS);
	for (j=0; j<p->numNodes; j++)
		{
		verts[j] = vertex_[p->vertexID[j]];
		norms[j] = p->normal[j];
		if (texture)
			{
			if (texture->method() == softTexture::UV_METHOD)
				texc[j] = p->uvTexture[j];
			else
				texc[j] = texture->generateTexCoord(verts[j],boundBox_);
			texture->transform(texc[j]);
			}
		}
#else
/* PFGS_POLYS leads to a core dump when setting the Isect mask, so this
   triangulates the polygons instead.  Assumes convex polygons, of course. */
/* The PFGS_POLYS bug is fixed in Performer 2.0.2 */
	{ int leftI,rightI;
	gset->setPrimType(PFGS_TRISTRIPS);
	for (j=0, rightI=0,leftI=p->numNodes-1; j<p->numNodes; )
		{
		verts[j] = vertex_[p->vertexID[leftI]];
		norms[j] = p->normal[leftI];
		if (texture)
			{
			if (texture->method() == softTexture::UV_METHOD)
				texc[j] = p->uvTexture[leftI];
			else
				texc[j] = texture->generateTexCoord(verts[j],boundBox_);
			texture->transform(texc[j]);
			}
		j++; leftI--;
		if (j < p->numNodes)
			{
			verts[j] = vertex_[p->vertexID[rightI]];
			norms[j] = p->normal[rightI];
			if (texture)
				{
				if (texture->method() == softTexture::UV_METHOD)
					texc[j] = p->uvTexture[rightI];
				else
					texc[j] = texture->generateTexCoord(verts[j],boundBox_);
				texture->transform(texc[j]);
				}
			j++; rightI++;
			}
		}
	}
#endif
	gset->setAttr(PFGS_COORD3,PFGS_PER_VERTEX,verts,NULL);
	gset->setAttr(PFGS_NORMAL3,PFGS_PER_VERTEX,norms,NULL);
	if (texture)
		gset->setAttr(PFGS_TEXCOORD2,PFGS_PER_VERTEX,texc,NULL);
	geode->addGSet(gset);
/* Too many (> ~1000) geosets per geode can cause core dumps; this makes multiple
  geodes if there are a lot of polygons */
	if (polyCount % 512 == 511)
		{
		geode = new pfGeode;
		parent->addChild(geode);
		}
	}
}


void softMesh::buildPolygonsInOneGeoset(struct _polylist *polylist,pfGroup *parent)
{
 pfGeode * geode = new pfGeode;
 softTexture *texture=NULL;
 int i,j,polyCount=0;
 pfGeoSet *gset;
 pfVec3 *verts=NULL;
 pfVec3 *norms=NULL;
 pfVec2 *texc=NULL;
 softMaterial *mat;
 int numVerts, vertOffset,
	*lengths = (int *) pfMalloc(polylist->numPolys*sizeof(int),pfGetSharedArena());
 parent->addChild(geode);
 for (i=0, numVerts=0; i < polylist->numPolys; i++)
	{
	lengths[i] = polylist->list[i].numNodes;
	numVerts += polylist->list[i].numNodes;
	}
 gset = new pfGeoSet;
 gset->setPrimType(PFGS_TRISTRIPS);
 gset->setNumPrims(polylist->numPolys);
 gset->setPrimLengths(lengths);
 mat = model_->findMaterial(polylist->list[0].materialID);
 if (mat)
	gset->setGState(mat->pfGState());
 else
	{
	char *name = "";
	if (model_) name = model_->name();
	printf("Warning: softMesh::pfGraph (%s): failed to find "
		    "material %d\n",name,polylist->list[0].materialID);
	gset->setGState(softMaterial::defaultpfGState());
	}
 verts = (pfVec3 *) pfMalloc(numVerts*sizeof(pfVec3), pfGetSharedArena());
 norms = (pfVec3 *) pfMalloc(numVerts*sizeof(pfVec3), pfGetSharedArena());
 if (mat)
	texture = mat->texture();
 if (!texture)
	texture = model_->texture();
 if (texture)
	texc = (pfVec2 *) pfMalloc(numVerts*sizeof(pfVec2),pfGetSharedArena());
 for (i=0, vertOffset=0; i < polylist->numPolys; i++)
	{
	struct _polygon *p = &polylist->list[i];
	int leftI,rightI;
	for (j=0, rightI=0,leftI=p->numNodes-1; j < p->numNodes; )
		{
		verts[vertOffset+j] = vertex_[p->vertexID[leftI]];
		norms[vertOffset+j] = p->normal[leftI];
		if (texture)
			{
			if (texture->method() == softTexture::UV_METHOD)
				texc[vertOffset+j] = p->uvTexture[leftI];
			else
				texc[vertOffset+j] = texture->generateTexCoord(
							verts[vertOffset+j],boundBox_);
			texture->transform(texc[vertOffset+j]);
			}
		j++; leftI--;
		if (j < p->numNodes)
			{
			verts[vertOffset+j] = vertex_[p->vertexID[rightI]];
			norms[vertOffset+j] = p->normal[rightI];
			if (texture)
				{
				if (texture->method() == softTexture::UV_METHOD)
					texc[vertOffset+j] = p->uvTexture[rightI];
				else
					texc[vertOffset+j] = texture->generateTexCoord(
								verts[vertOffset+j],boundBox_);
				texture->transform(texc[vertOffset+j]);
				}
			j++; rightI++;
			}
		}
	vertOffset += p->numNodes;
	}
 gset->setAttr(PFGS_COORD3,PFGS_PER_VERTEX,verts,NULL);
 gset->setAttr(PFGS_NORMAL3,PFGS_PER_VERTEX,norms,NULL);
 if (texture)
	gset->setAttr(PFGS_TEXCOORD2,PFGS_PER_VERTEX,texc,NULL);
 geode->addGSet(gset);
}
