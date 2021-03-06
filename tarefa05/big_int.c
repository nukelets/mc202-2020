#include "big_int.h"
#include <string.h>

void handle_no_memory_error(void)
{
	fprintf(stderr, "INSUFFICIENT MEMORY\n");
	exit(1);
}

big_int* create_big_int(void)
{
	node *pFirstDigit = malloc(sizeof(*pFirstDigit));

	if (pFirstDigit == NULL)
		handle_no_memory_error();

	pFirstDigit->previous = NULL;
	pFirstDigit->next = NULL;

	big_int *pBigInt = malloc(sizeof(*pBigInt));

	if (pBigInt == NULL)
		handle_no_memory_error();

	pBigInt->first = pFirstDigit;
	pBigInt->last = pFirstDigit;

	return pBigInt;
}

void destroy_big_int(big_int *pBigInt)
{
	node *it, *next = NULL;

	// iterates through the list, destroying each element
	for (it = pBigInt->first; it != pBigInt->last; it = next)
	{
		next = it->next;
		free(it);
	}

	// destroys the last element
	free(it);

	// destroys the big_int object itself
	free(pBigInt);

	return;
}

big_int* get_copy_big_int(big_int* source)
{
	big_int* copy = create_big_int(); 

	for (node *it = source->first->next; it != NULL; it = it->next)
	{
		append_to_last(copy, it->digit);
	}	

	return copy;
}

void append_to_last(big_int *pBigInt, int ArgDigit)
{
	node *pNewTail = malloc(sizeof(*pNewTail));

	if (pNewTail == NULL)
		handle_no_memory_error();

	pNewTail->digit = ArgDigit;

	// attach new tail
	pBigInt->last->next = pNewTail;
	
	// link new tail to previous element
	pNewTail->previous = pBigInt->last;
	pNewTail->next = NULL;

	// update pointer to tail
	pBigInt->last = pNewTail;

	return;
}

void append_to_head(big_int *pBigInt, int ArgDigit)
{
	node *pNewHead = malloc(sizeof(*pNewHead));

	if (pNewHead == NULL)
		handle_no_memory_error();

	pNewHead->digit = ArgDigit;

	pNewHead->previous = pBigInt->first;
	pNewHead->next = pBigInt->first->next;
	
	pNewHead->previous->next = pNewHead;
	pNewHead->next->previous = pNewHead;

	return;
}

void pop_last(big_int *pBigInt)
{
	node *tail = pBigInt->last;

	pBigInt->last = tail->previous;
	tail->previous->next = NULL;

	free(tail);

	return;
}

// it is strictly known that the buffer is large enough to fit the number
// (given the problem's constraints), but the size is passed just to make sure
void big_int_to_string(big_int* BigInt, char* buffer, int buffer_size)
{
	node *it;
	int i;

	for (it = BigInt->last, i = 0; it != BigInt->first; it = it->previous, i++)
	{
		if (i > buffer_size - 1)
		{
			fprintf(stderr, "Number too large for the buffer size (%d); exiting...\n",
					buffer_size);
			exit(1);
		}

		buffer[i] = (char) it->digit + 48;
	}

	// append \0 to the end
	buffer[i] = '\0';

	return;
}

// TODO: consider destroying the big_int object beforehand?
void string_to_big_int(char* buffer, int buffer_size, big_int* BigInt)
{
	if (buffer_size > MAX_BIG_INT_SIZE)
	{
		fprintf(stderr, 
				"Input number size (%d) larger than maximum BigInt size (%d); exiting...\n",
				buffer_size, MAX_BIG_INT_SIZE);
		exit(1);
	}

	// insert the actual number
	for (int i = buffer_size - 1; i >= 0; i--)
	{
		append_to_last(BigInt, (int) buffer[i] - 48);
	}

	// fill the rest of the list with 0's (to the left of MSD)
	for (int i = 0; i < MAX_BIG_INT_SIZE - buffer_size; i++)
	{
		append_to_last(BigInt, 0);
	}

	return;
}

