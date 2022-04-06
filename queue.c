#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *li =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (!li)
        return NULL;
    INIT_LIST_HEAD(li);
    return li;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *it, *it_n;
    list_for_each_entry_safe (it, it_n, l, list)
        q_release_element(it);
    free(l);
}

/*
 * New an element for s,
 * It will allocate memory for s
 * Return null if allocation failed.
 */
static element_t *new_element(char *s)
{
    element_t *new_ele = malloc(sizeof(element_t));
    if (new_ele == NULL)
        return NULL;
    new_ele->value = strdup(s);
    if (new_ele->value == NULL) {
        free(new_ele);
        return NULL;
    }
    return new_ele;
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *ele = new_element(s);
    if (!ele)
        return false;
    list_add(&ele->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *ele = new_element(s);
    if (!ele)
        return false;
    list_add_tail(&ele->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *ele = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, ele->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&ele->list);
    return ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head **indir = &(head->next);
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next)
        indir = &(*indir)->next;
    struct list_head *del = *indir;
    *indir = (*indir)->next;
    element_t *ele = list_entry(del, element_t, list);
    list_del(del);
    q_release_element(ele);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;

    element_t *n, *s;
    bool isdup = false;
    list_for_each_entry_safe (n, s, head, list) {
        if ((n->list.next != head) &&
            !strcmp(n->value,
                    list_entry(n->list.next, element_t, list)->value)) {
            list_del(&n->list);
            q_release_element(n);
            isdup = true;
        } else if (isdup) {
            list_del(&n->list);
            q_release_element(n);
            isdup = false;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *node;
    for (node = head->next; (node->next != head) && (node != head);
         node = node->next) {
        struct list_head *next = node->next;
        list_del(node);
        list_add(node, next);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *li = head->next, *prev, *next;
    while (li != head) {
        prev = li->prev, next = li->next;
        li->prev = next;
        li->next = prev;
        li = next;
    }
    struct list_head *tmp = head->prev;
    head->prev = head->next;
    head->next = tmp;
}

/*
 * Merge two sorted list to a sorted one
 * Must ensure the left list isn't empty
 */
static void merge_two_list(struct list_head *left_head,
                           struct list_head *right_head)
{
    struct list_head *safe, *right, *left = left_head->next;
    // If the left list is empty but the right isn't
    if (left == left_head && right_head->next != right_head) {
        list_splice(right_head, left_head);
        return;
    }
    // Put right list's nodes to left
    list_for_each_safe (right, safe, right_head) {
        element_t *l_node = list_entry(left, element_t, list);
        element_t *r_node = list_entry(right, element_t, list);

        int cmp_result = strcmp(l_node->value, r_node->value);
        // If left value <= right value, move the left pointer to it's next
        while (left->next != left_head && cmp_result <= 0) {
            left = left->next;
            l_node = list_entry(left, element_t, list);
            cmp_result = strcmp(l_node->value, r_node->value);
        }
        list_del(right);
        if (cmp_result > 0)
            list_add_tail(right, left);
        else
            list_add(right, left);
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head->next, *fast = head->next->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    // Now the slow pointer is at the middle
    LIST_HEAD(right);

    // Split the list from the next node of slow pointer
    right.next = slow->next;
    right.next->prev = &right;
    right.prev = head->prev;
    right.prev->next = &right;

    slow->next = head;
    head->prev = slow;

    q_sort(&right);
    q_sort(head);

    merge_two_list(head, &right);
}
