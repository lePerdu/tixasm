/**
 * @file z80.y
 * @author Zach Peltzer
 * @date Created: Sat, 03 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
*/

%{
#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "opcode.h"
#include "tixasm.h"

extern int yylineno;
FILE *yyout;

int yylex(void);
void yyerror(char *);

int instr_match_and_output(const struct opcode *oc,
        struct operand *op1, struct operand *op2);
%}

%union {
    int i;
    char *str;
    struct {
        int len;
        uint8_t *data;
    } buf;
    struct expr_node *expr;
    struct operand op;
    const struct opcode *oc;
    const struct symbol_ent *sym;
}

%token T_EOL T_ERROR

%token T_TEXT T_DATA T_ABS T_ORG T_DB T_DW T_FILL T_EQU T_DEFINE T_UNDEFINE

%token <i> T_LITERAL
%token <str> T_STRING
/* TODO Split up directives like registers are? */
%token <oc> T_OPCODE
%token <sym> T_SYMBOL T_LABEL T_LLABEL T_FLABEL

%token <i> T_A T_B T_C T_D T_E T_F T_H T_L T_IXH T_IXL T_IYH T_IYL T_I T_R
%token <i> T_AF T_BC T_DE T_HL T_SP T_IX T_IY T_sAF
%token <i> T_fNZ T_fZ T_fNC T_fC T_fPO T_fPE T_fP T_fM

%nonassoc '#'
%nonassoc T_EQ T_NE '<' '>' T_LE T_GE

%left T_LOR
%left T_LAND

%left '|'
%left '^'
%left '&'

%left '+' '-'
%left '*' '/' '%'

%right '~' '!' UNARY

%type <sym> label

%type <expr> expr_top
%type <expr> expr
%type <op> instr_operand

%type <op> register
%type <op> register_8
%type <op> register_16
%type <op> register_16_nonidx
%type <op> register_16_idx
%type <op> register_shadow
%type <op> register_indir
%type <op> register_idx_indir
%type <op> flag

%%

program: line
       | program T_EOL line
       ;

line:
    | label
    | label instruction
    | instruction
    | directive
    ;

label: T_LABEL
     | T_LLABEL
     | T_FLABEL
     ;

directive: T_TEXT                   { asm_set_sec(SEC_TEXT); }
         | T_DATA                   { asm_set_sec(SEC_DATA); }
         | T_ABS                    { asm_set_sec(SEC_ABS); }
         | T_ORG expr {
                /* TODO Allow setting inside a section? */
                asm_set_pc_expr($2);
            }
         | T_DB db_operand_list
         | T_DW dw_operand_list
         | T_FILL expr {
                if (!EXPR_IS_ABS($2)) {
                    fprintf(stderr, "ORG address must be defined.\n");
                }

                int len = $2->value;
                asm_inc_pc(len);
                while (len --> 0) {
                    fputc(0, yyout);
                }
            }
         | T_FILL expr ',' expr {
                if (!EXPR_IS_ABS($2) || !EXPR_IS_ABS($4)) {
                    fprintf(stderr, "FILL values must be absolute");
                }

                int len = $2->value;
                int byte = $4->value & 0xFF;
                asm_inc_pc(len);
                while (len --> 0) {
                    fputc(byte, yyout);
                }
            }
         | T_EQU T_SYMBOL expr
         | T_DEFINE T_SYMBOL {
                symtab_add(asm_symbol_table, $2->name, ST_OBJECT, SEC_ABS, 1);
            }
         | T_UNDEFINE T_SYMBOL {
                symtab_add(asm_symbol_table, $2->name, ST_UNDEF, SEC_UNDEF, 0);
            }
         ;

db_operand: expr {
                reltab_add_expr(asm_reloc_table,
                        RT_8_BIT, asm_get_pc()->sec, asm_get_pc()->value, 0, $1);
                expr_free($1);
                fputc(0, yyout);
                asm_inc_pc(1);
            }
          | T_STRING {
                int len = strlen($1);
                fwrite($1, 1, len, yyout);
                asm_inc_pc(len);
                free($1);
            }
          ;

db_operand_list: db_operand
               | db_operand_list ',' db_operand
               ;

dw_operand: expr {
                reltab_add_expr(asm_reloc_table,
                        RT_16_BIT, asm_get_pc()->sec, asm_get_pc()->value, 0, $1);
                expr_free($1);
                fputc(0, yyout);
                asm_inc_pc(2);
            }
          ;

dw_operand_list: dw_operand
               | dw_operand_list ',' dw_operand
               ;

instruction: T_OPCODE
                { instr_match_and_output($1, 0, 0); }
           | T_OPCODE instr_operand
                { instr_match_and_output($1, &$2, 0); }
           | T_OPCODE instr_operand ',' instr_operand
                { instr_match_and_output($1, &$2, &$4); }
           ;

instr_operand: expr_top
                { $$.type = OP_IMM; $$.expr = $1; }
             | '(' expr ')'
                { $$.type = OP_EXT; $$.expr = $2; }
             | register
                { $$ = $1; }
             | register_indir
                { $$ = $1; }
             | register_idx_indir
                { $$ = $1; }
             | flag
                { $$ = $1; }
             ;

