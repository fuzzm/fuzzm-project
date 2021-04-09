/************************************************************************
*									*
*			abc.c - V1.6.1 (Jan 97)				*
*									*
*		by    Chris Walshaw (C.Walshaw@gre.ac.uk)		*
*									*
*	Copyright Chris Walshaw. Permission is granted to use and	*
*	copy provided that this copyright notice remains attached.	*
*	This code may not be sold.					*
*									*
************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"

#define	CROTCHET	8
#define	QUAVER		4
#define	SEMIQUAVER	2
#define	DEMISEMIQUAVER	1

extern	void	tune2tex(char[][99],int,Record*,int,Symbol*,Field*,Field*,
				Field*);

FILE	*Trans;
Setting	settings = { 0, 0, 0, 0, 0, 0, 0 };
int	bass, treble;
int	dnl = 0;
int	transpose;
int	offset;
int	nblanks;

typedef struct {
	int	type;
	int	transpose;
	int	nbars;
	int	warnings;
} Output;

static	FILE	*In;
static	char	abc_file[99];
static	int	input_line;
static	int	just_comment;

static	Barline	*bar_start;
static	Note	*beam_start = NULL;
static	Note	*note;
static	Symbol	*current = NULL;
static	int	n_notes = 0;

static	const	char	*Key_array[] = {
				"Fb","Cb","Gb","Db","Ab","Eb","Bb",
				"F", "C", "G", "D", "A", "E", "B",
				"F#","C#","G#","D#","A#","E#","B#",
			"" };
static	const	char	**Key = Key_array+8;

static	const	int	Csemitones[] = { 0, 2, 4, 5, 7, 9, 11 };
static		int	 semitones[7];

static	int	max = 0; /* max notes in a beam */

static	int	token, prev_token;
static	int	token_length = 0;
static	char	*token_ptr = NULL;

#define	MAX_N_BLOCKS	10
#define	BLOCK_LENGTH	400

static	int	n_blocks;
static	Symbol	*block[MAX_N_BLOCKS];
static	int	n_symbols = 0;

static	int	compound_time = 0;
static	frac	fbar_total;
static	int	bar_no;
static	char	bar[3][99];
static	char	current_bar[99];
static	int	in_old_slur = 0;
static	int	bar_length;
static	int	stave;
static	Output	output;
static	int	tnote = 0;
static	int	beam_length = 0;
static	Note	*chord_root = NULL;	/* first note of chord */
static	int	tuplet_n_notes = 0;	/* number of notes required in tuplet */
static	int	tuplet_note_no = 0;	/* number of notes in tuplet so far */
static	frac	tuplet_fraction = {1,1};/* fractional length of a tuplet note */
static	frac	broken = { 0, 0 };
static	int	continuation = 0;
static	int	ignore;

enum	env_types {
		IN_GRACE,
		IN_CHORD,
		IN_BROKEN,
		IN_TIE,
		IN_TUPLET,
		MAX_ENVS
	};

static	int	env[MAX_ENVS] = { 0, 0, 0, 0, 0 };

static	const	char	*token_names[] = {
				"gchord",
				"accent",
				"accidental",
				"note",
				"octaver",
				"length",
				"divisor",
				"broken",
				"bar",
				"repeat",
				"space",
				"tie",
				"tuplet",
				"macro",
				"continuation",
				"justification",
				"newline",
				"open grace",
				"close grace",
				"open chord",
				"close chord",
				"open slur",
				"close slur",
				"ampersand",
				"comment",
				""
			};

enum token_types {
	GCHORD_TKN,	/* "*" */
	ACCENT_TKN,	/* ~.uv */
	ACCIDENTAL_TKN,	/* ^^ ^ = _ __ */
	NOTE_TKN,	/* A-Ga-gz */
	OCTAVER_TKN,	/* ', */
	LENGTH_TKN,	/* 1-99 */
	DIVISOR_TKN,	/* / */
	BROKEN_TKN,	/* > >> >>> < << <<< */
	BAR_TKN,	/* | [| || |] |: :: :| |1 :|2 */
	REPEAT_TKN,	/* [1 [2 */
	SPACE_TKN,	/*   \t */
	TIE_TKN,	/* - */
	TUPLET_TKN,	/* (n */
	MACRO_TKN,	/* H-Z */
	CONTINUE_TKN,	/* \ */
	JUSTIFY_TKN,	/* * */
	NEWLINE_TKN,	/* \n */
	OPEN_GRACE_TKN,	/* { */
	CLOSE_GRACE_TKN,/* } */
	OPEN_CHORD_TKN,	/* [ */
	CLOSE_CHORD_TKN,/* ] */
	OPEN_SLUR_TKN,	/* ( */
	CLOSE_SLUR_TKN,	/* ) */
	AMPERSAND_TKN,	/* & */
	TRAILING_TKN,	/* % */
	MAX_TKNS
};

static	int	syntax_table[MAX_TKNS+1][MAX_TKNS+1] = {
		/*	  " ~ = A ' 2 / > | [   - t H \ * n { } [ ] ( ) & % */
		/* " */	{ 0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0 },
		/* ~ */	{ 0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0 },
		/* = */	{ 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		/* A */	{ 1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0 },
		/* ' */	{ 1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0 },
		/* 2 */	{ 1,1,1,1,0,0,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0 },
		/* / */	{ 1,1,1,1,0,0,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0 },
		/* > */	{ 1,1,1,1,0,0,0,0,0,0,1,1,0,1,0,0,0,1,0,0,0,1,0,1,1,0 },
		/* | */	{ 1,1,1,1,0,0,0,0,1,1,1,0,1,1,1,1,1,1,0,1,0,1,0,0,1,0 },
		/* [ */	{ 1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,0,0,1,0,1,0,1,0,0,1,0 },
		/*   */	{ 1,1,1,1,0,0,0,0,1,1,1,0,1,1,1,1,0,1,0,1,0,1,0,1,0,0 },
		/* - */	{ 1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,1,1,1,0,0,0,1,0,1,1,0 },
		/* t */	{ 1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,0 },
		/* H */	{ 1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1,1,0 },
		/* \ */	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0 },
		/* * */	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0 },
		/* n */	{ 1,1,1,1,0,0,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,1,0,0,1,0 },
		/* { */	{ 0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0 },
		/* } */	{ 1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1,1,0 },
		/* [ */	{ 0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0 },
		/* ] */	{ 1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1,1,0 },
		/* ( */	{ 0,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0 },
		/* ) */	{ 1,1,1,1,0,0,0,1,1,0,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,0 },
		/* & */	{ 1,1,1,1,0,0,0,0,1,0,1,0,1,1,0,0,0,1,0,1,0,1,0,0,0,0 },
		/* % */	{ 1,1,1,1,0,0,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,1,0,0,1,0 },
			{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};

