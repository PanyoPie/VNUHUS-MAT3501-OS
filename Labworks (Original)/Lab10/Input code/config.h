#define CONFIG_H

#define FRAMEBITS	2		// bit length of frame address
#define PAGEBITS	3		// bit length of virtual page address
#define OFFSETBITS	12		// bit length of offset address within a frame/page
#define NUMFRAMES	(1<<FRAMEBITS)		// 2^FRAMEBITS
#define NUMPAGES	(1<<PAGEBITS)		// 2^PAGEBITS
#define FRAMESIZE	(1<<OFFSETBITS)		// 2^OFFSETBITS
#define PAGESIZE	FRAMESIZE
#define SPACE		"     "
#define FIFO		1
#define CLOCK		2
#define LRU			3