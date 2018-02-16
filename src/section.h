/**
 * @file section.h
 * @author Zach Peltzer
 * @date Created: Mon, 05 Feb 2018
 * @date Last Modified: Mon, 05 Feb 2018
 */

#ifndef SECTION_H_
#define SECTION_H_

/**
 * Section identifiers.
 */
enum section {
    SEC_UNDEF = 0,
    SEC_TEXT = 1,
    SEC_DATA = 2,

    /**
     * Section for values not in any section.
     */
    SEC_ABS = SEC_DATA | SEC_TEXT,
};

#endif /* SECTION_H_ */

/* vim: set tw=80 ft=c: */
