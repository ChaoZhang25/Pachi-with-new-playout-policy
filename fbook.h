#ifndef PACHI_FBOOK_H
#define PACHI_FBOOK_H

//#include<malloc.h>  
#include "move.h"
#include "board.h"

struct board;

/* Opening book (fbook as in "forcing book" since the move is just
 * played unconditionally if found, or possibly "fuseki book"). */

typedef struct F_Node{
    coord_t move;
    struct F_Node* pNext;
    hash_t hash;
}F_NODE, * PNODE;

struct fbook {
	int bsize;
	int handicap;

	int movecnt;

#define fbook_hash_bits 20 // 12M w/ 32-bit coord_t
#define fbook_hash_mask ((1 << fbook_hash_bits) - 1)
	/* pass == no move in this position */
	coord_t moves[1<<fbook_hash_bits];
	hash_t hashes[1<<fbook_hash_bits];
    F_NODE linked_moves[1<<fbook_hash_bits];
};

coord_t fbook_check(struct board *board);
struct fbook *fbook_init(char *filename, struct board *b);
void fbook_done(struct fbook *fbook);

#endif
