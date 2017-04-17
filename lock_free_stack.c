#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>

/* Определяем элемент списка */
typedef struct stack_node {
    _Atomic(struct stack_node *) next;
    void *data;
} stack_node_t;

/* Определяем сам список */
typedef struct stack {
    /* начало списка */
    _Atomic(stack_node_t *) head;
} stack_t;

/* Инициализация стэка */
stack_t * create_stack(void)
{
    stack_t *lt = malloc(sizeof(stack_t));

    lt->head = ATOMIC_VAR_INIT(NULL);

    return lt;
}

void stack_push(stack_t *lt, void * data)
{
	stack_node_t * node = malloc(sizeof(stack_node_t));
	node->data = data;

	stack_node_t *old_head = atomic_load(&(lt->head));
	while(true) {
		atomic_store(&(node->next), old_head);
		if(atomic_compare_exchange_weak(&(lt->head), &old_head, node))
			break;
	}
}


void * stack_pop(stack_t *lt)
{
	while(true) {
		stack_node_t *head = atomic_load(&(lt->head));
		if(head == NULL) {
			return NULL;
		}

		stack_node_t *next = atomic_load(&(lt->head->next));
		if(atomic_compare_exchange_weak(&(lt->head), &head, next)) {
			void *data = head->data;
			free(head);
			return data;
		}
	}
}

int main(int argc, char* argv[])
{
	stack_t *stack = create_stack();
	int data = 0, *ptr;

	/* Добавляем в конец списка */
	stack_push(stack, &data);
	/* Извлекаем из начала списка*/
	ptr = (int *)stack_pop(stack);

	assert(ptr == &data);

	return 0;
}