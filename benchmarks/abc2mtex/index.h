#define ALLOC(ptr,type,l) ptr = (type *) calloc((unsigned) l,sizeof(type)); \
			  if (!ptr) g_error("alloc failure in " #type "_array")

#define	CHORD	-1
#define	NORMAL	0
#define	GRACE	1

#define	MAX_FIELDS	12

#define	AREA		fields[ 0]
#define	BOOK		fields[ 1]
#define	COMPOSER	fields[ 2]
#define	DSCGRAPHY	fields[ 3]
#define	ELEMSKIP	fields[ 4]
#define	FILENAME	fields[ 5]
#define	GROUP		fields[ 6]
#define	HISTORY		fields[ 7]
#define	INFO		fields[ 8]
#define	JOINT		fields[ 9]
#define	KEY		fields[10]
#define	LENGTH		fields[11]
#define	METER		fields[12]
#define	NOTES		fields[13]
#define	ORIGIN		fields[14]
#define	PARTS		fields[15]
#define	TEMPO		fields[16]
#define	RHYTHM		fields[17]
#define	SOURCE		fields[18]
#define	TITLE		fields[19]
#define	UFIELD		fields[20]
#define	VFIELD		fields[21]
#define	WORDS		fields[22]
#define	NUMBER		fields[23]
#define	YFIELD		fields[24]
#define	ZNOTE		fields[25]
#define	BARS		bars

#define	REST	-999
#define	LAST	-1999

struct record {
	char	*fields[26];
	char	*bars;
	struct record	*next;
} ;

typedef	struct record	Record;

typedef struct {
	int	n;	/* numerator */
	int	d;	/* denominator */
} frac ;

typedef struct {
	int	length;
	int	type;
	int	pitch;
	char	attributes[9];
	char	*gchord;
	int	chord;
	int	tuplet;
	char	start[9];
	char	end[9];
	int	n_notes;
	int	iaccidental;
	frac	broken;
} Note ;

typedef struct {
	int	gchords_above;
	int	autobeam;
	int	old_slurs;
	int	old_chords;
	int	old_repeats;
	int	justification;
	int	mine;
} Setting ;

typedef struct {
	int	type;
	int	repeat_no;
	int	bar_no;
} Barline;

typedef struct {
	char	*string;
	int	info1,info2;
} Field;

typedef struct {
	int	level;
} Misc;

struct symbol {
	int	type;
	union {
		Note	note;
		Barline	bar;
		Field	field;
		Misc	misc;
	} u;
	int	newline;
	int	justify;	/* MusicTeX only */
	struct symbol	*next;
	struct symbol	*prev;
} ;

typedef	struct symbol	Symbol;

#ifndef atoi
extern	int	atoi(const char*);
#endif

/* functions in index.c */
extern	void	g_error(char*,...);
extern	void	get_index(char*,char*);
extern	void	size_record(char*,int*,char*);
extern	Record	*alloc_record(char*,int*);
extern	void	free_record(struct record*,char*);
extern	int	get_record(char*,FILE*,struct record*);
extern	int	put_record(char*,FILE*,struct record*);
extern	char	lower(char);

/* functions in abc.c */
extern	FILE	*openIn(char*);
extern	char	*getsIn(char*);
extern	void	closeIn(void);
extern	void	read_settings(void);
extern	int	is_field(char*);
extern	void	stripcpy(char*,char*);
extern	int	range(int*,int*,int*,int*,char**);
extern	void	process_abc(char[][99],int,Record*,char*,char*,char*,char*,int,
			int,int,int*);

enum	output_types {
	NO_OUTPUT,
	TEX_OUTPUT,
	INDEX_OUTPUT,
	HASH_OUTPUT
};

enum two_bar_types { NO_BARS, ONE_BAR, TWO_BARS, ONE_BAR_PLUS, TWO_BARS_PLUS };

enum symbol_types { UNDETERMINED, BAR_LINE, NOTE, FIELD, MISC};
enum bar_types { BAR, BAR1, DBL_BAR, LDBL_BAR, RDBL_BAR, REPEAT, LREPEAT,
		RREPEAT, RREPEAT2 };
enum accidental_types { NONE, DBL_FLAT, FLAT, NATURAL, SHARP, DBL_SHARP };

