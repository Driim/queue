#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>

void list_lock(atomic_flag lock)
{
	/* 
	 * функция устанавливает флаг и возвращает true если флаг уже был установлен,
	 * по-этому мы ждем пока функция не вернет false, означающий что флаг был установлен
	 * после очистки, т.е. мы захватили блокировку 
	 */
	while(atomic_flag_test_and_set(&lock) != false);
}

void list_unlock(atomic_flag lock)
{
	atomic_flag_clear(&lock);
}

/* Определяем элемент списка */
typedef struct list_node {
    struct list_node *next;
    void *data;
} list_node_t;

/* Определяем сам список */
typedef struct list {
	/* наш spinlock */
	atomic_flag lock;
    /* Сделаем размер атомарной переменной */
    atomic_int size;
    /* начало списка */
    list_node_t *head;
    /* конец списка */
    list_node_t *tail;
} list_t;

/* Инициализация массива */
list_t * create_list(void)
{
    list_t *lt = malloc(sizeof(list_t));

    atomic_flag_clear(&lt->lock);

    atomic_store(&(lt->size), 0);
    lt->head = NULL;
    lt->tail = lt->head;

    return lt;
}

/* Добавляем элемент в начало списка */
void list_push(list_t *lt, void * data)
{
    list_node_t * node = malloc(sizeof(list_node_t));
    node->data = data;

    list_lock(lt->lock);

    node->next = lt->head;
    lt->head = node;
    /* должна быть последней */
    atomic_fetch_add(&(lt->size), 1);

    list_unlock(lt->lock);
}

/* Извлекаем элемент из начала списка */
void * list_pop(list_t *lt)
{
	/* операция атомарная и не требует захвата блокировки */
    if(atomic_load(&(lt->size)) == 0){
        /* 
         * Список пуст и даже если кто-то уже добавляет туда элемент,
         * мы не ломаем очередь,
         * просто возвращает не совсем верную информацию.
         */
        return NULL;
    }

    list_lock(lt->lock);
    /* должна быть первой */
    atomic_fetch_sub(&(lt->size), 1);

    list_node_t *node = lt->head;
    void * ret_val = node->data;
    lt->head = node->next;

    if(atomic_load(&(lt->size)) == 0){
        /* Это был последний элемент */
        lt->head = NULL;
        lt->tail = NULL;
    }

    list_unlock(lt->lock);

    free(node);

    return ret_val;
}

/* Добавляем элемент в конец списка */
void list_push_back(list_t *lt, void * data)
{
    list_node_t * node = malloc(sizeof(list_node_t));
    node->data = data;

    list_lock(lt->lock);

    if(lt->tail != NULL)
        lt->tail->next = node;
    else {
        lt->head = node;
    }

    lt->tail = node;
    lt->size += 1;

    list_unlock(lt->lock);
}

int main(int argc, char* argv[])
{
	list_t *queue = create_list();
	int data = 0, *ptr;

	/* Добавляем в конец списка */
	list_push_back(queue, &data);
	/* Извлекаем из начала списка*/
	ptr = (int *)list_pop(queue);
}