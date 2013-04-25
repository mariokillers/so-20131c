#include <stdlib.h>
#include "stack.h"

/*
 * @NAME: stack_create
 * @DESC: Crea y devuelve un puntero a una pila
 */
t_stack *stack_create() {
	t_stack* stack = malloc(sizeof(t_stack));
	t_list* elements = list_create();
	stack->elements = elements;
	return stack;
}

/*
 * @NAME: stack_clean
 * @DESC: Elimina todos los nodos de la pila.
 */
void stack_clean(t_stack *self) {
	list_clean(self->elements);
}

/*
 * @NAME: stack_clean_and_destroy_elements
 * @DESC: Elimina todos los elementos de la pila.
 */
void stack_clean_and_destroy_elements(t_stack *self, void(*element_destroyer)(void*)) {
	list_clean_and_destroy_elements(self->elements, element_destroyer);
}

/*
 * @NAME: stack_destroy
 * @DESC: Destruye una pila.
 */
void stack_destroy(t_stack *self) {
	list_destroy(self->elements);
	free(self);
}

/*
 * @NAME: stack_destroy_and_destroy_elements
 * @DESC: Destruye una pila, recibiendo como argumento el metodo encargado de liberar cada
 * 		elemento de la pila.
 */
void stack_destroy_and_destroy_elements(t_stack *self, void(*element_destroyer)(void*)) {
	list_destroy_and_destroy_elements(self->elements, element_destroyer);
	free(self);
}

/*
 * @NAME: stack_push
 * @DESC: Agrega un elemento a la pila
 */
void stack_push(t_stack *self, void *element) {
	list_add(self->elements, element);
}

/*
 * @NAME: stack_pop
 * @DESC: Extrae y devuelve el primer elemento de la pila
 */
void *stack_pop(t_stack *self) {
	return list_remove(self->elements, (list_size(self->elements)-1));
}

/*
 * @NAME: stack_peek
 * @DESC: Devuelve el primer elemento de la pila sin extraerlo
 */
void *stack_peek(t_stack *self) {
	return list_get(self->elements, (list_size(self->elements)-1));
}

/*
 * @NAME: stack_size
 * @DESC: Devuelve la cantidad de elementos de la pila
 */
int stack_size(t_stack* self) {
	return list_size(self->elements);
}

/*
 * @NAME: stack_is_empty
 * @DESC: Verifica si la pila esta vacía
 */
int stack_is_empty(t_stack *self) {
	return list_is_empty(self->elements);
}
