/**
 * @file expr.h
 * @author Zach Peltzer
 * @date Created: Mon, 05 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#ifndef EXPR_H_
#define EXPR_H_

#include "symbol_table.h"

#define EXPR_IS_OP(e) ((e) && (e)->type > ET_OP_START)
#define EXPR_IS_ABS(e) ((e) && (e)->type == ET_CONST && (e)->sec == SEC_ABS)

/**
 * Possible types of an expression.
 * These values align with the token values and character symbols, when
 * applicable.
 */
enum expr_type {
    /* TODO Have an error message in the expression if this is the type */
    ET_INVAL = 0,

    /**
     * Numeric constant, relative to a section.
     *
     */
    ET_CONST = 1,

    /**
     * Symbol.
     */
    ET_SYM = 3,

    ET_OP_START,

    ET_ADD = '+',
    ET_SUB = '-',
    ET_MUL = '*',
    ET_DIV = '/',
    ET_MOD = '%',

    ET_AND = '&',
    ET_XOR = '^',
    ET_OR = '|',

    ET_NOT = '~',
    ET_NEG,
};

/**
 * Structure for storing an expression as a tree for later evaluation (for when
 * there are unresolved symbols).
 */
struct expr_node {
    enum expr_type type;

    union {
        /**
         * If type is ET_CONST, a section and value of the constant.
         * For equates and literal values, the section is SEC_ABS; for labels,
         * the value is an offset into the section.
         */
        struct {
            enum section sec;
            int value;
        };

        /**
         * If type is ET_SYM, a symbol and an addend to the symbol. This is used
         * for unresolved and global symbols.
         */
        struct {
            const struct symbol_ent *sym;
            int addend;
        };

        /**
         * If the expression is an operator, its operands.
         */
        struct expr_node *operands[2];

        /**
         * If type is ET_INVAL, an error message describing why the expression
         * was invalid.
         */
        const char *msg;
    };
};

/**
 * Creates/allocates an expression node from its operands and type.
 * For unary operator types, the second operand (@p op2) should be NULL.
 * This allocates a new node and assumes that the operands are also dynamically
 * allocated and will not be freed by external code. When expr_free() is called
 * on a head expression node, all of the children will be freed as well; this
 * means that child nodes should not be freed before their head nodes.
 *
 * @param type Type of the node to create.
 * @param op1 First operand.
 * @param op2 Second operand.
 * @return Newly allocated expression node, or NULL if there is an error.
 */
struct expr_node *expr_alloc(enum expr_type type,
        struct expr_node *op1, struct expr_node *op2);

/**
 * Creates/allocates an expression node representing a offset into a section.
 * @param sec Section of the value.
 * @param offset Offset into section @p sec.
 * @return Newly allocated expression node, or NULL if there is an error.
 */
struct expr_node *expr_alloc_const(enum section sec, int offset);

/**
 * Creates/allocates an expression node representing an entry in the symbol
 * table.
 * @param sym Symbol this expression points to.
 * @return Newly allocated expression node, or NULL if there is an error.
 */
struct expr_node *expr_alloc_sym(const struct symbol_ent *sym);

/**
 * Makes a deep clone of an expression node.
 * @param expr Expression to clone.
 * @return Clone of @p expr.
 */
struct expr_node *expr_clone(const struct expr_node *expr);

/**
 * Frees an expression node and all of its children.
 * @param expr Expression to free.
 */
void expr_free(struct expr_node *expr);

/**
 * Attempts to evaluate an expression.
 * If the evaluation results in an error (e.g. subtracting symbols from
 * different, non-absolute sections), the expression type will be set to
 * ET_INVAL and all of its children will be freed.
 * If evaluation was successful, @p expr will be of type ET_CONST and all of its
 * children (if it had any) will be freed.
 * @param expr Expression to evaluate.
 * @return 0 if the expression could be fully evaluated, -1 if not.
 */
int expr_eval(struct expr_node *expr);

#endif /* EXPR_H_ */

/* vim: set tw=80 ft=c: */