#ifdef TEST
void	main(void)
{
	int	i,j;

	for (i = 0; i < MAX_TKNS-1; ++i) {
		for (j = 0; j < MAX_TKNS-1; ++j) {
			(void) printf("%s followed by %s is ",
				token_names[i],token_names[j]);
			if (syntax_table[i][j])
				(void) printf("OK\n");
			if (syntax_table[i][j] == 0)
				(void) printf("BAD\n");
		}
	}
}
#else

void	abc_warning(char *fmt, ...)
{
	va_list	ap;
	if (output.warnings == 0) return;
	va_start(ap,fmt);
	(void) fprintf(stdout,"WARNING: line no. %d - ",input_line);
	(void) vfprintf(stdout,fmt,ap);
	(void) fprintf(stdout,"\n");
	va_end(ap);
}

void	abc_error(char *fmt, ...)
{
	va_list	ap;
	va_start(ap,fmt);
	(void) fprintf(stderr,"error in input file %s: line no. %d - ",
				abc_file,input_line);
	(void) vfprintf(stderr,fmt,ap);
	(void) fprintf(stderr,"\n");
	va_end(ap);
	exit(1);
}

FILE	*openIn(char *filename)
{
	char	savename[99];
	if ((strcmp(filename,"stdin")) == 0)
		In = stdin;
	else if ((In = fopen(filename,"r")) == NULL) {
		(void) strcpy(savename,filename);
		(void) strcat(filename,".abc");
		if ((In = fopen(filename,"r")) == NULL) {
			(void) printf("file \"%s\" does not exist\n",savename);
			(void) strcpy(filename,savename);
		}
	}
	(void) strcpy(abc_file,filename);
	input_line = 0;
	just_comment = 0;
	return(In);
}

char	*getsIn(char *s)
{
	if (fgets(s,999,In) == NULL) {
		return(NULL);
	} else {
		input_line += 1;
		if (s[0] == '%') just_comment = 1;
		else just_comment = 0;
		return(s);
	}
}

void	closeIn(void)
{
	(void) fclose(In);
}

void	read_settings(void)
{
	FILE	*fp;
	char	line[99];

	if ((fp = fopen("settings","r")) == NULL) return;
	while (fgets(line,99,fp) != NULL) {
		     if (strncmp(line,"justify",strlen("justify")) == 0)
			settings.justification = 1;
		else if (strncmp(line,"gchords above",
				strlen("gchords above")) == 0)
			settings.gchords_above = 1;
		else if (strncmp(line,"autobeam",strlen("autobeam")) == 0)
			settings.autobeam = 1;
		else if (strncmp(line,"oldrepeats",strlen("oldrepeats")) == 0)
			settings.old_repeats = 1;
		else if (strncmp(line,"oldchords",strlen("oldchords")) == 0)
			settings.old_chords = 1;
		else if (strncmp(line,"oldslurs",strlen("oldslurs")) == 0)
			settings.old_slurs = 1;
		else if (strncmp(line,"mine",strlen("mine")) == 0)
			settings.mine = 1;
		else
			g_error("in settings - unrecognised line: %s",line);
	}
	(void) fclose(fp);
}

static	int	hcf(int a, int b)
{
	int	ans = 1, p;
	const	int	prime[] = { 2, 3, 5, 7, 11, 13, 17, 0 };

	for (p = 0; prime[p] && a > prime[p] && b > prime[p]; ++p) {
		while (a%prime[p] == 0 && b%prime[p] == 0) {
			a /= prime[p];
			b /= prime[p];
			ans *= prime[p];
		}
	}
	return(ans);
}

static	void	add_frac(frac a, frac b, frac *ans)
{
	int	f;
	ans->n = (a.n*b.d) + (b.n*a.d);
	ans->d = a.d*b.d;
	f = hcf(ans->n, ans->d);
	ans->n /= f;
	ans->d /= f;
}

char	*end_of(char *s)
{
	return(&s[strlen(s)]);
}

int	is_field(char *line)
{
	if (line[1] == ':' && 'A' <= line[0] && line[0] <= 'Z')
		return(1);
	else
		return(0);
}

void	strip(char *str, char *comment)
{	/* removes trailing space, comment and
	   newline and copies it to comment */
	char	*c_ptr;

	comment[0] = '\0';
	c_ptr = strchr(str,'%');
	if (c_ptr == NULL) c_ptr = strchr(str,'\n');
	while (c_ptr > str && (*(c_ptr-1) == ' ' || *(c_ptr-1) == '\t'))
		--c_ptr;
	if (c_ptr) {
		(void) strcpy(comment,c_ptr);
		*c_ptr = '\0';
	}
}

void	stripcpy(char *out_str, char *in_str)
{
	static	char	dummy[999];
	strip(in_str,dummy);
	(void) strcpy(out_str,in_str);
}

static	const	char	*Notes[]={
				"",  "",
				"",  "",  "",  "",  "",  "",  "",
				"C,","D,","E,","F,","G,","A,","B,",
				"C", "D", "E", "F", "G", "A", "B",
				"c", "d", "e", "f", "g", "a", "b",
				"c'","d'","e'","f'","g'","a'","b'"
			};

static	void	transpose_note(void)
{

	const	char	*Accidental_array[] = { "__", "_", "=", "^", "^^" };
	const	char	**Accidental = Accidental_array+2;

	if (note->iaccidental)
		(void) fprintf(Trans,"%s",Accidental[note->iaccidental-3]);
	if (note->pitch == -1)
		(void) fprintf(Trans,"z");
	else
		(void) fprintf(Trans,"%s",Notes[note->pitch]);
	tnote = 0;
}

static	void	end_beam(void)
{
	if (note) {
		if (!env[IN_GRACE] && env[IN_TUPLET]) {
			if (tuplet_note_no == 0)
				abc_error("empty tuplet");
			if (tuplet_n_notes/2 == tuplet_note_no)
				note->tuplet = env[IN_TUPLET];
			if (++tuplet_note_no == tuplet_n_notes) {
				(void) strcat(note->end,"s");
				env[IN_TUPLET] = 0;
				/* shouldn't need to do this */
				tuplet_n_notes = tuplet_note_no = 0;
			}
		}
/*
		if (!strchr(note->end,'b') && !n_notes) g_error("no notes");
*/
		n_notes = 0;
		(void) strcat(note->end,"b");
	} else if (n_notes) g_error("no note");
	beam_length = 0;
}

static	int	global_accidentals[7];

