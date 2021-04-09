/************************************************************************
*									*
*			search.c - V1.6.1 (Jan 97)			*
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

/* for PC version */
extern	unsigned _stklen = 8192;

enum	results { FAILURE, SUCCESS, CONTINUE };

#define	DEFAULT_FORMAT_SIZE "F<99X<99J<99O<99R<99C<99T<99M<99L<99K<99|<99"

static	int	hash_compare(int *array1,int *array2)
{
	int	dist = 0;

	while(*array1 != LAST && *array2 != LAST)
		dist += abs(*array1++ - *array2++);
	return(dist);
}

static	int	get_abc_entry(char *dflt_meter,char *dflt_origin,
			char *dflt_rhythm,char *entry,Record *abc,int *x)
{
	char	dummy[30][99];
	char	temp[1999],line[999];
	int	in_header = 0,in_tune = 0,m = 0,o = 0,r = 0;
	temp[0] = '\0';
	entry[0] = '\0';
	abc->LENGTH[0] = '\0';
	abc->BARS[0] = '\0';
	(void) strcpy(abc->METER,&dflt_meter[2]);
	abc->METER[strlen(abc->METER)-1] = '\0';
	while (!in_tune && (getsIn(line)) != NULL) {
		if ((strncmp("X:",line,2)) == 0) {
			*x = atoi(&line[2]);
			in_header = 1;
		}
		if (in_header) {
			(void) strcat(temp,line);
			if (line[1] == ':') {
				switch (line[0]) {
				case 'M':
					stripcpy(abc->METER,&line[2]);
					m = 1;
					break;
				case 'L':
					stripcpy(abc->LENGTH,&line[2]);
					break;
				case 'O':
					o = 1;
					break;
				case 'R':
					r = 1;
					break;
				case 'K':
					stripcpy(abc->KEY,&line[2]);
					process_abc(dummy,0,abc,"",abc->BARS,
						temp,(char *) NULL,NO_OUTPUT,
						TWO_BARS,0, (int *) NULL);
					in_tune = 1;
					in_header = 0;
					break;
				}
			}
		} else {
			if (line[1] == ':') {
				switch (line[0]) {
				case 'M':
					(void) strcpy(dflt_meter,line);
					(void) strcpy(abc->METER,
						&dflt_meter[2]);
					abc->METER[strlen(abc->METER)-1] = '\0';
					break;
				case 'O':
					(void) strcpy(dflt_origin,line);
					break;
				case 'R':
					(void) strcpy(dflt_rhythm,line);
					break;
				}
			}
		}
	}
	if (in_tune) {
		if (m == 0) (void) strcat(entry,dflt_meter);
		if (o == 0) (void) strcat(entry,dflt_origin);
		if (r == 0) (void) strcat(entry,dflt_rhythm);
		(void) strcat(entry,temp);
		return(1);
	} else
		return(0);
}

static	int	str_compare(char *s,char *t)
{
	while ((lower(*t)) == (lower(*s)) || *s == '.') {
		++s;
		++t;
		if (*s == '\0') return(SUCCESS);
		if (*t == '\0') return(FAILURE);
	}
	return(CONTINUE);
}

static	int	str_search(char *s,char *t)
{
	int	result;
	while (*s == '.') {
		++s;
		++t;
		if (*s == '\0') return(SUCCESS);
		if (*t == '\0') return(FAILURE);
	}
	while (*t) {
		if ((result = str_compare(s,t)) == CONTINUE)
			++t;
		else
			return(result);
	}
	return(0);
}

