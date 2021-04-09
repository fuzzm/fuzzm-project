/************************************************************************
*									*
*			index.c - V1.6.1 (Jan 97)			*
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
#include <stdarg.h>
#include <string.h>
#include "index.h"

void	g_error(char *fmt, ...)
{
	va_list	ap;
	va_start(ap,fmt);
	(void) fprintf(stderr,"ERROR: ");
	(void) vfprintf(stderr,fmt,ap);
	(void) fprintf(stderr,"\n");
	va_end(ap);
	exit(1);
}

void	get_index(char *fmt, char *fname)
{
	FILE	*fp;
	int	colon;
	char	c;
	int	f = 0;

	if ((fp = fopen(fname,"r")) == NULL)
		g_error("file %s does not exist",fname);

	fmt[0] = '\0';
	while ((fgets(&fmt[strlen(fmt)],99,fp)) != NULL);

	/* now check for errors */
	while (fmt[f]) {
		if (strchr("FXJORCTMLK|",fmt[f])) {
			c = fmt[f];
			if (fmt[f++] == '|') {
				if (fmt[f] == '0') ++f;
				if (strchr("12",fmt[f])) ++f;
			}
			if (fmt[f] == ':')
				colon = 1;
			else if (fmt[f] == '<' || fmt[f] == '>')
				colon = 0;
			else
			  g_error("fmt file - %c not followed by [><:]",c);
			if (fmt[++f] < '0' || fmt[f] > '9')
			  g_error("fmt file - %c not followed by length",c);
			while (fmt[f] >= '0' && fmt[f] <= '9') ++f;
			if (colon && fmt[f] != '\n')
			  g_error("fmt file - %c: not at end of line",c);
		} else if (fmt[f] == '\\')
			f += 2;
		else
			f += 1;
	}
	if (fmt[f-1] != '\n')
		g_error("fmt file - not terminated by newline character");
}

void	size_record(char *fmt, int *size, char *field)
{
	int	s = 0;
	int	f = 0;
	while (fmt[f]) {
		if (strchr("FXJORCTMLK|",fmt[f])) {
			field[s] = fmt[f++];
			if (field[s] == '|' && fmt[f] == '0') ++f;
			if (field[s] == '|' && strchr("12",fmt[f])) ++f;
			size[s++] = atoi(&fmt[++f]);
			while (fmt[f] >= '0' && fmt[f] <= '9')
				f++;
		} else if (fmt[f] == '\\')
			f += 2;
		else
			f += 1;
	}
	field[s] = '\0';
	size[s] = 0;
}

Record	*alloc_record(char *fmt, int *size)
{
	Record	*entry;
	int	f;
	ALLOC(entry,Record,1);
	for (f = 0; fmt[f]; ++f) {
		if (strchr("FXJORCTMLK",fmt[f])) {
			ALLOC(entry->fields[fmt[f]-'A'],char,size[f]+1);
		} else if (fmt[f] == '|') {
			ALLOC(entry->BARS,char,size[f]+1);
		} else
			g_error("in size format");
	}
	return(entry);
}

void	free_record(Record *entry, char *fmt)
{
	int	f;

	for (f = 0; fmt[f]; ++f) {
		if (strchr("FXJORCTMLK",fmt[f]))
			free(entry->fields[fmt[f]-'A']);
		else if (fmt[f] == '|')
			free(entry->BARS);
	}
	free(entry);
}

static	void	str_get(char *string, char *fmt, int *f, char *index, int *i)
{
	int	c;
	int	n;
	int	l;
	char	temp[99];
	*f += 2;
	n = atoi(&fmt[*f]);
	if (fmt[*f-1] == '<') {
		(void) strncpy(string,&index[*i],n);
		string[n] = '\0';
		/* remove trailing whitespace */
		l = strlen(string);
		for (c = l; c > 0 && string[c-1] == ' '; c--);
		string[c] = '\0';
		*i += n;
	} else if (fmt[*f-1] == '>') {
		(void) strncpy(temp,&index[*i],n);
		temp[n] = '\0';
		/* remove leading whitespace */
		for (c = 0; temp[c] == ' ' && c < n; c++);
		(void) strcpy(string,&temp[c]);
		*i += n;
	} else {
		(void) strncpy(string,&index[*i],n);
		l = strlen(string);
		if (string[l-1] == '\n')
			string[--l] = '\0';
		*i += strlen(string);
	}
	while (fmt[*f] >= '0' && fmt[*f] <= '9')
		(*f)++;
}

int	get_record(char *fmt, FILE *In, Record *entry)
{
	int	new_line = 1;
	int	f = 0;
	int	i = 0;
	static	char	index[999];

	while (fmt[f]) {
		if (new_line == 1) {
			if ((fgets(index,999,In)) == NULL)
				return(0);
			new_line = 0;
		}
		if (strchr("FXJORCTMLK",fmt[f]))
			str_get(entry->fields[fmt[f]-'A'],fmt,&f,index,&i);
		else
			switch (fmt[f]) {
			case '|':
				if (fmt[f+1] == '0') ++f;
				if (strchr("12",fmt[f+1])) ++f;
				str_get(entry->BARS,fmt,&f,index,&i);
				break;
			case '\n':
				new_line = 1;
				f += 1;
				i = 0;
				break;
			case '\\':
				f += 1;
				/*FALLTHROUGH*/
			default:
				f += 1;
				i += 1;
				break;
			}
	}
	return(1);
}

static	void	str_put(char *string, char *fmt, int *f, char *index, int *i)
{
	int	c;
	int	n;
	int	l;
	*f += 2;
	n = atoi(&fmt[*f]);
	if (fmt[*f-1] == '<') {
		(void) strncpy(&index[*i],string,n);
/* might need to pad with whitespace */
		for (c = strlen(string); c < n; c++)
			index[*i+c] = ' ';
		*i += n;
		index[*i] = '\0';
	} else if (fmt[*f-1] == '>') {
/* might need to pad with whitespace */
		l = strlen(string);
		if (l < n) {
			for (c = 0; c < n-l; c++)
				index[*i+c] = ' ';
			index[*i+c] = '\0';
			(void) strcpy(&index[*i+c],string);
		} else
			(void) strncpy(&index[*i],string,n);
		*i += n;
		index[*i] = '\0';
	} else {
		(void) strcpy(&index[*i],string);
		*i += strlen(string);
	}
	while (fmt[*f] >= '0' && fmt[*f] <= '9')
		(*f)++;
}

int	put_record(char *fmt, FILE *Out, Record *entry)
{
	int	f = 0;
	int	i = 0;
	static	char	index[999];

	while (fmt[f]) {
		if (i == 0)
			index[0] = '\0';
		if (strchr("FXJORCTMLK",fmt[f]))
			str_put(entry->fields[fmt[f]-'A'],fmt,&f,index,&i);
		else
			switch(fmt[f]) {
			case '|':
				if (fmt[f+1] == '0') ++f;
				if (strchr("12",fmt[f+1])) ++f;
				str_put(entry->BARS,fmt,&f,index,&i);
				break;
			case '\\':
				f += 1;
				/*FALLTHROUGH*/
			default:
				(void) strncpy(&index[i++],&fmt[f],1);
				index[i] = '\0';
				if (fmt[f++] == '\n') {
					(void) fputs(index,Out);
					i = 0;
				}
				break;
			}
	}
	return(1);
}

char	lower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return(c-'A'+'a');
	else
		return(c);
}