static	void	new_symbol(int type)
{
	int	index;
	frac	fnote_length;
	if (note && note->pitch) {
		if (tnote) transpose_note();
		if (note->pitch != -1 && note->iaccidental == 0)
			note->iaccidental = global_accidentals[note->pitch%7];
	}

	if (note && note->pitch == 0) /* this is a macro */
		note->n_notes = 1;

	if (note && note->broken.n) {
		note->length *= note->broken.n;
		if (note->length%note->broken.d)
			abc_error("note length will not divide by %d",
				note->broken.d);
		note->length /= note->broken.d;
		note->broken.n = note->broken.d = 0;
	}

	if (note && note->type == NORMAL) {
		if (env[IN_TUPLET]) {
			if (tuplet_n_notes/2 == tuplet_note_no)
				note->tuplet = env[IN_TUPLET];
			if (++tuplet_note_no == tuplet_n_notes) {
				(void) strcat(note->end,"s");
				env[IN_TUPLET] = 0;
				/* shouldn't need to do this */
				tuplet_n_notes = tuplet_note_no = 0;
				end_beam();
			}
		} else {
			beam_length += note->length;
			if (settings.autobeam && beam_length >= max)
				end_beam();
		}

		fnote_length.n = note-> length*tuplet_fraction.n;
		fnote_length.d = tuplet_fraction.d;
		if (env[IN_TUPLET] == 0)
			tuplet_fraction.n = tuplet_fraction.d = 1;
		if (fbar_total.n >= 0)
			add_frac(fbar_total,fnote_length,&fbar_total);

	}

	n_symbols += 1;
	if (n_symbols%BLOCK_LENGTH == 0) {
		n_blocks += 1;
		if (n_blocks >= MAX_N_BLOCKS)
			g_error("out of memory - increase MAX_N_BLOCKS");
		ALLOC(block[n_blocks],Symbol,BLOCK_LENGTH);
	}
	index = n_symbols - ((n_symbols/BLOCK_LENGTH)*BLOCK_LENGTH);
	if (current) {
		current->next = &block[n_blocks][index];
		block[n_blocks][index].prev = current;
		current = current->next;
	} else {
		current =  &block[n_blocks][index];
	}
	current->type = type;
	if (type == NOTE)
		note = &current->u.note;
	else
		note = NULL;
}

static	void	output_transpose(void)
{
	int	i;

	if (output.transpose == 0) return;

	if (note && tnote) transpose_note();

	for (i = 0; i < token_length; ++i)
		(void) fputc(*(token_ptr+i),Trans);
}

void	output_transline(char *s)
{
	nblanks = 0;
	(void) fputs(s,Trans);
}

static	void	save_two_bars(void)
{
	if (output.nbars == 0) return;

	if (token == SPACE_TKN)
		(void) strcat(current_bar," ");
	else
		(void) strncat(current_bar,token_ptr,token_length);
}

static	void	check_syntax(int new_token)
{
	int	e;
	const	char	*env_names[] = {
				"in grace",
				"in chord",
				"in broken",
				"in tie",
				"in tuplet",
				""
			};
	const	int	env_table[MAX_ENVS][MAX_TKNS+1] = {
	/*		  " ~ = A ' 2 / > | [   - t H \ * n { } [ ] ( ) & % */
	/* grace  */	{ 0,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0 },
	/* chord  */	{ 0,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0 },
	/* broken */	{ 1,1,1,1,0,0,0,0,0,0,1,1,0,1,0,0,0,1,1,0,0,1,0,0,0,0 },
	/* tie	  */	{ 1,1,1,1,0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,1,1,0,1,0 },
	/* tuplet */	{ 1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0 },
	};

	prev_token = token;
	token = new_token;

	for (e = 0; e < MAX_ENVS; ++e)
		if (env[e] && env_table[e][token] == 0)
			abc_error("syntax error - %s cannot be used %s",
				token_names[token],env_names[e]);

	if (syntax_table[prev_token][token] == 0)
		abc_error("syntax error - %s cannot follow %s",
			token_names[token],token_names[prev_token]);
}

static	void	set_base(Field *meter)
{
	char	*meter_str = &meter->string[2];
	int	l;

	dnl = QUAVER;
	l = strlen(meter_str)-1;
	while (meter_str[l] == ' ') l--;
	if (meter_str[l] == 'l')
		dnl /= 2;
	else if (meter_str[l] == 's')
		dnl *= 2;

	max = 16;
	     if ((strncmp(meter_str, "2/4",3)) == 0) { max =  8; }
	else if ((strncmp(meter_str, "3/2",3)) == 0) { max =  8; }
	else if ((strncmp(meter_str, "3/4",3)) == 0) { max = 12; }
	else if ((strncmp(meter_str, "3/8",3)) == 0) { max = 12; }
	else if ((strncmp(meter_str, "5/8",3)) == 0) { max = 12; }
	else if ((strncmp(meter_str, "6/8",3)) == 0) { max = 12; }
	else if ((strncmp(meter_str, "9/8",3)) == 0) { max = 12; }
	else if ((strncmp(meter_str,"12/8",4)) == 0) { max = 12; }

	if (meter_str[0] == 'C') {
		meter->info2 = 4;
		if (meter_str[1] == '|')
			meter->info1 = -2;
		else
			meter->info1 = -4;
		bar_length = 32;
	} else {
		(void) sscanf(meter_str,"%d/%d",&meter->info1,&meter->info2);
		if ((meter->info1 < 1 || meter->info1 >100)
		 || (meter->info2 < 1 || meter->info2 >100)) {
			(void) printf("meter not recognised\n");
			meter->info1 = -4;
			meter->info2 = 4;
		} else {
			bar_length = (meter->info1*32)/meter->info2;
			if (((float) meter->info1/(float) meter->info2) < 0.75)
				dnl /= 2;
		}
	}
	if (abs(meter->info1)%3 == 0)
		compound_time = 1;
	else
		compound_time = 0;
}

void	get_dnl(Record *entry)
{
	char	meter[99];
	Field	meter_field;
	meter_field.string = &meter[0];
	meter_field.info1 = meter_field.info2 = 0;
	(void) strcpy(meter,"M:");
	(void) strcat(meter,entry->METER);
	set_base(&meter_field);
	(void) sprintf(entry->LENGTH,"1/%d",32/dnl);
}

void	set_dnl(char *dnl_str)
{
	while (*dnl_str == ' ') ++dnl_str;

	     if ((strncmp(dnl_str,"1/4" ,3)) == 0) dnl=CROTCHET;
	else if ((strncmp(dnl_str,"1/8" ,3)) == 0) dnl=QUAVER;
	else if ((strncmp(dnl_str,"1/16",3)) == 0) dnl=SEMIQUAVER;
	else if ((strncmp(dnl_str,"1/32",4)) == 0) dnl=DEMISEMIQUAVER;
	else {
		(void) printf("default note length not recognised\n");
		dnl = QUAVER;
	}
}

static	void	ignore_space(char *s, int *c)
{
	while (s[*c] == ' ') {
		if (transpose || offset)
			(void) fprintf(Trans," ");
		*c += 1;
	}
}

static	void	ignore_alpha(char *s, int *c)
{
	while ((s[*c] >= 'A' && s[*c] <= 'Z')
	    || (s[*c] >= 'a' && s[*c] <= 'z')) {
		if (transpose || offset)
			(void) fprintf(Trans,"%c",s[*c]);
		*c += 1;
	}
}

