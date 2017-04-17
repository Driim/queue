#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>

/* Определяем элемент списка */
typedef struct list_node {
    _Atomic(struct list_node *) next;
    void *data;
} list_node_t;

/* Определяем сам список */
typedef struct list {
    /* начало списка */
    _Atomic(list_node_t *) head;
    /* конец списка */
    _Atomic(list_node_t *) tail;
    list_node_t divider;
} list_t;

list * create_list(void)
{
	list_t * lt = malloc(sizeof(list_t));

	lt->head = ATOMIC_VAR_INIT(NULL);
	lt->tail = ATOMIC_VAR_INIT(NULL);

	lt->divider.next = NULL;
}

void queue_push_back(list_t *lt, list_node_t *n_node)
{
	list_node_t * tail;

	while(true) {
		list_node_t *next;

		tail = atomic_load(&(lt->tail));
		next = atomic_load(&(lt->tail->next));

		if(next != NULL) {
			/* подчищаем за другим потоком */
			atomic_compare_exchange_weak(&(lt->tail), tail, next);
			continue;
		}

		if(atomic_compare_exchange_strong(&(tail->next), &next, n_node)) {
			/* добавили в очередь */
			break;
		}
	}

	/* теперь меняем сам хвост */
	atomic_compare_exchange_strong(&(lt->tail), &tail, n_node);
	/* если не удалось изменить то не страшно, в другом потоке за нами подчистят */

}

list_node_t * queue_pop_front(list_t *lt)
{
	list_node_t * head, *next, *tail;

	while(true) {
		head = atomic_load(&(lt->head));
		next = atomic_load(&(lt->head->next));

		if(next == NULL)
			return NULL;

		tail = atomic_load(&(lt->tail));
		if(tail == head) {
			/* есть голова и следующий за ним элемент, значит хвоста тут не должно быть */
			atomic_compare_exchange_strong(&(lt->tail), &tail, next);
			continue;
		}

		if(atomic_compare_exchange_strong(&(lt->head), &head, next)) {
			break;
		}
	}

	/* голова всегда указывает на dummy элемент */
	return next;
}

int main(int argc, char* argv[])
{
	list_t *list = create_list();
	int data = 0, *ptr;

	/* Добавляем в конец списка */
	queue_push_back(list, &data);
	/* Извлекаем из начала списка*/
	ptr = (int *)queue_pop_front(list);

	assert(ptr == &data);

	return 0;
}