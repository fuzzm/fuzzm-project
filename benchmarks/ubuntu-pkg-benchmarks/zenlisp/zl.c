/*
 * zenlisp -- an interpreter for symbolic LISP
 * By Nils M Holm <nmh@t3x.org>, 2007,2008,2013
 * Feel free to copy, share, and modify this program.
 * See the file LICENSE for details.
 */

#include <stdlib.h>
#ifdef __TURBOC__
 #include <io.h>
 #include <alloc.h>
#else
 #include <unistd.h>
 #ifndef __MINGW32__
  #ifndef __CYGWIN__
   #define setmode(fd, mode)
  #endif
 #endif
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#define VERSION	2
#define RELEASE	"2013-11-22"

/*
 * Number of nodes and vector cells.
 * Memory = Nodes * (2 * sizeof(int) + 1) 
 */
#define	DEFAULT_NODES	131072
#define MINIMUM_NODES	12280

#ifndef DEFAULT_IMAGE
 #define DEFAULT_IMAGE	"/u/share/zenlisp/zenlisp"
#endif

struct counter {
	int	n, n1k, n1m, n1g;
};

struct Error_context {
	char	*msg;
	char	*arg;
	int	expr;
	char	*file;
	int	line;
	int	fun;
	int	frame;
};

#define	SYMBOL_LEN	256
#define MAX_PATH_LEN	256

/* Tags */
#define	ATOM_FLAG	0x01	/* Atom: (CAR = char, CDR = next) */
#define	MARK_FLAG	0x02	/* garbage collector: mark */
#define SWAP_FLAG	0x04	/* garbage collector: swap CAR/CDR */

#define NIL	-1
#define	EOT	-2
#define	DOT	-3
#define	R_PAREN	-4
#define NO_EXPR	-5

enum Evaluator_states {
	MATOM =	'0', 	/* Processing Atom */
	MLIST,	 	/* Processing List */
	MBETA,	 	/* Beta-reducing */
	MBIND, 		/* Processing bindings of LET */
	MBINR, 		/* Processing bindings of LETREC */
	MLETR, 		/* Finish LET or LETREC */
	MCOND, 		/* Processing predicates of COND */
	MCONJ, 		/* Processing arguments of AND */
	MDISJ 		/* Processing arguments of OR */
};

int	Pool_size;
int	*Car,			/* Car*Cdr*Tag = Node Pool */
	*Cdr;
char	*Tag;
int	Freelist;

int	Tmp_car, Tmp_cdr;	/* GC-safe */
int	Tmp, Tmp2;

char	*Infile;
FILE	*Input;
int	Rejected;
int	Line;
FILE	*Output;

char	Source_dir[MAX_PATH_LEN];
char	Expanded_path[MAX_PATH_LEN];
char	Current_path[MAX_PATH_LEN];

int	Error_flag;
struct Error_context
	Error;
int	Fatal_flag;

int	Symbols;
int	Safe_symbols;
int	Stack, Stack_bottom;
int	Mode_stack;
int	Arg_stack;
int	Bind_stack;
int	Env_stack;
int	Frame;
int	Function_name;
int	Traced_fn;

int	*Root[] = { &Symbols, &Stack, &Mode_stack, &Arg_stack, &Bind_stack,
			&Env_stack, &Tmp_car, &Tmp_cdr, &Tmp, &Tmp2,
			&Safe_symbols, NULL };

int	Lexical_env;
int	Bound_vars;

int	Paren_level;
int	Load_level;
int	Eval_level;
int	Quotedprint;
int	Max_atoms_used;
int	Max_trace;
int	Stat_flag;
int	Closure_form;
int	Verify_arrows;
int	Verbose_GC;

struct counter	Reductions,
		Allocations,
		Collections;

/* Builtin symbol pointers for fast lookup */
int	S_bottom, S_closure, S_false, S_lambda, S_primitive,
	S_quote, S_special, S_special_cbv, S_true, S_void,
	S_last;

/* Primitive function opcodes */
enum {	P_ATOM, P_BOTTOM, P_CAR, P_CDR, P_CONS, P_DEFINED, P_EQ,
	P_EXPLODE, P_GC, P_IMPLODE, P_QUIT, P_RECURSIVE_BIND,
	P_SYMBOLS, P_VERIFY_ARROWS, N_PRIMITIVES };

int	(*Primitives[N_PRIMITIVES])(int);

/* Special form opcodes */
enum {	SF_AND, SF_APPLY, SF_CLOSURE_FORM, SF_COND, SF_DEFINE,
	SF_DUMP_IMAGE, SF_EVAL, SF_LAMBDA, SF_LET, SF_LETREC,
	SF_LOAD, SF_OR, SF_QUOTE, SF_STATS, SF_TRACE,
	N_SPECIALS };

int	(*Specials[N_SPECIALS])(int, int *, int *, int *);

#ifdef LINT
  #define USE(arg)	(arg = NIL)
#else
  #define USE(arg)
#endif

int	_rdch(void);
int	add_primitive(char *name, int opcode);
int	add_special(char *name, int opcode, int cbv);
int	add_symbol(char *s, int v);
int	alloc3(int pcar, int pcdr, int ptag);
int	aunsave(int k);
int	bad_argument_list(int n);
void	bind_args(int n, int name);
int	bunsave(int k);
void	catch_int(int sig);
void	clear_stats(void);
void	collect_free_vars(int n);
int	cond_get_pred(void);
int	cond_eval_clause(int n);
int	cond_setup(int n);
int	copy_bindings(void);
void	count(struct counter *c, int k);
char	*counter_to_string(struct counter *c, char *buf);
int	define_function(int n);
int	dump_image(char *p);
int	equals(int n, int m);
void	eliminate_tail_calls(void);
int	error(char *m, int n);
int	eval(int n);
char	*expand_path(char *s, char *buf);
int	explode_string(char *sym);
void	fatal(char *m);
int	find_symbol(char *s);
void	fix_all_closures(int b);
void	fix_cached_closures(void);
void	fix_closures_of(int n, int bindings);
int	flat_copy(int n, int *lastp);
int	gc(void);
int	get_opt_val(int argc, char **argv, int *pi, int *pj, int *pk);
void	get_options(int argc, char **argv);
void	get_source_dir(char *path, char *pfx);
char	*symbol_to_string(int n, char *b, int k);
void	help(void);
void	init(void);
void	init1(void);
void	init2(void);
int	is_alist(int n);
int	is_bound(int n);
int	is_list_of_symbols(int m);
void	let_bind(int env);
int	let_eval_arg(void);
int	let_finish(int rec);
int	let_next_binding(int n);
int	let_setup(int n);
int	load(char *p);
int	make_closure(int n);
void	mark(int n);
int	make_lexical_env(int term, int locals);
char	*make_zen_path(char *s);
int	munsave(void);
void	nl(void);
void	print(int n);
int	reverse_in_situ(int n);
void	pr(char *s);
int	primitive(int *np);
void	print_call_trace(int n);
int	print_closure(int n, int dot);
int	print_condensed_list(int n, int dot);
int	print_primitive(int n, int dot);
int	print_quoted_form(int n, int dot);
void	print_trace(int n);
void	print_license(void);
void	pr_num(int n);
int	quote(int n);
int	read_condensed(void);
void	read_eval_loop(void);
int	read_list(void);
int	read_symbol(int c);
void	repl(void);
void	reset_counter(struct counter *c);
void	reset_state(void);
void	restore_bindings(int values);
int	setup_and_or(int n);
int	special(int *np, int *pcf, int *pmode, int *pcbn);
int	string_to_symbol(char *s);
char	*symbol_to_string(int n, char *b, int k);
void	unbind_args(void);
int	unreadable(void);
int	unsave(int k);
void	usage(void);
void	verify(void);
int	wrong_arg_count(int n);
int	z_and(int n, int *pcf, int *pmode, int *pcbn);
int	z_apply(int n, int *pcf, int *pmode, int *pcbn);
int	z_atom(int n);
int	z_bottom(int n);
int	z_car(int n);
int	z_cdr(int n);
int	z_closure_form(int n, int *pcf, int *pmode, int *pcbn);
int	z_cond(int n, int *pcf, int *pmode, int *pcbn);
int	z_cons(int n);
int	z_define(int n, int *pcf, int *pmode, int *pcbn);
int	z_defined(int n);
int	z_dump_image(int n, int *pcf, int *pmode, int *pcbn);
int	z_eq(int n);
int	z_eval(int n, int *pcf, int *pmode, int *pcbn);
int	z_explode(int n);
int	z_gc(int n);
int	z_implode(int n);
int	z_lambda(int n, int *pcf, int *pmode, int *pcbn);
int	z_let(int n, int *pcf, int *pmode, int *pcbn);
int	z_letrec(int n, int *pcf, int *pmode, int *pcbn);
int	z_load(int n, int *pcf, int *pmode, int *pcbn);
int	z_or(int n, int *pcf, int *pmode, int *pcbn);
int	z_quit(int n);
int	z_quote(int n, int *pcf, int *pmode, int *pcbn);
int	z_recursive_bind(int n);
int	z_stats(int n, int *pcf, int *pmode, int *pcbn);
int	z_symbols(int n);
int	z_trace(int n, int *pcf, int *pmode, int *pcbn);
int	z_verify_arrows(int n);
int	zen_eval(int n);
void	zen_fini(void);
int	zen_init(int nodes, int trackGc);
char	**zen_license(void);
int	zen_load_image(char *p);
void	zen_print(int n);
void	zen_print_error(void);
int	zen_read(void);
void	zen_stop(void);
int	zread(void);

