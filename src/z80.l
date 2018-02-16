/**
 * @file z80.l
 * @author Zach Peltzer
 * @date Created: Sat, 03 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
*/

%{
#include <stdlib.h>
#include <stdio.h>

#include "opcode.h"
#include "tixasm.h"

#include "z80.tab.h"
%}

BIN         [01]
OCT         [0-7]
DEC         [0-9]
HEX         [0-9a-fA-F]
ALPHAU      [a-z_]
ALNUMU      [0-9a-z_]
NALNUMU     [^0-9a-z_]
IDENT       [a-z_][0-9a-z_]*
WHITE       [ \t\n]

%s OPCODE
%s OPERAND
%s DIR_OP

%option caseless

%%

^[ \t]+     BEGIN(OPCODE);
[ \t]+      ;
;.*$        ;

"\n"    { yylineno++; BEGIN(INITIAL); return T_EOL; }
"\\"    { BEGIN(INITIAL); return T_EOL; }


<OPERAND,DIR_OP>"," |

<OPERAND,DIR_OP>"(" |
<OPERAND,DIR_OP>")" |

<OPERAND,DIR_OP>"#" |
<OPERAND,DIR_OP>"$" |

<OPERAND,DIR_OP>"+" |
<OPERAND,DIR_OP>"-" |
<OPERAND,DIR_OP>"*" |
<OPERAND,DIR_OP>"/" |
<OPERAND,DIR_OP>"%" |

<OPERAND,DIR_OP>"&" |
<OPERAND,DIR_OP>"|" |
<OPERAND,DIR_OP>"^" |

<OPERAND,DIR_OP>"<" |
<OPERAND,DIR_OP>">" return *yytext;

<OPERAND,DIR_OP>"=="    return T_EQ;
<OPERAND,DIR_OP>"!="    return T_NE;
<OPERAND,DIR_OP>">="    return T_LE;
<OPERAND,DIR_OP>"<="    return T_LE;

<OPERAND,DIR_OP>"&&"    return T_LAND;
<OPERAND,DIR_OP>"||"    return T_LOR;

<OPERAND,DIR_OP>\%{BIN}+/{NALNUMU}  {
    /* This is longer than just '%' so it has higher precedence */
    yylval.i = strtol(yytext+1, NULL, 2);
    return T_LITERAL;
}

<OPERAND,DIR_OP>{BIN}+b/{NALNUMU} {
    yylval.i = strtol(yytext, NULL, 2);
    return T_LITERAL;
}

<OPERAND,DIR_OP>0{OCT}+/{NALNUMU} {
    yylval.i = strtol(yytext, NULL, 8);
    return T_LITERAL;
}

<OPERAND,DIR_OP>{DEC}+/{NALNUMU} {
    yylval.i = strtol(yytext, NULL, 10);
    return T_LITERAL;
}

<OPERAND,DIR_OP>${HEX}+/{NALNUMU}    {
    yylval.i = strtol(yytext+1, NULL, 16);
    return T_LITERAL;
}

<OPERAND,DIR_OP>{HEX}+h/{NALNUMU} {
    yylval.i = strtol(yytext, NULL, 16);
    return T_LITERAL;
}

<OPERAND,DIR_OP>'[^'\\]' {
    yylval.i = yytext[1];
    return T_LITERAL;
}

<OPERAND,DIR_OP>'\\.' {
    int c = yytext[1];
    switch (c) {
    case '0':
        c = 0;
        break;
    case 'b':
        c = '\b';
        break;
    case 'f':
        c = '\f';
        break;
    case 'n':
        c  = '\n';
        break;
    case 'r':
        c = '\r';
        break;
    case 't':
        c = '\t';
        break;
    case 'v':
        c = '\v';
        break;
        /* For these, the character does not change */
    case '\\':
    case '\'':
        break;
    }

    yylval.i = c;
    return T_LITERAL;
}

<DIR_OP>\"([^\\\"]|\\.)*\" {
        /* Unescape the string */
        char *src = yytext + 1; /* Don't modify yytext */
        char *ptr;
        char *dst;
        int len;
        int c;

        yylval.str = malloc(yyleng + 1);
        dst = yylval.str;

        while ((ptr = strchr(src, '\\')) || (ptr = strchr(src, '"'))) {
            len = ptr - src;
            strncpy(dst, src, len);

            c = *++ptr;
            switch (c) {
            case '0':
                c = 0;
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c  = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'v':
                c = '\v';
                break;
            /* For these, the character does not change */
            case '\\':
            case '"':
                break;
            }

            dst += len;
            *dst = c;
            *++dst = 0;
            src = ++ptr;
        }

        return T_STRING;
}