static	void	transpose_accidental(int pitch)
{
	int	sharpness = note->iaccidental - 3;
	sharpness += ((54-2*(pitch-9))%7+transpose+14)/7-2;
	if (sharpness < -2)
		g_error("transpose - too many flats");
	if (sharpness >  2)
		g_error("transpose - too many sharps");
	note->iaccidental = sharpness + 3;
}

static	int	mystrncasecmp(char *s1, char *s2, int n)
{
	char	tmp1[99],tmp2[99];
	int	i;

	if (n > 98) g_error("string too long in mystrncasecmp");

	for (i = 0; i < n && s1[i]; ++i)
		tmp1[i] = lower(s1[i]);
	tmp1[i] = '\0';
	for (i = 0; i < n && s2[i]; ++i)
		tmp2[i] = lower(s2[i]);
	tmp2[i] = '\0';
	return(strncmp(tmp1,tmp2,n));
}

static	void	sharps_flats(Field *key)
{
	Note	dummy;
	int	pitch;
	int	acc;
	int	c = 0,s_f, i;
	char	*s = &key->string[2];
	const	char	*key_letter = { "FCGDAEB" };

	if (key->string[0] == 'K' && (offset || transpose))
		(void) fprintf(Trans,"K:");

	ignore_space(s,&c);

	for (s_f = -1; s_f < 6; ++s_f)
		if (s[c] == key_letter[s_f+1]) break;

	if (s_f == 6) {
		       if (s[c] == 'H' && s[c+1] == 'p') {
			s_f = 2;
			key->info2 = 1;
			c += 1;
		} else if (s[c] == 'H' && s[c+1] == 'P') {
			s_f = 2;
			key->info2 = 2;
			c += 1;
		} else {
			if (key->string[0] == 'G') {
			(void) printf("Gchord not recognised - using C\n");
			} else {
				(void) printf("Key not recognised - using C\n");
				bass = 0;
				treble = 1;
			}
			key->info1 = transpose;
			return;
		}
	}
	c += 1;

	       if (s[c] == '#') {
		s_f += 7;
		c += 1;
	} else if (s[c] == 'b') {
		s_f -= 7;
		c += 1;
	}

	if (offset || transpose)
		(void) fprintf(Trans,"%s",Key[s_f+transpose]);

	ignore_space(s,&c);

	       if ((mystrncasecmp(&s[c],"Lyd",3)) == 0) {	/* Lydian */
		s_f += 1;
		ignore_alpha(s,&c);
	} else if ((mystrncasecmp(&s[c],"Maj",3)) == 0		/* Major */
		|| (mystrncasecmp(&s[c],"Ion",3)) == 0) {	/* Ionian */
		ignore_alpha(s,&c);
	} else if ((mystrncasecmp(&s[c],"Mix",3)) == 0) {	/* Mixolydian */
		s_f -= 1;
		ignore_alpha(s,&c);
	} else if ((mystrncasecmp(&s[c],"Dor",3)) == 0) {	/* Dorian */
		s_f -= 2;
		ignore_alpha(s,&c);
	} else if ((mystrncasecmp(&s[c],"Min",3)) == 0		/* Minor */
		|| (mystrncasecmp(&s[c],"Aeo",3)) == 0) {	/* Aeolian */
		s_f -= 3;
		ignore_alpha(s,&c);
	} else if ((mystrncasecmp(&s[c],"Phr",3)) == 0) {	/* Phrygian */
		s_f -= 4;
		ignore_alpha(s,&c);
	} else if ((mystrncasecmp(&s[c],"Loc",3)) == 0) {	/* Locrian */
		s_f -= 5;
		ignore_alpha(s,&c);
	} else if (s[c] == 'm') {
		s_f -= 3;
		ignore_alpha(s,&c);
	}

	ignore_space(s,&c);

	if ('0' <= s[c] && s[c] <= '9'
	 && s[c+1] == '&'
	 && '0' <= s[c+2] && s[c+2] <= '9') { /* get bass & treble clef info */
		bass = s[c] - '0';
		treble = s[c+2] - '0';
		if (transpose || offset) {
			for (i = 0; i < 3; ++i)
				(void) fprintf(Trans,"%c",s[c+i]);
		}
	} else {
		bass = 0;
		treble = 1;
	}

	/* reset global accidentals */
	for (i = 0; i < 7; ++i)
		global_accidentals[i] = '\0';

	for (; s[c] != '\n' && s[c] != '\0'; ++c) {
		ignore_space(s,&c);
		if (strchr("_=^",s[c]) && s[c+1]) {
			c += 1;
			switch (s[c-1]) {
			case '^':
				if (s[c] == '^' && s[c+1]) {
					c += 1;
					acc = DBL_SHARP;
				} else acc = SHARP;
				break;
			case '_':
				if (s[c] == '_' && s[c+1]) {
					c += 1;
					acc = DBL_FLAT;
				} else acc = FLAT;
				break;
			case '=':
				acc = NATURAL;
				break;
			}
			note = &dummy;
			note->iaccidental = acc;

			switch (s[c]) {
			case 'c': case 'd': case 'e': case 'f': case 'g':
				pitch = s[c]-'c'+23+offset;
				break;
			case 'a': case 'b':
				pitch = s[c]-'a'+28+offset;
				break;
			default:
				abc_error("unrecognised accidental '%c'"
					  "in key string",s[c]);
				break;
			}

			if (transpose || offset) {
				transpose_accidental(pitch);
				while (pitch > 29) pitch -= 7;
				while (pitch < 23) pitch += 7;
				note->pitch = pitch;
				transpose_note();
			}

			global_accidentals[Notes[pitch][0]-'a']
				= note->iaccidental;
			note = NULL;
		}
	}

	key->info1 = s_f + transpose;
}

int	range(int *first,int *last,int *yfirst,int *ylast,char **input)
{
	*ylast = 4999;
	if (**input == '\0') {
		*first = 4999;
		*last = 4999;
		*yfirst = 4999;
		return(1);
	}
	if (**input == '-') {
		*first = 1;
		*yfirst = 1;
	} else if (**input >= '0' && **input <= '9') {
		(void) sscanf(*input,"%d",first);
		while (**input >= '0' && **input <= '9')
			(*input)++;
		if (**input == '.') {
			(*input)++;
			if (**input >= '1' && **input <= '9')
				*yfirst = **input - '0';
			else if (**input >= 'a' && **input <= 'z')
				*yfirst = **input - 'a' + 10;
			else
				return(0);
			(*input)++;
		} else {
			*yfirst = 0;
		}
	} else
		return(0);

	if (**input == '-') {
		(*input)++;
		if (**input == '\0') {
			*last = 4999;
		} else if (**input >= '0' && **input <= '9') {
			(void) sscanf(*input,"%d",last);
			while (**input >= '0' && **input <= '9')
				(*input)++;
			if (**input == '.') {
				(*input)++;
				if (**input >= '1' && **input <= '9')
					*ylast = **input - '0';
				else if (**input >= 'a' && **input <= 'z')
					*ylast = **input - 'a' + 10;
				else
					return(0);
				(*input)++;
			}
			if (**input == ',')
				(*input)++;
		} else
			return(0);
	} else if (**input == ',') {
		*last = *first;
		if (*yfirst != 0) *ylast = *yfirst;
		(*input)++;
	} else if (**input == '\0') {
		*last = *first;
		if (*yfirst != 0) *ylast = *yfirst;
	} else
		return(0);
	return(1);
}