#define caar(x) (Car[Car[x]])
#define cadr(x) (Car[Cdr[x]])
#define cdar(x) (Cdr[Car[x]])
#define cddr(x) (Cdr[Cdr[x]])
#define caaar(x) (Car[Car[Car[x]]])
#define caadr(x) (Car[Car[Cdr[x]]])
#define cadar(x) (Car[Cdr[Car[x]]])
#define caddr(x) (Car[Cdr[Cdr[x]]])
#define cdaar(x) (Cdr[Car[Car[x]]])
#define cddar(x) (Cdr[Cdr[Car[x]]])
#define cdddr(x) (Cdr[Cdr[Cdr[x]]])
#define caddar(x) (Car[Cdr[Cdr[Car[x]]]])
#define cadddr(x) (Car[Cdr[Cdr[Cdr[x]]]])

void nl(void) {
	putc('\n', Output);
	if (Output == stdout) fflush(Output);
}

void pr(char *s) {
	fputs(s, Output);
}

void pr_num(int n) {
	fprintf(Output, "%d", n);
}

void print_call_trace(int frame) {
	int	s, n;

	s = frame;
	n = Max_trace;
	while (s != NIL) {
		if (n == 0 || Cdr[s] == NIL || cadr(s) == NIL) break;
		if (n == Max_trace) pr("* Trace:");
		n = n-1;
		pr(" ");
		Quotedprint = 1;
		print(cadr(s));
		s = Car[s];
	}
	if (n != Max_trace) nl();
}

/* Register error */
int error(char *m, int n) {
	if (Error_flag) return NIL;
	Error.msg = m;
	Error.expr = n;
	Error.file = Infile;
	Error.line = Line;
	Error.fun = Function_name;
	Error.frame = Frame;
	Error_flag = 1;
	return NIL;
}

void zen_print_error(void) {
	pr("* ");
	if (Error.file) {
		pr(Error.file);
		pr(": ");
	}
	pr_num(Error.line);
	pr(": ");
	if (Error.fun != NIL) {
		Quotedprint = 1;
		print(Error.fun);
	}
	else {
		pr("REPL");
	}
	pr(": ");
	pr(Error.msg);
	if (Error.expr != NO_EXPR) {
		if (Error.msg[0]) pr(": ");
		Quotedprint = 1;
		print(Error.expr);
	}
	nl();
	if (Error.arg) {
		pr("* ");
		pr(Error.arg); nl();
		Error.arg = NULL;
	}
	if (!Fatal_flag && Error.frame != NIL)
		print_call_trace(Error.frame);
	Error_flag = 0;
}

void fatal(char *m) {
	Error_flag = 0;
	Fatal_flag = 1;
	error(m, NO_EXPR);
	zen_print_error();
	pr("* Fatal error, aborting");
	nl();
	exit(1);
}

void reset_counter(struct counter *c) {
	c->n = 0;
	c->n1k = 0;
	c->n1m = 0;
	c->n1g = 0;
}

/* Assert 0<=k<=1000 */
void count(struct counter *c, int k) {
	char	*msg = "statistics counter overflow";

	c->n = c->n+k;
	if (c->n >= 1000) {
		c->n = c->n - 1000;
		c->n1k = c->n1k + 1;
		if (c->n1k >= 1000) {
			c->n1k = 0;
			c->n1m = c->n1m+1;
			if (c->n1m >= 1000) {
				c->n1m = 0;
				c->n1g = c->n1g+1;
				if (c->n1g >= 1000) {
					error(msg, NO_EXPR);
				}
			}
		}
	}
}

char *counter_to_string(struct counter *c, char *buf) {
	int	i;

	i = 0;
	if (c->n1g) {
		sprintf(&buf[i], "%d,", c->n1g);
		i = strlen(buf);
	}
	if (c->n1m || c->n1g) {
		if (c->n1g)
			sprintf(&buf[i], "%03d,", c->n1m);
		else
			sprintf(&buf[i], "%d,", c->n1m);
		i = strlen(buf);
	}
	if (c->n1k || c->n1m || c->n1g) {
		if (c->n1g || c->n1m)
			sprintf(&buf[i], "%03d,", c->n1k);
		else
			sprintf(&buf[i], "%d,", c->n1k);
		i = strlen(buf);
	}
	if (c->n1g || c->n1m || c->n1k)
		sprintf(&buf[i], "%03d", c->n);
	else
		sprintf(&buf[i], "%d", c->n);
	return buf;
}

/*
 * Mark nodes which can be accessed through N.
 * This routine uses the Deutsch/Schorr/Waite algorithm
 * (aka pointer reversal algorithm) which marks the
 * nodes of a pool in constant space.
 * It uses the MARK_FLAG (M) and SWAP_FLAG (S) to keep track
 * of the state of the current node.
 * Each visited node goes through these states:
 * State 1: M=0 S=0; unvisited, process CAR (skipped for atoms)
 * State 2: M=1 S=1; CAR visited, process CDR
 * State 3: M=1 S=0; completely visited, return to parent
 */
void mark(int n) {
	int	p, parent;

	parent = NIL;
	while (1) {
		if (n == NIL || Tag[n] & MARK_FLAG) {
			if (parent == NIL) break;
			if (Tag[parent] & SWAP_FLAG) {
				/* State 2: */
				/* Swap CAR and CDR pointers and */
				/* proceed with CDR. Set State=3. */
				p = Cdr[parent];
				Cdr[parent] = Car[parent];
				Car[parent] = n;
				Tag[parent] &= ~SWAP_FLAG;	/* S=0 */
				Tag[parent] |=  MARK_FLAG;	/* M=1 */
				n = p;
			}
			else {
				/* State 3: */
				/* Return to the parent and restore */
				/* parent of parent */
				p = parent;
				parent = Cdr[p];
				Cdr[p] = n;
				n = p;
			}
		}
		else {
			/* State 1: */
			if (Tag[n] & ATOM_FLAG) {
				/* If this node is an atom, go directly */
				/* to state 3. */
				p = Cdr[n];
				Cdr[n] = parent;
				/*Tag[n] &= ~SWAP_FLAG;*/	/* S=0 */
				parent = n;
				n = p;
				Tag[parent] |= MARK_FLAG;	/* M=1 */
			}
			else {
				/* Go to state 2: */
				p = Car[n];
				Car[n] = parent;
				Tag[n] |= MARK_FLAG;		/* M=1 */
				parent = n;
				n = p;
				Tag[parent] |= SWAP_FLAG;	/* S=1 */
			}
		}
	}
}

/* Mark and Sweep Garbage Collection. */
int gc(void) {
	int	i, k;

	k = 0;
	for (i=0; Root[i]; i++) mark(Root[i][0]);
	if (Error_flag) {
		mark(Error.expr);
		mark(Error.fun);
		mark(Error.frame);
	}
	Freelist = NIL;
	for (i=0; i<Pool_size; i++) {
		if (!(Tag[i] & MARK_FLAG)) {
			Cdr[i] = Freelist;
			Freelist = i;
			k = k+1;
		}
		else {
			Tag[i] &= ~MARK_FLAG;
		}
	}
	if (Max_atoms_used < Pool_size-k) Max_atoms_used = Pool_size-k;
	if (Verbose_GC) {
		pr_num(k);
		pr(" nodes reclaimed");
		nl();
	}
	if (Stat_flag) count(&Collections, 1);
	return k;
}

int alloc3(int pcar, int pcdr, int ptag) {
	int	n;

	if (Stat_flag) count(&Allocations, 1);
	if (Freelist == NIL) {
		Tmp_cdr = pcdr;
		if (!ptag) Tmp_car = pcar;
		gc();
		Tmp_car = Tmp_cdr = NIL;
		if (Freelist == NIL) fatal("alloc3(): out of nodes");
	}
	n = Freelist;
	Freelist = Cdr[Freelist];
	Car[n] = pcar;
	Cdr[n] = pcdr;
	Tag[n] = ptag;
	return n;
}

#define alloc(pcar, pcdr) \
	alloc3(pcar, pcdr, 0)

#define save(n) \
	(Stack = alloc(n, Stack))

int unsave(int k) {
	int	n;

	USE(n);
	while (k) {
		if (Stack == NIL) fatal("unsave(): stack underflow");
		n = Car[Stack];
		Stack = Cdr[Stack];
		k = k-1;
	}
	return n;
}

/* Because the Mode_stack holds integer values rather than nodes */
/* the values are packaged in the character fields of atoms. */
#define msave(v) \
	(Car[Mode_stack] = alloc3(v, Car[Mode_stack], ATOM_FLAG))

int munsave(void) {
	int	v;

	if (Car[Mode_stack] == NIL) fatal("munsave(): m-stack underflow");
	v = caar(Mode_stack);
	Car[Mode_stack] = cdar(Mode_stack);
	return v;
}

#define asave(n) \
	(Arg_stack = alloc(n, Arg_stack))

int aunsave(int k) {
	int	n;

	USE(n);
	while (k) {
		if (Arg_stack == NIL) fatal("aunsave(): a-stack underflow");
		n = Car[Arg_stack];
		Arg_stack = Cdr[Arg_stack];
		k = k-1;
	}
	return n;
}

#define bsave(n) \
	(Bind_stack = alloc(n, Bind_stack))

int bunsave(int k) {
	int	n;

	USE(n);
	while (k) {
		if (Bind_stack == NIL) fatal("bunsave(): b-stack underflow");
		n = Car[Bind_stack];
		Bind_stack = Cdr[Bind_stack];
		k = k-1;
	}
	return n;
}

int find_symbol(char *s) {
	int	p, n, i;

	p = Symbols;
	while (p != NIL) {
		n = caar(p);
		i = 0;
		while (n != NIL && s[i]) {
			if (s[i] != (Car[n] & 255)) break;
			n = Cdr[n];
			i = i+1;
		}
		if (n == NIL && !s[i]) return Car[p];
		p = Cdr[p];
	}
	return NIL;
}

#define atomic(n) \
	((n) == NIL || (Car[n] != NIL && (Tag[Car[n]] & ATOM_FLAG)))

#define symbolic(n) \
	((n) != NIL && Car[n] != NIL && (Tag[Car[n]] & ATOM_FLAG))