int	main(int argc,char *argv[])
{
	char	dummy[30][99];
	static	char	cor[] = {'C','O','R','\0'};
	static	char	*m2[] = {"C","C|","4/4","2/4","2/2",""};
	static	char	*m3[] = {"3/8","6/8","9/8","12/8","3/4","3/2",""};
	char	input[30][99],file[99],fmt_file[99];
	char	format[999];
	int	arg,abc_input = 0;
	int	max_dist = 6,dist;
	Record	*search,*abc,*ientry;
	char	title_strs[10][99];
	char	abc_fields[30][99],*cptr,*fptr,*fnext,temp[999];
	int	titles = 0,ttl = 0,fields = 0,fld = 0,t = 0,i = 0,x = 0,m;
	int	first,last,y = 0,yfirst,ylast = 4999,prev_x = 0;
	char	*ranges_ptr,ranges[99];
	int	meter_group = 0,result,command_line = 0;
	int	hash_array[999];
	char	dflt_meter[99],dflt_origin[99],dflt_rhythm[99];
	char	abc_entry[1999];
	int	ihash_array[999];
	char	fmt_fields[MAX_FIELDS],*bar_fmt;
	int	sizes[MAX_FIELDS];
	int	no_meter = 0;
	FILE	*In;

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

	ranges[0] = '\0';
	read_settings();

	(void) strcpy(file,"index");

	size_record(DEFAULT_FORMAT_SIZE,sizes,fmt_fields);
	search = alloc_record(fmt_fields,sizes);
	(void) strcpy(search->LENGTH,"1/8");

	for (arg = 1; arg < argc ; ++arg) {
		if ((strcmp(argv[arg],"-f")) == 0) {
			(void) strcpy(file,argv[++arg]);
			if (arg == argc)
				g_error("missing argument");
		} else if ((strcmp(argv[arg],"-abc")) == 0) {
			abc_input = 1;
			abc = alloc_record(fmt_fields,sizes);
			++arg;
			break;
		} else if (argv[arg][0] >= '0' && argv[arg][0] <= '9') {
			max_dist = atoi(argv[arg]);
		} else if ((strcmp(argv[arg],"-i")) == 0) {
			command_line = 1;
			(void) strcpy(input[i++],argv[++arg]);
			if (arg == argc)
				g_error("missing argument");
		} else
			g_error("unrecognised argument %s",argv[arg]);
	}

	if (command_line)
		input[i][0] = '\0';
	else
		(void) gets(input[0]);
	i = 0;

	/* first get the search fields */
	while (input[i][0]) {
		if (input[i][1] == ':' && strchr("MLKX",input[i][0])) {
			switch (input[i][0]) {
			case 'M':
				if ((strcmp(&input[i][2],"2")) == 0)
					meter_group = 2;
				else if ((strcmp(&input[i][2],"3")) == 0)
					meter_group = 3;
				else {
					(void) strcpy(search->METER,
						&input[i][2]);
					search->LENGTH[0] = '\0';
				}
				break;
			case 'L':
				(void) strcpy(search->LENGTH,&input[i][2]);
				break;
			case 'K':
				(void) strcpy(search->KEY,&input[i][2]);
				if (command_line)
					(void) strcpy(search->BARS,input[++i]);
				else
					(void) gets(search->BARS);
				if (search->METER[0] == '\0') {
					(void) strcpy(search->METER,"C");
					no_meter = 1;
				}
				process_abc(dummy,0,search,"",(char *) NULL,"",
				 search->BARS,HASH_OUTPUT,0,1,hash_array);
				if (no_meter) {
					search->METER[0] = '\0';
					no_meter = 0;
				}
				break;
			case 'X':
				(void) strcpy(ranges,&input[i][2]);
			}
		} else if (abc_input) {
			if (!is_field(input[i]))
				g_error("input field not recognised - %s",
						input[i]);
			else if (fields < 30)
				(void) strcpy(abc_fields[fields++],input[i]);
			else
				(void) printf("too many search fields\n");
		} else if (input[i][1] == ':' && strchr("OCRF",input[i][0])) {
			(void) strcpy(search->fields[input[i][0]-'A'],
				&input[i][2]);
		} else if ((strncmp(input[i],"T:",2)) == 0) {
			if (titles < 10)
				(void) strcpy(title_strs[titles++],
					&input[i][2]);
			else
				(void) printf("too many title strings\n");
		} else
			(void) printf("line not recognised\n");

		if (command_line)
			i += 1;
		else
			(void) gets(input[0]);
	}

	do { /* each file */

	if (abc_input) (void) strcpy(file,argv[arg]);

	if ((In = openIn(file)) == NULL)
		continue;

	if (abc_input) { /* search abc */

	(void) strcpy(dflt_meter,"M:C\n");
	dflt_origin[0] = '\0';
	dflt_rhythm[0] = '\0';
	first = -999;
	last = -999;
	x = 0;
	ranges_ptr = &ranges[0];
	/* now search abc file */
	while ((get_abc_entry(dflt_meter,dflt_origin,dflt_rhythm,
		abc_entry,abc,&x)) != 0) {
		if (x == prev_x)
			y += 1;
		else
			y = 1;
		prev_x = x;
		if (ranges[0]) {
			if (x > last || (x == last && y > ylast))
				if (!(range(&first,&last,&yfirst,&ylast,
						&ranges_ptr)))
					g_error("in range");
			if (x < first || (x == first && y < yfirst))
				continue;
		}
		fld = 0;
		for (fld = 0; fld < fields; ++fld) {
			fptr = abc_entry;
			while ((fnext = strchr(fptr,'\n'))) {
				t = 0;
				for (cptr = fptr+2; cptr != fnext;
					temp[t++] = *(cptr++));
				temp[t] = '\0';
				if (*fptr == abc_fields[fld][0] &&
				 (result = str_search(&abc_fields[fld][2],temp))
					== SUCCESS)
					break;
				if ((strncmp("\nK:",fnext,3)) == 0)
					break;
				fptr = fnext+1;
				result = CONTINUE;
			}
			if (result != SUCCESS)
				break;
		}
		if (fld != fields)
			continue;
		if (meter_group == 2) {
			for (m = 0; m2[m][0]; ++m)
				if (strcmp(abc->METER,m2[m]) == 0) break;
			if (m2[m][0] == '\0') continue;
		} else if (meter_group == 3) {
			for (m = 0; m3[m][0]; ++m)
				if (strcmp(abc->METER,m3[m]) == 0) break;
			if (m3[m][0] == '\0') continue;
		} else
			if (search->METER[0]
			 && strcmp(search->METER,abc->METER))
				continue;
		if (search->BARS[0]) {
			process_abc(dummy,0,abc,"",(char *) NULL,"",abc->BARS,
				HASH_OUTPUT,0,0,ihash_array);
			dist = hash_compare(hash_array,ihash_array);
			if (dist > max_dist)
				continue;
			(void) printf("distance = %d\n",dist);
		}
		(void) printf("F:%s\n",file);
		(void) puts(abc_entry);
	}

	} else { /* search index */

	(void) strcpy(fmt_file,file);
	(void) strcat(fmt_file,".fmt");
	get_index(format,fmt_file);
	for (bar_fmt = format; *bar_fmt; ++bar_fmt) {
		if (*bar_fmt == '|' && *(bar_fmt-1) != '\\') {
			++bar_fmt;
			break;
		}
	}

	size_record(format,sizes,fmt_fields);
	ientry = alloc_record(fmt_fields,sizes);

	if (ranges[0]) {
		if (!(range(&first,&last,&yfirst,&ylast,&ranges_ptr)))
			g_error("in range");
		x = first;
		if (!ientry->NUMBER)
			g_error("X field not in index");
	}
	for (i = 0; cor[i]; ++i)
		if (search->fields[cor[i]-'A'][0]
		 && !ientry->fields[cor[i]-'A'])
			g_error("%c field not in index",cor[i]);
	if (titles && !ientry->TITLE)
		g_error("T field not in index");
	if ((search->METER[0] || meter_group) && !ientry->METER)
		g_error("M field not in index");

	while ((get_record(format,In,ientry)) != 0) {

		if (search->FILENAME[0]
		 && strcmp(search->FILENAME,ientry->FILENAME))
			continue;
		if (x && x != atoi(ientry->NUMBER))
			continue;
		for (i = 0; cor[i]; ++i)
			if (search->fields[cor[i]-'A'][0]
			 && strcmp(search->fields[cor[i]-'A'],
				ientry->fields[cor[i]-'A']))
				break;
		if (cor[i]) continue;
		ttl = 0;
		for (ttl = 0; ttl < titles; ++ttl)
			if ((str_search(title_strs[ttl],ientry->TITLE))
				== FAILURE)
				break;
		if (ttl != titles)
			continue;
		if (meter_group == 2) {
			for (m = 0; m2[m][0]; ++m)
				if (strcmp(ientry->METER,m2[m]) == 0) break;
			if (m2[m][0] == '\0') continue;
		} else if (meter_group == 3) {
			for (m = 0; m3[m][0]; ++m)
				if (strcmp(ientry->METER,m3[m]) == 0) break;
			if (m3[m][0] == '\0') continue;
		} else
			if (search->METER[0]
				&& strcmp(search->METER,ientry->METER))
				continue;
		if (search->BARS[0] && *bar_fmt) {
			process_abc(dummy,0,ientry,"",(char *) NULL,"",
				ientry->BARS,HASH_OUTPUT,0,0,ihash_array);
			dist = hash_compare(hash_array,ihash_array);
			if (dist > max_dist)
				continue;
			(void) printf("distance = %d\n",dist);
		} else if (search->BARS[0] && *bar_fmt == '\0')
			continue;
		(void) put_record(format,stdout,ientry);
	}

	} /* search index */

	closeIn();

	} while (++arg < argc); /* each file */

	if (!abc_input) free_record(ientry,fmt_fields);
	size_record(DEFAULT_FORMAT_SIZE,sizes,fmt_fields);
	free_record(search,fmt_fields);
	if (abc_input) free_record(abc,fmt_fields);

	return(0);
}

