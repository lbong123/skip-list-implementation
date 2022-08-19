/* 
leap.c

Implementations for leap list construction and manipulation.

Skeleton written by Grady Fitzaptrick for COMP20007 Assignment 1 2022
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "leap.h"
#include "utils.h"

/* create a new struct which holds the key and pointers*/
typedef struct node node_t;

struct node {
    int val;
    node_t **forward;
};

struct leapList {
    /* IMPLEMENT: Fill in structure. */
    int level;
    int maxHeight;
    double prob;
    node_t *header;
};

struct leapList *newList(int maxHeight, double p, enum problemPart part){
    /* IMPLEMENT: Set up list */
    int i;
    struct leapList *new_list;
    node_t *header;

    /* allocate memory */
    new_list = (struct leapList *)malloc(sizeof(*new_list));
    assert(new_list);
    header = (node_t *)malloc(sizeof(*header));
    assert(header);
    
    new_list->level = 0;
    new_list->prob = p;
    new_list->maxHeight = maxHeight;
    new_list->header = header;
    /* allocate memory for header to point to maxHeight items,
       pointing to a next on each level. */
    header->forward = (node_t **)malloc(sizeof(node_t *) * maxHeight);
    assert(header->forward);

    /* set the head to point to NULL on each level. */
    for (i = 0; i < maxHeight; i++) {
        header->forward[i] = NULL;
    }

    return new_list;
}

void printLevel(struct leapList *list, int level){
    if(! list){
        printf("\n");
        return;
    }
    /* IMPLEMENT (Part B): loop over list at given level, printing out each value. */
    /* Note: while additional next elements, print a space after the key. If no additional next elements, 
    print a new line and then return. */

    node_t *curr = list->header->forward[level];

    while (curr != NULL) {
        /* if the last element on the level, print no space. */
        if (curr->forward[level] == NULL) {
            printf("%d", curr->val);
        }
        else {
            printf("%d ", curr->val);
        }
        curr = curr->forward[level];
    }

    printf("\n");
    return;
}

void insertKey(int key, struct leapList *list){
    /* IMPLEMENT: Insert the key into the given leap list. */
    int i, j, max = list->maxHeight;
    double p = list->prob;
    node_t *curr = list->header;
    node_t *temp;
    node_t *new_node;

    /* turn key into a new node and allocate memory
       (under verified assumption, key is not already in list). */
    new_node = (node_t *)malloc(sizeof(*new_node));
    assert(new_node);
    new_node->val = key;
    new_node->forward = (node_t **)malloc(max * sizeof(new_node));

    i = 0;
    /* only create a new level, if we are not at the maxHeight 
       and the probability condition passes. */
    while (i == 0 || (i < max && ((double) rand() / RAND_MAX < p))) {
        curr = list->header;
        /* use the leap search algorithm (not the most efficient). */
        for (j = list->level; j >= i; j--) {
            while (curr->forward[j] != NULL && curr->forward[j]->val < key) {
                curr = curr->forward[j];
            }
        }

        temp = curr->forward[i];
        curr->forward[i] = new_node;
        new_node->forward[i] = temp;

        /* increases the known max level of the list if needed. */
        if (i > list->level) {
            list->level = i;
        }
        i++;
    }
    

}

/* Queries the leap list for the given key and places the result in the solution structure. */
int findKey(int key, struct leapList *list, enum problemPart part, struct solution *solution){
    int found = NOTFOUND;
    int element = key;
    int baseAccesses = 0;
    int requiredAccesses = 0;
    assert(solution);
    /* IMPLEMENT: Find the given key in the leap list. */

    int i, next_val;
    node_t *curr;
    curr = list->header;

    /* search using leap lists*/
    for (i = list->level; i >= 0; i--) {
        while (curr != NULL && found == NOTFOUND) {
            if (curr->forward[i] == NULL || 
                (i != list->level && curr->forward[i] == curr->forward[i+1])) {
                break;
            }

            /* we can increment the count early since we know a check will have
               to take place in this current iteration. */
            requiredAccesses++;


            /* storing the variable because we have to access it twice to
               check if we have found the key and thus drop down completely */
            next_val = curr->forward[i]->val;
            if (next_val <= key) {
                if (next_val == key) {
                    found = FOUND;
                }
                curr = curr->forward[i];
            }
            else {
                break;
            }
        }
        /* Once we find the key, we can drop down completely and we are done */
        if (found == FOUND) {
            break;
        }
    }


    /* Reset curr to perfom linear search using the base level of leap list */
    curr = list->header->forward[0];
    
    while (curr != NULL) {
        baseAccesses++;
        if (curr->val < key) {
            curr = curr->forward[0];
        }
        else {
            if (curr->val == key) {
                found = FOUND;
            }
            break;
        }
    }

    
    /* Insert result into solution. */
    (solution->queries)++;
    solution->queryResults = (int *) realloc(solution->queryResults, sizeof(int) * solution->queries);
    assert(solution->queryResults);
    (solution->queryResults)[solution->queries - 1] = found;
    solution->queryElements = (int *) realloc(solution->queryElements, sizeof(int) * solution->queries);
    assert(solution->queryElements);
    solution->queryElements[solution->queries - 1] = element;
    solution->baseAccesses = (int *) realloc(solution->baseAccesses, sizeof(int) * solution->queries);
    assert(solution->baseAccesses);
    solution->baseAccesses[solution->queries - 1] = baseAccesses;
    solution->requiredAccesses = (int *) realloc(solution->requiredAccesses, sizeof(int) * solution->queries);
    assert(solution->requiredAccesses);
    solution->requiredAccesses[solution->queries - 1] = requiredAccesses;
    return found;
}

void deleteKey(int key, struct leapList *list, enum problemPart part){
    /* IMPLEMENT: Remove the given key from the leap list. */
    
    int i, j;
    node_t *curr = list->header;
    node_t *temp;

    curr = list->header;

    /* since the previous pointer to the key is potentially different on
       each level - for the levels it is on, we have to perform a leap search 
       treating each level as a base level each time, until all levels 
       have been searched. */
    for (i = list->level; i >= 0; i--) {
        curr = list->header;
        for (j = list->level; j >= i && j >= 0; j--) {
            while (curr->forward[j] != NULL && curr->forward[j]->val < key) {
                curr = curr->forward[j];
            }
        }

        /* use the leap search algorithm */
        if (curr->forward[i] != NULL && curr->forward[i]->val == key) {
            temp = curr->forward[i];
            curr->forward[i] = curr->forward[i]->forward[i];
            
            /* free the deleted node, once we are at base level
               since we know that no other nodes point to it anymore */
            if (i == 0) {
                free(temp->forward);
                temp->forward = NULL;
                free(temp);
                temp = NULL;
            }
        }
    }
    

}

void freeList(struct leapList *list){
    /* IMPLEMENT: Free all memory used by the list. */
    
    node_t *curr = list->header;
    node_t *next;

    /* free all n-1 nodes and all of their respective forward pointers */
    while (curr->forward[0] != NULL) {
        next = curr->forward[0];
        free(curr->forward);
        curr->forward = NULL;
        free(curr);
        curr = next;
    }

    /* free last node */
    free(curr->forward);
    curr->forward = NULL;
    free(curr);
    curr = NULL;

    /* free the leap list */
    free(list);
    list = NULL;

}

void freeSolution(struct solution *solution){
    if(! solution){
        return;
    }
    freeList(solution->list);
    if(solution->queries > 0){
        free(solution->queryResults);
        free(solution->queryElements);
        free(solution->baseAccesses);
        free(solution->requiredAccesses);
    }
    free(solution);
}