int string_to_symbol(char *s) {
	int	i, n, m, a;

	i = 0;
	if (s[i] == 0) return NIL;
	a = n = NIL;
	while (s[i]) {
		m = alloc3(s[i], NIL, ATOM_FLAG);
		if (n == NIL) {
			n = m;
			save(n);
		}
		else {
			Cdr[a] = m;
		}
		a = m;
		i = i+1;
	}
	unsave(1);
	return n;
}

char *symbol_to_string(int n, char *b, int k) {
	int	i;

	n = Car[n];
	for (i=0; i<k-1; i++) {
		if (n == NIL) break;
		b[i] = Car[n];
		n = Cdr[n];
	}
	if (n != NIL) {
		error("symbol_to_string(): string too long", NO_EXPR);
		return NULL;
	}
	b[i] = 0;
	return b;
}

int add_symbol(char *s, int v) {
	int	n, m;

	n = find_symbol(s);
	if (n != NIL) return n;
	n = string_to_symbol(s);
	m = alloc(n, v? v: n);
	Symbols = alloc(m, Symbols);
	return m;
}

int add_primitive(char *name, int opcode) {
	int	y;

	y = add_symbol(name, 0);
	Cdr[y] = alloc(S_primitive, NIL);
	cddr(y) = alloc3(opcode, NIL, ATOM_FLAG);
	cdddr(y) = y;
	return y;
}

int add_special(char *name, int opcode, int cbv) {
	int	y;

	y = add_symbol(name, 0);
	Cdr[y] = alloc(cbv? S_special_cbv: S_special, NIL);
	cddr(y) = alloc3(opcode, NIL, ATOM_FLAG);
	cdddr(y) = y;
	return y;
}

int _rdch(void) {
	int	c;

	if (Rejected != EOT) {
		c = Rejected;
		Rejected = EOT;
		return c;
	}
	c = getc(Input);
	if (feof(Input)) return EOT;
	if (c == '\n') Line = Line+1;
	return c;
}

int rdch(void) {
	int	c = _rdch();

	if (c < 0) return c;
	return tolower(c);
}

int read_list(void) {
	int	n,
		lst,
		app,
		count;
	char	*badpair;

	badpair = "bad pair";
	Paren_level = Paren_level+1;
	lst = alloc(NIL, NIL);	/* Root node */
	save(lst);
	app = NIL;
	count = 0;
	while (1) {
		if (Error_flag) {
			unsave(1);
			return NIL;
		}
		n = zread();
		if (n == EOT)  {
			if (Load_level) return EOT;
			error("missing ')'", NO_EXPR);
		}
		if (n == DOT) {
			if (count < 1) {
				error(badpair, NO_EXPR);
				continue;
			}
			n = zread();
			Cdr[app] = n;
			if (n == R_PAREN || zread() != R_PAREN) {
				error(badpair, NO_EXPR);
				continue;
			}
			unsave(1);
			Paren_level = Paren_level-1;
			return lst;
		}
		if (n == R_PAREN) break;
		if (app == NIL) 
			app = lst;
		else
			app = Cdr[app];
		Car[app] = n;
		Cdr[app] = alloc(NIL, NIL);
		count = count+1;
	}
	Paren_level = Paren_level-1;
	if (app != NIL) Cdr[app] = NIL;
	unsave(1);
	return count? lst: NIL;
}

#define is_delimiter(c) \
		((c) == ' ' || \
		 (c) == '\t' || \
		 (c) == '\n' || \
		 (c) == '\r' || \
		 (c) == '(' || \
		 (c) == ')' || \
		 (c) == ';' || \
		 (c) == '.' || \
		 (c) == '#' || \
		 (c) == '{' || \
		 (c) == '\'')

int read_condensed(void) {
	int	n, c, a;
	char	s[2];

	n = alloc(NIL, NIL);
	save(n);
	a = NIL;
	s[1] = 0;
	c = rdch();
	while (!is_delimiter(c)) {
		if (a == NIL) {
			a = n;
		}
		else {
			Cdr[a] = alloc(NIL, NIL);
			a = Cdr[a];
		}
		s[0] = c;
		Car[a] = add_symbol(s, S_void);
		c = rdch();
	}
	unsave(1);
	Rejected = c;
	return n;
}

int explode_string(char *sym) {
	int	n, a, i;
	char	s[2];

	n = alloc(NIL, NIL);
	save(n);
	a = NIL;
	s[1] = 0;
	i = 0;
	while (sym[i]) {
		if (a == NIL) {
			a = n;
		}
		else {
			Cdr[a] = alloc(NIL, NIL);
			a = Cdr[a];
		}
		s[0] = sym[i];
		Car[a] = add_symbol(s, S_void);
		i += 1;
	}
	unsave(1);
	return n;
}

int quote(int n) {
	int	q;

	q = alloc(n, NIL);
	return alloc(S_quote, q);
}

int read_symbol(int c) {
	char	s[SYMBOL_LEN];
	int	i;

	i = 0;
	while (!is_delimiter(c)) {
		if (i >= SYMBOL_LEN-2) {
			error("symbol too long", NO_EXPR);
			i = i-1;
		}
		s[i] = c;
		i = i+1;
		c = rdch();
	}
	s[i] = 0;
	Rejected = c;
	return add_symbol(s, S_void);
}

int equals(int n, int m) {
	if (n == m) return 1;
	if (n == NIL || m == NIL) return 0;
	if (Tag[n] & ATOM_FLAG || Tag[m] & ATOM_FLAG) return 0;
	return equals(Car[n], Car[m])
	    && equals(Cdr[n], Cdr[m]);
}

void verify(void) {
	int	expected;

	expected = zread();
	if (!atomic(expected) && Car[expected] == S_quote)
		expected = cadr(expected);
	if (!equals(expected, Cdr[S_last]))
		error("Verification failed; expected", expected);
}

int unreadable(void) {
	#define	L 256
	int		c, i;
	static char	b[L];

	i = 0;
	b[0] = '{';
	c = '{';
	while (c != '}' && c != EOT && i < L-2) {
		b[i++] = c;
		c = rdch();
	}
	b[i] = '}';
	b[i+1] = 0;
	Error.arg = b;
	return error("unreadable object", NO_EXPR);
}

int zread(void) {
	int	c;

	c = rdch();
	while (1) {
		while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
			if (Error_flag) return NIL;
			c = rdch();
		}
		if (c == '=' && Paren_level == 0) {
			c = rdch();
			if (c != '>') {
				Rejected = c;
				c = '=';
				break;
			}
			if (Verify_arrows) verify();
		}
		else if (c != ';') {
			break;
		}
		while (c != '\n') c = rdch();
	}
	if (c == EOT) return EOT;
	if (c == '(') {
		return read_list();
	}
	else if (c == '\'') {
		return quote(zread());
	}
	else if (c == '#') {
		return read_condensed();
	}
	else if (c == ')') {
		if (!Paren_level) return error("unexpected ')'", NO_EXPR);
		return R_PAREN;
	}
	else if (c == '.') {
		if (!Paren_level) return error("unexpected '.'", NO_EXPR);
		return DOT;
	}
	else if (c == '{') {
		return unreadable();
	}
	else {
		return read_symbol(c);
	}
}

int wrong_arg_count(int n) {
	return error("wrong argument count", n);
}

int bad_argument_list(int n) {
	return error("bad argument list", n);
}

int z_cons(int n) {
	int	m, m2;

	m = Cdr[n];
	if (m == NIL || Cdr[m] == NIL || cddr(m) != NIL)
		return wrong_arg_count(n);
	m2 = cadr(m);
	m = alloc(Car[m], m2);
	return m;
}

int z_car(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	m = Car[m];
	if (	atomic(m) ||
		Car[m] == S_primitive ||
		Car[m] == S_special ||
		Car[m] == S_special_cbv
	)
		return error("car: cannot split atoms", m);
	return Car[m];
}

int z_cdr(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	m = Car[m];
	if (	atomic(m) ||
		Car[m] == S_primitive ||
		Car[m] == S_special ||
		Car[m] == S_special_cbv
	)
		return error("cdr: cannot split atoms", m);
	return Cdr[m];
}

int z_eq(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL || Cdr[m] == NIL || cddr(m) != NIL)
		return wrong_arg_count(n);
	return Car[m] == cadr(m)? S_true: S_false;
}

int z_atom(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	if atomic(Car[m]) return S_true;
	m = caar(m);
	return (m == S_primitive || m == S_special ||
		m == S_special_cbv || m == S_void)? S_true: S_false;
}

int z_explode(int n) {
	int	m, y, a;
	char	s[2];

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	m = Car[m];
	if (m == NIL) return NIL;
	if (!symbolic(m)) return error("explode: got non-symbol", m);
	y = alloc(NIL, NIL);
	save(y);
	a = y;
	m = Car[m];
	s[1] = 0;
	while (m != NIL) {
		s[0] = Car[m];
		Car[a] = add_symbol(s, S_void);
		m = Cdr[m];
		if (m != NIL) {
			Cdr[a] = alloc(NIL, NIL);
			a = Cdr[a];
		}
	}
	unsave(1);
	return y;
}

int z_implode(int n) {
	int	m, i;
	char	s[SYMBOL_LEN];

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	m = Car[m];
	if (m == NIL) return NIL;
	i = 0;
	while (m != NIL) {
		if (!symbolic(Car[m]))
			return error("implode: non-symbol in argument",
				Car[m]);
		if (cdaar(m) != NIL)
			return error(
			  "implode: input symbol has multiple characters",
				Car[m]);
		if (i >= SYMBOL_LEN-1)
			return error("implode: output symbol too long", m);
		s[i] = caaar(m);
		i += 1;
		m = Cdr[m];
	}
	s[i] = 0;
	return add_symbol(s, S_void);
}

void fix_cached_closures(void) {
	int	a, ee, e;

	if (Error_flag || Env_stack == NIL || Env_stack == S_true) return;
	a = Car[Bind_stack];
	while (a != NIL) {
		ee = Env_stack;
		while (ee != NIL && ee != S_true) {
			e = Car[ee];
			while (e != NIL) {
				if (Car[a] == caar(e)) {
					cdar(e) = cdar(a);
					break;
				}
				e = Cdr[e];
			}
			ee = Cdr[ee];
		}
		a = Cdr[a];
	}
}

