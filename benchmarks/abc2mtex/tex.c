/************************************************************************
*									*
*			tex.c - V1.6.1 (Jan 97)				*
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
#include "index.h"

#ifndef atof
extern	double	atof(const char*);
#endif

/*extern	void	set_dnl(char*);*/

extern	Setting	settings;
extern	int	bass, treble;
/*extern	int	dnl;*/

FILE	*Out;

enum	scan_types { SCAN_METER, SCAN_ALL };

static	int	in_tune = 0;
static	int	in_bar = 0;
static	int	bar_no, change_context, change_signature;
static	int	in_notes, hp, new_tune, *old_beam = NULL;
static	int	tempo_length = 0, bpm = 0;
static	int	stave;
static	int	slur_level = 0;
static	int	musix;

static	const	char	*mtex ={"ABCDEFGHIJKLMNabcdefghijklmnopqrstuvwxyz"};
static	const	char	*Accidental[] = { "", "<", "_", "=", "^", ">" };

static	int	abclog2(int x)
{
	int	log = 0;
	while (x > 1) {
		x /= 2;
		log += 1;
	}
	return(log);
}

void	open_TeX(char *s, int musix_out)
{
	if (s[0] == '\0') Out = fopen("music.tex","w");
	else Out = fopen(s,"w");

	musix = musix_out;

	if (musix) {
		(void) fprintf(Out,"\\def\\abcmusix{Y}\n");
		if (musix == 1) (void) fprintf(Out,"\\def\\abcopus{N}\n");
		if (musix == 2) (void) fprintf(Out,"\\def\\abcopus{Y}\n");
	} else (void) fprintf(Out,"\\def\\abcmusix{N}\n");
	if (settings.mine)
		(void) fprintf(Out,"\\input dscgrphy\n");
	(void) fprintf(Out,"\\input header\n%%\n");
	if (musix == 1) (void) fprintf(Out,"\\startmuflex%%\n");
}

static	void	open_grace(void)
{
	(void) fprintf(Out,"\\grace");
	in_notes = 1;
}

static	void	open_music(void)
{
	(void) fprintf(Out,"\\notes");
	in_notes = 1;
}

static	void	close_grace(void)
{
	(void) fprintf(Out,"\\egrace%%\n");
	in_notes = 0;
}

static	void	close_music(void)
{
	(void) fprintf(Out,"\\enotes%%\n");
	in_notes = 0;
}

static	void	open_tune(void)
{
	if (musix) (void) fprintf(Out,"\\startpiece%%\n");
	else (void) fprintf(Out,"\\debutmorceau%%\n");
	change_context = 0;
	change_signature = 0;
	bar_no = 0;
	new_tune = 0;
	stave = 0;
	in_tune = 1;
}

static	char	q_plus(int pitch,int beam)
{
	if (beam > 0)
		if (pitch < 29) return(mtex[30]);
		else return(mtex[pitch+2]);
	else
		if (pitch < 22) return(mtex[30]);
		else return(mtex[pitch+9]);
}

static	char	n_plus(int pitch,int beam)
{
	if (beam > 0)
		if (pitch < 26) return(mtex[27]);
		else return(mtex[pitch+2]);
	else
		if (pitch < 19) return(mtex[27]);
		else return(mtex[pitch+9]);
}

static	char	L_minus(int pitch,int beam)
{
	if (beam > 0)
		if (19 < pitch) return(mtex[11]);
		else return(mtex[pitch-9]);
	else
		if (15 < pitch) return(mtex[11]);
		else return(mtex[pitch-5]);
}

static	void	draw_header(void)
{
	(void) fprintf(Out,"\\tune{\\header%%\n");
}

void	draw_text(char *type,char *string)
{
	char	*ptr;
	if (type == NULL) /* TeX string */
		(void) fprintf(Out,"%s%%\n",string);
	else if ((strcmp(type,"Z")) == 0)
		(void) fprintf(Out,"\\message{%s}%%\n",string);
	else {
		for (ptr = string; (ptr = strpbrk(ptr,"#%&")); ptr += 1)
			if (*(ptr-1) != '\\')
			g_error("unescaped special TeX character %c detected\n\
	this will cause TeX to choke",*ptr);
		if (string[0])
			(void) fprintf(Out,
				"\\def\\%strue{Y}\\def\\%sstring{%s}\n",
				type,type,string);
		else
			(void) fprintf(Out,"\\def\\%strue{N}\n",type);
	}
}