static	void	process_field(char *str)
{
	static	char	comment[999];
	Symbol	*previous = current;

	if (ignore) return;

	check_syntax(NEWLINE_TKN);

	strip(str,comment);

	if (str[0] != 'K' && (offset || transpose)) {
		(void) fprintf(Trans,"%s",str);
		(void) fprintf(Trans,"%s",comment);
	}

	new_symbol(FIELD);
	if (str[0] == 'M') {
		ALLOC(current->u.field.string,char,20);
	} else {
		ALLOC(current->u.field.string,char,(strlen(str)+1));
	}
	(void) strcpy(current->u.field.string,str);

	switch (str[0]) {
	case 'L':
		set_dnl(&str[2]);
		break;
	case 'M':
		set_base(&current->u.field);
		break;
	case 'T': case 'W':
		previous->newline = 2; /* stop piece before T or W field */
		if (settings.justification) previous->justify = 1;
		break;
	case 'K':
		sharps_flats(&current->u.field);
		if (offset || transpose) (void) fprintf(Trans,"%s",comment);
		break;
	case 'Q':
		current->u.field.info1 = dnl;
		break;
	case 'I': case 'E': case 'P': case '\\':
		break;
	default:
		abc_error("%c field not allowed in tune body",str[0]);
	}

}

static	void	process_trailing(void)
{
	check_syntax(TRAILING_TKN);

	output_transpose();
}

static	void	process_gchord(char *str)
{
	int	ctr, g;
	char	temp[99];
	char	gchord[5];
	Field	gchord_field;

/*
	if (!note || note->pitch)
*/
		new_symbol(NOTE);

	check_syntax(GCHORD_TKN);

	if (offset || transpose) {
		temp[0] = '\0';
		ctr = 0;
		if (str[ctr] == '(')
			(void) sprintf(end_of(temp),"%c",str[ctr++]);
		gchord_field.string = &gchord[0];
		gchord_field.info1 = gchord_field.info2 = 0;
		(void) strcpy(gchord,"G:");
		g = 2;
		gchord[g++] = str[ctr++];
		if (str[ctr] == '#' || str[ctr] == 'b')
			gchord[g++] = str[ctr++];
		gchord[g] = '\0';
		(void) fprintf(Trans,"\"");
		sharps_flats(&gchord_field);
		(void) sprintf(end_of(temp),"%s%s",
			Key[gchord_field.info1],&str[ctr]);
		(void) fprintf(Trans,"%s\"",&str[ctr]);
		(void) strcpy(str,temp);
	}

	if (note->gchord)
		abc_error("gchord already set");
	ALLOC(note->gchord,char,(strlen(str)+1));
	(void) strcpy(note->gchord,str);
}

static	void	process_macro(char c)
{
	check_syntax(MACRO_TKN);

	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	(void) sprintf(end_of(note->attributes),"%c",c);

	output_transpose();
}

static	void	process_accent(char c)
{
	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	check_syntax(ACCENT_TKN);

	output_transpose();

/* not sure about this, maybe remove later */
	if (!env[IN_GRACE] && (!env[IN_CHORD] || chord_root->chord == 0))
		save_two_bars();

	(void) sprintf(end_of(note->attributes),"%c",c);
}

static	void	process_accidental(int accidental)
{
	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	check_syntax(ACCIDENTAL_TKN);

	if (!env[IN_GRACE] && (!env[IN_CHORD] || chord_root->chord == 0))
		save_two_bars();

	if (note->iaccidental)
		abc_error("accidental already set");

	note->iaccidental = accidental;
}

static	void	process_note(int pitch)
{
	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	check_syntax(NOTE_TKN);

	if (offset || transpose) {
		if (note->iaccidental) transpose_accidental(pitch);
		tnote = 1;
	}

	if (n_notes == 0)
		beam_start = &current->u.note;
	n_notes += 1;
	note->pitch = pitch;
	if (stave < bass && pitch != -1) note->pitch -= 7;
	if (env[IN_GRACE])
		note->length = 1;
	else
		note->length = dnl;

	if (env[IN_GRACE])
		note->type = GRACE;
	else {
		if (env[IN_TIE]) { /* tie does not affect grace notes */
			(void) strcat(note->end,"t");
			env[IN_TIE] = 0;
		}

		if (env[IN_BROKEN]) { /* broken does not affect grace notes */
			note->broken = broken;
			env[IN_BROKEN] = broken.n = broken.d = 0;
		}

		if (env[IN_TUPLET] && tuplet_note_no == 0) {
			(void) strcat(note->start,"s");
			(void) strcat(note->start,"p");
		}

	}

	if (env[IN_CHORD]) {
		chord_root->chord += 1;
		if (chord_root->chord > 0) note->type = CHORD;
	}

	if (note->type == NORMAL) save_two_bars();
}

static	void	process_octaver(int octaver)
{
	check_syntax(OCTAVER_TKN);

	if (!note || note->pitch <= 0)
		abc_error("octaver attached to non note");

	if (note->type == NORMAL) save_two_bars();

	note->pitch += octaver;

	if (note->pitch <= 0)
		abc_error("octaver puts note out of range");
}

static	void	process_length(int length)
{
	check_syntax(LENGTH_TKN);

	output_transpose();

	if (note->type == NORMAL) save_two_bars();

	if (!note || note->pitch == 0)
		abc_error("length attached to non note");
	if (length > 99)
		abc_error("length too long");

	note->length *= length;
}

static	void	process_divisor(int length)
{
	check_syntax(DIVISOR_TKN);

	output_transpose();

	if (note->type == NORMAL) save_two_bars();

	if (!note || note->pitch == 0)
		abc_error("divisor attached to non note");

	if (note->length%length)
		abc_error("note length will not divide by %d",length);
	note->length /= length;
}

static	void	process_broken(int power)
{
	int	i;

	check_syntax(BROKEN_TKN);

	output_transpose();

	save_two_bars();

	if (!note || note->pitch == 0)
		abc_error("length attached to non note");
	broken.d = 1;
	for (i = 0; i < abs(power); ++i)
		broken.d *= 2;
	if (power > 0) {
		note->length *= (2*broken.d - 1);
		broken.n = 1;
	} else {
		broken.n = 2*broken.d - 1;
	}
	if (note->length%broken.d)
		abc_error("note length will not divide by %d",broken.d);
	note->length /= broken.d;
	env[IN_BROKEN] = 1;
}

