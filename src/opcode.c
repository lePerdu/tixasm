/**
 * @file opcode.c
 * @author Zach Peltzer
 * @date Created: Fri, 02 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "tixasm.h"
#include "opcode.h"
#include "z80.tab.h"

static const struct instruction ld_instrs[] = {

    {OP_BC,     OP_IMM16,   3, -1,  1,   {0x01, 0x00, 0x00}},
    {OP_DE,     OP_IMM16,   3, -1,  1,   {0x11, 0x00, 0x00}},
    {OP_HL,     OP_IMM16,   3, -1,  1,   {0x21, 0x00, 0x00}},
    {OP_SP,     OP_IMM16,   3, -1,  1,   {0x31, 0x00, 0x00}},
    {OP_IX,     OP_IMM16,   4, -1,  2,   {0xDD, 0x21, 0x00, 0x00}},
    {OP_IY,     OP_IMM16,   4, -1,  2,   {0xFD, 0x21, 0x00, 0x00}},

    {OP_BC,     OP_EXT,     4, -1,  2,   {0xED, 0x43, 0x00, 0x00}},
    {OP_DE,     OP_EXT,     4, -1,  2,   {0xED, 0x53, 0x00, 0x00}},
    {OP_HL,     OP_EXT,     3, -1,  1,   {0x63, 0x00, 0x00}},
    {OP_HL,     OP_EXT,     3, -1,  1,   {0x22, 0x00, 0x00}},
    {OP_SP,     OP_EXT,     4, -1,  2,   {0xED, 0x73, 0x00, 0x00}},
    {OP_IX,     OP_EXT,     4, -1,  2,   {0xDD, 0x22, 0x00, 0x00}},
    {OP_IY,     OP_EXT,     4, -1,  2,   {0xFD, 0x22, 0x00, 0x00}},

    {OP_EXT,    OP_BC,      4,  2, -1,   {0xED, 0x4B, 0x00, 0x00}},
    {OP_EXT,    OP_DE,      4,  2, -1,   {0xED, 0x4B, 0x00, 0x00}},
    {OP_EXT,    OP_HL,      3,  2,  0,   {0x4B, 0x00, 0x00, 0x00}},
    {OP_EXT,    OP_HL,      3,  1, -1,   {0x2A, 0x00, 0x00}},
    {OP_EXT,    OP_SP,      4,  2, -1,   {0xED, 0x4B, 0x00, 0x00}},
    {OP_EXT,    OP_IX,      4,  2, -1,   {0xDD, 0x2A, 0x00, 0x00}},
    {OP_EXT,    OP_IY,      4,  2, -1,   {0xFD, 0x2A, 0x00, 0x00}},

    {OP_SP,     OP_HL,      1, -1, -1,   {0xF9}},
    {OP_SP,     OP_IX,      2, -1, -1,   {0xDD, 0xF9}},
    {OP_SP,     OP_IY,      2, -1, -1,   {0xFD, 0xF9}},

    {OP_A,      OP_iBC,     1, -1, -1,   {0x0A}},
    {OP_A,      OP_iDE,     1, -1, -1,   {0x1A}},
    {OP_iBC,    OP_A,       1, -1, -1,   {0x02}},
    {OP_iDE,    OP_A,       1, -1, -1,   {0x12}},

    {OP_A,      OP_EXT,     3, -1,  1,   {0x3A, 0x00, 0x00}},
    {OP_EXT,    OP_A,       3,  1, -1,   {0x32, 0x00, 0x00}},

    {OP_iIX,    OP_IMM8,    4,  2,  3,   {0xDD, 0x36, 0x00, 0x00}},
    {OP_iIY,    OP_IMM8,    4,  2,  3,   {0xFD, 0x36, 0x00, 0x00}},

    {OP_B,      OP_IMM8,    2, -1,  1,   {0x06, 0x00}},
    {OP_C,      OP_IMM8,    2, -1,  1,   {0x0E, 0x00}},
    {OP_D,      OP_IMM8,    2, -1,  1,   {0x16, 0x00}},
    {OP_E,      OP_IMM8,    2, -1,  1,   {0x1E, 0x00}},
    {OP_H,      OP_IMM8,    2, -1,  1,   {0x26, 0x00}},
    {OP_L,      OP_IMM8,    2, -1,  1,   {0x2E, 0x00}},
    {OP_iHL,    OP_IMM8,    2, -1,  1,   {0x36, 0x00}},
    {OP_A,      OP_IMM8,    2, -1,  1,   {0x3E, 0x00}},

    {OP_B,      OP_B,       1, -1, -1,   {0x40}},
    {OP_B,      OP_C,       1, -1, -1,   {0x41}},
    {OP_B,      OP_D,       1, -1, -1,   {0x42}},
    {OP_B,      OP_E,       1, -1, -1,   {0x43}},
    {OP_B,      OP_H,       1, -1, -1,   {0x44}},
    {OP_B,      OP_L,       1, -1, -1,   {0x45}},
    {OP_B,      OP_iHL,     1, -1, -1,   {0x46}},
    {OP_B,      OP_A,       1, -1, -1,   {0x47}},

    {OP_C,      OP_B,       1, -1, -1,   {0x48}},
    {OP_C,      OP_C,       1, -1, -1,   {0x49}},
    {OP_C,      OP_D,       1, -1, -1,   {0x4B}},
    {OP_C,      OP_E,       1, -1, -1,   {0x4C}},
    {OP_C,      OP_H,       1, -1, -1,   {0x4E}},
    {OP_C,      OP_L,       1, -1, -1,   {0x4D}},
    {OP_C,      OP_iHL,     1, -1, -1,   {0x4E}},
    {OP_C,      OP_A,       1, -1, -1,   {0x4F}},

    {OP_D,      OP_B,       1, -1, -1,   {0x50}},
    {OP_D,      OP_C,       1, -1, -1,   {0x51}},
    {OP_D,      OP_D,       1, -1, -1,   {0x52}},
    {OP_D,      OP_E,       1, -1, -1,   {0x53}},
    {OP_D,      OP_H,       1, -1, -1,   {0x54}},
    {OP_D,      OP_L,       1, -1, -1,   {0x55}},
    {OP_D,      OP_iHL,     1, -1, -1,   {0x56}},
    {OP_D,      OP_A,       1, -1, -1,   {0x57}},

    {OP_E,      OP_B,       1, -1, -1,   {0x58}},
    {OP_E,      OP_C,       1, -1, -1,   {0x59}},
    {OP_E,      OP_D,       1, -1, -1,   {0x5B}},
    {OP_E,      OP_E,       1, -1, -1,   {0x5C}},
    {OP_E,      OP_H,       1, -1, -1,   {0x5E}},
    {OP_E,      OP_L,       1, -1, -1,   {0x5D}},
    {OP_E,      OP_iHL,     1, -1, -1,   {0x5E}},
    {OP_E,      OP_A,       1, -1, -1,   {0x5F}},

    {OP_H,      OP_B,       1, -1, -1,   {0x60}},
    {OP_H,      OP_C,       1, -1, -1,   {0x61}},
    {OP_H,      OP_D,       1, -1, -1,   {0x62}},
    {OP_H,      OP_E,       1, -1, -1,   {0x63}},
    {OP_H,      OP_H,       1, -1, -1,   {0x64}},
    {OP_H,      OP_L,       1, -1, -1,   {0x65}},
    {OP_H,      OP_iHL,     1, -1, -1,   {0x66}},
    {OP_H,      OP_A,       1, -1, -1,   {0x67}},

    {OP_L,      OP_B,       1, -1, -1,   {0x68}},
    {OP_L,      OP_C,       1, -1, -1,   {0x69}},
    {OP_L,      OP_D,       1, -1, -1,   {0x6B}},
    {OP_L,      OP_E,       1, -1, -1,   {0x6C}},
    {OP_L,      OP_H,       1, -1, -1,   {0x6E}},
    {OP_L,      OP_L,       1, -1, -1,   {0x6D}},
    {OP_L,      OP_iHL,     1, -1, -1,   {0x6E}},
    {OP_L,      OP_A,       1, -1, -1,   {0x6F}},

    {OP_iHL,    OP_B,       1, -1, -1,   {0x70}},
    {OP_iHL,    OP_C,       1, -1, -1,   {0x71}},
    {OP_iHL,    OP_D,       1, -1, -1,   {0x72}},
    {OP_iHL,    OP_E,       1, -1, -1,   {0x73}},
    {OP_iHL,    OP_H,       1, -1, -1,   {0x74}},
    {OP_iHL,    OP_L,       1, -1, -1,   {0x75}},
    {OP_iHL,    OP_A,       1, -1, -1,   {0x77}},

    {OP_A,      OP_B,       1, -1, -1,   {0x78}},
    {OP_A,      OP_C,       1, -1, -1,   {0x79}},
    {OP_A,      OP_D,       1, -1, -1,   {0x7B}},
    {OP_A,      OP_E,       1, -1, -1,   {0x7C}},
    {OP_A,      OP_H,       1, -1, -1,   {0x7E}},
    {OP_A,      OP_L,       1, -1, -1,   {0x7D}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0x7E}},
    {OP_A,      OP_A,       1, -1, -1,   {0x7F}},

    {OP_B,      OP_iIX,     3, -1,  2,   {0xDD, 0x46, 0x00}},
    {OP_C,      OP_iIX,     3, -1,  2,   {0xDD, 0x4E, 0x00}},
    {OP_D,      OP_iIX,     3, -1,  2,   {0xDD, 0x56, 0x00}},
    {OP_E,      OP_iIX,     3, -1,  2,   {0xDD, 0x5E, 0x00}},
    {OP_H,      OP_iIX,     3, -1,  2,   {0xDD, 0x66, 0x00}},
    {OP_L,      OP_iIX,     3, -1,  2,   {0xDD, 0x6E, 0x00}},
    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0x7E, 0x00}},

    {OP_iIX,    OP_B,       3,  2, -1,   {0xDD, 0x70, 0x00}},
    {OP_iIX,    OP_C,       3,  2, -1,   {0xDD, 0x71, 0x00}},
    {OP_iIX,    OP_D,       3,  2, -1,   {0xDD, 0x72, 0x00}},
    {OP_iIX,    OP_E,       3,  2, -1,   {0xDD, 0x73, 0x00}},
    {OP_iIX,    OP_H,       3,  2, -1,   {0xDD, 0x74, 0x00}},
    {OP_iIX,    OP_L,       3,  2, -1,   {0xDD, 0x75, 0x00}},
    {OP_iIX,    OP_A,       3,  2, -1,   {0xDD, 0x77, 0x00}},

    {OP_B,      OP_iIY,     3, -1,  2,   {0xFD, 0x46, 0x00}},
    {OP_C,      OP_iIY,     3, -1,  2,   {0xFD, 0x4E, 0x00}},
    {OP_D,      OP_iIY,     3, -1,  2,   {0xFD, 0x56, 0x00}},
    {OP_E,      OP_iIY,     3, -1,  2,   {0xFD, 0x5E, 0x00}},
    {OP_H,      OP_iIY,     3, -1,  2,   {0xFD, 0x66, 0x00}},
    {OP_L,      OP_iIY,     3, -1,  2,   {0xFD, 0x6E, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0x7E, 0x00}},

    {OP_iIY,    OP_B,       3,  2, -1,   {0xFD, 0x70, 0x00}},
    {OP_iIY,    OP_C,       3,  2, -1,   {0xFD, 0x71, 0x00}},
    {OP_iIY,    OP_D,       3,  2, -1,   {0xFD, 0x72, 0x00}},
    {OP_iIY,    OP_E,       3,  2, -1,   {0xFD, 0x73, 0x00}},
    {OP_iIY,    OP_H,       3,  2, -1,   {0xFD, 0x74, 0x00}},
    {OP_iIY,    OP_L,       3,  2, -1,   {0xFD, 0x75, 0x00}},
    {OP_iIY,    OP_A,       3,  2, -1,   {0xFD, 0x77, 0x00}},

    {OP_A,      OP_I,       2, -1, -1,   {0xED, 0x57}},
    {OP_A,      OP_R,       2, -1, -1,   {0xED, 0x5F}},
    {OP_I,      OP_A,       2, -1, -1,   {0xED, 0x47}},
    {OP_R,      OP_A,       2, -1, -1,   {0xED, 0x4F}},

};

static const struct instruction push_instrs[] = {

    {OP_BC,     OP_NONE,    1, -1, -1,   {0xC6}},
    {OP_DE,     OP_NONE,    1, -1, -1,   {0xD6}},
    {OP_HL,     OP_NONE,    1, -1, -1,   {0xE6}},
    {OP_AF,     OP_NONE,    1, -1, -1,   {0xF6}},

    {OP_IX,     OP_NONE,    2, -1, -1,   {0xDD, 0xE6}},
    {OP_IY,     OP_NONE,    2, -1, -1,   {0xFD, 0xE6}},

};

static const struct instruction pop_instrs[] = {

    {OP_BC,     OP_NONE,    1, -1, -1,   {0xC1}},
    {OP_DE,     OP_NONE,    1, -1, -1,   {0xD1}},
    {OP_HL,     OP_NONE,    1, -1, -1,   {0xE1}},
    {OP_AF,     OP_NONE,    1, -1, -1,   {0xF1}},

    {OP_IX,     OP_NONE,    2, -1, -1,   {0xDD, 0xE1}},
    {OP_IY,     OP_NONE,    2, -1, -1,   {0xFD, 0xE1}},

};

static const struct instruction ex_instrs[] = {

    {OP_AF,     OP_sAF,     1, -1, -1,   {0x08}},
    {OP_DE,     OP_HL,      1, -1, -1,   {0xEB}},

    {OP_iSP,    OP_HL,      1, -1, -1,   {0xE3}},
    {OP_iSP,    OP_IX,      2, -1, -1,   {0xDD, 0xE3}},
    {OP_iSP,    OP_IY,      2, -1, -1,   {0xFD, 0xE3}},

};

static const struct instruction exx_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xD9}},
};

static const struct instruction ldi_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xA0}},
};

static const struct instruction ldd_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xB0}},
};

static const struct instruction ldir_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xA8}},
};

static const struct instruction lddr_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xB8}},
};

static const struct instruction cpi_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xA1}},
};

static const struct instruction cpd_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xB1}},
};

static const struct instruction cpir_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xA9}},
};

static const struct instruction cpdr_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xB9}},
};

static const struct instruction add_instrs[] = {

    {OP_A,      OP_B,       1, -1, -1,   {0x80}},
    {OP_A,      OP_C,       1, -1, -1,   {0x81}},
    {OP_A,      OP_D,       1, -1, -1,   {0x82}},
    {OP_A,      OP_E,       1, -1, -1,   {0x83}},
    {OP_A,      OP_H,       1, -1, -1,   {0x84}},
    {OP_A,      OP_L,       1, -1, -1,   {0x85}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0x86}},
    {OP_A,      OP_A,       1, -1, -1,   {0x87}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0x86, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0x86, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xC6, 0x00}},

    {OP_HL,     OP_BC,      1, -1, -1,   {0x09}},
    {OP_HL,     OP_DE,      1, -1, -1,   {0x19}},
    {OP_HL,     OP_HL,      1, -1, -1,   {0x29}},
    {OP_HL,     OP_SP,      1, -1, -1,   {0x39}},

    {OP_IX,     OP_BC,      2, -1, -1,   {0xDD, 0x09}},
    {OP_IX,     OP_DE,      2, -1, -1,   {0xDD, 0x19}},
    {OP_IX,     OP_IX,      2, -1, -1,   {0xDD, 0x29}},
    {OP_IX,     OP_SP,      2, -1, -1,   {0xDD, 0x39}},

    {OP_IY,     OP_BC,      2, -1, -1,   {0xFD, 0x09}},
    {OP_IY,     OP_DE,      2, -1, -1,   {0xFD, 0x19}},
    {OP_IY,     OP_IY,      2, -1, -1,   {0xFD, 0x29}},
    {OP_IY,     OP_SP,      2, -1, -1,   {0xFD, 0x39}},

};

static const struct instruction adc_instrs[] = {

    {OP_A,      OP_B,       1, -1, -1,   {0x88}},
    {OP_A,      OP_C,       1, -1, -1,   {0x89}},
    {OP_A,      OP_D,       1, -1, -1,   {0x8A}},
    {OP_A,      OP_E,       1, -1, -1,   {0x8B}},
    {OP_A,      OP_H,       1, -1, -1,   {0x8C}},
    {OP_A,      OP_L,       1, -1, -1,   {0x8D}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0x8E}},
    {OP_A,      OP_A,       1, -1, -1,   {0x8F}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0x8E, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0x8E, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xCE, 0x00}},

    {OP_HL,     OP_BC,      2, -1, -1,   {0xED, 0x4A}},
    {OP_HL,     OP_DE,      2, -1, -1,   {0xED, 0x5A}},
    {OP_HL,     OP_HL,      2, -1, -1,   {0xED, 0x6A}},
    {OP_HL,     OP_SP,      2, -1, -1,   {0xED, 0x7A}},

};

static const struct instruction sub_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0x90}},
    {OP_C,      OP_NONE,    1, -1, -1,   {0x91}},
    {OP_D,      OP_NONE,    1, -1, -1,   {0x92}},
    {OP_E,      OP_NONE,    1, -1, -1,   {0x93}},
    {OP_H,      OP_NONE,    1, -1, -1,   {0x94}},
    {OP_L,      OP_NONE,    1, -1, -1,   {0x95}},
    {OP_iHL,    OP_NONE,    1, -1, -1,   {0x96}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x97}},

    {OP_iIX,    OP_NONE,    3,  2, -1,   {0xDD, 0x96, 0x00}},
    {OP_iIY,    OP_NONE,    3,  2, -1,   {0xFD, 0x96, 0x00}},

    {OP_IMM8,   OP_NONE,    2,  1, -1,   {0xD6, 0x00}},

    {OP_A,      OP_B,       1, -1, -1,   {0x90}},
    {OP_A,      OP_C,       1, -1, -1,   {0x91}},
    {OP_A,      OP_D,       1, -1, -1,   {0x92}},
    {OP_A,      OP_E,       1, -1, -1,   {0x93}},
    {OP_A,      OP_H,       1, -1, -1,   {0x94}},
    {OP_A,      OP_L,       1, -1, -1,   {0x95}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0x96}},
    {OP_A,      OP_A,       1, -1, -1,   {0x97}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0x96, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0x96, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xD6, 0x00}},

};

static const struct instruction sbc_instrs[] = {

    {OP_A,      OP_B,       1, -1, -1,   {0x98}},
    {OP_A,      OP_C,       1, -1, -1,   {0x99}},
    {OP_A,      OP_D,       1, -1, -1,   {0x9A}},
    {OP_A,      OP_E,       1, -1, -1,   {0x9B}},
    {OP_A,      OP_H,       1, -1, -1,   {0x9C}},
    {OP_A,      OP_L,       1, -1, -1,   {0x9D}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0x9E}},
    {OP_A,      OP_A,       1, -1, -1,   {0x9F}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0x9E, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0x9E, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xDE, 0x00}},

    {OP_HL,     OP_BC,      2, -1, -1,   {0xED, 0x42}},
    {OP_HL,     OP_DE,      2, -1, -1,   {0xED, 0x52}},
    {OP_HL,     OP_HL,      2, -1, -1,   {0xED, 0x62}},
    {OP_HL,     OP_SP,      2, -1, -1,   {0xED, 0x72}},

};

static const struct instruction and_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0xA0}},
    {OP_C,      OP_NONE,    1, -1, -1,   {0xA1}},
    {OP_D,      OP_NONE,    1, -1, -1,   {0xA2}},
    {OP_E,      OP_NONE,    1, -1, -1,   {0xA3}},
    {OP_H,      OP_NONE,    1, -1, -1,   {0xA4}},
    {OP_L,      OP_NONE,    1, -1, -1,   {0xA5}},
    {OP_iHL,    OP_NONE,    1, -1, -1,   {0xA6}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0xA7}},

    {OP_iIX,    OP_NONE,    3,  2, -1,   {0xDD, 0xA6, 0x00}},
    {OP_iIY,    OP_NONE,    3,  2, -1,   {0xFD, 0xA6, 0x00}},

    {OP_IMM8,   OP_NONE,    2,  1, -1,   {0xE6, 0x00}},

    {OP_A,      OP_B,       1, -1, -1,   {0xA0}},
    {OP_A,      OP_C,       1, -1, -1,   {0xA1}},
    {OP_A,      OP_D,       1, -1, -1,   {0xA2}},
    {OP_A,      OP_E,       1, -1, -1,   {0xA3}},
    {OP_A,      OP_H,       1, -1, -1,   {0xA4}},
    {OP_A,      OP_L,       1, -1, -1,   {0xA5}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0xA6}},
    {OP_A,      OP_A,       1, -1, -1,   {0xA7}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0xA6, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0xA6, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xE6, 0x00}},

};

static const struct instruction xor_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0xA8}},
    {OP_C,      OP_NONE,    1, -1, -1,   {0xA9}},
    {OP_D,      OP_NONE,    1, -1, -1,   {0xAA}},
    {OP_E,      OP_NONE,    1, -1, -1,   {0xAB}},
    {OP_H,      OP_NONE,    1, -1, -1,   {0xAC}},
    {OP_L,      OP_NONE,    1, -1, -1,   {0xAD}},
    {OP_iHL,    OP_NONE,    1, -1, -1,   {0xAE}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0xAF}},

    {OP_iIX,    OP_NONE,    3,  2, -1,   {0xDD, 0xAE, 0x00}},
    {OP_iIY,    OP_NONE,    3,  2, -1,   {0xFD, 0xAE, 0x00}},

    {OP_IMM8,   OP_NONE,    2,  1, -1,   {0xEE, 0x00}},

    {OP_A,      OP_B,       1, -1, -1,   {0xA8}},
    {OP_A,      OP_C,       1, -1, -1,   {0xA9}},
    {OP_A,      OP_D,       1, -1, -1,   {0xAA}},
    {OP_A,      OP_E,       1, -1, -1,   {0xAB}},
    {OP_A,      OP_H,       1, -1, -1,   {0xAC}},
    {OP_A,      OP_L,       1, -1, -1,   {0xAD}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0xAE}},
    {OP_A,      OP_A,       1, -1, -1,   {0xAF}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0xAE, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0xAE, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xEE, 0x00}},

};
static const struct instruction or_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0xB0}},
    {OP_C,      OP_NONE,    1, -1, -1,   {0xB1}},
    {OP_D,      OP_NONE,    1, -1, -1,   {0xB2}},
    {OP_E,      OP_NONE,    1, -1, -1,   {0xB3}},
    {OP_H,      OP_NONE,    1, -1, -1,   {0xB4}},
    {OP_L,      OP_NONE,    1, -1, -1,   {0xB5}},
    {OP_iHL,    OP_NONE,    1, -1, -1,   {0xB6}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0xB7}},

    {OP_iIX,    OP_NONE,    3,  2, -1,   {0xDD, 0xB6, 0x00}},
    {OP_iIY,    OP_NONE,    3,  2, -1,   {0xFD, 0xB6, 0x00}},

    {OP_IMM8,   OP_NONE,    2,  1, -1,   {0xF6, 0x00}},

    {OP_A,      OP_B,       1, -1, -1,   {0xB0}},
    {OP_A,      OP_C,       1, -1, -1,   {0xB1}},
    {OP_A,      OP_D,       1, -1, -1,   {0xB2}},
    {OP_A,      OP_E,       1, -1, -1,   {0xB3}},
    {OP_A,      OP_H,       1, -1, -1,   {0xB4}},
    {OP_A,      OP_L,       1, -1, -1,   {0xB5}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0xB6}},
    {OP_A,      OP_A,       1, -1, -1,   {0xB7}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0xB6, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0xB6, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xF6, 0x00}},

};

static const struct instruction cp_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0xB8}},
    {OP_C,      OP_NONE,    1, -1, -1,   {0xB9}},
    {OP_D,      OP_NONE,    1, -1, -1,   {0xBA}},
    {OP_E,      OP_NONE,    1, -1, -1,   {0xBB}},
    {OP_H,      OP_NONE,    1, -1, -1,   {0xBC}},
    {OP_L,      OP_NONE,    1, -1, -1,   {0xBD}},
    {OP_iHL,    OP_NONE,    1, -1, -1,   {0xBE}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0xBF}},

    {OP_iIX,    OP_NONE,    3,  2, -1,   {0xDD, 0xBE, 0x00}},
    {OP_iIY,    OP_NONE,    3,  2, -1,   {0xFD, 0xBE, 0x00}},

    {OP_IMM8,   OP_NONE,    2,  1, -1,   {0xFE, 0x00}},

    {OP_A,      OP_B,       1, -1, -1,   {0xB8}},
    {OP_A,      OP_C,       1, -1, -1,   {0xB9}},
    {OP_A,      OP_D,       1, -1, -1,   {0xBA}},
    {OP_A,      OP_E,       1, -1, -1,   {0xBB}},
    {OP_A,      OP_H,       1, -1, -1,   {0xBC}},
    {OP_A,      OP_L,       1, -1, -1,   {0xBD}},
    {OP_A,      OP_iHL,     1, -1, -1,   {0xBE}},
    {OP_A,      OP_A,       1, -1, -1,   {0xBF}},

    {OP_A,      OP_iIX,     3, -1,  2,   {0xDD, 0xBE, 0x00}},
    {OP_A,      OP_iIY,     3, -1,  2,   {0xFD, 0xBE, 0x00}},

    {OP_A,      OP_IMM8,    2, -1,  1,   {0xFE, 0x00}},

};

static const struct instruction inc_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0x04}},
    {OP_C,      OP_NONE,    1, -1, -1,   {0x0C}},
    {OP_D,      OP_NONE,    1, -1, -1,   {0x14}},
    {OP_E,      OP_NONE,    1, -1, -1,   {0x1C}},
    {OP_H,      OP_NONE,    1, -1, -1,   {0x24}},
    {OP_L,      OP_NONE,    1, -1, -1,   {0x2C}},
    {OP_iHL,    OP_NONE,    1, -1, -1,   {0x34}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x3C}},

    {OP_iIX,    OP_NONE,    3,  2, -1,   {0xDD, 0x34, 0x00}},
    {OP_iIY,    OP_NONE,    3,  2, -1,   {0xFD, 0x34, 0x00}},

    {OP_BC,     OP_NONE,    1, -1, -1,   {0x03}},
    {OP_DE,     OP_NONE,    1, -1, -1,   {0x13}},
    {OP_HL,     OP_NONE,    1, -1, -1,   {0x23}},
    {OP_SP,     OP_NONE,    1, -1, -1,   {0x33}},

    {OP_IX,     OP_NONE,    2, -1, -1,   {0xFD, 0x23}},
    {OP_IY,     OP_NONE,    2, -1, -1,   {0xDD, 0x23}},

};

static const struct instruction dec_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0x05}},
    {OP_C,      OP_NONE,    1, -1, -1,   {0x0D}},
    {OP_D,      OP_NONE,    1, -1, -1,   {0x15}},
    {OP_E,      OP_NONE,    1, -1, -1,   {0x1D}},
    {OP_H,      OP_NONE,    1, -1, -1,   {0x25}},
    {OP_L,      OP_NONE,    1, -1, -1,   {0x2D}},
    {OP_iHL,    OP_NONE,    1, -1, -1,   {0x35}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x3D}},

    {OP_iIX,    OP_NONE,    3,  2, -1,   {0xDD, 0x35, 0x00}},
    {OP_iIY,    OP_NONE,    3,  2, -1,   {0xFD, 0x35, 0x00}},

    {OP_BC,     OP_NONE,    1, -1, -1,   {0x0B}},
    {OP_DE,     OP_NONE,    1, -1, -1,   {0x1B}},
    {OP_HL,     OP_NONE,    1, -1, -1,   {0x2B}},
    {OP_SP,     OP_NONE,    1, -1, -1,   {0x3B}},

    {OP_IX,     OP_NONE,    2, -1, -1,   {0xFD, 0x2B}},
    {OP_IY,     OP_NONE,    2, -1, -1,   {0xDD, 0x2B}},

};

static const struct instruction cpl_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x2F}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x2F}},
};

static const struct instruction neg_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0x44}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xED, 0x44}},
};

static const struct instruction daa_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x27}},
};

static const struct instruction scf_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x37}},
};

static const struct instruction ccf_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x3F}},
};

static const struct instruction rlc_instrs[] = {

    {OP_B,      OP_NONE,    1, -1, -1,   {0xCB}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x01}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x02}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x03}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x04}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x05}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x06}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x07}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x06}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x06}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x06}},

};

static const struct instruction rrc_instrs[] = {

    {OP_B,      OP_NONE,    2, -1, -1,   {0xCB, 0x08}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x09}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x0A}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x0B}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x0C}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x0D}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x0E}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x0F}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x0E}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x0E}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x0E}},

};

static const struct instruction rl_instrs[] = {

    {OP_B,      OP_NONE,    2, -1, -1,   {0xCB, 0x10}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x11}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x12}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x13}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x14}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x15}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x16}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x17}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x16}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x16}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x16}},

};

static const struct instruction rr_instrs[] = {

    {OP_B,      OP_NONE,    2, -1, -1,   {0xCB, 0x18}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x19}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x1A}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x1B}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x1C}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x1D}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x1E}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x1F}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x1E}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x1E}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x1E}},

};

static const struct instruction sla_instrs[] = {

    {OP_B,      OP_NONE,    2, -1, -1,   {0xCB, 0x20}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x21}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x22}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x23}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x24}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x25}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x26}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x27}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x26}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x26}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x26}},

};

static const struct instruction sra_instrs[] = {

    {OP_B,      OP_NONE,    2, -1, -1,   {0xCB, 0x28}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x29}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x2A}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x2B}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x2C}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x2D}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x2E}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x2F}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x2E}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x2E}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x2E}},

};

static const struct instruction sll_instrs[] = {

    {OP_B,      OP_NONE,    2, -1, -1,   {0xCB, 0x30}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x31}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x32}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x33}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x34}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x35}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x36}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x37}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x36}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x36}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x36}},

};

static const struct instruction srl_instrs[] = {

    {OP_B,      OP_NONE,    2, -1, -1,   {0xCB, 0x38}},
    {OP_C,      OP_NONE,    2, -1, -1,   {0xCB, 0x39}},
    {OP_D,      OP_NONE,    2, -1, -1,   {0xCB, 0x3A}},
    {OP_E,      OP_NONE,    2, -1, -1,   {0xCB, 0x3B}},
    {OP_H,      OP_NONE,    2, -1, -1,   {0xCB, 0x3C}},
    {OP_L,      OP_NONE,    2, -1, -1,   {0xCB, 0x3D}},
    {OP_A,      OP_NONE,    2, -1, -1,   {0xCB, 0x3E}},
    {OP_A,      OP_NONE,    1, -1, -1,   {0x3F}},
    {OP_iHL,    OP_NONE,    2, -1, -1,   {0xCB, 0x3E}},

    {OP_iIX,    OP_NONE,    4,  2, -1,   {0xDD, 0xCB, 0x00, 0x3E}},
    {OP_iIY,    OP_NONE,    4,  2, -1,   {0xFD, 0xCB, 0x00, 0x3E}},

};

static const struct instruction rlca_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x07}},
};

static const struct instruction rrca_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x0F}},
};

static const struct instruction rla_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x17}},
};

static const struct instruction rra_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x1F}},
};

static const struct instruction rrd_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0x67}},
};

static const struct instruction rld_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0x6F}},
};

static const struct instruction bit_instrs[] = {

    {OP_BIT,    OP_B,       2,  2, -1,   {0xCB, 0x40}},
    {OP_BIT,    OP_C,       2,  2, -1,   {0xCB, 0x41}},
    {OP_BIT,    OP_D,       2,  2, -1,   {0xCB, 0x42}},
    {OP_BIT,    OP_E,       2,  2, -1,   {0xCB, 0x43}},
    {OP_BIT,    OP_H,       2,  2, -1,   {0xCB, 0x44}},
    {OP_BIT,    OP_L,       2,  2, -1,   {0xCB, 0x45}},
    {OP_BIT,    OP_iHL,     2,  2, -1,   {0xCB, 0x46}},
    {OP_BIT,    OP_A,       2,  2, -1,   {0xCB, 0x47}},
    {OP_BIT,    OP_iIX,     4,  3,  2,   {0xDD, 0xCB, 0x00, 0x46}},
    {OP_BIT,    OP_iIY,     4,  3,  2,   {0xFD, 0xCB, 0x00, 0x46}},

};

static const struct instruction res_instrs[] = {

    {OP_BIT,    OP_B,       2,  2, -1,   {0xCB, 0x80}},
    {OP_BIT,    OP_C,       2,  2, -1,   {0xCB, 0x81}},
    {OP_BIT,    OP_D,       2,  2, -1,   {0xCB, 0x82}},
    {OP_BIT,    OP_E,       2,  2, -1,   {0xCB, 0x83}},
    {OP_BIT,    OP_H,       2,  2, -1,   {0xCB, 0x84}},
    {OP_BIT,    OP_L,       2,  2, -1,   {0xCB, 0x85}},
    {OP_BIT,    OP_iHL,     2,  2, -1,   {0xCB, 0x86}},
    {OP_BIT,    OP_A,       2,  2, -1,   {0xCB, 0x87}},
    {OP_BIT,    OP_iIX,     4,  3,  2,   {0xDD, 0xCB, 0x00, 0x86}},
    {OP_BIT,    OP_iIY,     4,  3,  2,   {0xFD, 0xCB, 0x00, 0x86}},

};

static const struct instruction set_instrs[] = {

    {OP_BIT,    OP_B,       2,  2, -1,   {0xCB, 0xC0}},
    {OP_BIT,    OP_C,       2,  2, -1,   {0xCB, 0xC1}},
    {OP_BIT,    OP_D,       2,  2, -1,   {0xCB, 0xC2}},
    {OP_BIT,    OP_E,       2,  2, -1,   {0xCB, 0xC3}},
    {OP_BIT,    OP_H,       2,  2, -1,   {0xCB, 0xC4}},
    {OP_BIT,    OP_L,       2,  2, -1,   {0xCB, 0xC5}},
    {OP_BIT,    OP_iHL,     2,  2, -1,   {0xCB, 0xC6}},
    {OP_BIT,    OP_A,       2,  2, -1,   {0xCB, 0xC7}},
    {OP_BIT,    OP_iIX,     4,  3,  2,   {0xDD, 0xCB, 0x00, 0xC6}},
    {OP_BIT,    OP_iIY,     4,  3,  2,   {0xFD, 0xCB, 0x00, 0xC6}},

};

static const struct instruction jp_instrs[] = {
    {OP_IMM16,  OP_NONE,    3,  1, -1,   {0xC3, 0x00, 0x00}},
    {OP_fNZ,    OP_IMM16,   3, -1,  1,   {0xC2, 0x00, 0x00}},
    {OP_fZ,     OP_IMM16,   3, -1,  1,   {0xCA, 0x00, 0x00}},
    {OP_fNC,    OP_IMM16,   3, -1,  1,   {0xD2, 0x00, 0x00}},
    {OP_fC,     OP_IMM16,   3, -1,  1,   {0xDA, 0x00, 0x00}},
    {OP_fPO,    OP_IMM16,   3, -1,  1,   {0xE2, 0x00, 0x00}},
    {OP_fPE,    OP_IMM16,   3, -1,  1,   {0xEA, 0x00, 0x00}},
    {OP_fP,     OP_IMM16,   3, -1,  1,   {0xF2, 0x00, 0x00}},
    {OP_fM,     OP_IMM16,   3, -1,  1,   {0xFA, 0x00, 0x00}},
    {OP_HL,     OP_NONE,    1, -1, -1,   {0xEB}},
};

static const struct instruction call_instrs[] = {
    {OP_IMM16,  OP_NONE,    3,  1, -1,   {0xCD, 0x00, 0x00}},
    {OP_fNZ,    OP_IMM16,   3, -1,  1,   {0xC4, 0x00, 0x00}},
    {OP_fZ,     OP_IMM16,   3, -1,  1,   {0xCC, 0x00, 0x00}},
    {OP_fNC,    OP_IMM16,   3, -1,  1,   {0xD4, 0x00, 0x00}},
    {OP_fC,     OP_IMM16,   3, -1,  1,   {0xDC, 0x00, 0x00}},
    {OP_fPO,    OP_IMM16,   3, -1,  1,   {0xE4, 0x00, 0x00}},
    {OP_fPE,    OP_IMM16,   3, -1,  1,   {0xEC, 0x00, 0x00}},
    {OP_fP,     OP_IMM16,   3, -1,  1,   {0xF4, 0x00, 0x00}},
    {OP_fM,     OP_IMM16,   3, -1,  1,   {0xFC, 0x00, 0x00}},
};

static const struct instruction ret_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xC9}},
    {OP_fNZ,    OP_NONE,    1, -1,  1,   {0xC0}},
    {OP_fZ,     OP_NONE,    1, -1,  1,   {0xC8}},
    {OP_fNC,    OP_NONE,    1, -1,  1,   {0xD0}},
    {OP_fC,     OP_NONE,    1, -1,  1,   {0xD8}},
    {OP_fPO,    OP_NONE,    1, -1,  1,   {0xE0}},
    {OP_fPE,    OP_NONE,    1, -1,  1,   {0xE8}},
    {OP_fP,     OP_NONE,    1, -1,  1,   {0xF0}},
    {OP_fM,     OP_NONE,    1, -1,  1,   {0xF8}},
};

static const struct instruction reti_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0x4D}},
};

static const struct instruction retn_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0x45}},
};

static const struct instruction jr_instrs[] = {
    {OP_REL,    OP_NONE,    2,  1, -1,   {0x18, 0x00}},
    {OP_fNZ,    OP_REL,     2, -1,  1,   {0x20, 0x00}},
    {OP_fZ,     OP_REL,     2, -1,  1,   {0x28, 0x00}},
    {OP_fNC,    OP_REL,     2, -1,  1,   {0x30, 0x00}},
    {OP_fC,     OP_REL,     2, -1,  1,   {0x38, 0x00}},
};

static const struct instruction djnz_instrs[] = {
    {OP_REL,    OP_NONE,    2,  1, -1,   {0x10, 0x00}},
};

static const struct instruction rst_instrs[] = {
    {OP_RST,    OP_NONE,    1,  1, -1,   {0xC7}},
};

static const struct instruction in_instrs[] = {
    {OP_A,      OP_PORT,    2, -1,  1,   {0xDB, 0x00}},

    {OP_B,      OP_iC,      2, -1, -1,   {0xED, 0x40}},
    {OP_C,      OP_iC,      2, -1, -1,   {0xED, 0x48}},
    {OP_D,      OP_iC,      2, -1, -1,   {0xED, 0x50}},
    {OP_E,      OP_iC,      2, -1, -1,   {0xED, 0x58}},
    {OP_H,      OP_iC,      2, -1, -1,   {0xED, 0x60}},
    {OP_L,      OP_iC,      2, -1, -1,   {0xED, 0x68}},
    {OP_A,      OP_iC,      2, -1, -1,   {0xED, 0x78}},
};

static const struct instruction ini_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xA2}},
};

static const struct instruction inir_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xB2}},
};

static const struct instruction ind_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xAA}},
};

static const struct instruction indr_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xBA}},
};

static const struct instruction out_instrs[] = {
    {OP_A,      OP_PORT,    2, -1,  1,   {0xD3, 0x00}},

    {OP_B,      OP_iC,      2, -1, -1,   {0xED, 0x41}},
    {OP_C,      OP_iC,      2, -1, -1,   {0xED, 0x49}},
    {OP_D,      OP_iC,      2, -1, -1,   {0xED, 0x51}},
    {OP_E,      OP_iC,      2, -1, -1,   {0xED, 0x59}},
    {OP_H,      OP_iC,      2, -1, -1,   {0xED, 0x61}},
    {OP_L,      OP_iC,      2, -1, -1,   {0xED, 0x69}},
    {OP_A,      OP_iC,      2, -1, -1,   {0xED, 0x79}},
};

static const struct instruction outi_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xA3}},
};

static const struct instruction outir_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xB3}},
};

static const struct instruction outd_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xAB}},
};

static const struct instruction outdr_instrs[] = {
    {OP_NONE,   OP_NONE,    2, -1, -1,   {0xED, 0xBB}},
};

static const struct instruction nop_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x00}},
};

static const struct instruction halt_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0x76}},
};

static const struct instruction di_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xF3}},
};

static const struct instruction ei_instrs[] = {
    {OP_NONE,   OP_NONE,    1, -1, -1,   {0xFB}},
};

static const struct instruction im_instrs[] = {
    {OP_IM,     OP_NONE,    1,  1, -1,   {0x46}},
};


#define ARR_LEN(a) (sizeof(a) / sizeof((a)[0]))

const struct opcode opcodes_builtin[] = {
    {"ld",      ARR_LEN(ld_instrs),        ld_instrs},
    {"push",    ARR_LEN(push_instrs),      push_instrs},
    {"pop",     ARR_LEN(pop_instrs),       pop_instrs},
    {"ex",      ARR_LEN(ex_instrs),        ex_instrs},
    {"exx",     ARR_LEN(exx_instrs),       exx_instrs},
    {"ldi",     ARR_LEN(ldi_instrs),       ldi_instrs},
    {"ldir",    ARR_LEN(ldir_instrs),      ldir_instrs},
    {"ldd",     ARR_LEN(ldd_instrs),       ldd_instrs},
    {"lddr",    ARR_LEN(lddr_instrs),      lddr_instrs},
    {"cpi",     ARR_LEN(cpi_instrs),       cpi_instrs},
    {"cpir",    ARR_LEN(cpir_instrs),      cpir_instrs},
    {"cpd",     ARR_LEN(cpd_instrs),       cpd_instrs},
    {"cpdr",    ARR_LEN(cpdr_instrs),      cpdr_instrs},
    {"add",     ARR_LEN(add_instrs),       add_instrs},
    {"adc",     ARR_LEN(adc_instrs),       adc_instrs},
    {"sub",     ARR_LEN(sub_instrs),       sub_instrs},
    {"sbc",     ARR_LEN(sbc_instrs),       sbc_instrs},
    {"and",     ARR_LEN(and_instrs),       and_instrs},
    {"xor",     ARR_LEN(xor_instrs),       xor_instrs},
    {"or",      ARR_LEN(or_instrs),        or_instrs},
    {"cp",      ARR_LEN(cp_instrs),        cp_instrs},
    {"inc",     ARR_LEN(inc_instrs),       inc_instrs},
    {"dec",     ARR_LEN(dec_instrs),       dec_instrs},
    {"cpl",     ARR_LEN(cpl_instrs),       cpl_instrs},
    {"neg",     ARR_LEN(neg_instrs),       neg_instrs},
    {"daa",     ARR_LEN(daa_instrs),       daa_instrs},
    {"scf",     ARR_LEN(scf_instrs),       scf_instrs},
    {"ccf",     ARR_LEN(ccf_instrs),       ccf_instrs},
    {"rlca",    ARR_LEN(rlca_instrs),      rlca_instrs},
    {"rlc",     ARR_LEN(rlc_instrs),       rlc_instrs},
    {"rrca",    ARR_LEN(rrca_instrs),      rrca_instrs},
    {"rrc",     ARR_LEN(rrc_instrs),       rrc_instrs},
    {"rla",     ARR_LEN(rla_instrs),       rla_instrs},
    {"rl",      ARR_LEN(rl_instrs),        rl_instrs},
    {"rra",     ARR_LEN(rra_instrs),       rra_instrs},
    {"rr",      ARR_LEN(rr_instrs),        rr_instrs},
    {"sla",     ARR_LEN(sla_instrs),       sla_instrs},
    {"sra",     ARR_LEN(sra_instrs),       sra_instrs},
    {"sll",     ARR_LEN(sll_instrs),       sll_instrs},
    {"srl",     ARR_LEN(srl_instrs),       srl_instrs},
    {"bit",     ARR_LEN(bit_instrs),       bit_instrs},
    {"res",     ARR_LEN(res_instrs),       res_instrs},
    {"set",     ARR_LEN(set_instrs),       set_instrs},
    {"jp",      ARR_LEN(jp_instrs),        jp_instrs},
    {"call",    ARR_LEN(call_instrs),      call_instrs},
    {"ret",     ARR_LEN(ret_instrs),       ret_instrs},
    {"reti",    ARR_LEN(reti_instrs),      reti_instrs},
    {"retn",    ARR_LEN(retn_instrs),      retn_instrs},
    {"jr",      ARR_LEN(jr_instrs),        jr_instrs},
    {"djnz",    ARR_LEN(djnz_instrs),      djnz_instrs},
    {"in",      ARR_LEN(in_instrs),        in_instrs},
    {"ini",     ARR_LEN(ini_instrs),       ini_instrs},
    {"inir",    ARR_LEN(inir_instrs),      inir_instrs},
    {"ind",     ARR_LEN(ind_instrs),       ind_instrs},
    {"indr",    ARR_LEN(indr_instrs),      indr_instrs},
    {"out",     ARR_LEN(out_instrs),       out_instrs},
    {"outi",    ARR_LEN(outi_instrs),      outi_instrs},
    {"outir",   ARR_LEN(outir_instrs),     outir_instrs},
    {"outd",    ARR_LEN(outd_instrs),      outd_instrs},
    {"outdr",   ARR_LEN(outdr_instrs),     outdr_instrs},
    {"nop",     ARR_LEN(nop_instrs),       nop_instrs},
    {"halt",    ARR_LEN(halt_instrs),      halt_instrs},
    {"di",      ARR_LEN(di_instrs),        di_instrs},
    {"ei",      ARR_LEN(ei_instrs),        ei_instrs},
    {"im",      ARR_LEN(im_instrs),        im_instrs},
};

enum operand_type op_type_indir(enum operand_type type) {
    switch (type) {
        case OP_A:  return OP_iA;
        case OP_C:  return OP_iC;
        case OP_BC: return OP_iBC;
        case OP_DE: return OP_iDE;
        case OP_HL: return OP_iHL;
        case OP_SP: return OP_iSP;
        case OP_IX: return OP_iIX;
        case OP_IY: return OP_iIY;
        default:    return OP_INVAL;
    }
}

void operand_free_expr(struct operand *op) {
    if (!op) {
        return;
    }

    switch (op->type) {
    case OP_IMM:
    case OP_IMM8:
    case OP_IMM16:
    case OP_RST:
    case OP_REL:
    case OP_BIT:
    case OP_IM:
    case OP_EXT:
    case OP_PORT:
    case OP_iIX:
    case OP_iIY:
        free(op->expr);
    default:
        break;
    }
}

const struct opcode *opcode_search(const char *mnemonic) {
    for (int i = 0; i < ARR_LEN(opcodes_builtin); i++) {
        if (strcasecmp(opcodes_builtin[i].mnemonic, mnemonic) == 0) {
            return &opcodes_builtin[i];
        }
    }

    return 0;
}

/**
 * Determines whether or not an operand type can be "cast" to another (i.e.
 * the latter could be replaced with it when matching an instruction).
 * It is necessary to use this when matching instructions since all immediate
 * values are stored the same way (because all could be used in multiple
 * situations).
 */
