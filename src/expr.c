/**
 * @file expr.c
 * @author Zach Peltzer
 * @date Created: Mon, 05 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#include <stdlib.h>

#include "expr.h"

/* These funcitons simplify low-level expressions, if possible.
 * They are used in expr_eval() and in expr_alloc() to simplify expressions at a
 * single depth.
 * If evaluation is possible, the operand(s) are freed and the result is stored
 * in @p res; if not, none of the parameters are modified or freed.
 *
 * @param[out] res Place to store the result.
 * @param op1 First operand.
 * @param op2 Second operand.
 * @return 0 if evaluation was possible, -1 if not.
 */
static int expr_add(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_sub(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_mul(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_div(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_mod(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_and(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_xor(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_or(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2);
static int expr_neg(struct expr_node *res, struct expr_node *op);
static int expr_not(struct expr_node *res, struct expr_node *op);

/**
 * Attemps to resolve a symbol into a constant value, incorporating both the
 * symbol and the addend.
 * @param expr Expression to resolve.
 * @return 0 if the expression is resolved, -1 if not. If the expression was not
 * a symbol to begin with, this still returns 0, as the expression is not
 * unresolved. This also applies to operators: this will return 0 even if there
 * are child expressions which are unresolved symbols.
 */
static int expr_resolve_sym(struct expr_node *expr);

struct expr_node *expr_alloc(enum expr_type type,
        struct expr_node *op1, struct expr_node *op2) {
    /* TODO Check the type? */
    struct expr_node *expr = malloc(sizeof(*expr));
    if (!expr) {
        return NULL;
    }

    /* Evaluate non-recursively to prevent simple expressions from taking up
     * more memory than necessary (i.e. ones only involving constants).
     */
    /* TODO Share a function with expr_eval(). */
    switch (expr->type) {
    case '+':
        if (expr_add(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case '-':
        if (expr_sub(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case '*':
        if (expr_mul(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case '/':
        if (expr_div(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case '%':
        if (expr_mod(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case '&':
        if (expr_and(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case '^':
        if (expr_xor(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case '|':
        if (expr_or(expr, op1, op2) == 0) {
            return 0;
        }
        break;

    case ET_NEG:
        if (expr_neg(expr, op1) == 0) {
            return 0;
        }
        break;

    case '~':
        if (expr_not(expr, op1) == 0) {
            return 0;
        }
        break;

    default:
        /* TODO Error out? */
        break;
    }

    /* If the expression is not a simple evaluation, store it as a normal node
     */
    expr->type = type;
    expr->operands[0] = op1;
    expr->operands[1] = op2;
    return expr;
}

struct expr_node *expr_alloc_const(enum section sec, int value) {
    struct expr_node *expr = malloc(sizeof(*expr));
    if (!expr) {
        return NULL;
    }

    expr->type = ET_CONST;
    expr->sec = sec;
    expr->value = value;
    return expr;
}

struct expr_node *expr_alloc_sym(const struct symbol_ent *sym) {
    if (!sym) {
        return NULL;
    }

    struct expr_node *expr = malloc(sizeof(*expr));
    if (!expr) {
        return NULL;
    }

    expr->type = ET_SYM;
    expr->sym = sym;
    expr->addend = 0;

    /* Try to resolve the symbol, but don't worry if it cannot be resolved. */
    expr_resolve_sym(expr);
    return expr;
}

struct expr_node *expr_clone(const struct expr_node *expr) {
    if (!expr) {
        return NULL;
    }

    if (EXPR_IS_OP(expr)) {
        return expr_alloc(expr->type, expr->operands[0], expr->operands[1]);
    } else if (expr->type == ET_SYM) {
        return expr_alloc_sym(expr->sym);
    } else if (expr->type == ET_CONST) {
        return expr_alloc_const(expr->sec, expr->value);
    } else {
        /* TODO Return invalid expression? */
        return NULL;
    }
}

void expr_free(struct expr_node *expr) {
    /* Only free operands if it should have any (just checking for NULL operands
     * does not work because the operand pointers are stored in a union).
     * EXPR_IS_OP() returns false for NULL, so this doubles as a NULL check.
     */
    if (EXPR_IS_OP(expr)) {
        expr_free(expr->operands[0]);
        expr_free(expr->operands[1]);
    }

    free(expr);
}

static int expr_resolve_sym(struct expr_node *expr) {
    if (expr->type != ET_SYM) {
        return 0;
    }

    if (expr->sym->type != ST_OBJECT) {
        return -1;
    }

    /* Compute the value here because they are stored in a union with the
     * value and sec fields
     */
    enum section sec = expr->sym->sec;
    int value = expr->sym->value + expr->addend;

    expr->type = ET_CONST;
    expr->sec = sec;
    expr->value = value;
    return 0;
}

static int expr_add(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    /* Symbols and constants can be added together in any configuration, as long
     * as at least one is absolute. expr_resolve_sym() is called on each operand
     * first, so that the case of SYM + SYM can be ruled out, as that means
     * neither symbol was resolvable.
     */

    expr_resolve_sym(op1);
    expr_resolve_sym(op2);

    if (op1->type == ET_CONST && op2->type == ET_CONST) {
        if (op1->sec != SEC_ABS && op2->sec != SEC_ABS) {
            return -1;
        }

        res->type = ET_CONST;
        res->sec = op1->sec & op2->sec;
        res->value = op1->value + op2->value;
    } else if (op1->type == ET_CONST && op2->type == ET_SYM) {
        if (op1->sec != SEC_ABS) {
            return -1;
        }

        res->type = ET_SYM;
        res->sym = op2->sym;
        res->addend = op2->addend + op1->value;
    } else if (op1->type == ET_SYM && op2->type == ET_CONST) {
        if (op2->sec != SEC_ABS) {
            return -1;
        }

        res->type = ET_SYM;
        res->sym = op1->sym;
        res->addend = op1->addend + op2->value;
    } else {
        return -1;
    }

    expr_free(op1);
    expr_free(op2);
    return 0;
}

static int expr_sub(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    /* Symbols and constants can be added as long as the subtrahend (fancy,
     * old-school term for the second one) is absolute. We try to resolve
     * symbols first so that the only condition that needs to be checked is that
     * op2 is an absolute constant (and that op1 is a symbol or constant).
     */

    expr_resolve_sym(op1);
    expr_resolve_sym(op2);

    if (!EXPR_IS_ABS(op2)) {
        return -1;
    }

    if (op1->type == ET_CONST) {
        res->type = ET_CONST;
        res->sec = op1->sec;
        res->value = op1->value - op2->value;
    } else if (op1->type == ET_SYM) {
        res->type = ET_SYM;
        res->sym = op1->sym;
        res->addend = op1->addend - op2->value;
    } else {
        return -1;
    }

    expr_free(op1);
    expr_free(op2);
    return 0;
}

/* For all other operations, both operands must be absolute constants */

static int expr_mul(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    expr_resolve_sym(op1);
    expr_resolve_sym(op2);
    if (EXPR_IS_ABS(op1) && EXPR_IS_ABS(op2)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = op1->value * op2->value;
    expr_free(op1);
    expr_free(op2);
    return 0;
}

static int expr_div(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    expr_resolve_sym(op1);
    expr_resolve_sym(op2);
    if (EXPR_IS_ABS(op1) && EXPR_IS_ABS(op2)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = op1->value / op2->value;
    expr_free(op1);
    expr_free(op2);
    return 0;
}

static int expr_mod(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    expr_resolve_sym(op1);
    expr_resolve_sym(op2);
    if (EXPR_IS_ABS(op1) && EXPR_IS_ABS(op2)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = op1->value % op2->value;
    expr_free(op1);
    expr_free(op2);
    return 0;
}

static int expr_and(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    expr_resolve_sym(op1);
    expr_resolve_sym(op2);
    if (EXPR_IS_ABS(op1) && EXPR_IS_ABS(op2)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = op1->value & op2->value;
    expr_free(op1);
    expr_free(op2);
    return 0;
}

static int expr_xor(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    expr_resolve_sym(op1);
    expr_resolve_sym(op2);
    if (EXPR_IS_ABS(op1) && EXPR_IS_ABS(op2)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = op1->value ^ op2->value;
    expr_free(op1);
    expr_free(op2);
    return 0;
}

static int expr_or(struct expr_node *res,
        struct expr_node *op1, struct expr_node *op2) {
    expr_resolve_sym(op1);
    expr_resolve_sym(op2);
    if (EXPR_IS_ABS(op1) && EXPR_IS_ABS(op2)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = op1->value | op2->value;
    expr_free(op1);
    expr_free(op2);
    return 0;
}

static int expr_neg(struct expr_node *res, struct expr_node *op) {
    expr_resolve_sym(op);
    if (EXPR_IS_ABS(op)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = -op->value;
    expr_free(op);
    return 0;
}

static int expr_not(struct expr_node *res, struct expr_node *op) {
    expr_resolve_sym(op);
    if (EXPR_IS_ABS(op)) {
        return -1;
    }

    res->type = ET_CONST;
    res->sec = SEC_ABS;
    res->value = ~op->value;
    expr_free(op);
    return 0;
}

int expr_eval(struct expr_node *expr) {
    /* Used to pass the error message to the EXPR_INVAL label when there is an
     * error in evaluation.
     */
    const char *msg = NULL;
    struct expr_node *op1, *op2;

    if (!expr) {
        return -1;
    }

    /* Test for leaf nodes first */
    switch (expr->type) {
    case ET_CONST:
        return 0;
    case ET_SYM:
        /* Try to resolve the symbol, but still return it if the symbol cannot
         * be resolved.
         */
        expr_resolve_sym(expr);
        return 0;
    case ET_INVAL:
        return -1;
    default:
        break;
    }

    op1 = expr->operands[0];
    if (expr_eval(op1) < 0) {
        msg = op1->msg;
        goto EXPR_INVAL;
    }

    op2 = expr->operands[1];
    if (expr_eval(op2) < 0) {
        msg = op2->msg;
        goto EXPR_INVAL;
    }

    /* Both operands were successfully evaluated, so both are ET_CONSTs. */
    /* TODO Improve qualiity of error messages, probably by returning them from
     * expr_*() partial evaluation functions.
     */
    switch (expr->type) {
    case '+':
        if (expr_add(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not add operands";
            break;
        }

    case '-':
        if (expr_sub(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not subtract operands";
            break;
        }

    case '*':
        if (expr_mul(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not multiply operands";
            break;
        }

    case '/':
        if (expr_div(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not divide operands";
            break;
        }

    case '%':
        if (expr_mod(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not modulo operands";
            break;
        }

    case '&':
        if (expr_and(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not AND operands";
            break;
        }

    case '^':
        if (expr_xor(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not XOR operands";
            break;
        }

    case '|':
        if (expr_or(expr, op1, op2) == 0) {
            return 0;
        } else {
            msg = "Could not OR operands";
            break;
        }

    case ET_NEG:
        if (expr_neg(expr, op1) == 0) {
            return 0;
        } else {
            msg = "Could not negate operand";
            break;
        }

    case '~':
        if (expr_not(expr, op1) == 0) {
            return 0;
        } else {
            msg = "Could not complement operand";
            break;
        }

    default:
        msg = "Invalid expression type";
        break;
    }

EXPR_INVAL:
    expr->type = ET_INVAL;
    expr_free(expr->operands[0]);
    expr_free(expr->operands[1]);
    expr->msg = msg;
    return -1;
}

/* vim: set tw=80 ft=c: */
