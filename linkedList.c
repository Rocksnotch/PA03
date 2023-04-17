#include "linkedList.h"

struct localEnv *head = NULL;
struct localEnv *current = NULL;

void insertFirst(char *name, char *data) {
    //create a link
   struct localEnv *link = (struct localEnv*) malloc(sizeof(struct localEnv));
	
   strcpy(link->envName, name);
   strcpy(link->envData, data);
	
   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

int length() { //find the length of the linked list
    int length = 0;
    struct localEnv *temp;

    for (current = head; current != NULL; current = current->next) {
        length++;
    }   

    return length;
}

void printList() { //prints out entire local environment
   struct localEnv *ptr = head;
   printf("\n---Local Environment---\n");
	
   //start from the beginning
   while(ptr != NULL) {
      printf("%s=%s",ptr->envName,ptr->envData);
      ptr = ptr->next;
      printf("\n");
   }
}

struct localEnv* find(char *name) { //find a node

   //start from the first link
   struct localEnv* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(strcmp(current->envName, name) != 0) {
	
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if data found, return the current Link
   return current;
}

void deleteAll() { //used to delete all

   // deref head to get the real head 
   struct localEnv* current = head;
   struct localEnv* next;
 
   while (current != NULL)
   {
       next = current->next;
       free(current);
       current = next;
   }
   
   // deref head to affect the real head back in the caller
   head = NULL;
}