static	void	process_bar(int bar_type)
{
	int	bar_total;

	end_beam();

	new_symbol(BAR_LINE);

	check_syntax(BAR_TKN);

	output_transpose();

	save_two_bars();

	if (fbar_total.n >= 0 && bass + treble == 1) {
		if (bar_no != 0 && fbar_total.n%fbar_total.d)
			abc_error("non-integer bar length");
		bar_total = fbar_total.n/fbar_total.d;
		if (bar_no == 0 && bar_total == bar_length) {
			bar_start->bar_no = 1;
			bar_no = 1;
		}
		if (bar_no != 0 && bar_total < bar_length
		&& (bar_type == BAR || bar_type == BAR1))
			abc_warning("bar number %d is too short",bar_no);
		if (bar_total > bar_length)
			abc_warning("bar number %d is too long",bar_no);
		if (output.nbars && bar_no < 3)
			(void) strcpy(bar[bar_no],current_bar);
		current_bar[0] = '\0';
/* this is wrong */
		if (bar_type == BAR || bar_type == BAR1
		 /*|| bar_type == LDBL_BAR || bar_type == LREPEAT*/)
			bar_no += 1;
		else {
			if (bar_no)
				output.nbars = 0;
			bar_no = 0;
		}
	}
	fbar_total.n = 0;
	fbar_total.d = 1;

	if (bar_no > 2) output.nbars = 0;

	current->u.bar.type = bar_type;
	bar_start = &current->u.bar;
	bar_start->bar_no = bar_no;
	stave = 0;
	beam_length = 0;
}

static	void	process_repeat(int no)
{
	Symbol	*bar;

	check_syntax(REPEAT_TKN);

	output_transpose();

/* not sure about this, maybe remove later */
	save_two_bars();

	for (bar = current; bar && bar->type != BAR_LINE; bar = bar->prev);
	if (!bar) abc_error("");
	if (bar->u.bar.repeat_no)
		abc_error("repeat already set");
	if (no == 1 && bar->u.bar.type == BAR)
		bar->u.bar.type = BAR1;
	else if (no == 2 && bar->u.bar.type == RREPEAT)
		bar->u.bar.type = RREPEAT2;
	else
		abc_error("repeat inappropriate for bar type");
}

static	void	process_space(void)
{
	end_beam();

	check_syntax(SPACE_TKN);

	output_transpose();

	save_two_bars();
}

static	void	process_tie(void)
{
	check_syntax(TIE_TKN);

	output_transpose();

	save_two_bars();

	env[IN_TIE] = 1;
	(void) strcat(note->start,"t");

	end_beam();
}

static	void	process_continuation(void)
{
	end_beam();

	check_syntax(CONTINUE_TKN);

	output_transpose();

	continuation = 1;
}

static	void	process_newline(void)
{
	end_beam();

	check_syntax(NEWLINE_TKN);

	output_transpose();

	if (!continuation) {
		current->newline = 1;
		if (settings.justification) current->justify = 1;
	}
}

static	void	process_open_chord(void)
{
	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	check_syntax(OPEN_CHORD_TKN);

	output_transpose();

	env[IN_CHORD] = 1;
	chord_root = &current->u.note;
	chord_root->chord = -1;
}

static	void	process_close_chord(void)
{
	check_syntax(CLOSE_CHORD_TKN);
	if (!env[IN_CHORD]) abc_error("not in chord");

	if (chord_root->length == 0) abc_error("empty chord");
	if (chord_root->chord <= 0) abc_error("empty chord");

	if (chord_root->chord == 1
	 && chord_root->length == note->length
	 && chord_root->pitch == note->pitch) { /* this is a unison */
		(void) strcat(chord_root->start,"u");
		end_beam();
	}

	output_transpose();

	env[IN_CHORD] = 0;

	chord_root = NULL;
}

static	void	process_open_close_chord(void)
{
	if (settings.old_chords == 0)
		abc_error("++ syntax obsolete; use [] or change settings file");
	if (env[IN_CHORD]) process_close_chord();
	else process_open_chord();
}

static	void	process_open_grace(void)
{
	check_syntax(OPEN_GRACE_TKN);

	output_transpose();

	env[IN_GRACE] = 1;
}

static	void	process_close_grace(void)
{
	check_syntax(CLOSE_GRACE_TKN);
	if (!env[IN_GRACE]) abc_error("not in grace");

	output_transpose();

	if (beam_start->type == GRACE) end_beam();
	env[IN_GRACE] = 0;
}

static	void	process_open_slur(void)
{
	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	check_syntax(OPEN_SLUR_TKN);

	output_transpose();

	(void) strcat(note->start,"s");
}

static	void	process_close_slur(void)
{
	check_syntax(CLOSE_SLUR_TKN);

	output_transpose();

	(void) strcat(note->end,"s");
}

static	void	process_open_close_slur(void)
{
	if (settings.old_slurs == 0)
		abc_error("ss syntax obsolete; use () or change settings file");

	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	check_syntax(OPEN_SLUR_TKN);

	output_transpose();

	if (in_old_slur) {
		(void) strcat(note->end,"s");
		in_old_slur = 0;
	} else {
		(void) strcat(note->start,"s");
		in_old_slur = 1;
	}
}

static	void	process_tuplet(char *s)
{
	int	p, q = -1, r = -1;
	char	*save_s = s;

	if (n_notes)
		end_beam();

	check_syntax(TUPLET_TKN);

	output_transpose();

	save_two_bars();

	if (!note || note->pitch || strchr(note->end,'b'))
		new_symbol(NOTE);

	p = atoi(s);
	while ('0' <= *s && *s <= '9') ++s;
	if (*s == ':') {
		++s;
		if ('0' <= *s && *s <= '9') {
			q = atoi(s);
			while ('0' <= *s && *s <= '9') ++s;
		}
	}
	if (*s == ':') {
		++s;
		if ('0' <= *s && *s <= '9') {
			r = atoi(s);
			while ('0' <= *s && *s <= '9') ++s;
		}
	}
	if (s-save_s != token_length-1) abc_error("in tuplet");
	if (q == -1) {
		switch (p) {
		case 2: case 4: case 8:
			q = 3;
			break;
		case 3: case 6:
			q = 2;
			break;
		case 5: case 7: case 9:
			if (compound_time)
				q = 3;
			else
				q = 2;
			break;
		default:
			abc_error("tuplet not recognised");
			break;
		}
	}
	if (r == -1) r = p;

	tuplet_fraction.n = q;
	tuplet_fraction.d = p;
	env[IN_TUPLET] = p;
	tuplet_n_notes = r;
	tuplet_note_no = 0;
}

static	void	process_justify(void)
{
	check_syntax(JUSTIFY_TKN);

	output_transpose();

	current->justify = 1;
}

static	void	process_ampersand(int level)
{
	end_beam();

	new_symbol(MISC);

	check_syntax(AMPERSAND_TKN);

	output_transpose();

	if (level == 1) stave += 1;
	else if (level == 2) stave = 0;

	current->u.misc.level = level;
}

