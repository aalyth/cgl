
#include <stdio.h>

#include "vector.h"

struct point {
	int x;
	int y;
};

struct test {
	vector(int) nums;
};

void print(int n)
{
	printf("%d, ", n);
}

int main()
{
	vector(int) aaa = { 0 };
	vector_init(&aaa, .capacity = 5);
	for (int i = 0; i < 10; i++)
		vector_push(&aaa, i * i);
	vector_for_each(&aaa, print);
	return 0;
}