static inline int op_type_castable(enum operand_type t1, enum operand_type t2) {
    /* TODO Should more be done here or should bounds checking be left until the
     * instruction is actually matched?
     */

    if (t1 == OP_IMM) {
        return OP_IMM_START < t2 && t2 < OP_IMM_END;
    } else if (t1 == OP_EXT) {
        return OP_IMM_EXT_START < t2 && t2 < OP_IMM_EXT_END;
    } else {
        return t1 == t2;
    }
}

const struct instruction *opcode_match(const struct opcode *oc,
        const struct operand *op1, const struct operand *op2) {
    if (!oc) {
        return 0;
    }

    enum operand_type t1 = op1 ? op1->type : OP_NONE;
    enum operand_type t2 = op2 ? op2->type : OP_NONE;

    for (int i = 0; i < oc->instr_count; i++) {
        if (op_type_castable(t1, oc->instrs[i].op1)
                && op_type_castable(t2, oc->instrs[i].op2)) {
            return &oc->instrs[i];
        }
    }

    return 0;
}

static int instr_apply_op(uint8_t bytes[INSTR_MAX_LEN], int size,
        int offset, enum operand_type type, const struct operand *op) {
    if (offset < 0) {
        return 0;
    }

    /* Have to enumerate most combinations of operand and symbol types.
     * Constants can be written directly, though relocation entries have to be
     * created for smbols.
     */
    switch (type) {
    case OP_IMM8:
        reltab_add_expr(asm_reloc_table,
                RT_8_BIT, asm_section, asm_pc + offset, 0,
                op->expr);
        bytes[offset] = 0;
        break;

    case OP_PORT:
        reltab_add_expr(asm_reloc_table,
                RT_U_8_BIT, asm_section, asm_pc + offset, 0,
                op->expr);
        bytes[offset] = 0;
        break;

    case OP_REL:
        reltab_add_expr(asm_reloc_table,
                RT_REL_JUMP, asm_section, asm_pc + offset, asm_pc + size,
                op->expr);
        bytes[offset] = 0;
        break;

    case OP_iIX:
    case OP_iIY:
        reltab_add_expr(asm_reloc_table,
                RT_S_8_BIT, asm_section, asm_pc + offset, 0,
                op->expr);
        bytes[offset] = 0;
        break;

    case OP_EXT:
        reltab_add_expr(asm_reloc_table,
                RT_U_16_BIT, asm_section, asm_pc + offset, 0,
                op->expr);

        bytes[offset]   = op->expr->value & 0xFF;
        bytes[offset+1] = (op->expr->value >> 8) & 0xFF;
        break;

    case OP_IMM16:
        reltab_add_expr(asm_reloc_table,
                RT_16_BIT, asm_section, asm_pc + offset, 0,
                op->expr);

        bytes[offset]   = op->expr->value & 0xFF;
        bytes[offset+1] = (op->expr->value >> 8) & 0xFF;
        break;

    case OP_RST:
        /* Valid restart values are 0x00, 0x08, ..., 0x38, or
         * 0b00000000, 0b00001000, ..., 0b00111000 in binary. This value is OR'd
         * with the base instruction (rst 0x00) to produce the others.
         */
        reltab_add_expr(asm_reloc_table,
                RT_RST, asm_section, asm_pc + offset, bytes[offset],
                op->expr);
        /* Don't modify the instruct bytes */
        break;

    case OP_IM:
        /* Interrupt modes are 0, 1, and 2. The value is not incorporated into
         * the instruction like it is for restarts, so we just have to go though
         * the options individually.
         */
        reltab_add_expr(asm_reloc_table,
                RT_IM, asm_section, asm_pc + offset, bytes[offset],
                op->expr);
        /* Don't modify the instruct bytes */
        break;

    default:
        return -1;
    }

    return 0;
}

int instr_output(const struct instruction *instr,
        const struct operand *op1, const struct operand *op2, FILE *stream) {
    if (!instr) {
        return -1;
    }

    /* Prepare the data in a buffer first to check bounds on immediate values */
    uint8_t bytes[INSTR_MAX_LEN];
    memcpy(bytes, instr->bytes, instr->size);

    if (instr_apply_op(bytes, instr->size, instr->op1_off, instr->op1, op1)
            < 0) {
        return -1;
    }

    if (instr_apply_op(bytes, instr->size, instr->op2_off, instr->op2, op2)
            < 0) {
        return -1;
    }

    fwrite(bytes, 1, instr->size, stream);
    asm_pc += instr->size;

    return 0;
}

/* vim: set tw=80 ft=c: */