<INITIAL,OPCODE>\.text     return T_TEXT;
<INITIAL,OPCODE>\.data     return T_DATA;
<INITIAL,OPCODE>\.abs      return T_ABS;
<INITIAL,OPCODE>\.org      BEGIN(DIR_OP); return T_ORG;
<INITIAL,OPCODE>\.db       BEGIN(DIR_OP); return T_DB;
<INITIAL,OPCODE>\.dw       BEGIN(DIR_OP); return T_DW;
<INITIAL,OPCODE>\.fill     BEGIN(DIR_OP); return T_FILL;
<INITIAL,OPCODE>\.equ      BEGIN(DIR_OP); return T_EQU;
<INITIAL,OPCODE>\.define   BEGIN(DIR_OP); return T_DEFINE;
<INITIAL,OPCODE>\.undefine BEGIN(DIR_OP); return T_UNDEFINE;

<INITIAL>_:  | /* TODO Implement local labels */
<INITIAL>{IDENT}:  {
    yylval.sym = symtab_add_len(asm_symbol_table,
            yytext, yyleng-1,
            ST_OBJECT, asm_get_pc()->sec, asm_get_pc()->value);
    if (yylval.sym) {
        return T_LABEL;
    }

    /* Symbol already exists */
    return 3;
}

<INITIAL>\.{IDENT} {
    /* TODO Use a different format that doesn't conflict with directive format
     * for these?
     */
    yylval.sym = symtab_add(asm_symbol_table, yytext+1,
            ST_OBJECT, asm_get_pc()->sec, asm_get_pc()->value);
    if (yylval.sym) {
        return T_LABEL;
    }

    /* Symbol already exists */
    return 4;
}

<INITIAL>{IDENT} {
    /* Only accept macros, not other symbols. */
    /* yylval is here, so there is no reason to declare another variable */
    yylval.sym = symtab_search(asm_symbol_table, yytext);
    if (yylval.sym && yylval.sym->type == ST_MACRO) {
        return T_ERROR;
    } else {
        return T_ERROR;
    }
}

<OPERAND>a       return T_A;
<OPERAND>b       return T_B;
<OPERAND>c       return T_C; /* The same token is used for the flag c. */
<OPERAND>d       return T_D;
<OPERAND>e       return T_E;
<OPERAND>f       return T_F;
<OPERAND>h       return T_H;
<OPERAND>l       return T_L;
<OPERAND>ixh     return T_IXH;
<OPERAND>ixl     return T_IXL;
<OPERAND>iyh     return T_IYH;
<OPERAND>iyl     return T_IYL;
<OPERAND>i       return T_I;
<OPERAND>r       return T_R;
<OPERAND>af      return T_AF;
<OPERAND>bc      return T_BC;
<OPERAND>de      return T_DE;
<OPERAND>hl      return T_HL;
<OPERAND>sp      return T_SP;
<OPERAND>ix      return T_IX;
<OPERAND>iy      return T_IY;
<OPERAND>af'     return T_sAF;

<OPERAND>nz  return T_fNZ;
<OPERAND>z   return T_fZ;
<OPERAND>nc  return T_fNC;
<OPERAND>po  return T_fPO;
<OPERAND>pe  return T_fPE;
<OPERAND>p   return T_fP;
<OPERAND>m   return T_fM;

<OPCODE>{IDENT} {
    yylval.oc = opcode_search(yytext);
    if (yylval.oc) {
        BEGIN(OPERAND);
        return T_OPCODE;
    }

    /* TODO Search macros */
    return 2;
}

<OPERAND,DIR_OP>{IDENT} {
    yylval.sym = symtab_search(asm_symbol_table, yytext);
    if (!yylval.sym) {
        /* Create a new, empty symbol */
        yylval.sym = symtab_add(asm_symbol_table, yytext,
            SEC_UNDEF, ST_UNDEF, 0);
        if (!yylval.sym) {
            /* Memorr error */
            return T_ERROR;
        }

        return T_SYMBOL;
    }

    /* TODO Expand the symbol if it is a macro */

   return T_SYMBOL;
}

.   {
    printf("Error: unknown token: %s\n", yytext);
    exit(EXIT_FAILURE);
}

%%

void yyerror(char *s) {
    fprintf(stderr, "Error on line %d: %s\n", yylineno, s);
}

int yywrap(void) {
    return 1;
}

/* vim: set tw=80 ft=lex: */
