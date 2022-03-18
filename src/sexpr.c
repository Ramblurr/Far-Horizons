// building lisp - https://github.com/lwhjp/building-lisp
// Copyright (C) 2021 Leo Uino


#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>


#include "sexpr.h"


typedef struct sexpr_allocation {
    sexpr_pair_t pair;
    int mark: 1;
    struct sexpr_allocation *next;
} sexpr_allocation_t;


static const sexpr_atom_t nil = {sexpr_atomtype_nil};

static sexpr_atom_t sym_table = {sexpr_atomtype_nil};

static struct sexpr_allocation *global_allocations = NULL;


int builtin_car(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_cdr(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_cons(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_eq(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_pairp(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_procp(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_add(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_subtract(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_multiply(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_divide(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_numeq(sexpr_atom_t args, sexpr_atom_t *result);

int builtin_less(sexpr_atom_t args, sexpr_atom_t *result);

sexpr_atom_t copy_list(sexpr_atom_t list);

char *dos2unix(char *buf);

int eval_do_apply(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env, sexpr_atom_t *result);

int eval_do_bind(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env);

int eval_do_exec(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env);

int eval_do_return(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env, sexpr_atom_t *result);

int eval_expr(sexpr_atom_t expr, sexpr_atom_t env, sexpr_atom_t *result);

void gc_mark(sexpr_atom_t root);

sexpr_atom_t list_create(int n, ...);

sexpr_atom_t list_get(sexpr_atom_t list, int k);

void list_reverse(sexpr_atom_t *list);

void list_set(sexpr_atom_t list, int k, sexpr_atom_t value);

int listp(sexpr_atom_t expr);

void load_expr(sexpr_atom_t env, const char *path);

int make_closure(sexpr_atom_t env, sexpr_atom_t args, sexpr_atom_t body, sexpr_atom_t *result);

sexpr_atom_t make_frame(sexpr_atom_t parent, sexpr_atom_t env, sexpr_atom_t tail);

int read_expr(const char *input, const char **end, sexpr_atom_t *result);

int read_list(const char *start, const char **end, sexpr_atom_t *result);

char *slurp(const char *path);


int builtin_add(sexpr_atom_t args, sexpr_atom_t *result) {
    sexpr_atom_t a, b;

    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    a = car(args);
    b = car(cdr(args));

    if (a.type != sexpr_atomtype_integer || b.type != sexpr_atomtype_integer) {
        return Error_Type;
    }

    *result = make_int(a.value.integer + b.value.integer);

    return Error_OK;
}


int builtin_car(sexpr_atom_t args, sexpr_atom_t *result) {
    if (nilp(args) || !nilp(cdr(args))) {
        return Error_Args;
    }

    if (car(args).type == sexpr_atomtype_pair) {
        *result = car(car(args));
    } else {
        *result = nil;
    }

    return Error_OK;
}


int builtin_cdr(sexpr_atom_t args, sexpr_atom_t *result) {
    if (nilp(args) || !nilp(cdr(args))) {
        return Error_Args;
    }

    if (car(args).type == sexpr_atomtype_pair) {
        *result = cdr(car(args));
    } else {
        *result = nil;
    }

    return Error_OK;
}


int builtin_cons(sexpr_atom_t args, sexpr_atom_t *result) {
    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    *result = cons(car(args), car(cdr(args)));

    return Error_OK;
}


int builtin_divide(sexpr_atom_t args, sexpr_atom_t *result) {
    sexpr_atom_t a, b;

    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    a = car(args);
    b = car(cdr(args));

    if (a.type != sexpr_atomtype_integer || b.type != sexpr_atomtype_integer) {
        return Error_Type;
    }

    *result = make_int(a.value.integer / b.value.integer);

    return Error_OK;
}


int builtin_eq(sexpr_atom_t args, sexpr_atom_t *result) {
    sexpr_atom_t a, b;
    int eq;

    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    a = car(args);
    b = car(cdr(args));

    if (a.type == b.type) {
        switch (a.type) {
            case sexpr_atomtype_nil:
                eq = 1;
                break;
            case sexpr_atomtype_pair:
            case sexpr_atomtype_closure:
            case sexpr_atomtype_macro:
                eq = (a.value.pair == b.value.pair);
                break;
            case sexpr_atomtype_symbol:
                eq = (a.value.symbol == b.value.symbol);
                break;
            case sexpr_atomtype_integer:
                eq = (a.value.integer == b.value.integer);
                break;
            case sexpr_atomtype_builtin:
                eq = (a.value.builtin == b.value.builtin);
                break;
        }
    } else {
        eq = 0;
    }

    *result = eq ? make_sym("T") : nil;
    return Error_OK;
}


int builtin_less(sexpr_atom_t args, sexpr_atom_t *result) {
    sexpr_atom_t a, b;

    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    a = car(args);
    b = car(cdr(args));

    if (a.type != sexpr_atomtype_integer || b.type != sexpr_atomtype_integer) {
        return Error_Type;
    }

    *result = (a.value.integer < b.value.integer) ? make_sym("T") : nil;

    return Error_OK;
}


int builtin_multiply(sexpr_atom_t args, sexpr_atom_t *result) {
    sexpr_atom_t a, b;

    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    a = car(args);
    b = car(cdr(args));

    if (a.type != sexpr_atomtype_integer || b.type != sexpr_atomtype_integer) {
        return Error_Type;
    }

    *result = make_int(a.value.integer * b.value.integer);

    return Error_OK;
}


int builtin_numeq(sexpr_atom_t args, sexpr_atom_t *result) {
    sexpr_atom_t a, b;

    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    a = car(args);
    b = car(cdr(args));

    if (a.type != sexpr_atomtype_integer || b.type != sexpr_atomtype_integer) {
        return Error_Type;
    }

    *result = (a.value.integer == b.value.integer) ? make_sym("T") : nil;

    return Error_OK;
}


int builtin_pairp(sexpr_atom_t args, sexpr_atom_t *result) {
    if (nilp(args) || !nilp(cdr(args))) {
        return Error_Args;
    }

    *result = (car(args).type == sexpr_atomtype_pair) ? make_sym("T") : nil;
    return Error_OK;
}


int builtin_procp(sexpr_atom_t args, sexpr_atom_t *result) {
    if (nilp(args) || !nilp(cdr(args))) {
        return Error_Args;
    }

    *result = (car(args).type == sexpr_atomtype_builtin
               || car(args).type == sexpr_atomtype_closure) ? make_sym("T") : nil;
    return Error_OK;
}


int builtin_subtract(sexpr_atom_t args, sexpr_atom_t *result) {
    sexpr_atom_t a, b;

    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_Args;
    }

    a = car(args);
    b = car(cdr(args));

    if (a.type != sexpr_atomtype_integer || b.type != sexpr_atomtype_integer) {
        return Error_Type;
    }

    *result = make_int(a.value.integer - b.value.integer);

    return Error_OK;
}


sexpr_atom_t cons(sexpr_atom_t car_val, sexpr_atom_t cdr_val) {
    sexpr_allocation_t *a;
    sexpr_atom_t p;

    a = calloc(sizeof(sexpr_allocation_t), 1);
    a->mark = 0;
    a->next = global_allocations;
    global_allocations = a;

    p.type = sexpr_atomtype_pair;
    p.value.pair = &a->pair;

    car(p) = car_val;
    cdr(p) = cdr_val;

    return p;
}


sexpr_atom_t copy_list(sexpr_atom_t list) {
    sexpr_atom_t a, p;

    if (nilp(list)) {
        return nil;
    }

    a = cons(car(list), nil);
    p = a;
    list = cdr(list);

    while (!nilp(list)) {
        cdr(p) = cons(car(list), nil);
        p = cdr(p);
        list = cdr(list);
    }

    return a;
}


// mdhender - added some ugly code to convert LF + CR to CR
char *dos2unix(char *buf) {
    if (buf != NULL) {
        char *src = buf;
        for (; *src && *src != '\r'; src++) {
            //
        }
        if (*src == '\r') {
            char *dst = src;
            src++; // skip this LF
            for (; *src; src++) {
                if (*src == '\r') {
                    continue;
                }
                *dst = *src;
                dst++;
            }
            *dst = 0;
        }
    }
    return buf;
}


sexpr_atom_t env_create(sexpr_atom_t parent) {
    return cons(parent, nil);
}


int env_define(sexpr_atom_t env, sexpr_atom_t symbol, sexpr_atom_t value) {
    sexpr_atom_t bs = cdr(env);

    while (!nilp(bs)) {
        sexpr_atom_t b = car(bs);
        if (car(b).value.symbol == symbol.value.symbol) {
            cdr(b) = value;
            return Error_OK;
        }
        bs = cdr(bs);
    }

    cdr(env) = cons(cons(symbol, value), cdr(env));

    return Error_OK;
}


int env_get(sexpr_atom_t env, sexpr_atom_t symbol, sexpr_atom_t *result) {
    sexpr_atom_t parent = car(env);
    sexpr_atom_t bs = cdr(env);

    while (!nilp(bs)) {
        sexpr_atom_t b = car(bs);
        if (car(b).value.symbol == symbol.value.symbol) {
            *result = cdr(b);
            return Error_OK;
        }
        bs = cdr(bs);
    }

    if (nilp(parent)) {
        return Error_Unbound;
    }

    return env_get(parent, symbol, result);
}


int env_set(sexpr_atom_t env, sexpr_atom_t symbol, sexpr_atom_t value) {
    sexpr_atom_t parent = car(env);
    sexpr_atom_t bs = cdr(env);

    while (!nilp(bs)) {
        sexpr_atom_t b = car(bs);
        if (car(b).value.symbol == symbol.value.symbol) {
            cdr(b) = value;
            return Error_OK;
        }
        bs = cdr(bs);
    }

    if (nilp(parent)) {
        return Error_Unbound;
    }

    return env_set(parent, symbol, value);
}


int eval_do_apply(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env, sexpr_atom_t *result) {
    sexpr_atom_t op, args;

    op = list_get(*stack, 2);
    args = list_get(*stack, 4);

    if (!nilp(args)) {
        list_reverse(&args);
        list_set(*stack, 4, args);
    }

    if (op.type == sexpr_atomtype_symbol) {
        if (strcmp(op.value.symbol, "APPLY") == 0) {
            /* Replace the current frame */
            *stack = car(*stack);
            *stack = make_frame(*stack, *env, nil);
            op = car(args);
            args = car(cdr(args));
            if (!listp(args)) {
                return Error_Syntax;
            }

            list_set(*stack, 2, op);
            list_set(*stack, 4, args);
        }
    }

    if (op.type == sexpr_atomtype_builtin) {
        *stack = car(*stack);
        *expr = cons(op, args);
        return Error_OK;
    } else if (op.type != sexpr_atomtype_closure) {
        return Error_Type;
    }

    return eval_do_bind(stack, expr, env);
}


int eval_do_bind(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env) {
    sexpr_atom_t op, args, arg_names, body;

    body = list_get(*stack, 5);
    if (!nilp(body)) {
        return eval_do_exec(stack, expr, env);
    }

    op = list_get(*stack, 2);
    args = list_get(*stack, 4);

    *env = env_create(car(op));
    arg_names = car(cdr(op));
    body = cdr(cdr(op));
    list_set(*stack, 1, *env);
    list_set(*stack, 5, body);

    /* Bind the arguments */
    while (!nilp(arg_names)) {
        if (arg_names.type == sexpr_atomtype_symbol) {
            env_define(*env, arg_names, args);
            args = nil;
            break;
        }

        if (nilp(args)) {
            return Error_Args;
        }
        env_define(*env, car(arg_names), car(args));
        arg_names = cdr(arg_names);
        args = cdr(args);
    }
    if (!nilp(args)) {
        return Error_Args;
    }

    list_set(*stack, 4, nil);

    return eval_do_exec(stack, expr, env);
}


int eval_do_exec(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env) {
    sexpr_atom_t body;

    *env = list_get(*stack, 1);
    body = list_get(*stack, 5);
    *expr = car(body);
    body = cdr(body);
    if (nilp(body)) {
        /* Finished function; pop the stack */
        *stack = car(*stack);
    } else {
        list_set(*stack, 5, body);
    }

    return Error_OK;
}


int eval_do_return(sexpr_atom_t *stack, sexpr_atom_t *expr, sexpr_atom_t *env, sexpr_atom_t *result) {
    sexpr_atom_t op, args, body;

    *env = list_get(*stack, 1);
    op = list_get(*stack, 2);
    body = list_get(*stack, 5);

    if (!nilp(body)) {
        /* Still running a procedure; ignore the result */
        return eval_do_apply(stack, expr, env, result);
    }

    if (nilp(op)) {
        /* Finished evaluating operator */
        op = *result;
        list_set(*stack, 2, op);

        if (op.type == sexpr_atomtype_macro) {
            /* Don't evaluate macro arguments */
            args = list_get(*stack, 3);
            *stack = make_frame(*stack, *env, nil);
            op.type = sexpr_atomtype_closure;
            list_set(*stack, 2, op);
            list_set(*stack, 4, args);
            return eval_do_bind(stack, expr, env);
        }
    } else if (op.type == sexpr_atomtype_symbol) {
        /* Finished working on special form */
        if (strcmp(op.value.symbol, "DEFINE") == 0) {
            sexpr_atom_t sym = list_get(*stack, 4);
            (void) env_define(*env, sym, *result);
            *stack = car(*stack);
            *expr = cons(make_sym("QUOTE"), cons(sym, nil));
            return Error_OK;
        } else if (strcmp(op.value.symbol, "SET!") == 0) {
            sexpr_atom_t sym = list_get(*stack, 4);
            *stack = car(*stack);
            *expr = cons(make_sym("QUOTE"), cons(sym, nil));
            return env_set(*env, sym, *result);
        } else if (strcmp(op.value.symbol, "IF") == 0) {
            args = list_get(*stack, 3);
            *expr = nilp(*result) ? car(cdr(args)) : car(args);
            *stack = car(*stack);
            return Error_OK;
        } else {
            goto store_arg;
        }
    } else if (op.type == sexpr_atomtype_macro) {
        /* Finished evaluating macro */
        *expr = *result;
        *stack = car(*stack);
        return Error_OK;
    } else {
        store_arg:
        /* Store evaluated argument */
        args = list_get(*stack, 4);
        list_set(*stack, 4, cons(*result, args));
    }

    args = list_get(*stack, 3);
    if (nilp(args)) {
        /* No more arguments left to evaluate */
        return eval_do_apply(stack, expr, env, result);
    }

    /* Evaluate next argument */
    *expr = car(args);
    list_set(*stack, 3, cdr(args));
    return Error_OK;
}


int eval_expr(sexpr_atom_t expr, sexpr_atom_t env, sexpr_atom_t *result) {
    static int count = 0;
    sexpr_error_t err = Error_OK;
    sexpr_atom_t stack = nil;

    do {
        if (++count == 100000) {
            gc_mark(expr);
            gc_mark(env);
            gc_mark(stack);
            gc();
            count = 0;
        }

        if (expr.type == sexpr_atomtype_symbol) {
            err = env_get(env, expr, result);
        } else if (expr.type != sexpr_atomtype_pair) {
            *result = expr;
        } else if (!listp(expr)) {
            return Error_Syntax;
        } else {
            sexpr_atom_t op = car(expr);
            sexpr_atom_t args = cdr(expr);

            if (op.type == sexpr_atomtype_symbol) {
                /* Handle special forms */

                if (strcmp(op.value.symbol, "QUOTE") == 0) {
                    if (nilp(args) || !nilp(cdr(args))) {
                        return Error_Args;
                    }

                    *result = car(args);
                } else if (strcmp(op.value.symbol, "DEFINE") == 0) {
                    sexpr_atom_t sym;

                    if (nilp(args) || nilp(cdr(args))) {
                        return Error_Args;
                    }

                    sym = car(args);
                    if (sym.type == sexpr_atomtype_pair) {
                        err = make_closure(env, cdr(sym), cdr(args), result);
                        sym = car(sym);
                        if (sym.type != sexpr_atomtype_symbol) {
                            return Error_Type;
                        }
                        (void) env_define(env, sym, *result);
                        *result = sym;
                    } else if (sym.type == sexpr_atomtype_symbol) {
                        if (!nilp(cdr(cdr(args)))) {
                            return Error_Args;
                        }
                        stack = make_frame(stack, env, nil);
                        list_set(stack, 2, op);
                        list_set(stack, 4, sym);
                        expr = car(cdr(args));
                        continue;
                    } else {
                        return Error_Type;
                    }
                } else if (strcmp(op.value.symbol, "LAMBDA") == 0) {
                    if (nilp(args) || nilp(cdr(args))) {
                        return Error_Args;
                    }

                    err = make_closure(env, car(args), cdr(args), result);
                } else if (strcmp(op.value.symbol, "IF") == 0) {
                    if (nilp(args) || nilp(cdr(args)) || nilp(cdr(cdr(args)))
                        || !nilp(cdr(cdr(cdr(args))))) {
                        return Error_Args;
                    }

                    stack = make_frame(stack, env, cdr(args));
                    list_set(stack, 2, op);
                    expr = car(args);
                    continue;
                } else if (strcmp(op.value.symbol, "DEFMACRO") == 0) {
                    sexpr_atom_t name, macro;

                    if (nilp(args) || nilp(cdr(args))) {
                        return Error_Args;
                    }

                    if (car(args).type != sexpr_atomtype_pair) {
                        return Error_Syntax;
                    }

                    name = car(car(args));
                    if (name.type != sexpr_atomtype_symbol) {
                        return Error_Type;
                    }

                    err = make_closure(env, cdr(car(args)),
                                       cdr(args), &macro);
                    if (!err) {
                        macro.type = sexpr_atomtype_macro;
                        *result = name;
                        (void) env_define(env, name, macro);
                    }
                } else if (strcmp(op.value.symbol, "APPLY") == 0) {
                    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
                        return Error_Args;
                    }

                    stack = make_frame(stack, env, cdr(args));
                    list_set(stack, 2, op);
                    expr = car(args);
                    continue;
                } else if (strcmp(op.value.symbol, "SET!") == 0) {
                    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
                        return Error_Args;
                    }
                    if (car(args).type != sexpr_atomtype_symbol) {
                        return Error_Type;
                    }
                    stack = make_frame(stack, env, nil);
                    list_set(stack, 2, op);
                    list_set(stack, 4, car(args));
                    expr = car(cdr(args));
                    continue;
                } else {
                    goto push;
                }
            } else if (op.type == sexpr_atomtype_builtin) {
                err = (*op.value.builtin)(args, result);
            } else {
                push:
                /* Handle function application */
                stack = make_frame(stack, env, args);
                expr = op;
                continue;
            }
        }

        if (nilp(stack)) {
            break;
        }

        if (!err) {
            err = eval_do_return(&stack, &expr, &env, result);
        }
    } while (!err);

    return err;
}


void gc() {
    sexpr_allocation_t *a, **p;

    gc_mark(sym_table);

    /* Free unmarked allocations */
    p = &global_allocations;
    while (*p != NULL) {
        a = *p;
        if (!a->mark) {
            *p = a->next;
            free(a);
        } else {
            p = &a->next;
        }
    }

    /* Clear marks */
    a = global_allocations;
    while (a != NULL) {
        a->mark = 0;
        a = a->next;
    }
}


void gc_mark(sexpr_atom_t root) {
    sexpr_allocation_t *a;

    if (!(root.type == sexpr_atomtype_pair || root.type == sexpr_atomtype_closure ||
          root.type == sexpr_atomtype_macro)) {
        return;
    }
    // offsetof is defined in stddef.h
    a = (sexpr_allocation_t *) ((char *) root.value.pair - offsetof(sexpr_allocation_t, pair));
    if (a->mark) {
        return;
    }
    a->mark = 1;
    gc_mark(car(root));
    gc_mark(cdr(root));
}


int lex(const char *str, const char **start, const char **end) {
    const char *ws = " \t\n";
    const char *delim = "(); \t\n";
    const char *prefix = "()\'`";

    str += strspn(str, ws);

    if (str[0] == '\0') {
        *start = *end = NULL;
        return Error_Syntax;
    }

    *start = str;

    if (strchr(prefix, str[0]) != NULL) {
        *end = str + 1;
    } else if (str[0] == ',') {
        *end = str + (str[1] == '@' ? 2 : 1);
    } else if (str[0] == ';') {
        str = strchr(str, '\n');
        if (!str) {
            *start = *end = NULL;
            return Error_Syntax;
        }
        return lex(str, start, end);
    } else {
        *end = str + strcspn(str, delim);
    }

    return Error_OK;
}


sexpr_atom_t list_create(int n, ...) {
    va_list ap;
    sexpr_atom_t list = nil;

    va_start(ap, n);
    while (n--) {
        sexpr_atom_t item = va_arg(ap, sexpr_atom_t);
        list = cons(item, list);
    }
    va_end(ap);

    list_reverse(&list);
    return list;
}


sexpr_atom_t list_get(sexpr_atom_t list, int k) {
    while (k--) {
        list = cdr(list);
    }
    return car(list);
}


void list_reverse(sexpr_atom_t *list) {
    sexpr_atom_t tail = nil;
    while (!nilp(*list)) {
        sexpr_atom_t p = cdr(*list);
        cdr(*list) = tail;
        tail = *list;
        *list = p;
    }
    *list = tail;
}


void list_set(sexpr_atom_t list, int k, sexpr_atom_t value) {
    while (k--) {
        list = cdr(list);
    }
    car(list) = value;
}


int listp(sexpr_atom_t expr) {
    while (!nilp(expr)) {
        if (expr.type != sexpr_atomtype_pair) {
            return 0;
        }
        expr = cdr(expr);
    }
    return 1;
}


void load_file(sexpr_atom_t env, const char *path) {
    char *text;

    printf("Reading %s...\n", path);
    text = slurp(path);
    if (text) {
        const char *p = text;
        sexpr_atom_t expr;
        while (read_expr(p, &p, &expr) == Error_OK) {
            sexpr_atom_t result;
            sexpr_error_t err = eval_expr(expr, env, &result);
            if (err) {
                printf("Error in expression:\n\t");
                print_expr(expr);
                putchar('\n');
            } else {
                print_expr(result);
                putchar('\n');
            }
        }
        free(text);
    }
}


sexpr_atom_t make_builtin(Builtin fn) {
    sexpr_atom_t a;
    a.type = sexpr_atomtype_builtin;
    a.value.builtin = fn;
    return a;
}


int make_closure(sexpr_atom_t env, sexpr_atom_t args, sexpr_atom_t body, sexpr_atom_t *result) {
    sexpr_atom_t p;

    if (!listp(body)) {
        return Error_Syntax;
    }

    /* Check argument names are all symbols */
    p = args;
    while (!nilp(p)) {
        if (p.type == sexpr_atomtype_symbol) {
            break;
        } else if (p.type != sexpr_atomtype_pair
                   || car(p).type != sexpr_atomtype_symbol) {
            return Error_Type;
        }
        p = cdr(p);
    }

    *result = cons(env, cons(args, body));
    result->type = sexpr_atomtype_closure;

    return Error_OK;
}


sexpr_atom_t make_frame(sexpr_atom_t parent, sexpr_atom_t env, sexpr_atom_t tail) {
    return cons(parent,
                cons(env,
                     cons(nil, /* op */
                          cons(tail,
                               cons(nil, /* args */
                                    cons(nil, /* body */
                                         nil))))));
}


sexpr_atom_t make_int(long x) {
    sexpr_atom_t a;
    a.type = sexpr_atomtype_integer;
    a.value.integer = x;
    return a;
}


sexpr_atom_t make_sym(const char *s) {
    sexpr_atom_t a, p;

    p = sym_table;
    while (!nilp(p)) {
        a = car(p);
        if (strcmp(a.value.symbol, s) == 0) {
            return a;
        }
        p = cdr(p);
    }

    a.type = sexpr_atomtype_symbol;
    a.value.symbol = strdup(s);
    sym_table = cons(a, sym_table);

    return a;
}


void print_expr(sexpr_atom_t atom) {
    switch (atom.type) {
        case sexpr_atomtype_nil:
            printf("NIL");
            break;
        case sexpr_atomtype_pair:
            putchar('(');
            print_expr(car(atom));
            atom = cdr(atom);
            while (!nilp(atom)) {
                if (atom.type == sexpr_atomtype_pair) {
                    putchar(' ');
                    print_expr(car(atom));
                    atom = cdr(atom);
                } else {
                    printf(" . ");
                    print_expr(atom);
                    break;
                }
            }
            putchar(')');
            break;
        case sexpr_atomtype_symbol:
            printf("%s", atom.value.symbol);
            break;
        case sexpr_atomtype_integer:
            printf("%ld", atom.value.integer);
            break;
        case sexpr_atomtype_builtin:
            printf("#<BUILTIN:%p>", atom.value.builtin);
            break;
        case sexpr_atomtype_closure:
            printf("#<CLOSURE:%p>", atom.value.pair);
            break;
        case sexpr_atomtype_macro:
            printf("#<MACRO:%p>", atom.value.pair);
            break;
    }
}


int parse_simple(const char *start, const char *end, sexpr_atom_t *result) {
    char *buf, *p;

    /* Is it an integer? */
    long val = strtol(start, &p, 10);
    if (p == end) {
        result->type = sexpr_atomtype_integer;
        result->value.integer = val;
        return Error_OK;
    }

    /* NIL or symbol */
    buf = calloc(sizeof(char), end - start + 1);
    p = buf;
    while (start != end) {
        *p++ = toupper(*start), ++start;
    }
    *p = '\0';

    if (strcmp(buf, "NIL") == 0) {
        *result = nil;
    } else {
        *result = make_sym(buf);
    }

    free(buf);

    return Error_OK;
}


int read_expr(const char *input, const char **end, sexpr_atom_t *result) {
    const char *token;
    sexpr_error_t err;

    err = lex(input, &token, end);
    if (err) {
        return err;
    }

    if (token[0] == '(') {
        return read_list(*end, end, result);
    } else if (token[0] == ')') {
        return Error_Syntax;
    } else if (token[0] == '\'') {
        *result = cons(make_sym("QUOTE"), cons(nil, nil));
        return read_expr(*end, end, &car(cdr(*result)));
    } else if (token[0] == '`') {
        *result = cons(make_sym("QUASIQUOTE"), cons(nil, nil));
        return read_expr(*end, end, &car(cdr(*result)));
    } else if (token[0] == ',') {
        *result = cons(make_sym(
                               token[1] == '@' ? "UNQUOTE-SPLICING" : "UNQUOTE"),
                       cons(nil, nil));
        return read_expr(*end, end, &car(cdr(*result)));
    } else {
        return parse_simple(token, *end, result);
    }
}


int read_list(const char *start, const char **end, sexpr_atom_t *result) {
    sexpr_atom_t p;

    *end = start;
    p = *result = nil;

    for (;;) {
        const char *token;
        sexpr_atom_t item;
        sexpr_error_t err;

        err = lex(*end, &token, end);
        if (err) {
            return err;
        }

        if (token[0] == ')') {
            return Error_OK;
        }

        if (token[0] == '.' && *end - token == 1) {
            /* Improper list */
            if (nilp(p)) {
                return Error_Syntax;
            }

            err = read_expr(*end, end, &item);
            if (err) {
                return err;
            }

            cdr(p) = item;

            /* Read the closing ')' */
            err = lex(*end, &token, end);
            if (!err && token[0] != ')') {
                err = Error_Syntax;
            }

            return err;
        }

        err = read_expr(token, end, &item);
        if (err) {
            return err;
        }

        if (nilp(p)) {
            /* First item */
            *result = cons(item, nil);
            p = *result;
        } else {
            cdr(p) = cons(item, nil);
            p = cdr(p);
        }
    }
}


int sexprCommand(int argc, char **argv) {
    sexpr_atom_t env = env_create(nil);

    /* Set up the initial environment */
    env_define(env, make_sym("CAR"), make_builtin(builtin_car));
    env_define(env, make_sym("CDR"), make_builtin(builtin_cdr));
    env_define(env, make_sym("CONS"), make_builtin(builtin_cons));
    env_define(env, make_sym("+"), make_builtin(builtin_add));
    env_define(env, make_sym("-"), make_builtin(builtin_subtract));
    env_define(env, make_sym("*"), make_builtin(builtin_multiply));
    env_define(env, make_sym("/"), make_builtin(builtin_divide));
    env_define(env, make_sym("T"), make_sym("T"));
    env_define(env, make_sym("="), make_builtin(builtin_numeq));
    env_define(env, make_sym("<"), make_builtin(builtin_less));
    env_define(env, make_sym("EQ?"), make_builtin(builtin_eq));
    env_define(env, make_sym("PAIR?"), make_builtin(builtin_pairp));
    env_define(env, make_sym("PROCEDURE?"), make_builtin(builtin_procp));

    load_file(env, "library.lisp");

    load_expr(env, "galaxy.txt");
    //load_expr(env, "stars.txt");
    //load_expr(env, "planets.txt");
    load_expr(env, "species001.txt");
    load_expr(env, "species002.txt");
    load_expr(env, "species003.txt");
    load_expr(env, "species004.txt");
    //load_expr(env, "locations.txt");


//    /* Main loop */
//    char *input;
//    while ((input = readline("> ")) != NULL) {
//        const char *p = input;
//        sexpr_error_t err;
//        sexpr_atom_t expr, result;
//
//        err = read_expr(p, &p, &expr);
//
//        if (!err)
//            err = eval_expr(expr, env, &result);
//
//        switch (err) {
//            case Error_OK:
//                print_expr(result);
//                putchar('\n');
//                break;
//            case Error_Syntax:
//                puts("Syntax error");
//                break;
//            case Error_Unbound:
//                puts("Symbol not bound");
//                break;
//            case Error_Args:
//                puts("Wrong number of arguments");
//                break;
//            case Error_Type:
//                puts("Wrong type");
//                break;
//        }
//
//        free(input);
//    }

    return 0;
}

// slurp reads in a file
char *slurp(const char *path) {
    struct stat sb;
    if (stat(path, &sb) == 0) {
        // file exists
        FILE *file = fopen(path, "rb");
        if (file != NULL) {
            char *buf = calloc(sizeof(char), sb.st_size + 1);
            if (buf != NULL) {
                fread(buf, 1, sb.st_size, file);
                fclose(file);
                return dos2unix(buf);
            }
        }
    }
    return NULL;
}


void load_expr(sexpr_atom_t env, const char *path) {
    printf("Reading expression from %s...\n", path);
    char *text = slurp(path);
    if (text) {
        const char *p = text;
        sexpr_atom_t expr;
        while (read_expr(p, &p, &expr) == Error_OK) {
            print_expr(expr);
            putchar('\n');
            putchar('\n');
        }
        free(text);
    }
}