static	void	process_beams(int n, Symbol *s)
{
	Note	*beam = NULL;
	Note	*grace = NULL;
	int	i;

	for (i = 0; i < n; ++i) {
		if (s->type != NOTE || s->u.note.pitch == 0) {
			s = s->next;
			continue;
		}
		beam = &s->u.note;
		beam->n_notes = 1;
		if (!strchr(beam->start,'t') && beam->pitch == -1) {
			s = s->next;
			continue;
		}
		while (s->next && s->next->type == NOTE
		 && s->next->u.note.pitch && !strchr(s->u.note.end,'b')
		 && (s->next->u.note.type == CHORD
		  || (beam->length < CROTCHET
		  && s->next->u.note.length < CROTCHET))) {
			if (!strchr(beam->start,'t')
			 && s->next->u.note.pitch == -1) break;
			if (beam->type == GRACE) {
				if (s->next->u.note.type != GRACE) break;
			} else { /* beam->type != GRACE */
				if (s->next->u.note.type == GRACE) {
					if (!grace) grace = &s->next->u.note;
					grace->n_notes += 1;
				} else { /* s->next->u.note.type != GRACE */
					grace = NULL;
				}
			}
			beam->n_notes += 1;
			s = s->next;
			i += 1;
		}
		if (grace) {
			beam->n_notes -= grace->n_notes;
			grace = NULL;
		}
		s = s->next;
	}
}

static	void	set_semitones(int s_f)
{
	int	i,no;

	for (i = 0; i < 7; ++i)
		semitones[i] = Csemitones[i];

	if (s_f > 0) {
		no = 3;
		for (i = 0; i < s_f; ++i) {
			semitones[no] += 1;
			no = (no+4)%7;
		}
	} else if (s_f < 0) {
		no = 6;
		for (i = 0; i > s_f; --i) {
			semitones[no] -= 1;
			no = (no+3)%7;
		}
	}
}

static	int	distance_from_C(Note *n)
{
	int	note, octave;

	if (n->pitch == -1) return(REST);

	note = (n->pitch - 2)%7;
	octave = ((n->pitch - 2)/7) - 2;

	if (n->iaccidental) {
		if (semitones[note] == Csemitones[note])
			semitones[note] += n->iaccidental - 3;
		else if (semitones[note] == Csemitones[note] + 1)
			semitones[note] += n->iaccidental - 4;
		else if (semitones[note] == Csemitones[note] - 1)
			semitones[note] += n->iaccidental - 2;
		else
			g_error("semitones wrong");
	}

	return(semitones[note] + 12*octave);
}

static	void	tune2hash(Field *key, int *hash_array, int force)
{
	int	h = 0;
	int	note1,note2,first_note;
	int	plet = 0;
	Symbol	*s;
	int	j;

	set_semitones(key->info1);

	s = &block[0][0];
	if (!force) { /* force makes the conversion from all symbols */
		      /* - used for search strings */
		for (; s && s->u.bar.bar_no == 0; s = s->next)
			while (s->next && s->next->type != BAR_LINE)
				s = s->next;
	}

	while (s && (s->type != NOTE || s->u.note.type != NORMAL)) s = s->next;
	if (s) {
		if (strchr(s->u.note.start,'p')) plet = 1;
		note1 = distance_from_C(&s->u.note);
		first_note = note1;
		for (j = 1; j < s->u.note.length; ++j)
			hash_array[h++] = 0;
		s = s->next;
	}

	while (s) {
		while (s && (s->type != NOTE || s->u.note.type != NORMAL)) {
			if (s->type == BAR_LINE)
				set_semitones(key->info1);
			s = s->next;
		}
		if (s) {
			note2 = distance_from_C(&s->u.note);
			if (strchr(s->u.note.start,'p')) plet = 2;
			if (note2 == REST)
				note2 = note1;
/* plet doesn't work for other than triplets or quadruplets */
			if (plet == 1) /* second note of a tuplet */
				plet = 0; /* ignore it */
			else { /* print out the internote distances */
				hash_array[h++] = note2 - first_note;
				/* relative notes
				hash_array[h++] = note2 - note1; */
				for (j = 1; j < s->u.note.length; ++j)
					hash_array[h++] = note2 - first_note;
					/* relative notes
					hash_array[h++] = 0; */
				if (plet == 2) /* first note of a tuplet */
					plet = 1; /* reset plet */
				note1 = note2;
			}
			s = s->next;
		}
	}
	if (h == 0) hash_array[h++] = 999;
	hash_array[h++] = LAST;
}

