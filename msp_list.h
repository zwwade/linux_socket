#ifndef _LIST_H_jq9gjowe_
#define _LIST_H_jq9gjowe_
/* =========================================================================

DESCRIPTION
   Single linked LIST.

Copyright (c) 2012 by ANHUI USTC iFLYTEK, Co,LTD.  All Rights Reserved.
============================================================================ */

/* =========================================================================

                             REVISION

when            who              why
--------        ---------        -------------------------------------------
2012/07/18      chenzhang        Created.
============================================================================ */

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */
typedef struct _list_link {
   struct _list_link *next;
} list_link;

typedef struct _list_node {
   list_link link;
   void *val;
} list_node;

typedef struct {
   list_link *front;
   list_link *back;
   unsigned int size;
} list;

typedef int (*PFNListNodeCompare)(void *user_data, void *item);

/* ------------------------------------------------------------------------
** Function Declarations
** ------------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
#endif

extern list* list_new(void);
extern int list_release(list *li);
extern list_node* list_node_new(void *val, unsigned int extsz, void **extmem);
extern int list_node_release(list_node *n);
extern void* list_node_get(list_node *n);
extern int list_init(list *li);
extern unsigned int list_size(list *li);
extern int list_empty(list *li);
extern list_node* list_peek_front(list *li);
extern list_node* list_peek_back(list *li);
extern list_node* list_peek_next(list *li, list_node *after_which);
extern list_node* list_peek_prev(list *li, list_node *before_which);
extern void list_insert_before(list *li, list_node *item, list_node *before_which);
extern void list_insert_after(list *li, list_node *item, list_node *after_which);
extern void list_remove(list *li, list_node *item);
extern void list_push_front(list *li, list_node *item);
extern list_node* list_pop_front(list *li);
extern void list_push_back(list *li, list_node *item);
extern list_node* list_pop_back(list *li);
extern list_node* list_search(list *li,  PFNListNodeCompare compare_func, void *user_ontext);

#ifdef __cplusplus
}
#endif

#endif /* _LIST_H_jq9gjowe_ */