static	void	draw_tempo(Field *tempo)
{
	char	*str = &tempo->string[2];
	int	level,old_tempo_length = tempo_length,length = 1,ctr = 0,j;

	if (new_tune) open_tune();

	if (str[ctr] == 'C') {
		if (str[++ctr] >= '0' && str[ctr] <= '9') {
			length = atoi(&str[ctr++]);
			if (length > 9) ctr += 1;
		}
		tempo_length = length*tempo->info1;
		if (str[ctr] != '=') g_error("in Q field");
		ctr += 1;
	} else if ((strncmp(&str[ctr],"1/",2)) == 0) {
		ctr += 2;
		tempo_length = 32/atoi(&str[ctr++]);
		if (tempo_length <= 2) ctr += 1;
		if (str[ctr] != '=') g_error("in Q field");
		ctr += 1;
	} else
		tempo_length = tempo->info1;
	if (str[ctr] == 'C') {
		if (str[++ctr] >= '0' && str[ctr] <= '9') {
			length = atoi(&str[ctr]);
		}
		bpm = (bpm*old_tempo_length)/(length*tempo->info1);
	} else if (str[ctr] >= '0' && str[ctr] <= '9') {
		bpm = atoi(&str[ctr]);
	} else
		g_error("in Q field");

	old_tempo_length = tempo_length;

	if (in_notes) close_music();
	(void) fprintf(Out,"\\notes\\Uptext{\\metron{\\");
	if (tempo_length%3 == 0) (void) fprintf(Out,"pt1\\");
	level = abclog2(tempo_length);
	if (level == 5)
		(void) fprintf(Out,"wh");
	else if (level == 4)
		(void) fprintf(Out,"hu");
	else if (level == 3)
		(void) fprintf(Out,"qu");
	else {
		for (j = 0; j < 3-level; ++j)
			(void) fprintf(Out,"c");
		(void) fprintf(Out,"u");
	}
	(void) fprintf(Out,"}{%d}}\\enotes%%\n",bpm);
}

static	void	close_open(void)
{
	(void) fprintf(Out,"\\enotes\\notes");
	stave = 0;
}

static	void	next_stave(void)
{
	(void) fprintf(Out,"&");
	stave += 1;
}

static	void	draw_rest(int level)
{
	if (musix) {
		if (level == 5)
			(void) fprintf(Out,"\\wr");
		else if (level == 4)
			(void) fprintf(Out,"\\hr");
		else if (level == 3)
			(void) fprintf(Out,"\\qr");
		else if (level == 2)
			(void) fprintf(Out,"\\er");
		else if (level == 1)
			(void) fprintf(Out,"\\eer");
		else if (level == 0)
			(void) fprintf(Out,"\\eeer");
	} else {
		if (level == 5)
			(void) fprintf(Out,"\\pause");
		else if (level == 4)
			(void) fprintf(Out,"\\hpause");
		else if (level == 3)
			(void) fprintf(Out,"\\qp");
		else if (level == 2)
			(void) fprintf(Out,"\\ds");
		else if (level == 1)
			(void) fprintf(Out,"\\qs");
		else if (level == 0)
			(void) fprintf(Out,"\\hs");
	}
}

static	void	draw_pt(Note note)
{
	if (note.length%15== 0)
		(void) fprintf(Out,"\\pppt %c",mtex[note.pitch]);
	else if (note.length%7 == 0)
		(void) fprintf(Out,"\\ppt %c",mtex[note.pitch]);
	else if (note.length%3 == 0)
		(void) fprintf(Out,"\\pt %c",mtex[note.pitch]);
}

static	void	draw_slur(Note note,char type,char ud, int change)
{
	int	pitch;
	char	mtype[5];

	if (note.pitch == -1) {
/*
		if (ud == 'u') pitch = 16;
		else pitch = 24;
*/
		pitch = 20;
	} else
		pitch = note.pitch;
	if (type == 's') (void) strcpy(mtype,"slur");
	else if (type == 't') (void) strcpy(mtype,"tie");
	if (strchr(note.attributes,'.') && type == 's') {
		if (ud == 'u') pitch += 2;
		else pitch -= 2;
	}
	if (change > 0) {
		(void) fprintf(Out,"\\i%s%c%d{%c}",mtype,ud,slur_level,
			mtex[pitch]);
	} else if (change < 0) {
		(void) fprintf(Out,"\\t%s%d",mtype,slur_level - 1);
		if (type == 's')
			(void) fprintf(Out,"{%c}",mtex[pitch]);
	}
	slur_level += change;
}

