// -*- C++ -*-

/* Copyright (C) 2016-  Free Software Foundation, Inc.
   Written by Bertrand Garrigues (bertrand.garrigues@laposte.net)

This file is part of groff.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or
(at your option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>. */

/*
 * Simple doubly linked list implementation.
 * Linux Kernel's list style.
 */

#ifndef LIST_H
#define LIST_H

#ifndef NULL
#define NULL 0
#endif

struct list_head {
  struct list_head *prev;
  struct list_head *next;
  void *container;
};

#define LIST_HEAD_INIT(name, container) { &(name), &(name), container}

static inline void INIT_LIST_HEAD(struct list_head * list, void *container)
{
   list->next = list;
   list->prev = list;
   list->container = container;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head * nnew, struct list_head * prev,
      struct list_head * next)
{
   next->prev = nnew;
   nnew->next = next;
   nnew->prev = prev;
   prev->next = nnew;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head * nnew, struct list_head * head)
{
   __list_add(nnew, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head * nnew,
      struct list_head * head)
{
   __list_add(nnew, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
   next->prev = prev;
   prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void __list_del_entry(struct list_head * entry)
{
   __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head * entry)
{
   __list_del(entry->prev, entry->next);
   entry->next = NULL;
   entry->prev = NULL;
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(struct list_head * entry)
{
   __list_del_entry(entry);
   entry->next = entry;
   entry->prev = entry;
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct list_head * list,
      const struct list_head * head)
{
   return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_head * head)
{
   return head->next == head;
}


/**
 * list_entry - get the struct for this entry
 * @ptr: the &struct list_head pointer.
 * @type:   the type of the struct this is embedded in.
 */
#define list_entry(ptr, type) \
  (type *)((ptr)->container)

/**
 * list_first_entry - get the first element from a list
 * @ptr: the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type) \
  (type *)((ptr)->next->container)


/**
 * list_for_each  -  iterate over a list
 * @pos: the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each(pos, head) \
   for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos: the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 * @head:   the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
   for (pos = (head)->next, n = pos->next; \
        pos != (head); \
        pos = n, n = pos->next)

/**
 * list_for_each_entry  -  iterate over list of given type
 * @pos: the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 * @type: type of the container
 */
#define list_for_each_entry(pos, head, member, type)     \
   for (pos = list_entry((head)->next, type); \
        pos != NULL && &pos->member != (head); \
        pos = list_entry(pos->member.next, type))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against
 * removal of list entry
 * @pos: the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @type: type of the container
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member, type) \
   for (pos = list_entry((head)->next, type), \
            n = list_entry((head)->next->next, type); \
        pos != NULL && &pos->member != (head); \
        pos = n, n = (n != NULL ? list_entry(n->member.next, type) : NULL))
#endif /* LIST_H */