register_8: T_A     { $$.type = OP_A; }
          | T_B     { $$.type = OP_B; }
          | T_C     { $$.type = OP_C; }
          | T_D     { $$.type = OP_D; }
          | T_E     { $$.type = OP_E; }
          | T_F     { $$.type = OP_F; }
          | T_H     { $$.type = OP_H; }
          | T_L     { $$.type = OP_L; }
          | T_IXH   { $$.type = OP_IXH; }
          | T_IXL   { $$.type = OP_IXL; }
          | T_IYH   { $$.type = OP_IYH; }
          | T_IYL   { $$.type = OP_IYL; }
          | T_I     { $$.type = OP_I; }
          | T_R     { $$.type = OP_R; }
          ;

register_16_nonidx: T_AF    { $$.type = OP_AF; }
                  | T_BC    { $$.type = OP_BC; }
                  | T_DE    { $$.type = OP_DE; }
                  | T_HL    { $$.type = OP_HL; }
                  | T_SP    { $$.type = OP_SP; }
                  ;

register_16_idx: T_IX   { $$.type = OP_IX; }
               | T_IY   { $$.type = OP_IY; }
               ;

register_16: register_16_nonidx | register_16_idx
           ;

register_shadow: T_sAF  { $$.type = OP_sAF; }
               ;

register: register_8 | register_16 | register_shadow
        ;

register_indir: '(' register_16_nonidx ')'
                { $$.type = op_type_indir($2.type); }
              | '(' T_A ')'
                { $$.type = OP_iA; }
              | '(' T_C ')'
                { $$.type = OP_iC; }
              ;

register_idx_indir: '(' register_16_idx ')'
                      { $$.type = op_type_indir($2.type);
                        $$.expr = expr_alloc_const(SEC_ABS, 0); }
                  | '(' register_16_idx '+' expr ')'
                      { $$.type = op_type_indir($2.type);
                        $$.expr = $4; }
                  | '(' register_16_idx '-' expr ')'
                      { $$.type = op_type_indir($2.type);
                        $$.expr = expr_alloc(ET_NEG, $4, NULL); }
                  | '(' expr '+' register_16_idx ')'
                      { $$.type = op_type_indir($4.type);
                        $$.expr = $2; }
                  | '(' expr '+' register_16_idx '+' expr ')'
                      { $$.type = op_type_indir($4.type);
                        $$.expr = expr_alloc('+', $2, $6); }
                  | '(' expr '+' register_16_idx '-' expr ')'
                      { $$.type = op_type_indir($4.type);
                        $$.expr = expr_alloc('-', $2, $6); }
                  ;

flag: T_fNZ { $$.type = OP_fNZ; }
    | T_fZ  { $$.type = OP_fZ; }
    | T_fNC { $$.type = OP_fNC; }
    | T_fC  { $$.type = OP_fC; }
    | T_fPO { $$.type = OP_fPO; }
    | T_fPE { $$.type = OP_fPE; }
    | T_fP  { $$.type = OP_fP; }
    | T_fM  { $$.type = OP_fM; }
    ;

expr_top: '$'                   { $$ = expr_clone(asm_get_pc()); }
        | T_LITERAL             { $$ = expr_alloc_const(SEC_ABS, $1); }
        | T_SYMBOL              { $$ = expr_alloc_sym($1); }
        | '+' expr              { $$ = $2; }
        | '-' expr %prec UNARY  { $$ = expr_alloc(ET_NEG, $2, NULL); }
        | '~' expr              { $$ = expr_alloc('~', $2, NULL); }
        | expr '*' expr         { $$ = expr_alloc('*', $1, $3); }
        | expr '/' expr         { $$ = expr_alloc('/', $1, $3); }             
        | expr '%' expr         { $$ = expr_alloc('%', $1, $3); }
        | expr '+' expr         { $$ = expr_alloc('+', $1, $3); }
        | expr '-' expr         { $$ = expr_alloc('-', $1, $3); }
        | expr '&' expr         { $$ = expr_alloc('&', $1, $3); }
        | expr '^' expr         { $$ = expr_alloc('^', $1, $3); }
        | expr '|' expr         { $$ = expr_alloc('|', $1, $3); }
        ;

expr: expr_top      { $$ = $1; }
    | '(' expr ')'  { $$ = $2; }
    ;

%%

int instr_match_and_output(const struct opcode *oc,
        struct operand *op1, struct operand *op2) {
    const struct instruction *instr = opcode_match(oc, op1, op2);
    int ret;

    if (instr) {
        ret = instr_output(instr, op1, op2, yyout);
    } else {
        yyerror("Undefined instruction");
        ret = -1;
    }

    /* The expressions can be freed now since the operands go out of scope after
     * this function is called in the instruction rule.
     */
    operand_free_expr(op1);
    operand_free_expr(op2);

    return ret;
}

/* vim: set tw=80 ft=yacc: */