int is_alist(int n) {
	if (symbolic(n)) return 0;
	while (n != NIL) {
		if (symbolic(Car[n]) || !symbolic(caar(n)))
			return 0;
		n = Cdr[n];
	}
	return 1;
}

void fix_closures_of(int n, int bindings) {
	int	ee, e;
	int	bb, b;

	if (atomic(n)) return;
	if (Car[n] == S_closure) {
		fix_closures_of(caddr(n), bindings);
		ee = cdddr(n);
		if (ee == NIL) return;
		ee = Car[ee];
		while (ee != NIL) {
			e = Car[ee];
			bb = bindings;
			while (bb != NIL) {
				b = Car[bb];
				if (Car[b] == Car[e])
					Cdr[e] = Cdr[b];
				bb = Cdr[bb];
			}
			ee = Cdr[ee];
		}
		return;
	}
	fix_closures_of(Car[n], bindings);
	fix_closures_of(Cdr[n], bindings);
}

void fix_all_closures(int b) {
	int	p;

	p = b;
	while (p != NIL) {
		fix_closures_of(cdar(p), b);
		p = Cdr[p];
	}
}

int z_recursive_bind(int n) {
	int	m, env;

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	env = Car[m];
	if (!is_alist(env))
		return error("recursive-bind: bad environment", env);
	fix_all_closures(env);
	return env;
}

int z_bottom(int n) {
	n = alloc(S_bottom, Cdr[n]);
	return error("", n);
}

int z_defined(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	if (!symbolic(Car[m]))
		return error("defined: got non-symbol", Car[m]);
	return cdar(m) == S_void? S_false: S_true;
}

int z_gc(int n) {
	int	m;
	char	s[20];

	m = Cdr[n];
	if (m != NIL) return wrong_arg_count(n);
	n = alloc(NIL, NIL);
	save(n);
	sprintf(s, "%d", gc());
	Car[n] = explode_string(s);
	Cdr[n] = alloc(NIL, NIL);
	sprintf(s, "%d", Max_atoms_used);
	Max_atoms_used = 0;
	cadr(n) = explode_string(s);
	unsave(1);
	return n;
}

int z_quit(int n) {
	int	m;

	m = Cdr[n];
	if (m != NIL) return wrong_arg_count(n);
	zen_fini();
	exit(0);
}

int z_symbols(int n) {
	int	m;

	m = Cdr[n];
	if (m != NIL) return wrong_arg_count(n);
	return Symbols;
}

int z_verify_arrows(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	m = Car[m];
	if (m != S_true && m != S_false)
		return error("verify-arrows: got non truth-value", m);
	Verify_arrows = m == S_true;
	return m;
}

/* If (CAR NP[0]) is a builtin procedure, run it. */
int primitive(int *np) {
	int	n, y;
	int	(*op)(int);

	n = np[0];
	y = Car[n];
	if (Error_flag) return 0;
	if (Car[y] == S_primitive) {
		op = Primitives[cadr(y)];
	}
	else {
		return 0;
	}
	n = (*op)(n);
	np[0] = n;
	return 1;
}

int setup_and_or(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL) return wrong_arg_count(n);
	bsave(m);
	return Car[m];
}

int z_and(int n, int *pcf, int *pmode, int *pcbn) {
	USE(pcbn);
	if (Cdr[n] == NIL) {
		return S_true;
	}
	else if (cddr(n) == NIL) {
		*pcf = 1;
		return cadr(n);
	}
	else {
		*pcf = 2;
		*pmode = MCONJ;
		return setup_and_or(n);
	}
}

int flat_copy(int n, int *lastp) {
	int     a, m, last;

	if (n == NIL) {
		lastp[0] = NIL;
		return NIL;
	}
	m = alloc(NIL, NIL);
	save(m);
	a = m;
	last = m;
	while (n != NIL) {
		Car[a] = Car[n];
		last = a;
		n = Cdr[n];
		if (n != NIL) {
			Cdr[a] = alloc(NIL, NIL);
			a = Cdr[a];
		}
	}
	unsave(1);
	lastp[0] = last;
	return m;
}

int z_apply(int n, int *pcf, int *pmode, int *pcbn) {
	int	m, p, q, last;
	char	*err1 = "apply: got non-function",
		*err2 = "apply: improper argument list";

	*pcf = 1;
	USE(pmode);
	*pcbn = 1;
	m = Cdr[n];
	if (m == NIL || Cdr[m] == NIL) return wrong_arg_count(n);
	if (atomic(Car[m])) return error(err1, Car[m]);
	p = caar(m);
	if (	p != S_primitive &&
		p != S_special &&
		p != S_special_cbv &&
		p != S_closure
	)
		return error(err1, Car[m]);
	p = Cdr[m];
	USE(last);
	while (p != NIL) {
		if (symbolic(p)) return error(err2, cadr(m));
		last = p;
		p = Cdr[p];
	}
	p = Car[last];
	while (p != NIL) {
		if (symbolic(p)) return error(err2, Car[last]);
		p = Cdr[p];
	}
	if (cddr(m) == NIL) {
		p = cadr(m);
	}
	else {
		p = flat_copy(Cdr[m], &q);
		q = p;
		while (cddr(q) != NIL) q = Cdr[q];
		Cdr[q] = Car[last];
	}
	return alloc(Car[m], p);
}

int cond_get_pred(void) {
	int	e;

	e = caar(Bind_stack);
	if (atomic(e) || atomic(Cdr[e]) || cddr(e) != NIL)
		return error("cond: bad clause", e);
	return Car[e];
}

int cond_setup(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL) return wrong_arg_count(n);
	bsave(m);
	return cond_get_pred();
}

/*
 * Evaluate next clause of COND.
 * N is the value of the current predicate.
 * If N=T, return the expression of the predicate.
 * If N=:F, return the predicate of the next clause.
 * When returning the expression of a predicate (N=T),
 * set the context on the Bind_stack to NIL to signal that
 * a true clause was found.
 */
int cond_eval_clause(int n) {
	int	e;

	e = Car[Bind_stack];
	if (n == S_false) {
		Car[Bind_stack] = Cdr[e];
		if (Car[Bind_stack] == NIL)
			return error("cond: no default", NO_EXPR);
		return cond_get_pred();
	}
	else {
		e = cadar(e);
		Car[Bind_stack] = NIL;
		return e;
	}
}

int z_cond(int n, int *pcf, int *pmode, int *pcbn) {
	*pcf = 2;
	*pmode = MCOND;
	USE(pcbn);
	return cond_setup(n);
}

int is_list_of_symbols(int m) {   
	while (m != NIL) {
		if (!symbolic(Car[m])) return 0;
		if (symbolic(Cdr[m])) break;
		m = Cdr[m];
	}
	return 1;
}

int define_function(int n) {
	int	m, y;

	m = Cdr[n];
	if (Car[m] == NIL)
		return error("define: missing function name",
			Car[m]);
	if (!is_list_of_symbols(Car[m])) return bad_argument_list(Car[m]);
	y = caar(m);
	save(cadr(m));
	Tmp2 = alloc(S_lambda, NIL);
	Cdr[Tmp2] = alloc(cdar(m), NIL);
	cddr(Tmp2) = alloc(cadr(m), NIL);
	cdddr(Tmp2) = alloc(NIL, NIL);
	Cdr[y] = eval(Tmp2);
	Tmp2 = NIL;
	unsave(1);
	return y;
}

int z_define(int n, int *pcf, int *pmode, int *pcbn) {
	int	m, v, y;

	USE(pcf);
	USE(pmode);
	USE(pcbn);
	if (Eval_level > 1) {
		error("define: limited to top level", NO_EXPR);
		return NIL;
	}
	m = Cdr[n];
	if (m == NIL || Cdr[m] == NIL || cddr(m) != NIL)
		return wrong_arg_count(n);
	y = Car[m];
	if (!symbolic(y)) return define_function(n);
	v = cadr(m);
	save(v);
	/* If we are binding to a lambda expression, */
	/* add a null environment */
	if (!atomic(v) && Car[v] == S_lambda) {
		if (	Cdr[v] != NIL && cddr(v) != NIL &&
			cdddr(v) == NIL
		) {
			cdddr(v) = alloc(NIL, NIL);
		}
	}
	Cdr[y] = eval(cadr(m));
	unsave(1);
	return y;
}

int z_eval(int n, int *pcf, int *pmode, int *pcbn) {
	int	m;

	*pcf = 1;
	USE(pmode);
	*pcbn = 0;
	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	return (Car[m]);
}

int is_bound(int n) {
	int	b;

	b = Bound_vars;
	while (b != NIL) {
		if (symbolic(b)) {
			if (n == b) return 1;
			break;
		}
		if (n == Car[b]) return 1;
		b = Cdr[b];
	}
	b = Car[Lexical_env];
	while (b != NIL) {
		if (caar(b) == n) return 1;
		b = Cdr[b];
	}
	return 0;
}

void collect_free_vars(int n) {
	if (n == NIL || (Tag[n] & ATOM_FLAG)) return;
	if (symbolic(n)) {
		if (is_bound(n)) return;
		Car[Lexical_env] = alloc(NIL, Car[Lexical_env]);
		caar(Lexical_env) = alloc(n, Car[n] == Cdr[n]? n: Cdr[n]);
		return;
	}
	/*
	 * Avoid inclusion of quoted forms.
	 * We cannot just check for Car[n] == S_quote,
	 * because this would also catch (list quote foo).
	 * By checking caar(n), we make sure that QUOTE
	 * actually is in a car position.
	 * NOTE: this also prevents (quote . (internal quote))
	 * from being included, but who wants to re-define
	 * QUOTE anyway?
	 */
	if (atomic(Car[n]) || caar(n) != S_quote)
		collect_free_vars(Car[n]);
	collect_free_vars(Cdr[n]);
}