static	void	draw_attributes(Note note,char ul,char lu,char ud,int beam)
{
	int	g = 0,i;

	if (note.gchord) {
		(void) fprintf(Out,"\\zcharnote ");
		if (settings.gchords_above) {
			if (note.pitch == -1)
				(void) fprintf(Out,"q");
			else
				(void) fprintf(Out,"%c",
					q_plus(note.pitch,beam));
		} else {
			if (note.pitch == -1)
				(void) fprintf(Out,"L");
			else
				(void) fprintf(Out,"%c",
					L_minus(note.pitch,beam));
		}
		(void) fprintf(Out,"{\\gfont ");
		if (note.gchord[0] == '(') {
			g = 1;
			(void) fprintf(Out,"(");
		}
		(void) fprintf(Out,"%c",note.gchord[g]);
		if (note.gchord[g+1] == '#')
			(void) fprintf(Out,"\\Zsh %s}",&note.gchord[g+2]);
		else if (note.gchord[g+1] == 'b')
			(void) fprintf(Out,"\\Zfl %s}",&note.gchord[g+2]);
		else
			(void) fprintf(Out,"%s}",&note.gchord[g+1]);
	}

	for (i = 0; note.end[i]; ++i)
		switch (note.end[i]) {
		case 's': case 't':
			if (musix) draw_slur(note,note.end[i],ud,-1);
			break;
		case 'b':
			break;
		default:
			g_error("end unrecognised");
			break;
		}

	for (i = 0; note.start[i]; ++i)
		switch (note.start[i]) {
		case 's': case 't':
			if (musix) draw_slur(note,note.start[i],ud,1);
			break;
		case 'p':
			/* tuplet */
			break;
		case 'u':
			/* unison */
			break;
		default:
			g_error("start unrecognised");
			break;
		}

	for (i = 0; note.attributes[i]; ++i) {
		switch(note.attributes[i]) {
		case 'u': case 'v':
			(void) fprintf(Out,"\\%cbow %c",note.attributes[i],
				n_plus(note.pitch,beam));
			break;
		case '~':
			(void) fprintf(Out,"\\%croll %c",ul,mtex[note.pitch]);
			break;
		case '.':
			(void) fprintf(Out,"\\%cpz %c",lu,mtex[note.pitch]);
			break;
		default:
			(void) fprintf(Out,"\\user%c%c{%c}",
				note.attributes[i],ul,mtex[note.pitch]);
			break;
		}
	}

}

static	void	draw_usercmd(char *s)
{
	int	i;

	if (new_tune) open_tune();

	if (in_notes) close_music();
	for (i = 0; s[i]; ++i) {
		(void) fprintf(Out,"\\user%c",s[i]);
	}
}

static	void	draw_chord(Symbol *root)
{
	int	j,level;
	Symbol	*s = root;
	Note	*n;
	char	l;
	for (j = 1; j <= root->u.note.chord; ++j) {
		s = s->next;
		n = &s->u.note;
		level = abclog2(n->length);
		     if (level == 5) l = 'w';
		else if (level == 4) l = 'h';
		else		     l = 'q';
		(void) fprintf(Out,"\\z%c{%s%c}",l,Accidental[n->iaccidental],
			mtex[n->pitch]);
	}
}

static	void	draw_tie(Note note,char *str)
{
	int	p_m;
	char	u_l;

	if (musix) return;

	if (old_beam[stave] <= 0)	{ u_l = 'u'; p_m = -1;}
	else		{ u_l = 'l'; p_m =  1;}
	if (strchr("fhjl",mtex[note.pitch]))
		p_m *= 2;

	(void) fprintf(Out,"\\%ctie%s{%c}",u_l,str,mtex[note.pitch+p_m]);
}

static	void	draw_part(char *part)
{
	if (new_tune) open_tune();
	(void) fprintf(Out,"\\Pline{%s}%%\n",part);
}

static	void	draw_tex(char *line)
{
/* probably shouldn't do this */
if (in_notes) close_music();
	(void) fprintf(Out,"%s%%\n",line);
}

static	void	draw_size(char *size)
{
	double	esize;
	if (settings.mine && musix) esize = 7.0;
	else esize = 8.5;
	if (size[0] && '0' <= size[0] && size[0] <= '9')
		/* ignore non numerical */
		esize = atof(size);
	if (musix == 0 && new_tune) (void) fprintf(Out,"\\normal");
	if (musix == 0 || (size[0] && '0' <= size[0] && size[0] <= '9')
	 || settings.mine) /* don't print anything for MusiXTeX unless asked */
		(void) fprintf(Out,"\\elemskip=%.1fpt%%\n",esize);
}

void	close_TeX(void)
{
	if (musix == 1) (void) fprintf(Out,"\\endmuflex%%\n");
	(void) fprintf(Out,"\\end%%\n");
	//(void) fclose(Out);
}

static	void	draw_old_repeat(int repeat)
{
	(void) fprintf(Out,"\\rpt{%d}",repeat);
}

