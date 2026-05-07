#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include "config.h"

/* ---------------------------------------------
 * Replacement module public API
 *
 * initReplacement()   – initialise internal data structures
 * notifyLoad(frame)   – tell the replacer a frame was just loaded
 * notifyAccess(frame) – tell the replacer a frame was just used
 * choose_victim()      – return the frame number to evict
 * --------------------------------------------- */

void initReplacement(ReplacementAlgo a);
void notifyLoad(unsigned int frame);
void notifyAccess(unsigned int frame);
unsigned int chooseVictim(void);
void printReplacementInfo();

#endif /* REPLACEMENT_H */