int make_lexical_env(int term, int locals) {
	Lexical_env = alloc(NIL, NIL);
	save(Lexical_env);
	Bound_vars = locals;
	collect_free_vars(term);
	unsave(1);
	return Car[Lexical_env];
}

int make_closure(int n) {
	int	cl, env, args, term;

	if (Error_flag) return NIL;
	args = cadr(n);
	term = caddr(n);
	if (cdddr(n) == NIL) {
		env = make_lexical_env(term, args);
		if (env != NIL) {
			if (Env_stack != NIL) Env_stack = alloc(env, Env_stack);
			cl = alloc(env, NIL);
		}
		else {
			cl = NIL;
		}
	}
	else {
		cl = alloc(cadddr(n), NIL);
	}
	cl = alloc(term, cl);
	cl = alloc(args, cl);
	cl = alloc(S_closure, cl);
	return cl;
}

int z_lambda(int n, int *pcf, int *pmode, int *pcbn) {
	int	m;

	m = Cdr[n];
	if (	m == NIL || Cdr[m] == NIL ||
		(cddr(m) != NIL && cdddr(m) != NIL)
	)
		return wrong_arg_count(n);
	if (cddr(m) != NIL && !is_alist(caddr(m)))
		return error("lambda: bad environment",
			caddr(m));
	if (!symbolic(Car[m]) && !is_list_of_symbols(Car[m]))
		return bad_argument_list(Car[m]);
	return Car[n] == S_closure? n: make_closure(n);
}

void unbind_args(void) {
	int	v;

	Frame = unsave(1);
	Function_name = unsave(1);
	v = bunsave(1);
	while (v != NIL) {
		cdar(v) = unsave(1);
		v = Cdr[v];
	}
}

/*
 * Set up a context for reduction of
 *     N=(LET ((MA1 eval[MX2]) ...) MN)
 * and N=(LETREC ((MA1 eval[MX2]) ...) MN).
 * Save
 * - the complete LET/LETREC expression on the Bind_stack
 * - the environment on the Bind_stack
 * - a list of new bindings on the Bind_stack (initially empty)
 * - a list of saved names on the Stack (initially empty)
 */
int let_setup(int n) {
	int	m;

	m = Cdr[n];
	if (m == NIL || Cdr[m] == NIL || cddr(m) != NIL)
		return wrong_arg_count(n);
	m = Car[m];
	if (symbolic(m))
		return error("let/letrec: bad environment", m);
	bsave(n);	/* save entire LET/LETREC */
	bsave(m);	/* save environment */
	bsave(NIL);	/* list of bindings */
	bsave(NIL);	/* save empty name list */
	save(Env_stack);	/* get outer bindings out of the way */
	Env_stack = NIL;
	return m;
}

/*
 * Process one binding of LET/LETREC.
 * Return:
 * non-NIL - more bindings in environment
 * NIL     - last binding done
 */
int let_next_binding(int n) {
	int	m, p;

	m = caddr(Bind_stack);	/* rest of environment */
	if (m == NIL) return NIL;
	p = Car[m];
	Tmp2 = n;
	cadr(Bind_stack) = alloc(NIL, cadr(Bind_stack));
	caadr(Bind_stack) = alloc(Car[p], n);
	Tmp2 = NIL;
	caddr(Bind_stack) = Cdr[m];
	return Cdr[m];
}

int let_eval_arg(void) {
	int	m, p, v;

	m = caddr(Bind_stack);
	p = Car[m];
	if (	atomic(p) || Cdr[p] == NIL || atomic(Cdr[p]) ||
		cddr(p) != NIL || !symbolic(Car[p])
	) {
		/* Error, get rid of the partial environment. */
		v = bunsave(1);
		bunsave(3);
		bsave(v);
		Env_stack = unsave(1);
		save(Function_name);
		save(Frame);
		unbind_args();
		return error("let/letrec: bad binding", p);
	}
	Car[Bind_stack] = alloc(Car[p], Car[Bind_stack]);
	return cadr(p);
}

int reverse_in_situ(int n) {
	int	this, next, x;

	if (n == NIL) return NIL;
	this = n;
	next = Cdr[n];
	Cdr[this] = NIL;
	while (next != NIL) {
		x = Cdr[next];
		Cdr[next] = this;
		this = next;
		next = x;
	}
	return this;
}

void let_bind(int env) {
	int	b;

	while (env != NIL) {
		b = Car[env];
		save(cdar(b));		/* Save old value */
		cdar(b) = Cdr[b];	/* Bind new value */
		env = Cdr[env];
	}
}

int let_finish(int rec) {
	int	m, v, b, e;

	Tmp2 = alloc(NIL, NIL);	/* Create safe storage */
	Cdr[Tmp2] = alloc(NIL, NIL);
	cddr(Tmp2) = alloc(NIL, NIL);
	cdddr(Tmp2) = alloc(NIL, NIL);
	v = bunsave(1);
	b = bunsave(1);		/* bindings */
	m = bunsave(2);		/* drop environment, get full LET/LETREC */
	b = reverse_in_situ(b);	/* needed for UNBINDARGS() */
	e = unsave(1);
	Car[Tmp2] = b;
	cadr(Tmp2) = m;
	caddr(Tmp2) = v;
	cdddr(Tmp2) = e;
	let_bind(b);
	bsave(v);
	if (rec) fix_cached_closures();
	Env_stack = e;
	save(Function_name);
	save(Frame);
	Tmp2 = NIL;
	return caddr(m); /* term */
}

int z_let(int n, int *pcf, int *pmode, int *pcbn) {
	*pcf = 2;
	*pmode = MBIND;
	USE(pcbn);
	if (let_setup(n) != NIL)
		return let_eval_arg();
	else
		return NIL;
}

int z_letrec(int n, int *pcf, int *pmode, int *pcbn) {
	int	m;

	*pcf = 2;
	*pmode = MBINR;
	USE(pcbn);
	if (let_setup(n) != NIL)
		m = let_eval_arg();
	else
		m = NIL;
	Env_stack = S_true;
	return m;
}

int z_or(int n, int *pcf, int *pmode, int *pcbn) {
	USE(pcbn);
	if (Cdr[n] == NIL) {
		return S_false;
	}
	else if (cddr(n) == NIL) {
		*pcf = 1;
		return cadr(n);
	}
	else {
		*pcf = 2;
		*pmode = MDISJ;
		return setup_and_or(n);
	}
}

int z_quote(int n, int *pcf, int *pmode, int *pcbn) {
	int	m;

	USE(pcf);
	USE(pmode);
	USE(pcbn);
	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	return (Car[m]);
}

int z_closure_form(int n, int *pcf, int *pmode, int *pcbn) {
	int		m;

	USE(pcf);
	USE(pmode);
	USE(pcbn);
	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	if (!symbolic(Car[m]))
		return error("closure-form: got non-symbol", Car[m]);
	if (Car[m] == add_symbol("args", S_void))
		Closure_form = 0;
	else if (Car[m] == add_symbol("body", S_void))
		Closure_form = 1;
	else if (Car[m] == add_symbol("env", S_void))
		Closure_form = 2;
	else
		return S_false;
	return Car[m];
}

int *Image_vars[] = {
	&Closure_form, &Verify_arrows,
	&Symbols, &Freelist, &S_bottom, &S_closure, &S_false,
	&S_lambda, &S_primitive, &S_quote, &S_special,
	&S_special_cbv, &S_true, &S_void, &S_last,
NULL };

