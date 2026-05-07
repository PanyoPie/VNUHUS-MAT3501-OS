#ifndef PAGE_SUPERVISOR_H
#define PAGE_SUPERVISOR_H

#include "config.h"

/*
 * page_supervisor.h
 * =================
 * The Page Supervisor handles page faults raised by the MMU.
 * When a virtual page is not in the page table (valid bit = 0),
 * the MMU calls handle_page_fault() to:
 *   1. Find a free frame, OR evict an existing page via the
 *      replacement algorithm.
 *   2. Load the requested page into the chosen frame.
 *   3. Update the page table.
 */

/* Initialise the supervisor (free-frame tracking etc.) */
void initPageSupervisor(void);

/*
 * handle_page_fault(vpn)
 * ----------------------
 * Called when virtual page 'vpn' is not present in RAM.
 * Returns the physical frame number where the page is now loaded.
 */
unsigned int handlePageFault(unsigned int vpn);

#endif /* PAGE_SUPERVISOR_H */
