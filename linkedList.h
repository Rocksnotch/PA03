
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

struct localEnv {
    char envName[1024];
    char envData[1024];

    struct localEnv *next;
};

void insertFirst(char *name, char *data); //inserts at beginning of list

int length(); //returns length of list

void printList(); //prints ALL elements in the list

struct localEnv* find(char *name); //finds and returns the specific element in list

void deleteAll(); //deletes (and frees up, the most important part) ALL Entries in list!!!

#endif