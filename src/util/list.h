/** @file list.h
 * @brief Linked lists implemented entirely in C preprocessor
 */

#ifndef IL_LIST_H
#define IL_LIST_H

/** @section Description
 *
 * This is some support code for the engine, to simplify creation of data
 * structures used throughout. This is a simple doubley linked list
 * implementation. It is entirely made using C preprocessor. All macros
 * follow the form of:

        IL_LIST_<NAME>([list,] el [, item], ...)

 * List is a constant struct that represents the head/tail of your linked list
 * El is the identifier used for the list in your struct (see below)
 * Item is a pointer to a specific item in your linked list
 *
 * Make sure the expressions handed to the macros are "pure", in that they
 * have no side effects. They may be expanded many times throughout a macro.
 *
 * The following is how you would create a linked list struct for your code:

        struct MyStruct {
            int n;
            char *whatever;
            IL_LIST(struct MyStruct) ll;
        };

 * You would then create a global instance of this struct to contain head/tail:

        struct MyStruct MyStruct_list;

 * You can then perform various operations on your linked lists
 * Getting the next element:

        struct MyStruct *mystruct = ...;
        struct MyStruct *next = IL_LIST_NEXT(ll, mystruct);

 * Appending an element:

        struct MyStruct *mystruct = ...;
        IL_LIST_APPEND(MyStruct_list, ll, mystruct);

 */

#define IL_LIST(T) struct {T *next; T *last;}

#define IL_LIST_NEXT(el, item) ((item)->el.next)

#define IL_LIST_PREV(el, item) ((item)->el.last)

#define IL_LIST_HEAD(list, el) ((list).el.next)

#define IL_LIST_TAIL(list, el) ((list).el.last)

#define IL_LIST_POPHEAD(list, el, out) {    \
    (out) = IL_LIST_HEAD(list, el);         \
    if (out)                                \
        (list)->el.next = (out)->el.next;   \
    /* in case this is the only item */     \
    if (!(list)->el.next)                   \
        (list)->el.last = NULL;             \
}

#define IL_LIST_POPTAIL(list, el, out) {    \
    (out) = (list)->el.last;                \
    if (out)                                \
        (list)->el.last = (out)->el.last;   \
    /* in case this is the only item */     \
    if (!list->el.last)                     \
        (list)->el.next = NULL;             \
}

#define IL_LIST_INDEX(list, el, out, id)    \
    while (out && id > 0) {                 \
        (out) = (out)->el.next;             \
        id--;                               \
    }

#define IL_LIST_PREPEND(list, el, item) {                                     \
    (item)->el.next = (list).el.next;   /* set the item's next to the head */ \
    if ((list).el.next)                 /* if it exists, */                   \
        (list).el.next->el.last = (item);/* set the head's tail to the item */\
    (list).el.next = (item);            /* set the head to the item */        \
    (item)->el.last = (list);           /* set the item's last to the list */ \
    if (!(list)->el.last)               /* in case we're the only item */     \
        (list)->el.last = (item);                                             \
}

#define IL_LIST_APPEND(list, el, item) {                                      \
    if ((list)->el.last)                /* set the tail's next to the item */ \
        (list)->el.last->el.next = (item);                                    \
    (item)->el.last = (list)->el.last;  /* set the item's last to the tail */ \
    (list)->el.last = (item);                  /* set the tail to the item */ \
    if (!list)->el.next)                /* in case we're the only item */     \
        (list)->el.next = (item);                                             \
}

#endif

