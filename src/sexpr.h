// building lisp - https://github.com/lwhjp/building-lisp
// Copyright (C) 2021 Leo Uino

#ifndef FAR_HORIZONS_SEXPR_H
#define FAR_HORIZONS_SEXPR_H


typedef enum {
    Error_OK = 0,
    Error_Syntax,
    Error_Unbound,
    Error_Args,
    Error_Type
} sexpr_error_t;


// forward declaration for builtin type
struct sexpr_atom;

typedef int (*Builtin)(struct sexpr_atom args, struct sexpr_atom *result);


typedef struct sexpr_atom {
    enum {
        sexpr_atomtype_nil,
        sexpr_atomtype_pair,
        sexpr_atomtype_symbol,
        sexpr_atomtype_integer,
        sexpr_atomtype_builtin,
        sexpr_atomtype_closure,
        sexpr_atomtype_macro
    } type;

    union {
        Builtin builtin;
        long integer;
        struct sexpr_pair *pair;
        const char *symbol;
    } value;
} sexpr_atom_t;


typedef struct sexpr_pair {
    struct sexpr_atom atom[2];
} sexpr_pair_t;




#define car(p) ((p).value.pair->atom[0])
#define cdr(p) ((p).value.pair->atom[1])
#define nilp(atom) ((atom).type == sexpr_atomtype_nil)


int read_expr(const char *input, const char **end, sexpr_atom_t *result);

void print_expr(sexpr_atom_t atom);

sexpr_atom_t cons(sexpr_atom_t car_val, sexpr_atom_t cdr_val);

sexpr_atom_t env_create(sexpr_atom_t parent);

int env_define(sexpr_atom_t env, sexpr_atom_t symbol, sexpr_atom_t value);

int env_get(sexpr_atom_t env, sexpr_atom_t symbol, sexpr_atom_t *result);

int env_set(sexpr_atom_t env, sexpr_atom_t symbol, sexpr_atom_t value);

int eval_expr(sexpr_atom_t expr, sexpr_atom_t env, sexpr_atom_t *result);

void gc();

sexpr_atom_t make_builtin(Builtin fn);

sexpr_atom_t make_int(long x);

sexpr_atom_t make_sym(const char *s);

int sexprCommand(int argc, char **argv);


#endif //FAR_HORIZONS_SEXPR_H
