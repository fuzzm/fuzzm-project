/************************************************************************
*									*
*			sort_in.c - V1.6.1 (Jan 97)			*
*									*
*		by    Chris Walshaw (C.Walshaw@gre.ac.uk)		*
*									*
*	Copyright Chris Walshaw. Permission is granted to use and	*
*	copy provided that this copyright notice remains attached.	*
*	This code may not be sold.					*
*									*
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__MWERKS__)
#ifndef MACABC
#include <Console.h>
#endif
#endif
#include "index.h"

typedef int (compFun)(const void*, const void*);

static	char	priority[99];

static	char	*next(char *s)
{
	while ((*s < 'a' || *s > 'z')
	    && (*s < 'A' || *s > 'Z')
	    && (*s < '0' || *s > '9')
	    && *s != ' ' && *s != '\t' && *s != '\0') {
		++s;
	}
	return(s);
}

static	int	str_cmp(char *s,char *t)
{
	char	u,v;
	if (s == NULL || t == NULL)
		return(s - t);
	/* ignore leading whitespace */
	while (*s == ' ') ++s;
	while (*t == ' ') ++t;
	while (*s != '\0' && *t != '\0') {
		s = next(s);
		u = lower(*s);
		t = next(t);
		v = lower(*t);
		if (u != v)
			return(u - v);
		++s;
		++t;
	}
	s = next(s);
	u = lower(*s);
	t = next(t);
	v = lower(*t);
	return(u - v);
}

static	int	compare(Record **entry1,Record **entry2)
{
	int	result = 0,p;
	for (p = 0; priority[p] != '\0' && result == 0; ++p) {
		if (strchr("TRFJOCMKL",priority[p]))
			result = str_cmp((*entry1)->fields[priority[p]-'A'],
					 (*entry2)->fields[priority[p]-'A']);
		else
			switch (priority[p]) {
			case 'X':
				result = atoi((*entry1)->fields['X'-'A'])
				       - atoi((*entry2)->fields['X'-'A']);
				break;
			case '|':
				result = str_cmp((*entry1)->bars,
						 (*entry2)->bars);
				break;
			default:
				g_error("in priority format");
				break;
			}
	}
	return(result);
}

int	main(int argc,char *argv[])
{
	char	format[999],file[99],fmt_file[99];
	FILE	*index;
	char	fields[MAX_FIELDS];
	int	sizes[MAX_FIELDS];
	Record	*entry,*first_entry,*prev_entry,*next_entry = NULL;
	Record	**list;
	int	i,n = 0,arg;

/* If we are just compiling a direct port onto the Mac (without the MacABC code)
we need to increase the stack allocation and prompt for the command line args */
#if defined(__MWERKS__)
#ifndef MACABC
#define	kStackExtra		(1024+100*sizeof(Note))
	SetApplLimit((Ptr)(((unsigned long)GetApplLimit())-kStackExtra));
	MaxApplZone();
	argc=ccommand(&argv);
#endif
#endif

	(void) strcpy(file,"index");
	(void) strcpy(priority,"T");

	for (arg = 1; arg < argc ; ++arg) {
		if (arg+1 == argc)
			g_error("missing argument");
		if ((strcmp(argv[arg],"-f")) == 0)
			(void) strcpy(file,argv[++arg]);
		else if ((strcmp(argv[arg],"-p")) == 0)
			(void) strcpy(priority,argv[++arg]);
		else
			g_error("unrecognised argument %s",argv[arg]);
	}

	if ((strcmp(file,"-")) == 0) {
		index = stdin;
		(void) strcpy(fmt_file,"index");
	} else {
		index = fopen(file,"r");
		if (index == (FILE *) NULL)
			g_error("cannot open file %s",file);
		(void) strcpy(fmt_file,file);
	}
	(void) strcat(fmt_file,".fmt");
	get_index(format,fmt_file);
	size_record(format,sizes,fields);

	/* read index */
	entry = alloc_record(fields,sizes);
	first_entry = entry;
	while ((get_record(format,index,entry)) != 0) {
		prev_entry = entry;
		entry = alloc_record(fields,sizes);
		prev_entry->next = entry;
		n += 1;
	}
	if ((strcmp(file,"-")) != 0)
		(void) fclose(index);

	ALLOC(list,Record*,n);
	list[0] = first_entry;
	for (i = 1; i < n; ++i)
		list[i] = list[i-1]->next;

	qsort((char *) list,(unsigned) n,sizeof(Record*),
		(compFun*)&compare);

	/* write index */
	if ((strcmp(file,"-")) == 0)
		index = stdout;
	else {
		index = fopen(file,"w");
		if (index == (FILE *) NULL)
			g_error("cannot open file %s",file);
	}
	for (i = 0; i < n; ++i)
		(void) put_record(format,index,list[i]);
	if ((strcmp(file,"-")) != 0)
		(void) fclose(index);

	for (entry = first_entry; entry; entry = next_entry) {
		next_entry = entry->next;
		free_record(entry,fields);
	}

	free(list);

	return(0);
}