static	void	draw_meter_new(Field *meter)
{
	if (!in_notes) {
		(void) fprintf(Out,"\\generalmeter{");
		if (!new_tune) change_context = 1;
	}

	if (meter->info1 == -4 && meter->info2 == 4)
		(void) fprintf(Out,"\\meterC");
	else if (meter->info1 == -2 && meter->info2 == 4)
		(void) fprintf(Out,"\\allabreve");
	else
		(void) fprintf(Out,"\\meterfrac{%d}{%d}",
			meter->info1,meter->info2);

	if (!in_notes) (void) fprintf(Out,"}");
	(void) fprintf(Out,"%%\n");
}

static	void	key2tex(Field *f)
{
	if (f->info2 == 2) /* key is HP */
		(void) fprintf(Out,"\\generalsignature{0}%%\n");
	else
		(void) fprintf(Out,"\\generalsignature{%d}%%\n",f->info1);

	if (old_beam) free(old_beam);
	ALLOC(old_beam,int,bass+treble);

	if (new_tune) {
		if (f->info2 == 1) /* key is Hp */
			(void) fprintf(Out,"\\beginHp\n");
		hp = f->info2;
	} else {
		if (in_bar) (void) fprintf(Out,"\\changesignature%%\n");
		else change_signature = 1;
	}

}

static	void	staves(void)
{
	int	i,j;

	if (bass + treble > 1)
		if (musix)
			(void) fprintf(Out,"\\instrumentnumber%d\n",
				bass+treble);
		else
			(void) fprintf(Out,"\\def\\nbinstruments{%d}\n",
				bass+treble);
	for (i = 0; i < bass; ++i)
		if (musix)
			(void) fprintf(Out,"\\setclef%d\\bass\n",i+1);
		else {
			(void) fprintf(Out,"\\cleftoks");
			for (j = 0; j <= i; ++j)
				(void) fprintf(Out,"i");
			(void) fprintf(Out,"{6}\n");
		}
	if (bass + treble > 1)
		for (i = 0; i < treble; ++i)
			if (musix)
				(void) fprintf(Out,"\\setclef%d\\treble\n",
					i+1+bass);
			else {
				(void) fprintf(Out,"\\cleftoks");
				for (j = 0; j <= i+bass; ++j)
					(void) fprintf(Out,"i");
				(void) fprintf(Out,"{0}\n");
			}

}

static	void	scan_fields(Symbol *s, int scan)
{
	Symbol	*f;

	/* look ahead for changing meters and keys */
	for (f = s->next; f && f->type == FIELD; f = f->next) {
		       if (f->u.field.string[0] == 'M') {
			draw_meter_new(&f->u.field);
			/* nullify it so it doesn't get used again */
			f->u.field.string[0] = '\0';
		} else if (f->u.field.string[0] == 'K') {
			if (scan == SCAN_ALL) {
				key2tex(&f->u.field);
				/* nullify it so it doesn't get used again */
				f->u.field.string[0] = '\0';
			} else
				change_signature = 1;
		}
	}
}

