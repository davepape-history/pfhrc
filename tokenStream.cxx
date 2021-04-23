#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "tokenStream.h"


tokenStream::tokenStream(char *filename)
{
 filename_ = strdup(filename);
 fp_ = fopen(filename_,"r");
 if (!fp_)
	perror(filename_);
}

int tokenStream::next(char *buf,int buflen)
{
#define ADDCHAR(c) { if (index < buflen) buf[index++] = c; }
#define ENDSTRING() { if (index < buflen) buf[index] = '\0'; }
 int c,index=0;
 if (!fp_)
	return 0;
 while (((c=getc(fp_))!=-1) && (isspace(c)))
	;
 if (c==-1)
	return 0;
 if ((c=='{') || (c=='}') || (c=='(') || (c==')'))
	{
	ADDCHAR(c);
	ENDSTRING();
	return index;
	}
 if (c=='"')
	{
	while (((c=getc(fp_)) != -1) && (c != '"'))
		{
		if (c=='\\')
			c = getc(fp_);
		ADDCHAR(c);
		}
	ENDSTRING();
	return index;
	}
 if (c=='[')
	{
	while ((c != -1) && (c != ']'))
		{
		ADDCHAR(c);
		c = getc(fp_);
		}
	ADDCHAR(c);
	ENDSTRING();
	return index;
	}
 while ((c != -1) && (!isspace(c)))
	{
	ADDCHAR(c);
	c = getc(fp_);
	}
 ENDSTRING();
 return index;
}

void tokenStream::skipBlock(char *begin,char *end)
{
 char token[1024];
 int depth=1;
 while (depth)
	{
	if (!next(token,sizeof(token)))
		return;
	if (!strcmp(token,begin))
		depth++;
	else if (!strcmp(token,end))
		depth--;
	}
}

int tokenStream::lookup(char *str,tokenVal *table)
{
 int i;
 if ((!str) || (!table))
	return -1;
 for (i=0; table[i].str; i++)
	if (!strcmp(str,table[i].str))
		break;
 return table[i].val;
}

int tokenStream::nextInt(void)
{
 char buf[256];
 if (next(buf,sizeof(buf)))
	return atoi(buf);
 return 0;
}

float tokenStream::nextFloat(void)
{
 char buf[256];
 if (next(buf,sizeof(buf)))
	return atof(buf);
 return 0;
}

void tokenStream::getNextVec3(pfVec3 *vec)
{
 char buf[256];
 if (next(buf,sizeof(buf)))
	(*vec)[0] = atof(buf);
 else
	return;
 if (next(buf,sizeof(buf)))
	(*vec)[1] = atof(buf);
 else
	return;
 if (next(buf,sizeof(buf)))
	(*vec)[2] = atof(buf);
}

void tokenStream::getNextVec2(pfVec2 *vec)
{
 char buf[256];
 if (next(buf,sizeof(buf)))
	(*vec)[0] = atof(buf);
 else
	return;
 if (next(buf,sizeof(buf)))
	(*vec)[1] = atof(buf);
}
