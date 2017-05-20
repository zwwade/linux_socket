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
#include "stdlib.h"
#include "memory.h"
#include "msp_list.h"

/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */
#define MALLOC(size) malloc(size)
#define MFREE(p) free(p)

/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Global Variable Definitions
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Function Definitions
** ------------------------------------------------------------------------ */
list* list_new(void)
{
   return (list *)MALLOC(sizeof(list));
}

int list_release(list *li)
{
   if (NULL != li)
      MFREE((void *)li);
   return 0;
}

list_node* list_node_new(void *val, unsigned int extsz, void **extmem)
{
   list_node *n = (list_node *)MALLOC(sizeof(list_node) + extsz);
   
   if (NULL != n) {
      n->val = val;
      if (extsz > 0) {
         void *usrmem = (void *)((char *)n + sizeof(list_node));

         if (NULL != extmem)
            *extmem = usrmem;
         if (NULL == val)
            n->val = usrmem;
	  }
   }

   return n;
}

int list_node_release(list_node *n)
{
   if (NULL != n)
      MFREE((void *)n);
   return 0;
}

void* list_node_get(list_node *n)
{
   void *val = NULL;

   if (NULL != n)
      val = n->val;
   return val;
}

int list_init(list *li)
{
   memset((void *)li, 0, sizeof(list));
   return 0;
}

unsigned int list_size(list *li)
{
	if (NULL == li)
		return 0;
	else
		return li->size;
}

int list_empty(list *li)
{
	if (NULL == li)
		return 1; //list is empty
	else
		return (li->size == 0);
}

list_node* list_peek_front(list *li)
{
	if (NULL == li)
		return NULL;
	else
		return (list_node *)li->front;
}

list_node* list_peek_back(list *li)
{
	if (NULL == li)
		return NULL;
	else
		return (list_node *)li->back;
}


list_node* list_peek_next(list *li, list_node *after_which)
{
	if (NULL == after_which)
		return NULL;
	else
		return (list_node *)after_which->link.next;
}


list_node* list_peek_prev(list *li, list_node *before_which)
{
   list_link *before_which_l = (list_link *)before_which;
   list_link *temp;
   
   if (NULL == li)
	   return NULL;

   if ( before_which_l == li->front )
      return NULL;
    
   temp = li->front;
   while ( temp != NULL && temp->next != before_which_l )
      temp = temp->next;
		 
   return (list_node *)temp;
}


void list_insert_before(list *li, list_node *item, list_node *before_which)
{
   list_link *item_l = (list_link *)item;
   list_link *before_which_l = (list_link *)before_which;

   if (NULL == li)
	   return;

   item_l->next = before_which_l;
   if ( li->front == before_which_l )
      li->front = item_l;
   else {
      list_link *temp = li->front;
	  
      while ( temp->next != before_which_l )
         temp = temp->next;
      temp->next = item_l;
   }
   li->size++;

   return;
}


void list_insert_after(list *li, list_node *item, list_node *after_which)
{
   list_link *item_l = (list_link *)item;
   list_link *after_which_l = (list_link *)after_which;
   
   if (NULL == li)
	   return;

   item_l->next = after_which_l->next;
   after_which_l->next =item_l;
   if ( li->back == after_which_l )
      li->back = item_l;
   li->size++;

   return;
}


void list_remove(list *li, list_node *item)
{
   list_link *item_l = (list_link *)item;
   
   if (NULL == li)
	   return;

   if ( li->front == item_l ) {
      li->front = item_l->next;
      if ( li->back == item_l)
         li->back = NULL;
   }
   else {
      list_link *temp = li->front;
	  
      while ( temp->next != item_l )
         temp = temp->next;
		 
      temp->next = item_l->next;
      if ( li->back == item_l )
         li->back = temp;
   }
   li->size--;

   return;
}


void list_push_front(list *li, list_node *item)
{
   list_link *item_l = (list_link *)item;
   
   if (NULL == li)
	   return;

   item_l->next = li->front;
   li->front = item_l;
   if( NULL == li->back )
      li->back = item_l;
   li->size++;
   
   return;
}


list_node* list_pop_front(list *li)
{
   list_link *ret_l = NULL;
   
   if (NULL == li)
	   return NULL;

   if( li->size > 0 ) {
      ret_l = li->front;
      li->front = ret_l->next;
      if( NULL == li->front )
         li->back = NULL;
      li->size--;      
   }
   
   return (list_node *)ret_l;
}


void list_push_back(list *li, list_node *item)
{
   list_link *item_l = (list_link *)item;
   
   if (NULL == li)
	   return;

   item_l->next = NULL;
   if ( 0 == li->size )
      li->front = item_l;
   else
      li->back->next = item_l;
   li->back = item_l;
   li->size++;
   
   return;
}


list_node* list_pop_back(list *li)
{
   list_link *ret_prev_l = NULL;
   list_link *ret_l = NULL;
   
   if (NULL == li)
	   return NULL;

   if( li->size > 0 ) {
      ret_l = li->back;
	  
      if ( li->front == ret_l )
         li->back = li->front = NULL;
      else {
         ret_prev_l = li->front;
         while ( ret_prev_l->next != ret_l )
            ret_prev_l = ret_prev_l->next;
         li->back = ret_prev_l;
         ret_prev_l->next = NULL;
      }
	  
      li->size--;
   }
   
   return (list_node *)ret_l;
}

list_node* list_search(list *li,  PFNListNodeCompare compare_func, void *udata)
{
   list_link *l = NULL;

   if (NULL == li)
	   return NULL;

   l = li->front;
   while (NULL != l) {
	  if (compare_func(udata, ((list_node *)l)->val))
         return (list_node *)l; 
      l = l->next;
   }

   return NULL;
}
