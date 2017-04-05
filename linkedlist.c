#include<stdio.h>  
#include<malloc.h>  
#include<stdlib.h>
#include "move.h"

typedef struct Node{
    coord_t move;
    struct Node* pNext;
}NODE, * PNODE;

bool add_move_to_list(PNODE pHead, int val);
void traverseLinkList(PNODE pHead); 

bool add_move_to_list(PNODE pHead, int val){
    PNODE p = pHead;
    while(p != NULL){
        p = p->pNext;
    }
    PNODE new_move = (PNODE)malloc(sizeof(NODE));
    new_move->move = val;
    new_move->pNext = NULL;
    p->pNext = new_move;
}
