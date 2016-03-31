#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//concat function using for loops
char* concat_forloop(char* first, int first_size, char* second, int second_size)
{
	int size = first_size + second_size;
	char * concat;
	concat = malloc(first_size + second_size + 1);

	printf("size of 1st: %d \n", first_size);
	
	for(int i = 0; i < first_size; i++)
	{
		concat[i] = first[i];		
	}

	for(int i = first_size; i < size+1; i++)
	{
		concat[i] = second[i - first_size];
	}
	
	return concat;
}

//concat function use memcpy
char* concat_memcpy(char* first, int first_size, char* second, int second_size)
{
	int size = first_size + second_size;
	char * concat;
	concat = malloc(first_size + second_size + 1);
	
	memcpy(concat, first, first_size);
	memcpy(concat + first_size, second, second_size);

	return concat;
}

int main()
{
	char first[] = "Hello";
	char second[] = "World!!";
	char *concat_word1;
	char *concat_word2;

	concat_word1 = concat_forloop(first, sizeof(first)-1, second, sizeof(second)-1);
	printf("new word is: %s \n", concat_word1);
	//outputs HelloWorld!!

	concat_word2 = concat_memcpy(first, sizeof(first)-1, second, sizeof(second)-1);
	printf("new word is: %s \n", concat_word2);
	//outputs HelloWorld!!
}
