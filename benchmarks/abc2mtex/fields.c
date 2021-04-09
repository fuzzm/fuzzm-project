/************************************************************************
*									*
*			fields.c - V1.6.1 (Jan 97)			*
*									*
*		by    Chris Walshaw (C.Walshaw@gre.ac.uk)		*
*									*
*	Copyright Chris Walshaw. Permission is granted to use and	*
*	copy provided that this copyright notice remains attached.	*
*	This code may not be sold.					*
*									*
************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__MWERKS__)
#ifndef MACABC
#include <Console.h>
#endif
#endif
#include "index.h"

/* for PC version */
extern	unsigned _stklen = 8192;

extern	void	abc_error(char*,...);
extern	void	abc_warning(char*,...);
extern	char	*end_of(char*);
extern	void	strip(char*,char*);
extern	void	get_dnl(Record*);
extern	void	output_transline(char*);

extern	void	open_TeX(char*,int);
extern	void	close_TeX(void);
extern	void	draw_text(char*,char*);

extern	FILE	*Trans;
extern	int	transpose;
extern	int	offset;
extern	Setting	settings;
extern	int	nblanks;

FILE	*Log;
FILE	*Index;

static	void	article(char *s,char *t)
{
	int	j;
	char	*i = s;
	const	char	*a[]={"The","An","A","Na","Da","Le","La","Lo","Lou",
				"Un","Une","Les","L'",""};
	while (*s && *s++ != ',');
	if (*(s-1) == ',') {
		s += 1;
		for (j=0; a[j][0]; ++j) {
			if ((strcmp(s,a[j])) == 0) {
				while (*s)
					*t++ = *s++;
				if (*(t-1) != '\'')
					*t++ = ' ';
				break;
			}
		}
	}
	while (*i) {
		if (*i == ',' && a[j][0])
			break;
		*t++ = *i++;
	}
	*t = '\0';
}

static	void	detex(char *s,char *t)
{
	while (*s) {
		if (*s == '\\') {
			if (*(++s) != '&')
				if (*(++s) == ' ')
					s += 1;
		}
		*t++ = *s++;
	}
	*t = '\0';
}

static	void	interval(char *s)
{
	offset = atoi(&s[1])-1;
	for (transpose = -1; transpose < 3; transpose++)
		if (offset%7 == (7+transpose*4)%7) break;
	if (transpose == 3 || offset < 1 || offset > 7)
		g_error("transpose interval not recognised");
	if (s[0] == '_') {
		transpose *= -1;
		offset *= -1;
	}
}

static	int	is_comment(char *str)
{
	char	*c_ptr;

	c_ptr = strchr(str,'%');
	if (c_ptr == NULL) return(0);
	while (c_ptr > str && (*(c_ptr-1) == ' ' || *(c_ptr-1) == '\t'))
		--c_ptr;
	if (c_ptr == str) return(1);
	return(0);
}

static	void	strip_path(char *filename,char *file)
{
	char	*f_ptr;
	static	char	temp[99];
	stripcpy(temp,file);
	if ((strcmp(&temp[strlen(temp)-4],".abc")) == 0)
		temp[strlen(temp)-4] = '\0';
	if ((f_ptr = strrchr(temp,'/')))
		++f_ptr;
	else
		f_ptr = temp;
	(void) strcpy(filename,f_ptr);
}

