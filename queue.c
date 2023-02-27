#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *queue = malloc(sizeof(struct list_head));
    if (!queue)
        return NULL;
    INIT_LIST_HEAD(queue);
    return queue;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list)
        q_release_element(entry);
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *temp = malloc(sizeof(element_t));
    if (!temp)
        return false;
    temp->value = strdup(s);
    if (!temp->value) {
        free(temp);
        return false;
    }
    list_add(&temp->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *temp = malloc(sizeof(element_t));
    if (!temp)
        return false;
    temp->value = strdup(s);
    if (!temp->value) {
        free(temp);
        return false;
    }
    list_add_tail(&temp->list, head);
    return true;
}
/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *temp = list_entry(head->next, element_t, list);
    list_del(head->next);
    if (sp) {
        strncpy(sp, temp->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return temp;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *temp = list_entry(head->prev, element_t, list);
    list_del(head->prev);
    if (sp) {
        strncpy(sp, temp->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return temp;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int length = 0;
    struct list_head *li;
    list_for_each (li, head)
        length++;
    return length;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head)
        return false;
    struct list_head *behind = head->next, *back = head->prev;
    while (behind != back && behind->next != back) {
        behind = behind->next;
        back = back->prev;
    }
    list_del(behind);
    q_release_element(list_entry(behind, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    element_t *entry, *next, *temp;
    list_for_each_entry_safe (entry, next, head, list) {
        if (&next->list != head && !strcmp(entry->value, next->value)) {
            while (&next->list != head && !strcmp(entry->value, next->value)) {
                temp = next;
                next = list_entry(next->list.next, element_t, list);
                q_release_element(temp);
            }
            entry->list.prev->next = &next->list;
            next->list.prev = entry->list.prev;
            q_release_element(entry);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *first = head->next;
    for (struct list_head *next = first->next; first != head && next != head;
         first = first->next, next = first->next) {
        list_del(first);
        list_add(first, next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (head == NULL)
        return;
    struct list_head *position, *n, *temp;
    list_for_each_safe (position, n, head) {
        temp = position->next;
        position->next = position->prev;
        position->prev = temp;
    }
    temp = head->next;
    head->next = head->prev;
    head->prev = temp;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head)
        return;
    struct list_head *entry, *safe, start, *target;
    INIT_LIST_HEAD(&start);
    target = head;
    int count = 0;
    list_for_each_safe (entry, safe, head) {
        count++;
        if (count == k) {
            list_cut_position(&start, target, entry);
            q_reverse(&start);
            list_splice_init(&start, target);
            target = safe->prev;
            count = 0;
        }
    }
}
void merge(struct list_head *left,
           struct list_head *right,
           struct list_head *result)
{
    while (!list_empty(left) && !list_empty(right)) {
        element_t *first = list_entry(left->next, element_t, list);
        element_t *second = list_entry(right->next, element_t, list);
        if (strcmp(first->value, second->value) <= 0)
            list_move_tail(&first->list, result);
        else
            list_move_tail(&second->list, result);
    }
    if (!list_empty(left))
        list_splice_tail(left, result);
    else
        list_splice_tail(right, result);
}

void q_sort(struct list_head *head)
{
    if (list_empty(head) || list_is_singular(head))
        return;
    struct list_head *slow = head, *fast = head;
    do {
        fast = fast->next->next;
        slow = slow->next;
    } while (fast != head && fast->next != head);
    LIST_HEAD(left);
    LIST_HEAD(right);
    list_splice_tail_init(head, &right);
    list_cut_position(&left, &right, slow);
    q_sort(&left);
    q_sort(&right);
    merge(&left, &right, head);
}
/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *entry = list_entry(head->prev, element_t, list),
              *ori = list_entry(entry->list.prev, element_t, list);
    char *largest = entry->value;
    while (&entry->list != head) {
        if (strcmp(entry->value, largest) < 0) {
            list_del(&entry->list);
            q_release_element(entry);
        } else {
            largest = entry->value;
        }

        entry = ori;
        ori = list_entry(entry->list.prev, element_t, list);
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    queue_contex_t *first = list_first_entry(head, queue_contex_t, chain),
                   *ctx = NULL;
    int sum = first->size;
    list_for_each_entry (ctx, head, chain) {
        if (ctx == first)
            continue;
        sum += ctx->size;
        list_splice_tail_init(ctx->q, first->q);
    }
    q_sort(first->q);
    return sum;
}