static	void	beam2tex(int n, Symbol *first, int beam)
{
	int	i,beam_length,l,level,prev_level,next_level,j;
	int	plet,k,se,position,orig_level,beamed = 1,igrace;
	Symbol	*s,*t,*grace,*next,*last;
	int	start_pitch,end_pitch;
	char	ul,lu,bh,ud;
	float	slope;
	int	median;

	if (new_tune) open_tune();

	if (!in_notes) {
		if (first->u.note.type == NORMAL || first->u.note.type == CHORD)
			open_music();
		else if (first->u.note.type == GRACE)
			open_grace();
	}

	if (first->u.note.pitch == -1) {
		draw_attributes(first->u.note,'u','l','d',0);
		if (first->u.note.tuplet) {
			(void) fprintf(Out,"\\zcharnote{b}{\\sevenrm %d}",
				first->u.note.tuplet);
		}
		level = abclog2(first->u.note.length);
		draw_rest(level);
		if (first->u.note.length%15 == 0) {
			draw_rest(level-1);
			draw_rest(level-2);
			draw_rest(level-3);
		} else if (first->u.note.length%7 == 0) {
			draw_rest(level-1);
			draw_rest(level-2);
		} else if (first->u.note.length%3 == 0) {
			draw_rest(level-1);
		}
		return;
	}

	if (first->u.note.type != GRACE) {
		if (hp)
			beam = 1;
		else {
			s = first;
			if (stave < bass) median = 12;
			else median = 22;
			for (i = 0; i < n; ++i) {
				beam += s->u.note.pitch - median;
				s = s->next;
			}
		}
	}

	if (beam == 0) beam = old_beam[stave];
	old_beam[stave] = beam;

	if (beam > 0) {
		ul = 'l';
		lu = 'u';
		bh = 'b';
		ud = 'u';
	} else {
		ul = 'u';
		lu = 'l';
		if (musix) bh = 'b';
		else bh = 'h';
		ud = 'd';
	}

	if (first->u.note.iaccidental) (void) fprintf(Out,"\\qsk");

	s = first;
	for (i = 0; i < n-1; ++i)
		s = s->next;
	last = s;

	start_pitch = first->u.note.pitch;
	for (s = last; s->u.note.type == CHORD; s = s->prev);
	end_pitch = s->u.note.pitch;

	if (n == 1+first->u.note.chord) beamed = 0;

	if (beamed) {

	/* work out beam length */
	beam_length = n-1;

	s = first;
	for (i = 0; i < n-1; ++i) {
		if (s->next->u.note.iaccidental)
			beam_length += 1;
		if (s->u.note.type == NORMAL) {
			if (s->u.note.length%3 == 0 || s->u.note.length%7 == 0)
				beam_length += 1;
		} else if (s->u.note.type == CHORD)
			beam_length -= 1;
		else if (s->u.note.type == GRACE) {
			if (first->u.note.type != GRACE) {
				if (s->prev->u.note.type == GRACE)
					beam_length -= 1;
			}
		}
		s = s->next;
	}
	if (s != last) g_error("not last");
	if (s->u.note.type == CHORD)
		beam_length -= 1;

	se = 1;
retry:
	if (beam_length == 0) g_error("beam length is 0");
	slope = (float) (end_pitch - start_pitch)/(float) beam_length;
	position = 0;
	orig_level = abclog2(first->u.note.length);
	s = first;
	for (i = 1; i < n; ++i) {
		s = s->next;
		if (s->u.note.iaccidental)
			position += 1;
		if (s->u.note.type == NORMAL) {
			position += 1;
			if (s->prev->u.note.length%3 == 0
			 || s->prev->u.note.length%7 == 0)
				position += 1;
		} else if (s->u.note.type == GRACE) {
			if (first->u.note.type != GRACE) {
				if (s->prev->u.note.type != GRACE) {
					position += 1;
					if (s->prev->u.note.length%3 == 0
					 || s->prev->u.note.length%7 == 0)
						position += 1;
				}
			} else
				position += 1;
		}
		level = abclog2(s->u.note.length);
		if (level >= orig_level && s->u.note.type != GRACE)
			level = 2;
		else if (level < orig_level && s->u.note.type != GRACE)
			level -= 1;
		if (s->u.note.type == GRACE && first->u.note.type == GRACE)
			level = 0;
		else if (s->u.note.type == GRACE && first->u.note.type != GRACE)
			level = 2;
		if (ul == 'u' && (float) s->u.note.pitch >
			(float) start_pitch + 1.0 + level + position*slope) {
			if (se%2) start_pitch += 1;
			else	    end_pitch += 1;
			se += 1;
			goto retry;
		} else if (ul == 'l' && (float) s->u.note.pitch <
			(float) start_pitch - 1.0 - level + position*slope) {
			if (se%2) start_pitch -= 1;
			else	    end_pitch -= 1;
			se += 1;
			goto retry;
		}
	}
	if (position != beam_length)
		g_error("beam lengths are different");

	(void) fprintf(Out,"\\I");
	level = 3 - abclog2(first->u.note.length);
	for (j = 0; j < level; ++j)
		(void) fprintf(Out,"b");
	if (beam_length < 10)
		(void) fprintf(Out,"%c%d%c%c%d",ul,first->u.note.type,
			mtex[start_pitch],mtex[end_pitch],beam_length);
	else
		(void) fprintf(Out,"%c%d%c%c{%d}",ul,first->u.note.type,
			mtex[start_pitch],mtex[end_pitch],beam_length);
	}

	/* plet calculates the vertical position of a tuplet number */
	plet = start_pitch-11+(end_pitch-start_pitch)/2;
	if (beam <= 0) {
		plet += 19;
		if (beamed && first->u.note.length <= 2) plet += 2;
	} else {
		if (beamed && first->u.note.length <= 2) plet -= 1;
	}

	/* draw the notes */
	s = first;
	for (i = 0; i < n; ++i) {

		if (i > 0 && s->u.note.type == GRACE
			&& s->prev->u.note.type != GRACE) {
			/* start of grace notes */

			close_music();

			grace = s;
			igrace = i;
			while (s->u.note.type == GRACE) {
				s = s->next;
				i += 1;
			}
			if (beam <= 0)
				beam2tex(i-igrace,grace,1);
			else
				beam2tex(i-igrace,grace,-1);

			open_music();

			if (i == n)
				g_error("grace notes at end of beam");
		}

		if (s->u.note.tuplet) {
			(void) fprintf(Out,"\\zcharnote{%c}{",mtex[plet]);
			k = 0;
			if (s->u.note.tuplet%2 == 0) {
				if (ul == 'l') k = -3;
				else k = -1;
			} else
				if (ul == 'u') k = 1;
			if (k) (void) fprintf(Out,"\\kern %d\\Internote",k);
			(void) fprintf(Out,"\\sevenrm %d}",s->u.note.tuplet);
		}

		t = s;
		for (j = 0; j <= s->u.note.chord; ++j) {
			draw_attributes(t->u.note,ul,lu,ud,beam);
			t = t->next;
		}

		t = s;
		for (j = 0; j <= s->u.note.chord; ++j) {
			draw_pt(t->u.note);
			t = t->next;
		}

		if (beamed) {

		if (i == 0)
			prev_level = 0;
		else {
			for (t = s->prev; t->u.note.type != s->u.note.type;)
				t = t->prev;
			prev_level = 3 - abclog2(t->u.note.length);
		}
		level = 3 - abclog2(s->u.note.length);
		if (i == n-1-s->u.note.chord)
			next_level = 0;
		else {
			for (t = s->next; t->u.note.type != s->u.note.type;)
				t = t->next;
			next_level = 3 - abclog2(t->u.note.length);
		}

		if (level > prev_level && i != 0 && level <= next_level) {
			for (l = prev_level; l < level; ++l) {
				(void) fprintf(Out,"\\n");
				for (j = 0; j <= l; ++j)
					(void) fprintf(Out,"b");
				(void) fprintf(Out,"%c%d",ul,s->u.note.type);
			}
		}

		if (next_level < level) {
			if (musix == 0 && level - prev_level > 1) {
				if (i == n-1-s->u.note.chord)
				  (void) fprintf(Out,"\\off{-1\\qn@width}");
				if (i != 0) {
					(void) fprintf(Out,"\\n");
					for (j = 0; j < level; ++j)
						(void) fprintf(Out,"b");
					(void) fprintf(Out,"%c%d",ul,
						s->u.note.type);
				}
				if (i == n-1-s->u.note.chord)
					(void) fprintf(Out,"\\qsk");
			}
			for (l = level-1; l > next_level-1; --l) {
				if ((i == 0 || (level - prev_level > 1
					&& i != n-1-s->u.note.chord)) && l > 0)
					(void) fprintf(Out,"\\rlap{\\qsk");
				(void) fprintf(Out,"\\t");
				for (j = 0; j <= l; ++j)
					(void) fprintf(Out,"b");
				(void) fprintf(Out,"%c%d",ul,s->u.note.type);
				if ((i == 0 || (level - prev_level > 1
					&& i != n-1-s->u.note.chord)) && l > 0)
					(void) fprintf(Out,"}");
			}
			prev_level = level;
		}

		}

		if (s->u.note.chord) {
			if (strchr(s->u.note.start,'u')) { /* unison */
				level = abclog2(s->u.note.length);
				       if (level == 5) {
					(void) fprintf(Out,"\\zwh ");
				} else if (level == 4) {
					(void) fprintf(Out,"\\zh%c ",lu);
				} else if (level == 3) {
					(void) fprintf(Out,"\\zq%c ",lu);
				} else {
					(void) fprintf(Out,"\\z");
					for (j = 0; j < 3-level; ++j)
						(void) fprintf(Out,"c");
					(void) fprintf(Out,"%c ",lu);
				}
/*
				if (s->u.note.iaccidental)
					(void) fprintf(Out,"{%s%c}",
					Accidental[s->u.note.iaccidental],
						mtex[s->u.note.pitch]);
				else
*/
					(void) fprintf(Out,"%c",
						mtex[s->u.note.pitch]);
			} else
				draw_chord(s);
		}

		if (beamed)
			(void) fprintf(Out,"\\q%c%d",bh,s->u.note.type);
		else {
			level = abclog2(first->u.note.length);
			if (level == 5) {
				(void) fprintf(Out,"\\wh ");
			} else if (level == 4) {
				(void) fprintf(Out,"\\h%c ",ul);
			} else if (level == 3) {
				(void) fprintf(Out,"\\q%c ",ul);
			} else {
				(void) fprintf(Out,"\\");
				for (j = 0; j < 3-level; ++j)
					(void) fprintf(Out,"c");
				(void) fprintf(Out,"%c ",ul);
			}
		}
		if (s->u.note.iaccidental)
			(void) fprintf(Out,"{%s%c}",
				Accidental[s->u.note.iaccidental],
				mtex[s->u.note.pitch]);
		else
			(void) fprintf(Out,"%c",mtex[s->u.note.pitch]);

		next = s->next;
		while (next && next->type == NOTE && next->u.note.type == CHORD)
			next = next->next;
		if (musix == 0 && first->u.note.type != GRACE &&
		   ((beamed && next && next->prev != last
		 && next->u.note.iaccidental)
		 || (beamed == 0 && ul == 'u' && level < 3)
		 || s->u.note.length%3 == 0 || s->u.note.length%7 == 0))
				(void) fprintf(Out,"\\qsk");

		i += s->u.note.chord;
		s = next;
	}

	if (s && s->prev != last) g_error("");

	if (last->u.note.type == GRACE) close_grace();
	if (beamed) (void) fprintf(Out,"%%\n");
	if (strchr(last->u.note.start,'t')) {
		if (s && s->type == NOTE) draw_tie(last->u.note,"in");
		else draw_tie(last->u.note,"");
	}

	if (last->newline == 2) {
		if (in_notes) close_music();
		if (musix)
			(void) fprintf(Out,"\\zstoppiece%%\n");
		else {
			if (!last->justify)
				(void) fprintf(Out,"\\hfil");
			(void) fprintf(Out,"\\zsuspmorceau%%\n");
		}
		in_tune = 0;
	} else if (last->newline == 1) {
		if (in_notes) close_music();
		scan_fields(last,SCAN_ALL);

		if (!musix && !last->justify)
			(void) fprintf(Out,"\\hfil");
		(void) fprintf(Out,"\\zalaligne%%\n");
	}

}

