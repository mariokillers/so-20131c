#ifndef STACK_H_
#define STACK_H_

  #include "list.h"

	typedef struct {
		t_list* elements;
	} t_stack;

	t_stack *stack_create();
	void stack_destroy(t_stack *);

	void stack_push(t_stack *, void *element);
	void *stack_pop(t_stack *);
	void *stack_peek(t_stack *);
	void stack_clean(t_stack *);

	int stack_size(t_stack *);
	int stack_is_empty(t_stack *);

#endif /*STACK_H_*/
