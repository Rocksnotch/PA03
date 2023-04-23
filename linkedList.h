
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#define MAX_CHAR 1024

struct localEnv {
    char envName[MAX_CHAR];
    char envData[MAX_CHAR];

    struct localEnv *next;
};

void insertFirst(char *name, char *data); //inserts at beginning of list

int length(); //returns length of list

void printList(); //prints ALL elements in the list

struct localEnv* find(char *name); //finds and returns the specific element in list

void deleteAll(); //deletes (and frees up, the most important part) ALL Entries in list!!!

struct localEnv* returnHead();

#endif