static	void	bar2tex(Symbol *s)
{
	if (new_tune) open_tune();

	if (in_notes) close_music();

	in_bar = 0;

	switch (s->u.bar.type) {
	case BAR1:
		if (settings.old_repeats == 0) {
			if (musix)
				(void) fprintf(Out,"\\setvolta1");
			else if (s->newline != 1)
				(void) fprintf(Out,"\\setprimavolta");
		}
		break;
	case  DBL_BAR:
		(void) fprintf(Out,"\\setdoublebar%%\n");
		break;
	case LDBL_BAR:
		(void) fprintf(Out,"\\setdoubleRAB%%\n");
		break;
	case RDBL_BAR:
		(void) fprintf(Out,"\\setdoubleBAR%%\n");
		break;
	case  REPEAT:
		(void) fprintf(Out,"\\setleftrightrepeat%%\n");
		break;
	case LREPEAT:
		(void) fprintf(Out,"\\setleftrepeat%%\n");
		break;
	case RREPEAT2:
		if (settings.old_repeats == 0) {
			if (musix)
				(void) fprintf(Out,"\\setvolta2");
			else if (s->newline != 1)
				(void) fprintf(Out,"\\setsecondavolta");
		}
		/*FALLTHROUGH*/
	case RREPEAT:
		(void) fprintf(Out,"\\setrightrepeat%%\n");
		break;
	}
	if (s->newline == 2) {
		if (musix)
			(void) fprintf(Out,"\\stoppiece%% bar no %d\n",
				++bar_no);
		else if (s->justify)
			(void) fprintf(Out,"\\suspmorceau%% bar no %d\n",
				++bar_no);
		else {
			(void) fprintf(Out,"\\barre%% no %d\n",++bar_no);
			(void) fprintf(Out,"\\hfil\\zsuspmorceau%%\n");
		}
		in_tune = 0;
	} else if (s->newline == 1) {
		if (!musix && !s->justify)
			(void) fprintf(Out,"\\barre%% no %d\n",++bar_no);

		/* should do with SCAN_ALL, but causes problems with MusiXTeX */
		scan_fields(s,SCAN_METER);

		if (musix && change_signature)
			(void) fprintf(Out,"\\bar%% bar no %d\n",++bar_no);

		scan_fields(s,SCAN_ALL);

		if (musix && change_signature)
			(void) fprintf(Out,"\\changesignature\\zalaligne%%\n");
		else if (musix || s->justify)
			(void) fprintf(Out,"\\alaligne%% bar no %d\n",++bar_no);
		else {
			(void) fprintf(Out,"\\hfil");
			if (s->u.bar.type == BAR1)
				(void) fprintf(Out,"\\setprimavolta");
			else if (s->u.bar.type == RREPEAT2)
				(void) fprintf(Out,"\\setsecondavolta");
			(void) fprintf(Out,"\\zalaligne%%\n");
		}
	} else {
		scan_fields(s,SCAN_ALL);

		if (change_context || change_signature)
			(void) fprintf(Out,"\\changecontext%% bar");
		else if (musix)
			(void) fprintf(Out,"\\bar%%");
		else
			(void) fprintf(Out,"\\barre%%");
		(void) fprintf(Out," no %d\n",++bar_no);
	}
	if (settings.old_repeats) {
		if (s->u.bar.type == BAR1) {
			open_music();
			draw_old_repeat(1);
		} else if (s->u.bar.type == RREPEAT2) {
			open_music();
			draw_old_repeat(2);
		}
	}

	for (stave = 0; stave < bass+treble; ++stave)
		old_beam[stave] = 0; /* reset old_beam to zero
				- indeterminate upper or lower beams
				  do not depend on beams in previous bars */
	stave = 0;

	change_context = 0; /* context should have been changed now */
	change_signature = 0; /* signature should have been changed now */

	in_bar = 1; /* now in bar */
}