void print_big_int(big_int* BigInt)
{
	char buffer[MAX_BIG_INT_SIZE];
	big_int_to_string(BigInt, buffer, MAX_BIG_INT_SIZE);
	int size = strlen(buffer);
	int flag = 1;
	
	for (int i = 0; i < size; i++)
	{
		if (buffer[i] == '0' && flag == 1)
		{
			continue;
		}

		flag = 0;
		printf("%c", buffer[i]);
	}

	if (flag == 1)
		printf("0");

	printf("\n");

	return;
}


big_int* add(big_int* A, big_int* B)
{
	int sum, carry = 0;

	big_int* Sum = create_big_int();

	// set iterators to least significant bits
	node *it_a = A->first->next, *it_b = B->first->next;

	for (int i = 0; i < MAX_BIG_INT_SIZE; i++, it_a = it_a->next, it_b = it_b->next)
	{
		sum = it_a->digit + it_b->digit + carry;
		carry = sum/10;
		append_to_last(Sum, sum%10);
	}
	
	return Sum;
}

big_int* max_big_int(big_int* A, big_int* B)
{
	node *it_a = A->last, *it_b = B->last;

	for (int i = 0; i < MAX_BIG_INT_SIZE; i++, it_a = it_a->previous, it_b = it_b->previous)
	{
		if (it_a->digit > it_b->digit)
		{
			return A;
		}
		else if (it_b->digit > it_a->digit)
		{
			return B;
		}
	}

	return A;
}

big_int* subtract(big_int* A, big_int* B)
{
	big_int* Difference = create_big_int();

	node *it_max, *it_min;

	if (max_big_int(A, B) == A)
	{
		it_max = A->first->next;
		it_min = B->first->next;
	}

	else
	{
		it_max = B->first->next;
		it_min = A->first->next;
	}

	int diff, carry = 0;

	for (int i = 0; i < MAX_BIG_INT_SIZE; i++, it_max = it_max->next, it_min = it_min->next)
	{
		diff = it_max->digit - carry - it_min->digit;

		if (diff < 0)
		{
			carry = 1;
			append_to_last(Difference, 10 + diff);
		}

		else
		{
			carry = 0;
			append_to_last(Difference, diff);
		}
	}

	return Difference;
}

// naive (i.e., O(n^2)) multiplication: multiply by each digit
// and left-shift appropriately, then sum everything
big_int* multiply(big_int* A, big_int* B)
{
	big_int* Product = create_big_int();
	string_to_big_int("0", strlen("0"), Product);

	node *it_a, *it_b = B->first->next;

	for (int i = 0; i < MAX_BIG_INT_SIZE; i++, it_b = it_b->next)
	{
		big_int* dummy = create_big_int();

		it_a = A->first->next;
		int product, carry = 0;

		for (int j = 0; j < MAX_BIG_INT_SIZE; j++, it_a = it_a->next)
		{
			product = it_a->digit * it_b->digit + carry;
			carry = product/10;
			append_to_last(dummy, product%10);
		}

		// left shift, i.e., multiplication by 10
		for (int j = 0; j < i; j++)
		{
			append_to_head(dummy, 0);
			pop_last(dummy);
		}

		big_int* tmp = Product;
		Product = add(Product, dummy);
		destroy_big_int(tmp);

		destroy_big_int(dummy);
	}

	return Product;
}

// naive (slow) division algorithm: counts how many
// times you can remove B from A
big_int* divide(big_int* A, big_int* B)
{
	A = get_copy_big_int(A);
	B = get_copy_big_int(B);

	big_int *Quotient = create_big_int();
	string_to_big_int("0", strlen("0"), Quotient);

	big_int *one = create_big_int();
	string_to_big_int("1", strlen("1"), one);

	big_int *check_one;
	check_one = subtract(one, B);

	// avoids calculating A divided by 1 directly
	if (max_big_int(Quotient, check_one) != check_one)
	{
		destroy_big_int(B);

		destroy_big_int(Quotient);
		destroy_big_int(one);
		destroy_big_int(check_one);

		return A;
	}

	while (max_big_int(A, B) == A)
	{
		big_int *tmp = A;
		A = subtract(A, B);
		destroy_big_int(tmp);

		tmp = Quotient;
		Quotient = add(Quotient, one);
		destroy_big_int(tmp);
	}

	destroy_big_int(A);
	destroy_big_int(B);
	destroy_big_int(one);
	destroy_big_int(check_one);

	return Quotient;
}