int dump_image(char *p) {
	int	fd, n, i;
	int	**v;
	char	magic[17];

	fd = open(p, O_CREAT | O_WRONLY, 0644);
	setmode(fd, O_BINARY);
	if (fd < 0) {
		error("cannot create file", NO_EXPR);
		Error.arg = p;
		return -1;
	}
	strcpy(magic, "ZEN_____________");
	magic[7] = sizeof(int);
	magic[8] = VERSION;
	n = 0x12345678;
	memcpy(&magic[10], &n, sizeof(int));
	write(fd, magic, 16);
	n = Pool_size;
	write(fd, &n, sizeof(int));
	v = Image_vars;
	i = 0;
	while (v[i]) {
		write(fd, v[i], sizeof(int));
		i = i+1;
	}
	if (	write(fd, Car, Pool_size*sizeof(int))
			!= Pool_size*sizeof(int) ||
		write(fd, Cdr, Pool_size*sizeof(int))
			!= Pool_size*sizeof(int) ||
		write(fd, Tag, Pool_size) != Pool_size
	) {
		error("dump failed", NO_EXPR);
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int z_dump_image(int n, int *pcf, int *pmode, int *pcbn) {
	int		m;
	static char	buf[SYMBOL_LEN], *s;

	USE(pcf);
	USE(pmode);
	USE(pcbn);
	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	if (!symbolic(Car[m]))
		return error("dump-image: got non-symbol",
				Car[m]);
	s = symbol_to_string(Car[m], buf, SYMBOL_LEN);
	if (s) dump_image(s);
	return S_true;
}

void get_source_dir(char *path, char *buf) {
	char	*p;

	if (strlen(path) > 256) {
		error("load: path too long", NO_EXPR);
		return;
	}
	strcpy(buf, path);
	p = strrchr(buf, '/');
	if (p == NULL)
		strcpy(buf, ".");
	else
		*p = 0;
}

/* Expand leading ~ in path names */
char *expand_path(char *s, char *buf) {
	char	*r, *v;

	if (s[0] == '~')
		r = &s[1];
	else
		return s;
	if ((v = getenv("ZENSRC")) == NULL) return s;
	if (strlen(v) + strlen(r) + 4 >= MAX_PATH_LEN) {
		error("load: path too long", NO_EXPR);
		return s;
	}
	sprintf(buf, "%s/%s", v, r);
	return buf;
}

/* Bug: should restore Source_dir after loading a file */
int load(char *p) {
	FILE	*ofile, *nfile;
	int	r;
	char	*oname;
	char	*arg;
	int	oline;

	arg = p;
	if (Load_level > 0) {
		if (strlen(p) + strlen(Source_dir) + 4 >= MAX_PATH_LEN) {
			error("load: path too long", NO_EXPR);
			return -1;
		}
		if (*p != '.' && *p != '/' && *p != '~')
			sprintf(Current_path, "%s/%s", Source_dir, p);
		else
			strcpy(Current_path, p);
		p = Current_path;
	}
	p = expand_path(p, Expanded_path);
	get_source_dir(p, Source_dir);
	strcat(p, ".l");
	if ((nfile = fopen(p, "r")) == NULL) {
		error("cannot open source file", NO_EXPR);
		Error.arg = arg;
		return -1;
	}
	Load_level = Load_level + 1;
	/* Save I/O state and redirect */
	r = Rejected;
	ofile = Input;
	Input = nfile;
	oline = Line;
	Line = 1;
	oname = Infile;
	Infile = p;
	read_eval_loop();
	Infile = oname;
	Line = oline;
	/* Restore previous I/O state */
	Rejected = r;
	Input = ofile;
	Load_level = Load_level - 1;
	fclose(nfile);
	if (Paren_level) error("unbalanced parentheses in loaded file",
				NO_EXPR);
	return 0;
}

int z_load(int n, int *pcf, int *pmode, int *pcbn) {
	int	m;
	char	buf[SYMBOL_LEN+1], *s;

	USE(pcf);
	USE(pmode);
	USE(pcbn);
	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	if (!symbolic(Car[m])) return error("load: got non-symbol", Car[m]);
	s = symbol_to_string(Car[m], buf, SYMBOL_LEN);
	if (s) {
		s = strdup(s);
		if (s == NULL) fatal("load: strdup() failed");
		load(s);
		free(s);
	}
	return S_true;
}

int z_stats(int n, int *pcf, int *pmode, int *pcbn) {
	int	m;
	char	buf[100];

	USE(pcf);
	USE(pmode);
	USE(pcbn);
	m = Cdr[n];
	if (m == NIL || Cdr[m] != NIL) return wrong_arg_count(n);
	reset_counter(&Allocations);
	reset_counter(&Reductions);
	reset_counter(&Collections);
	Stat_flag = 1;
	n = eval(Car[m]);
	Stat_flag = 0;
	n = alloc(n, NIL);
	save(n);
	Cdr[n] = alloc(NIL, NIL);
	cadr(n) = explode_string(counter_to_string(&Reductions, buf));
	cddr(n) = alloc(NIL, NIL);
	caddr(n) = explode_string(counter_to_string(&Allocations, buf));
	cdddr(n) = alloc(NIL, NIL);
	cadddr(n) = explode_string(counter_to_string(&Collections, buf));
	unsave(1);
	return n;
}

int z_trace(int n, int *pcf, int *pmode, int *pcbn) {
	int		m;
	static char	buf[SYMBOL_LEN], *s;

	USE(pcf);
	USE(pmode);
	USE(pcbn);
	m = Cdr[n];
	if (m == NIL) {
		Traced_fn = NIL;
		return S_true;
	}
	if (Cdr[m] != NIL) return wrong_arg_count(n);
	if (!symbolic(Car[m])) return error("trace: got non-symbol", Car[m]);
	s = symbol_to_string(Car[m], buf, SYMBOL_LEN);
	if (!s) return S_false;
	Traced_fn = find_symbol(s);
	return S_true;
}

/* If (CAR NP[0]) is a special form handler, run it. */
int special(int *np, int *pcf, int *pmode, int *pcbn) {
	int	n, y;
	int	(*op)(int, int *, int *, int *);

	n = np[0];
	y = Car[n];
	if (Error_flag) return 0;
	if (Car[y] == S_special || Car[y] == S_special_cbv) 
		op = Specials[cadr(y)];
	else if (symbolic(y) &&
		(cadr(y) == S_special ||
		 cadr(y) == S_special_cbv)
	)
		op = Specials[caddr(y)];
	else
		return 0;
	np[0] = (*op)(n, pcf, pmode, pcbn);
	return 1;
}

/*
 * Bind the arguments of a LAMBDA function.
 * For a lambda application N=((LAMBDA (X1 ... Xn) S [ENV]) Y1 ... Yn)
 * this includes the following steps for j in {1,...,n}:
 *	1) add Xj to Car[Bind_stack]
 *	2) save the value of Xj
 *	3) bind Xj to Yj
 */
void bind_args(int n, int name) {
	int	fa,	/* formal arg list */
		aa,	/* actual arg list */
		e;	/* term */
	int	env;	/* optional lexical environment */
	int	p;
	int	at;	/* atomic argument list flag */

	if (Error_flag) return;
	fa = cadar(n);
	at = symbolic(fa);
	aa = Cdr[n];
	p = cddar(n);
	e = Car[p];
	env = Cdr[p] != NIL ? cadr(p): NIL;
	bsave(NIL); /* names */
	while ((fa != NIL && aa != NIL) || at) {
		if (!at) {
			Car[Bind_stack] = alloc(Car[fa], Car[Bind_stack]);
			save(cdar(fa));
			cdar(fa) = Car[aa];
			fa = Cdr[fa];
			aa = Cdr[aa];
		}
		if (symbolic(fa)) {
			Car[Bind_stack] = alloc(fa, Car[Bind_stack]);
			save(Cdr[fa]);
			Cdr[fa] = aa;
			fa = NIL;
			aa = NIL;
			break;
		}
	}
	while (env != NIL) {
		p = Car[env];
		Car[Bind_stack] = alloc(Car[p], Car[Bind_stack]);
		save(cdar(p));
		cdar(p) = Cdr[p];
		env = Cdr[env];
	}
	if (fa != NIL || aa != NIL) {
		wrong_arg_count(n);
		n = NIL;
	}
	else {
		n = e;
	}
	save(Function_name);
	Function_name = name;
	save(Frame);
	Frame = Stack;
}

void print_trace(int n) {
	pr("+ ");
	pr("(");
	Quotedprint = 1;
	print(Traced_fn);
	while (1) {
		n = Cdr[n];
		if (n == NIL) break;
		pr(" ");
		print(Car[n]);
	}
	pr(")"); nl();
}

void eliminate_tail_calls(void) {
	int	m, y;

	m = Car[Mode_stack];
	/* Skip over callee's local frames, if any */
	while (m != NIL && Car[m] == MLETR) {
		m = Cdr[m];
	}
	/* Parent not beta-reducing? Give up. */
	if (m == NIL || Car[m] != MBETA)
		return;
	/* Yes, this is a tail call: */
	/* remove callee's frames. */
	while (1) {
		Tmp2 = unsave(1); /* M */
		unbind_args();
		unsave(1);
		y = munsave();
		save(Tmp2);
		Tmp2 = NIL;
		if (y == MBETA) break;
	}
}

/* Evaluate the term N and return its normal form. */
int eval(int n) {
	int	m,	/* Result node */
		m2,	/* Root of result lists */
		a;	/* Used to append to result */
	int	mode,	/* Current state */
		cf,	/* Continue flag */
		cbn;	/* Call by name flag */
	int	nm;	/* Name of function to apply */

	Eval_level = Eval_level + 1;
	save(n);
	save(Arg_stack);
	save(Bind_stack);
	save(Car[Mode_stack]);
	save(Stack_bottom);
	Stack_bottom = Stack;
	mode = MATOM;
	cf = 0;
	cbn = 0;
	while (!Error_flag) {
		if (Stat_flag) count(&Reductions, 1);
		if (n == NIL) {			/* () -> () */
			m = NIL;
			cbn = 0;
		}
		else if (symbolic(n)) {		/* Symbol -> Value */
			if (cbn) {
				m = n;
				cbn = 0;
			}
			else {
				m = Cdr[n] == Car[n]? n: Cdr[n];
				if (m == S_void) {
					error("symbol not bound", n);
					break;
				}
			}
		}
		else if (Car[n] == S_closure ||
			Car[n] == S_primitive ||
			Car[n] == S_special ||
			Car[n] == S_special_cbv ||
			cbn == 2
		) {
			m = n;
			cbn = 0;
		}
		else {				/* List (...) and Pair (X.Y) */
			/*
			 * This block is used to descend into lists.
			 * The following nodes/variables will be saved:
			 *	1) the original list (on Stack)
			 *	2) the current state (on Mode_stack)
			 *	3) the root of the result list (on Arg_stack)
			 *	4) a ptr to the next free node
			 *	   in the result list (on Arg_stack)
			 *	5) a ptr to the next member of
			 *	   the original list (on Arg_stack)
			 */
			m = Car[n];
			save(n);
			msave(mode);
			if ((symbolic(m) && cadr(m) == S_special) || cbn) {
				cbn = 0;
				asave(NIL);
				asave(NIL);
				asave(n);	/* Root of result list */
				n = NIL;
			}
			else {
				a = alloc(NIL, NIL);
				asave(a);
				asave(Cdr[n]);
				asave(a);	/* Root of result list */
				n = Car[n];
			}
			mode = MLIST;
			continue;
		}
		/*
		 * The following loop is used to ascend back to the
		 * root of a list, thereby performing BETA reduction
		 * and creating result lists.
		 */
		while (1) if (mode == MBETA || mode == MLETR) {
			/* Finish BETA reduction */
			unbind_args();
			unsave(1);
			mode = munsave();
		}
		else if (mode == MLIST) {
			n = cadr(Arg_stack);	/* Next member */
			a = caddr(Arg_stack);	/* Place to append to */
			m2 = aunsave(1);	/* Root of result list */
			if (a != NIL) Car[a] = m;
			if (n == NIL) {		/* End of list */
				m = m2;
				aunsave(2);	/* Drop N,A */
				nm = Car[unsave(1)];
				save(m);	/* Save result */
				if (Traced_fn == nm) print_trace(m);
				if (primitive(&m))
					;
				else if (special(&m, &cf, &mode, &cbn))
					n = m;
				else if (!atomic(Car[m]) &&
					caar(m) == S_closure
				) {
					nm = symbolic(nm)? nm: NIL;
					eliminate_tail_calls();
					bind_args(m, nm);
					/* N=E of ((LAMBDA (...) E) ...) */
					n = caddar(m);
					cf = 2;
					mode = MBETA;
				}
				else {
					error("application of non-function",
						nm);
					n = NIL;
				}
				if (cf != 2) {
					unsave(1);
					mode = munsave();
				}
				/* Leave the list loop and re-evaluate N */
				if (cf) break;
			}
			else {		/* N =/= NIL: Append to list */
				asave(m2);
				Cdr[a] = alloc(NIL, NIL);
				caddr(Arg_stack) = Cdr[a];
				cadr(Arg_stack) = Cdr[n];
				if (symbolic(n))
					error("improper list in application",
						n);
				n = Car[n];	/* Evaluate next member */
				break;
			}
		}
		else if (mode == MCOND) {
			n = cond_eval_clause(m);
			if (Car[Bind_stack] == NIL) {
				unsave(1);
				bunsave(1);
				mode = munsave();
			}
			cf = 1;
			break;
		}
		else if (mode == MCONJ || mode == MDISJ) {
			Car[Bind_stack] = cdar(Bind_stack);
			if (	(m == S_false && mode == MCONJ) || 
				(m != S_false && mode == MDISJ) ||
				Car[Bind_stack] == NIL
			) {
				unsave(1);
				bunsave(1);
				mode = munsave();
				n = m;
				cbn = 2;
			}
			else if (cdar(Bind_stack) == NIL) {
				n = caar(Bind_stack);
				unsave(1);
				bunsave(1);
				mode = munsave();
			}
			else {
				n = caar(Bind_stack);
			}
			cf = 1;
			break;
		}
		else if (mode == MBIND || mode == MBINR) {
			if (let_next_binding(m) == NIL) {
				n = let_finish(mode == MBINR);
				mode = MLETR;
			}
			else {
				n = let_eval_arg();
			}
			cf = 1;
			break;
		}
		else {	/* Atom */
			break;
		}
		if (cf) {	/* Continue evaluation if requested */
			cf = 0;
			continue;
		}
		if (Stack == Stack_bottom) break;
	}
	while (Stack != Stack_bottom) unsave(1);
	Stack_bottom = unsave(1);
	Car[Mode_stack] = unsave(1);
	Bind_stack = unsave(1);
	Arg_stack = unsave(1);
	unsave(1);
	Eval_level = Eval_level - 1;
	return m;
}

/* Print (QUOTE X) as 'X */
int print_quoted_form(int n, int dot) {
	if (	Car[n] == S_quote &&
		Cdr[n] != NIL &&
		cddr(n) == NIL
	) {
		if (dot) pr(" . ");
		n = cadr(n);
		if (n != S_true && n != S_false) pr("'");
		print(n);
		return 1;
	}
	return 0;
}

int print_condensed_list(int n, int dot) {
	int	m;
	char	s[2];

	m = n;
	if (m == NIL) return 0;
	while (m != NIL) {
		if (!symbolic(Car[m])) return 0;
		if (cdaar(m) != NIL) return 0;
		m = Cdr[m];
	}
	if (dot) pr(" . ");
	pr("#");
	m = n;
	s[1] = 0;
	while (m != NIL) {
		s[0] = caaar(m);
		pr(s);
		m = Cdr[m];
	}
	return 1;
}

int print_closure(int n, int dot) {
	if (	Car[n] == S_closure &&
		!atomic(Cdr[n]) &&
		!atomic(cddr(n))
	) {
		Quotedprint = 1;
		if (dot) pr(" . ");
		pr(Closure_form==2? "(closure ": "{closure ");
		print(cadr(n));
		if (Closure_form > 0) {
			pr(" ");
			print(caddr(n));
			if (Closure_form > 1 && cdddr(n) != NIL) {
				pr(" ");
				print(cadddr(n));
			}
		}
		pr(Closure_form==2? ")": "}");
		return 1;
	}
	return 0;
}

int print_primitive(int n, int dot) {
	if (	Car[n] != S_primitive &&
		Car[n] != S_special &&
		Car[n] != S_special_cbv
	)
		return 0;
	if (dot) pr(" . ");
	pr("{internal ");
	Quotedprint = 1;
	print(cddr(n));
	pr("}");
	return 1;
}

void print(int n) {
	char	s[SYMBOL_LEN+1];
	int	i;

	if (n == NIL) {
		pr("()");
	}
	else if (n == S_void) {
		pr("{void}");
	}
	else if (Tag[n] & ATOM_FLAG) {
		/* Characters are limited to the symbol table */
		pr("{unprintable form}");
	}
	else if (symbolic(n)) {
		if (!Quotedprint && n != S_true && n != S_false) {
			pr("'");
			Quotedprint = 1;
		}
		i = 0;		/* Symbol */
		n = Car[n];
		while (n != NIL) {
			s[i] = Car[n];
			if (i > SYMBOL_LEN-2) break;
			i += 1;
			n = Cdr[n];
		}
		s[i] = 0;
		pr(s);
	}
	else {	/* List */
		if (print_closure(n, 0)) return;
		if (print_primitive(n, 0)) return;
		if (!Quotedprint) {
			pr("'");
			Quotedprint = 1;
		}
		if (print_quoted_form(n, 0)) return;
		if (print_condensed_list(n, 0)) return;
		pr("(");
		while (n != NIL) {
			print(Car[n]);
			n = Cdr[n];
			if (symbolic(n) || n == S_void) {
				pr(" . ");
				print(n);
				n = NIL;
			}
			if (print_closure(n, 1)) break;
			if (print_primitive(n, 1)) break;
			if (print_quoted_form(n, 1)) break;
			if (n != NIL) pr(" ");
		}
		pr(")");
	}
}

void reset_state(void) {
	Stack = NIL;
	Arg_stack = NIL;
	Bind_stack = NIL;
	Env_stack = NIL;
	Frame = NIL;
	Function_name = NIL;
	Eval_level = 0;
	Paren_level = 0;
}

/* Initialize interpreter variables. */
void init1() {
	/* Misc. variables */
	reset_state();
	Mode_stack = NIL;
	Error_flag = 0;
	Error.arg = NULL;
	Fatal_flag = 0;
	Symbols = NIL;
	Safe_symbols = NIL;
	Tmp_car = NIL;
	Tmp_cdr = NIL;
	Tmp = NIL;
	Tmp2 = NIL;
	Load_level = 0;
	Traced_fn = NIL;
	Max_atoms_used = 0;
	Max_trace = 10;
	Stat_flag = 0;
	Closure_form = 0;
	Verify_arrows = 0;
	Line = 1;
	/* Initialize Freelist */
	Freelist = NIL;
	/* Clear input buffer */
	Infile = NULL;
	Source_dir[0] = 0;
	Input = stdin;
	Output = stdout;
	Rejected = EOT;
}

/*
 * Second stage of initialization:
 * build the free list,
 * create built-in symbols.
 */
void init2(void) {
	/* 
	 * Create builtin symbols.
	 * Tags (especially 'primitive and 'special*)
	 * must be defined before any primitives.
	 * First GC will be triggered HERE
	 */
	S_void = add_symbol("{void}", 0);
	S_special = add_symbol("{special}", 0);
	S_special_cbv = add_symbol("{special/cbv}", 0);
	S_primitive = add_symbol("{primitive}", 0);
	S_closure = add_symbol("closure", 0);
	add_primitive("atom", P_ATOM);
	add_special("and", SF_AND, 0);
	add_special("apply", SF_APPLY, 1);
	S_bottom = add_primitive("bottom", P_BOTTOM);
	add_primitive("car", P_CAR);
	add_primitive("cdr", P_CDR);
	add_special("closure-form", SF_CLOSURE_FORM, 0);
	add_special("cond", SF_COND, 0);
	add_primitive("cons", P_CONS);
	add_special("define", SF_DEFINE, 0);
	add_primitive("defined", P_DEFINED);
	add_special("dump-image", SF_DUMP_IMAGE, 0);
	add_special("eval", SF_EVAL, 1);
	add_primitive("eq", P_EQ);
	add_primitive("explode", P_EXPLODE);
	S_false = add_symbol(":f", 0);
	add_primitive("gc", P_GC);
	add_primitive("implode", P_IMPLODE);
	S_lambda = add_special("lambda", SF_LAMBDA, 0);
	add_special("let", SF_LET, 0);
	add_special("letrec", SF_LETREC, 0);
	add_special("load", SF_LOAD, 0);
	add_special("or", SF_OR, 0);
	add_primitive("quit", P_QUIT);
	S_quote = add_special("quote", SF_QUOTE, 0);
	add_primitive("recursive-bind", P_RECURSIVE_BIND);
	add_special("stats", SF_STATS, 0);
	add_primitive("symbols", P_SYMBOLS);
	S_true = add_symbol(":t", 0);
	add_symbol("t", S_true);
	add_special("trace", SF_TRACE, 0);
	add_primitive("verify-arrows", P_VERIFY_ARROWS);
	S_last = add_symbol("**", 0);
	Mode_stack = alloc(NIL, NIL);
	Primitives[P_ATOM] = &z_atom;
	Primitives[P_BOTTOM] = &z_bottom;
	Primitives[P_CAR] = &z_car;
	Primitives[P_CDR] = &z_cdr;
	Primitives[P_CONS] = &z_cons;
	Primitives[P_DEFINED] = &z_defined;
	Primitives[P_EQ] = &z_eq;
	Primitives[P_EXPLODE] = &z_explode;
	Primitives[P_GC] = &z_gc;
	Primitives[P_IMPLODE] = &z_implode;
	Primitives[P_QUIT] = &z_quit;
	Primitives[P_RECURSIVE_BIND] = &z_recursive_bind;
	Primitives[P_SYMBOLS] = &z_symbols;
	Primitives[P_VERIFY_ARROWS] = &z_verify_arrows;
	Specials[SF_AND] = &z_and;
	Specials[SF_APPLY] = &z_apply;
	Specials[SF_CLOSURE_FORM] = &z_closure_form;
	Specials[SF_COND] = &z_cond;
	Specials[SF_DEFINE] = &z_define;
	Specials[SF_DUMP_IMAGE] = &z_dump_image;
	Specials[SF_EVAL] = &z_eval;
	Specials[SF_LAMBDA] = &z_lambda;
	Specials[SF_LET] = &z_let;
	Specials[SF_LETREC] = &z_letrec;
	Specials[SF_LOAD] = &z_load;
	Specials[SF_OR] = &z_or;
	Specials[SF_QUOTE] = &z_quote;
	Specials[SF_STATS] = &z_stats;
	Specials[SF_TRACE] = &z_trace;
}

void clear_stats(void) {
	reset_counter(&Reductions);
	reset_counter(&Allocations);
	reset_counter(&Collections);
}

int zen_load_image(char *p) {
	int	fd, n, i;
	char	buf[17];
	int	**v;
	int	bad = 0;
	int	inodes;

	fd = open(p, O_RDONLY);
	setmode(fd, O_BINARY);
	if (fd < 0) {
		error("cannot open image", NO_EXPR);
		Error.arg = p;
		return -1;
	}
	memset(Tag, 0, Pool_size);
	read(fd, buf, 16);
	if (memcmp(buf, "ZEN____", 7)) {
		error("bad image (magic match failed)", NO_EXPR);
		bad = 1;
	}
	if (buf[7] != sizeof(int)) {
		error("bad image (wrong cell size)", NO_EXPR);
		bad = 1;
	}
	if (buf[8] != VERSION) {
		error("bad image (wrong version)", NO_EXPR);
		bad = 1;
	}
	memcpy(&n, &buf[10], sizeof(int));
	if (n != 0x12345678) {
		error("bad image (wrong architecture)", NO_EXPR);
		bad = 1;
	}
	read(fd, &inodes, sizeof(int));
	if (inodes > Pool_size) {
		error("bad image (too many nodes)", NO_EXPR);
		bad = 1;
	}
	v = Image_vars;
	i = 0;
	while (v[i]) {
		read(fd, v[i], sizeof(int));
		i = i+1;
	}
	if (	!bad &&
		(read(fd, Car, inodes*sizeof(int)) != inodes*sizeof(int) ||
		 read(fd, Cdr, inodes*sizeof(int)) != inodes*sizeof(int) ||
		 read(fd, Tag, inodes) != inodes)
	) {
		error("bad image (bad file size)", NO_EXPR);
		bad = 1;
	}
	close(fd);
	if (bad) Error.arg = p;
	return Error_flag;
}

int zen_init(int nodes, int vgc) {
	Pool_size = nodes? nodes: DEFAULT_NODES;
	Verbose_GC = vgc;
	if (Pool_size < MINIMUM_NODES) return -1;
	if (	(Car = (int *) malloc(Pool_size * sizeof(int))) == NULL ||
		(Cdr = (int *) malloc(Pool_size * sizeof(int))) == NULL ||
		(Tag = (char *) malloc(Pool_size)) == NULL
	) {
		if (Car) free(Car);
		if (Cdr) free(Cdr);
		if (Tag) free(Tag);
		Car = Cdr = NULL;
		Tag = NULL;
		return -1;
	}
	memset(Tag, 0, Pool_size);
	init1();
	init2();
	return 0;
}

void zen_fini() {
	if (Car) free(Car);
	if (Cdr) free(Cdr);
	if (Tag) free(Tag);
	Car = Cdr = NULL;
	Tag = NULL;
}

void zen_stop(void) {
	error("interrupted", NO_EXPR);
}

void zen_print(int n) {
	Quotedprint = 0;
	print(n);
}

int zen_read(void) {
	Paren_level = 0;
	return zread();
}

int copy_bindings(void) {
	int	y, p, ny, q;

	p = alloc(NIL, NIL);
	save(p);
	ny = p;
	q = NIL;
	y = Symbols;
	while (y != NIL) {
		Car[p] = alloc(Car[y], cdar(y));
		y = Cdr[y];
		Cdr[p] = alloc(NIL, NIL);
		q = p;
		p = Cdr[p];
	}
	if (q != NIL) Cdr[q] = NIL;
	unsave(1);
	return Car[ny] == NIL? NIL: ny;
}

void restore_bindings(int values) {
	int	b;

	while (values != NIL) {
		b = Car[values];
		cdar(b) = Cdr[b];
		values = Cdr[values];
	}
}

/* Safely evaluate an expression. */
int zen_eval(int n) {
	save(n);
	Safe_symbols = copy_bindings();
	if (Stat_flag) clear_stats();
	n = eval(n);
	unsave(1);
	if (!Error_flag) {
		Cdr[S_last] = n;
		if (Stack != NIL)
			fatal("eval(): unbalanced stack");
	}
	else {
		restore_bindings(Safe_symbols);
	}
	reset_state();
	while (Car[Mode_stack] != NIL) munsave();
	return n;
}

char **zen_license() {
	static char	*license_text[] = {
"",
"zenlisp -- An interpreter for symbolic LISP",
"By Nils M Holm, 2007, 2008, 2013",
"",
"Don't worry, be happy.",
"",
"THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND",
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE",
"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE",
"ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE",
"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL",
"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS",
"OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)",
"HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT",
"LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY",
"OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF",
"SUCH DAMAGE.",
"",
	NULL};
	return license_text;
}

void read_eval_loop(void) {
	int	n, evl;

	Error_flag = 0;
	evl = Eval_level;
	Eval_level = 0;
	while(!Error_flag) {
		n = zen_read();
		if (n == EOT) break;
		n = eval(n);
	}
	Eval_level = evl;
}

#include <signal.h>

char	Image[MAX_PATH_LEN];
int	Nodes;
int	Batch;
int	GC_stats;

void usage(void) {
	fprintf(stderr,
		"Usage: zl [-L] [-bgi] [-n nodes] [image]\n");
}

int get_opt_val(int argc, char **argv, int *pi, int *pj, int *pk) {
	int	n, c;

	if (++(*pi) >= argc) {
		usage();
		exit(1);
	}
	n = atoi(argv[*pi]);
	c = argv[*pi][strlen(argv[*pi])-1];
	switch (c) {
	case 'K':	n = n * 1024; break;
	case 'M':	n = n * 1024 * 1024; break;
	}
	*pj = *pk = 0;
	return n;
}

void help(void) {
	fputc('\n', stderr);
	usage();
	fprintf(stderr,
		"\n"
		"-b    batch mode (quiet, exit on first error)\n"
		"-g    report number of free nodes after each GC\n"
		"-i    init mode (do not load any image)\n"
		"-n #  number of nodes to allocate (default: %dK)\n"
		"-L    print license and exit\n"
		"\n"
		"default image: %s\n\n",
		DEFAULT_NODES/1024, DEFAULT_IMAGE);
}

void print_license(void) {
	char	**s;

	s = zen_license();
	while (*s) {
		printf("%s\n", *s);
		s++;
	}
	exit(0);
}

void get_options(int argc, char **argv) {
	char	*a;
	int	i, j, k;
	int	v;

	strncpy(Image, DEFAULT_IMAGE, strlen(DEFAULT_IMAGE));
	Image[MAX_PATH_LEN-1] = 0;
	Nodes = DEFAULT_NODES;
	GC_stats = 0;
	Batch = 0;
	v = 0;
	i = 1;
	while (i < argc) {
		a = argv[i];
		if (a[0] != '-') break;
		k = strlen(a);
		for (j=1; j<k; j++) {
			switch (a[j]) {
			case 'b':
				Batch = 1;
				break;
			case 'n':
				Nodes = get_opt_val(argc, argv, &i, &j, &k);
				break;
			case 'g':
				GC_stats = 1;
				break;
			case 'i':
				Image[0] = 0;
				break;
			case 'L':
				print_license();
				break;
			case '?':
			case 'h':
				help();
				exit(1);
				break;
			default:
				usage();
				exit(1);
			}
		}
		i = i+1;
	}
	if (i < argc) {
		strncpy(Image, a, strlen(a)+1);
		Image[MAX_PATH_LEN-1] = 0;
	}
	if (Nodes < MINIMUM_NODES) {
		fprintf(stderr, "zenlisp: minimal pool size is %d\n",
			MINIMUM_NODES);
		exit(1);
	}
}

void catch_int(int sig) {
	USE(sig);
	zen_stop();
	signal(SIGINT, catch_int);
}

void repl(void) {
	int	n;

	while(1) {
		Error_flag = 0;
		n = zen_read();
		if (n == EOT) return;
		if (Error_flag) {
			zen_print_error();
			if (Batch) exit(1);
			continue;
		}
		n = zen_eval(n);
		if (Error_flag) {
			zen_print_error();
			if (Batch) exit(1);
		}
		else {
			if (!Batch) pr("=> ");
			zen_print(n);
			nl();
		}
	}
}

void init(void) {
	if (zen_init(Nodes, GC_stats)) {
		fprintf(stderr, "zenlisp init failed (memory problem)\n");
		exit(1);
	}
}

int main(int argc, char **argv) {
	get_options(argc, argv);
	init();
	get_options(argc, argv);
	if (!Batch) {
		pr("zenlisp ");
		pr(RELEASE);
		pr(" by Nils M Holm");
		nl();
	}
	if (Image[0]) {
		if (zen_load_image(Image)) {
			zen_print_error();
			if (Batch) exit(1);
			zen_fini();
			init();
			get_options(argc, argv);
		}
	}
	else if (!Batch) {
		pr("Warning: no image loaded");
		nl();
	}
	signal(SIGINT, catch_int);
	repl();
	zen_fini();
	return 0;
}