static	void	fields2tex(Field *f)
{
	switch (f->string[0]) {
	case 'E':
		draw_size(&f->string[2]);
		break;
	case 'K':
		key2tex(f);
		break;
	case 'L':
		/*set_dnl(&f->string[2]);*/
		break;
	case 'M':
		draw_meter_new(f);
		break;
	case 'P':
		draw_part(&f->string[2]);
		break;
	case 'T':
	case 'W':
		new_tune = 1;
		(void) fprintf(Out,"\\%cline{%s}%%\n",
				f->string[0],&f->string[2]);
		break;
	case 'Q':
		draw_tempo(f);
		break;
	case 'I':
	case '\0':
		break;
	case '\\':
		draw_tex(f->string);
		break;
	default:
		g_error("");
	}
}

static	void	end_tune(void)
{
	free(old_beam);
	old_beam = NULL;
	if (in_tune) {
		if (musix)
			(void) fprintf(Out,"\\zstoppiece%%\n");
		else
			(void) fprintf(Out,"\\zsuspmorceau%%\n");
		in_tune = 0;
	}
	(void) fprintf(Out,"}%%\n");
	if (hp == 1) /* key is Hp */
		(void) fprintf(Out,"\\endHp%%\n");
	(void) fprintf(Out,"\n");
	(void) fflush(Out);
}