void	process_abc(char title[][99], int titles, Record *entry,
		char *key_comment, char *bars, char *tune, char *input,
		int output_type, int nbars, int force, int *hash_array)
{
	int	c;
	int	i;
	char	*line;
	char	str[99];
	char	key[99];
	Field	key_field;
	char	meter[99];
	Field	meter_field;
	char	tempo[99];
	Field	tempo_field;
	Symbol	*s;

	output.type = output_type;
	if (offset || transpose)
		output.transpose = 1;
	else
		output.transpose = 0;
	output.nbars = nbars;
	if (output_type == TEX_OUTPUT || output_type == INDEX_OUTPUT)
		output.warnings = 1;
	else
		output.warnings = 0;

	/* initialisation */
	ignore = 0;
	n_symbols = -1;
	n_blocks = -1;
	current = NULL;
	note = NULL;
	beam_length = 0;
	new_symbol(BAR_LINE);
	if (!input) {
		ALLOC(line,char,999);
	}
	bar_start = &current->u.bar;
	token = NEWLINE_TKN;
	stave = bass = treble = 0;
	current_bar[0] = bar[0][0] = bar[1][0] = bar[2][0] = '\0';

	key_field.string = &key[0];
	key_field.info1 = key_field.info2 = 0;
	(void) strcpy(key,"K:");
	(void) strcat(key,entry->KEY);
	sharps_flats(&key_field);
	if (offset || transpose) (void) fprintf(Trans,"%s",key_comment);

	meter_field.string = &meter[0];
	meter_field.info1 = meter_field.info2 = 0;
	(void) strcpy(meter,"M:");
	(void) strcat(meter,entry->METER);
	set_base(&meter_field);
	if (entry->LENGTH[0]) set_dnl(entry->LENGTH);

	tempo_field.string = &tempo[0];
	tempo_field.info1 = tempo_field.info2 = 0;
	if (entry->TEMPO && entry->TEMPO[0]) {
		(void) strcpy(tempo,"Q:");
		(void) strcat(tempo,entry->TEMPO);
		tempo_field.info1 = dnl;
	}

	while (input || (getsIn(line)) != NULL) {

	fbar_total.n = 0;
	fbar_total.d = 1;
	bar_no = 0;

	if (input) line = input;

	if (tune[0]) (void) strcat(tune,line);

	for (i = 0; line[i] == ' ' || line[i] == '\t'; ++i);
	if (line[i] == '\n' ) {
		token_ptr = line;
		token_length = i+1;
		output_transpose();
		nblanks = 1;
		ignore = 0;
		break;
	}

	if (output.type == INDEX_OUTPUT && output.nbars == 0) continue;

	if (line[0] == '\\' || is_field(line)) {
		/* this is a field or raw tex */
		token_ptr = line;
		token_length = strlen(line);
		process_field(line);
		if (output.nbars) {
			     if (line[0] == 'L')
				(void) strcpy(entry->LENGTH,&line[2]);
			else if (line[0] == 'M')
				(void) strcpy(entry->METER,&line[2]);
		}
		continue;
	}

	continuation = 0;

	for (c = 0; line[c]; c += token_length) {

	if (output.nbars == 0 && nbars) break;

	token_ptr = &line[c];
	token_length = 1;

	switch(line[c]) {
	case 'C': case 'D': case 'E': case 'F': case 'G':
		process_note(line[c]-'C'+16+offset);
		break;
	case 'A': case 'B':
		process_note(line[c]-'A'+21+offset);
		break;
	case 'c': case 'd': case 'e': case 'f': case 'g':
		process_note(line[c]-'c'+23+offset);
		break;
	case 'a': case 'b':
		process_note(line[c]-'a'+28+offset);
		break;
	case 'z':
		process_note(-1);
		break;
	case ' ': case '\t':
		while (line[c+token_length] == ' '
		    || line[c+token_length] == '\t')
			++token_length;
		if (line[c+token_length] != '%'
		 && line[c+token_length] != '\n') {
			process_space();
			break;
		}
		/*FALLTHROUGH*/
	case '%':
		while (line[c+token_length] && line[c+token_length] != '\n')
			++token_length;
		if (just_comment && line[c+token_length] == '\n')
			++token_length;
		process_trailing();
		break;
	case '|':
		if (line[c+1] == '|') {
			token_length += 1;
			process_bar(DBL_BAR);
		} else if (line[c+1] == ']') {
			token_length += 1;
			process_bar(RDBL_BAR);
		} else if (line[c+1] == ':') {
			token_length += 1;
			process_bar(LREPEAT);
		} else if (line[c+1] == '1') {
			token_length += 1;
			process_bar(BAR1);
		} else {
			process_bar(BAR);
		}
		break;
	case ':':
		if (line[c+1] == '|') {
			token_length += 1;
			if (line[c+2] == '2') {
				token_length += 1;
				process_bar(RREPEAT2);
			} else {
				process_bar(RREPEAT);
			}
		} else if (line[c+1] == ':') {
			token_length += 1;
			process_bar(REPEAT);
		} else
			abc_error("");
		break;
	case '[':
		if (line[c+1] == '1' || line[c+1] == '2') {
			token_length += 1;
			process_repeat(atoi(&line[c+1]));
		} else if (line[c+1] == '|') {
			token_length += 1;
			process_bar(LDBL_BAR);
		} else
			process_open_chord();
		break;
	case ']':
		process_close_chord();
		break;
	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8':
		while (strchr("0123456789",line[c+token_length]))
			token_length += 1;
		process_length(atoi(&line[c]));
		break;
	case '/':
		if (strchr("0123456789",line[c+1])) {
			while (strchr("0123456789",line[c+token_length]))
				token_length += 1;
			process_divisor(atoi(&line[c+1]));
		} else if (line[c+1] == '/') {
			token_length += 1;
			process_divisor(4);
		} else
			process_divisor(2);
		break;
	case '>':
		while (line[c+token_length] == '>')
			token_length += 1;
		process_broken(token_length);
		break;
	case '<':
		while (line[c+token_length] == '<')
			token_length += 1;
		process_broken(-1*token_length);
		break;
/* obsolete */
	case '+':
		process_open_close_chord();
		break;
	case 's':
		process_open_close_slur();
		break;
/* obsolete */
	case '*':
		process_justify();
		break;
	case '{':
		process_open_grace();
		break;
	case '}':
		process_close_grace();
		break;
	case '(':
		if (strchr("123456789",line[c+1])) {
			while (strchr(":0123456789",line[c+token_length]))
				token_length += 1;
			process_tuplet(&line[c+1]);
		} else process_open_slur();
		break;
	case ')':
		process_close_slur();
		break;
	case '-':
		process_tie();
		break;
	case '~': case '.': case 'u': case 'v':
		process_accent(line[c]);
		break;
	case '^':
		if (line[c+1] == '^') {
			token_length += 1;
			process_accidental(DBL_SHARP);
		} else process_accidental(SHARP);
		break;
	case '_':
		if (line[c+1] == '_') {
			token_length += 1;
			process_accidental(DBL_FLAT);
		} else process_accidental(FLAT);
		break;
	case '=':
		process_accidental(NATURAL);
		break;
	case '\'':
		process_octaver(7);
		break;
	case ',':
		process_octaver(-7);
		break;
	case '"':
		for (; line[c+token_length] && line[c+token_length] != '"';
				++token_length)
			str[token_length-1] = line[c+token_length];
		if (line[c+token_length] == '\0')
			abc_error("line ended mid gchord");
		token_length += 1;
		str[token_length-2] = '\0';
		process_gchord(str);
		break;
	case '\\':
		process_continuation();
		break;
	case '\n':
		process_newline();
		break;
	case '&':
		if (line[c+1] == '&') token_length += 1;
		process_ampersand(token_length);
		break;
	default:
		if (strchr("HIJKLMNOPQRSTUVWXYZ",line[c]))
			process_macro(line[c]);
		else
			abc_error("unrecognised symbol");
		break;
	}
	}

	if (just_comment == 0) {
		if (output.nbars && current_bar[0] && bar_no < 3)
			(void) strcpy(bar[bar_no],current_bar);
		output.nbars = 0;
	}

	if (input) break;

	}

	end_beam();

	current->newline = 2;
	if (settings.justification) current->justify = 1;

	n_symbols += 1;

	process_beams(n_symbols,&block[0][0]);

	if (output.type == TEX_OUTPUT)
		tune2tex(title,titles,entry,n_symbols,&block[0][0],
			&key_field,&meter_field,&tempo_field);
	else if (hash_array)
		tune2hash(&key_field,hash_array,force);
	else if (nbars) {
		bars[0] = '\0';
		if (nbars == ONE_BAR_PLUS || nbars == TWO_BARS_PLUS)
			(void) strcpy(bars,bar[0]);
		(void) strcat(bars,bar[1]);
		if (nbars == TWO_BARS || nbars == TWO_BARS_PLUS)
			(void) strcat(bars,bar[2]);
	}

	for (s = &block[0][0]; s; s = s->next) {
		if (s->type == NOTE && s->u.note.gchord)
			free(s->u.note.gchord);
		else if (s->type == FIELD)
			free(s->u.field.string);
	}
	for (i = 0; i <= n_blocks; ++i)
		free(block[i]);
	if (!input) free(line);
}

#endif

