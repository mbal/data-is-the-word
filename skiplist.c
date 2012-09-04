#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define MAXLEVEL 16
/* to get the best theoretical performances, this list can hold up to 2**16
 * elements. If more than 2**16 elements are saved, the complexity is no
 * longer guaranteed to be logN; usual values for MAXLEVEL are from 16 to 25.*/

typedef enum {
    ERROR_MALLOC,
    ERROR_NOT_FOUND,
    ERROR_DUPLICATE,
    OK,
} ret_codes;

typedef struct tcell {
    int value;
    int level;
    struct tcell *forward[1]; //this will be resized at runtime, with malloc
} cell;

typedef struct {
    cell *head;
    int level;
} slist;

int random_level();
void print_list(slist);
void setup_list(slist *);
int insert(slist *, int);
int delete(slist *, int);
cell *make_node(int, int);
cell *search(slist *, int);
cell *seqsearch(slist *, int);
void delete_list(slist *);

int main(int argc, char *argv[])
{
    slist list;
    int i;
    setup_list(&list);
    
    srand(time(NULL));
    for (i = 0; i < 100; i++)
        insert(&list, i+1);
    insert(&list, 100);
    insert(&list, 101);
    insert(&list, 0);
    print_list(list);
    for (i = 0; i < 100; i++)
        search(&list, rand()%1000);
    for (i = 0; i < 50; i++)
        delete(&list, rand()%100);
    delete_list(&list);
    //free(list);
    return 0;

}

void setup_list(slist *list)
{
    int i;
    list->head = malloc(sizeof(cell) + sizeof(cell *) * MAXLEVEL);
    for (i = 0; i < MAXLEVEL; i++)
        list->head->forward[i] = NULL;
    // this value doesn't really matters, it should be -inf, 
    // but any value works, since it's always ignored.
    list->head->value = 0;
    list->head->level = 0;
}

void delete_list(slist *list)
{
    cell *x, *tmp; 
    for(x = list->head->forward[0]; x != NULL; x = tmp)
    {
        tmp = x->forward[0];
        free(x);
    }
    free(list->head);
}

void print_list(slist list)
{
    int i;
    cell *tmp;
    for (tmp = list.head->forward[0]; tmp != NULL; tmp = tmp->forward[0])
    {
        printf("%x %d ",tmp, tmp->value);
        for (i = 0; i < tmp->level; i++)
            printf("%x ", tmp->forward[i]);
        printf("\n");
    }
    return;
}

cell *make_node(int level, int element)
{
    cell *new_cell;
    if ((new_cell = malloc(sizeof(cell) + level * sizeof(cell *))) == 0)
        return ERROR_MALLOC; 
    new_cell->value = element;
    new_cell->level = level;
    return new_cell;
}

cell *seqsearch(slist *list, int element)
{
    cell *x;
    for (x = list->head->forward[0]; x->forward[0] != NULL; x = x->forward[0]) {
        if (x->value == element)
            return x;
    }
    return NULL;
}

cell *search(slist *list, int element)
{
   int i;
   cell *x;
   x = list->head;
   for (i = list->level-1; i >= 0; i--)
   {
       while (x->forward[i] != NULL && x->forward[i]->value < element) {
           x = x->forward[i];
        }
    }
   x = x->forward[0];
   if (x != NULL && x->value == element)
       return x;
   return NULL;   
}

int insert(slist *list, int element)
{
    int i, lvl;
    cell *update[MAXLEVEL];
    cell *x = list->head;
    //find the position where the new element can be
    //inserted, since this is an ordered list
    for (i=list->level-1; i >= 0; i--) {
        while (x->forward[i] != NULL && x->forward[i]->value < element)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[0];
    //if the element is already in the list, stop
    if (x != NULL && x->value == element)
        return ERROR_DUPLICATE;
    lvl = random_level();
    if (lvl > list->level) {
        /* this list can grow only one level at time; this is different
         * from the original paper, where the list grows up to MAXLEVEL
         * levels. However this small change shouldn't change the 
         * running time of the algorithm. */
        list->level += 1;
        lvl = list->level;
        update[lvl-1] = list->head;
    }
    x = make_node(lvl, element);
    for (i = 0; i < lvl; i++) {
        x->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = x;
    }
    return OK;
}

int delete(slist *list, int key)
{
    int i;
    cell *x;
    cell *update[MAXLEVEL];
    x = list->head;
    for (i = list->level-1; i >= 0; i--) {
        while (x->forward[i] != NULL && x->forward[i]->value < key)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[0];
    if (x != NULL && x->value == key) {
        for (i = 0; i < list->level; i++) {
            if (update[i]->forward[i] == x)
                update[i]->forward[i] = x->forward[i];
            else
                break;
        }
        free(x);
        while (list->level > 1 && list->head->forward[list->level] == NULL)
            list->level--;
        return OK;
    }
    return ERROR_NOT_FOUND;
}

int random_level()
{
    /* this function uses as probability 1/2, which is not the
    best option, since in various tests often 1/3 works better
    and 1/e is the theoretical optimum. However, in real code
    this shouldn't be a problem (nitpick, as someone could say)*/
    
    /*it should be possible to optmize this function
    using an "even/odd" approach on a bit level, but it's
    probably not worth it, since it's quite fast as is.
    Something like this:
        lvl = 1
        number = rand();
        while (number % 2 == 0 && lvl < MAXLEVEL) {
            lvl += 1
            number /= 2
        }
    However, since rand() returns a 16 bit number (I think), 
    the maximum value that lvl can assume is 16. It should be possible 
    implement correctly this strategy using a bit more code, but 
    maybe the performance won't be as good as expected, so I'll leave it here.
    */
    int lvl = 1;
    while ((rand()&1) == 0 && lvl < MAXLEVEL)
        lvl++;
    return lvl;
}
