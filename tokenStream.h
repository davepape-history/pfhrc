#ifndef _tokenStream_h_
#define _tokenStream_h_

#include <stdio.h>
#include <Performer/pr/pfLinMath.h>

class tokenStream
	{
	public:
	 typedef struct { char *str; int val; } tokenVal;
	 tokenStream(char *filename);
	 int next(char *buf,int buflen);
	 void skipBlock(char *begin,char *end);
	 int lookup(char *str,tokenVal *table);
	 int nextInt(void);
	 float nextFloat(void);
	 void getNextVec3(pfVec3*);
	 void getNextVec2(pfVec2*);
	 char *filename(void) { return filename_; }
	private:
	 char *filename_;
	 FILE *fp_;
	};

#endif