void	tune2tex(char title[][99], int titles, Record *entry, int n_symbols,
		Symbol *symbols, Field *key_field, Field *meter_field,
		Field *tempo_field)
{
	int	i,j,n;
	char	other_titles[999];
	int	ttl;

	new_tune = 1;
	hp = 0;

	draw_text("X",entry->NUMBER);
	draw_text("T",title[0]);
	draw_text("S",entry->SOURCE);
	draw_text("C",entry->COMPOSER);
	draw_text("A",entry->AREA);
	draw_text("N",entry->NOTES);
	other_titles[0] = '\0';
	if (titles > 1) {
		for (ttl = 1; ttl < titles; ++ttl) {
			(void) strcat(other_titles,title[ttl]);
			if (ttl < titles-1)
			(void) strcat(other_titles,"; ");
		}
	}
	if (titles < 6) {
		draw_text("Ta",other_titles);
		draw_text("Tb","");
	} else {
		draw_text("Tb","");
		draw_text("Ta",other_titles);
	}
	draw_text("P",entry->PARTS);

	draw_header();
	key2tex(key_field);
	staves();
	draw_meter_new(meter_field);
	/*if (entry->LENGTH[0]) set_dnl(entry->LENGTH);*/
	draw_size(entry->ELEMSKIP);
	if (entry->TEMPO[0]) draw_tempo(tempo_field);

	in_bar = 1;

	symbols = symbols->next;
	for (i = 1; i < n_symbols;) {
		switch (symbols->type) {
		case NOTE:
			n = symbols->u.note.n_notes;
			if (symbols->u.note.pitch == 0)
				draw_usercmd(symbols->u.note.attributes);
			else if (symbols->u.note.type == GRACE) {
				if (in_notes) close_music();
				beam2tex(n,symbols,-1);
			} else
				beam2tex(n,symbols,0);
			for (j = 0; j < n; ++j)
				symbols = symbols->next;
			i += j;
			break;
		case BAR_LINE:
			bar2tex(symbols);
			i += 1;
			symbols = symbols->next;
			break;
		case FIELD:
			fields2tex(&symbols->u.field);
			i += 1;
			symbols = symbols->next;
			break;
		case MISC:
			if (symbols->u.misc.level == 2)
				close_open();
			else if (symbols->u.misc.level == 1)
				next_stave();
			else
				g_error("");
			symbols = symbols->next;
			i += 1;
			break;
		default:
			g_error("");
			break;
		}
	}

	end_tune();
}

