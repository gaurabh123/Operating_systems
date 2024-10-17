// list/list.c
// 
// Implementation for linked list.
//
// Gaurav Adhikari 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"

list_t *list_alloc() { 
  list_t* mylist =  (list_t *) malloc(sizeof(list_t)); 
  mylist->head = NULL;
  return mylist;
}

void list_free(list_t *l) {
  node_t *cur = l->head;
  node_t *next;
  while(cur != NULL){
    next = cur->next;
    free(cur);
    cur = next;
  }
  free(l);
}

node_t * getNode(elem value) {
  node_t *mynode;

  mynode = (node_t *) malloc(sizeof(node_t));
  mynode->value = value;
  mynode->next = NULL;

  return mynode;
}


void list_print(list_t *l) {
  node_t* cur = l->head;
  while (cur != NULL) {
    printf("%d ", cur->value);
    cur = cur->next;
  }
  printf("\n");
}

char *listToString(list_t *l) {
  char *buf = (char *) malloc(sizeof(char) * 10024);
  char tbuf[20];

	node_t *cur = l->head;
  while (cur != NULL) {
    sprintf(tbuf, "%d->", cur->value);
    strcat(buf, tbuf);
    cur = cur->next;
  }
  strcat(buf, "NULL");

  return buf;
}

int list_length(list_t *l) {
  int length = 0;
  node_t *cur = l->head;
  while(cur != NULL){
    length++;
    cur = cur->next;
  }
  return length;
}

void list_add_to_back(list_t *l, elem value) {
  node_t* new_node = (node_t*) malloc(sizeof(node_t));
  new_node ->value = value;
  new_node -> next = NULL;

  if(l->head == NULL){
    l->head = new_node;
  }else{
    node_t *cur = l->head;
    while (cur ->next != NULL){
      cur = cur->next;
    }
    cur->next = new_node;
  }

}
void list_add_to_front(list_t *l, elem value) {
  node_t *new_node = (node_t *) malloc(sizeof(node_t));
  new_node->value = value;
  new_node->next = l->head;
  l->head = new_node;
}

void list_add_at_index(list_t *l, elem value, int index) {

  if (index < 1 || index > list_length(l)) {
    return;
  };

  if (index == 1) {
    list_add_to_front(l, value);
    return;
  }

  node_t *new_node = (node_t *) malloc(sizeof(node_t));
  new_node->value = value;
  node_t *curr = l->head;
  int i = 1;

  while (i < index - 1) {
    if (curr == NULL) {
      free(new_node);
      return;
    }
    curr = curr->next;
    i += 1;
  }
  if (curr == NULL) {
    free(new_node);
    return;
  }
  new_node->next = curr->next;
  curr->next = new_node;
}

elem list_remove_from_back(list_t *l) { 
  
  if (l == NULL || l->head == NULL) {
    return -1;
  };

  node_t *curr = l->head;
  if (curr->next == NULL) {
    elem val = curr->value;
    free(curr);
    l->head = NULL;
    return val;
  };

  while (curr->next->next != NULL) {
    curr = curr->next;
  };

  elem val = curr->next->value;
  free(curr->next);
  curr->next = NULL;
  return val;
}

elem list_remove_from_front(list_t *l) {
  if (l == NULL || l->head == NULL) {
    return -1;
  }
  
  node_t *curr = l->head;
  elem val = curr->value;
  l->head = curr->next;
  free(curr);

  return val;
}

elem list_remove_at_index(list_t *l, int index) { 
  if (l->head == NULL) {
    return -1;
  }

  if (index == 1) {
    return list_remove_from_front(l);
  }

  node_t *curr = l->head;
  int i = 1;

  while (curr != NULL && i != index - 1) {
    curr = curr->next;
    i += 1;
  }

  if (curr == NULL || curr->next == NULL) {
    return -1;
  }

  node_t *next = curr->next;
  elem val = next->value;
  curr->next = next->next;
  free(next);

  return val;
}

bool list_is_in(list_t *l, elem value) {
  node_t *cur =l->head;
  while (cur !=NULL){
    if (cur->value == value) return true;
    cur = cur->next;
  }
  return false;
}

elem list_get_elem_at(list_t *l, int index) { 
  node_t *curr;
  curr = l->head;
  int i = 1;

  while (curr != NULL && i != index) {
    curr = curr->next;
    i += 1;
  };

  if (curr == NULL) {
    return -1;
  }

  return curr->value;
}

int list_get_index_of(list_t *l, elem value) { 
  node_t *curr;
  curr = l->head;
  int i = 1;

  while (curr != NULL) {
    if (curr->value == value) {
      return i;
    }
    curr = curr->next;
    i += 1;
  }
  return -1;
}