int	main(int argc,char **argv)
{
	int	c,first,last,x,logn=0,logn_total=0,log=0,log_total=0,trnsps=0,i;
	int	m,titles,ttl,in_tune,prev_x,njoint=0,arg = 1,history = 0;
	char	f = 'm',input[99],trans[99],filename[99],basename[99];
	static	char	title[30][99];
	char	dflt_meter[9],full_title[99],xline[999];
	char	*bar_fmt,*pinput,head[999],number[99];
	char	format[999],dflt_origin[99],dflt_rhythm[99];
	int	nbars = 0,command_line = 0,output = TEX_OUTPUT;
	Record	real_entry,*entry;
	int	yfirst,ylast = 4999,y;
	int	musix_out;
	char	key_comment[999],dummy[999];

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

	read_settings();
	entry = &real_entry;
	musix_out = 0;
	filename[0] = '\0';
	offset = 0;
	for (i = 0; i < 26; ++i) {
		ALLOC(entry->fields[i],char,256);
	}
	ALLOC(entry->BARS,char,256);

	Log = fopen("abc2mtex.log","w");

	for (arg = 1; (arg < argc && argv[arg][0] == '-' && argv[arg][1]);
			++arg) {
		switch (argv[arg][1]) {
		case 'x':
			musix_out = 1;
			break;
		case 'y':
			musix_out = 2;
			break;
		case 't':
			if (f == 'i') g_error("option -t incompatible with -i");
			if (argv[arg][2] == ':') {
				if (argv[arg][3] == '^' || argv[arg][3] == '_')
					interval(&argv[arg][3]);
				else
					(void) sscanf(argv[arg],"-t:%d:%d",
						&transpose,&offset);
				trnsps = 2;
			} else
				trnsps = 1;
			Trans = fopen("transpose.abc","w");
			break;
		case 'i':
			if (f == 't') g_error("option -i incompatible with -t");
			f = 'i';
			break;
		case 'o':
			if (++arg == argc || argv[arg][0] == '-')
				g_error("no output file");
			(void) strcpy(filename,argv[arg]);
			break;
		default:
			g_error("unrecognised option %s",argv[arg]);
		}
	}

	if (argc > arg) command_line = 1;

	if (f == 'm') {
		open_TeX(filename,musix_out);
	} else if (f == 'i') {
		output = INDEX_OUTPUT;
		get_index(format,"index.fmt");
		for (bar_fmt = format; *bar_fmt; ++bar_fmt) {
				if (*bar_fmt == '|' && *(bar_fmt-1) != '\\') {
					if (*(++bar_fmt) == '0') {
						if (*(++bar_fmt) == '1')
							nbars = ONE_BAR_PLUS;
						else if (*bar_fmt == '2')
							nbars = TWO_BARS_PLUS;
					} else {
						if (*bar_fmt == '1')
							nbars = ONE_BAR;
						else if (*bar_fmt == '2')
							nbars = TWO_BARS;
						else
							nbars = TWO_BARS_PLUS;
					}
					break;
				}
		}
		if (filename[0] == '\0') (void) strcpy(filename,"index");
		Index = fopen(filename,"w");
	}

	do { /* loop over input files */

	logn_total += logn;
	logn = 0;
	log_total += log;
	log = 0;

	if (command_line) {
			if ((strcmp(argv[argc-1],"-")) == 0) {
			(void) strcpy(input,"stdin");
			if (trnsps == 1)
				g_error("cannot transpose without parameters");
		} else if (arg < argc) {
			(void) strcpy(input,argv[arg++]);
		} else
			break;
	} else {
		(void) printf("\nselect tunes: ");
		//if ((gets(input)) == NULL)
		if ((fgets(input, 99, stdin)) == NULL)
			break;
		(void) printf("\n");
	}

	if ((!strcmp(input,"q")) || (!strcmp(input,"quit")))
		break;

	if (input[0] == '\\') {
		draw_text(NULL,input);
		continue;
	}

	for (c = 0; input[c] != ':' && input[c]; ++c)
		filename[c] = input[c];
	filename[c] = '\0';

	if (c == 0)
		break;

	if (input[c] == '\0')
		(void) strcpy(&input[c],":-");
	pinput = &input[c+1];

	first = -999;
	last = -999;

	if (openIn(filename) == NULL)
		continue;

	(void) printf("\nselection: %s\n",filename);

	strip_path(basename,filename);

	x = 0;
	nblanks = 0;
	(void) strcpy(dflt_meter,"C");
	dflt_origin[0] = '\0';
	dflt_rhythm[0] = '\0';
	in_tune = 0;

	while ((getsIn(xline)) != NULL) { /* in file */

	if ((strncmp(xline,"M:",2)) == 0 && !in_tune) {
		if (trnsps) output_transline(xline);
		stripcpy(dflt_meter,&xline[2]);
	} else if ((strncmp(xline,"F:",2)) == 0 && !in_tune) {
		if (trnsps) output_transline(xline);
		strip_path(basename,&xline[2]);
	} else if ((strncmp(xline,"O:",2)) == 0 && !in_tune) {
		if (trnsps) output_transline(xline);
		stripcpy(dflt_origin,&xline[2]);
	} else if ((strncmp(xline,"R:",2)) == 0 && !in_tune) {
		if (trnsps) output_transline(xline);
		stripcpy(dflt_rhythm,&xline[2]);
	} else if (xline[0] == '\\' && !in_tune) {
		if (trnsps) output_transline(xline);
		strip(xline,dummy);
		if (f == 'm') draw_text(NULL,xline);
	} else if (is_comment(xline)) {
		if (trnsps && !in_tune) output_transline(xline);
	} else if (xline[0] == '\n') {
		if (trnsps && nblanks == 0) {
			output_transline(xline);
			nblanks += 1;
		}
		in_tune = 0;
	} else if ((strncmp(xline,"X:",2)) == 0) { /* in tune */

	in_tune = 1;
	(void) strcpy(number,&xline[2]);
	prev_x = x;
	x = atoi(number);
	if (x != prev_x) {
		y = 1;
		njoint = 0;
	} else
		y+= 1;

	if (x > last || (x == last && y > ylast)) {
		if (!(range(&first,&last,&yfirst,&ylast,&pinput))) {
			(void) printf("error in input format\n");
			(void) printf("abandoning file \"%s\"\n",filename);
			closeIn();
			continue;
		}
	}

	if (x < first || (x == first && y < yfirst))
		continue;

	++logn;
	++log;
	(void) getsIn(full_title);
	if ((strncmp(full_title,"T:",2)) != 0)
		abc_error("T: field should follow X: field");
	if (trnsps) (void) strcat(xline,full_title);

	strip(full_title,dummy);
	if (f == 'm') {
		article(&full_title[2],title[0]);
		detex(title[0],full_title);
	} else {
		detex(&full_title[2],title[0]);
		article(title[0],full_title);
	}
	(void) printf(" processing \"%s%4d %s\"\n",
		basename,x,full_title);
	(void) fflush(stdout);
	if (f == 'm') {
		for (m = 0; m < 79; ++m) head[m] = '*';
		head[10] = ' ';
		for (m = 0; full_title[m]; ++m)
			head[m+11] = full_title[m];
		head[m+11] = ' ';
		head[79] = '\0';
		draw_text("Z",head);
	}

	if (trnsps == 1) {
		transpose = 0;
		(void) printf("  Transpose? ");
		//(void) gets(trans);
		(void) fgets(trans, 99, stdin);
		if (trans[0] == '^' || trans[0] == '_')
			interval(trans);
		else {
			(void) sscanf(trans,"%d",
				&transpose);

			offset = 0;
			if (transpose) {
				(void) printf("  Note offset? ");
				//(void) gets(trans);
				(void) fgets(trans, 99, stdin);
				(void) sscanf(trans,"%d",&offset);
			}
		}
	}
	if (offset || transpose) output_transline(xline);

	for (i = 0; i < 26; ++i)
		entry->fields[i][0] = '\0';
	entry->BARS[0] = '\0';
	(void) strcpy(entry->FILENAME,basename);
	(void) strcpy(entry->METER ,dflt_meter );
	(void) strcpy(entry->ORIGIN,dflt_origin);
	(void) strcpy(entry->RHYTHM,dflt_rhythm);
	stripcpy(entry->NUMBER,number);
	titles = 1;
	(void) getsIn(xline);

	while ((strncmp(xline,"K:",2)) != 0) {

		if (offset || transpose) output_transline(xline);

		if (is_field(xline)) { /* is field */

		if (strchr("BEGILMNOPQR",xline[0]))
			stripcpy(entry->fields[xline[0]-'A'],&xline[2]);
		else {
			strip(xline,dummy);
			switch (xline[0]) {
			case 'T':
				++logn;
				if (f == 'm')
					article(&xline[2],title[titles++]);
				else {
					if (settings.mine && entry->SOURCE[0]) {
						(void) strcat(title[titles-1],
							entry->SOURCE);
						entry->SOURCE[0] = '\0';
					}
					detex(&xline[2],title[titles++]);
				}
				break;
			case 'D':
				if (settings.mine) {
				  if (f == 'm') {
				    if (entry->SOURCE[0])
				      (void) strcat(entry->SOURCE,", ");
				    (void) sprintf(end_of(entry->SOURCE),
				      "{\\%.2s}",&xline[2]);
				    if ((int) (strlen(xline)) > 7)
				      (void) sprintf(end_of(entry->SOURCE),
				        " -- %s",&xline[7]);
				  } else {
				    if (entry->SOURCE[0] == '\0')
				      (void) strcpy(entry->SOURCE," ");
				    if (xline[6] == '_')
				      (void) strncat(entry->SOURCE,&xline[1],5);
				    else
				      (void) strncat(entry->SOURCE,&xline[1],6);
				  }
				}
				break;
			case 'S':
				if (f == 'm') {
				  if (entry->SOURCE[0])
				    (void) strcat(entry->SOURCE,", ");
				  (void) strcat(entry->SOURCE,&xline[2]);
				}
				break;
			case 'C':
				if (f == 'i')
				  detex(&xline[2],entry->COMPOSER);
				else
				  (void) strcpy(entry->COMPOSER,&xline[2]);
				break;
			case 'A':
				if (f == 'i') {
					detex(&xline[2],entry->AREA);
					(void) sprintf(end_of(title[titles-1]),
						" (%s)",entry->AREA);
				} else
					(void) strcpy(entry->AREA,&xline[2]);
				break;
			case 'Z':
				detex(&xline[2],entry->ZNOTE);
				if (f == 'm') draw_text("Z",entry->ZNOTE);
				break;
			case 'H':
				history = 1;
				while ((getsIn(xline)) != NULL) {
					if (xline[1] == ':')
						break;
					if (offset || transpose)
						output_transline(xline);
				}
				break;
			default:
				abc_warning("ignoring %c field in header",
					xline[0]);
				break;
			}
		}

		} /* is field */ else if (!is_comment(xline))
			abc_error("unrecognised line");

		if (history == 1)
			history = 0;
		else
			(void) getsIn(xline);
	}

	strip(xline,key_comment);
	(void) strcpy(entry->KEY,&xline[2]);

	process_abc(title,titles,entry,key_comment,
		entry->BARS,"",(char *) NULL,output,
		nbars,0,(int *) NULL);
	if (f == 'i') {
		if (settings.mine && entry->SOURCE[0])
			(void) strcat(title[titles-1],
				entry->SOURCE);
		if (titles != 1)
			(void) sprintf(entry->JOINT,
				".%d",++njoint);
		for (ttl = 0; ttl < titles; ++ttl) {
/*
			if (settings.mine) {
				for (t = 0; title[ttl][c]; ++t)
					if (strchr(" [-(",title[ttl][c-1])
						&& title[ttl][c] >= 'a'
						&& title[ttl][c] <= 'z')
						title[ttl][c] += 'A' - 'a';
				if (ttl == 0)
					for (c = 0; entry->GROUP[c]; ++c)
						(void) sprintf(end_of(title[0]),
						  " {%c}",entry->GROUP[c]);
			}
*/
			(void) strcpy(entry->TITLE,title[ttl]);
			if (entry->LENGTH[0] == '\0') {
				get_dnl(entry);
			}
			(void) put_record(format,Index,entry);
		}
	}

	in_tune = 0;

	} /* in tune */

	} /* in file */

	if ((strcmp(filename,"stdin")) == 0) {
		logn_total += logn;
		log_total += log;
		break;
	} else {
		closeIn();
		(void) printf("end of file \"%s\"\n",filename);
	}

	(void) fprintf(Log,"%-20.20s %4d %4d\n",basename,logn,log);

	} while (input[0]); /* loop over input files */

	(void) printf("\n");
	if (f == 'm')
		close_TeX();
	(void) fprintf(Log,"%-20.20s %4d %4d\n","Total",logn_total,log_total);
	for (i = 0; i < 26; ++i)
		free(entry->fields[i]);
	free(entry->BARS);
	return(0);
}
