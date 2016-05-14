#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_SIZE 128
unsigned int HEAP_SIZE = 400;
unsigned int HEADER_SIZE = 2;

char *HEAD;
char *END_HEAP;

/**
 * list of pointers to allocated blocks
 */
char *ALLOCATED_LIST[LIST_SIZE];

/**
 * CURRENT holds the current index
 */
int COUNTER = 0;

/**
 * checks if header block is allocated or not
 * @param char * pointer
 * @return unsigned char 1|0
 */
unsigned char is_allocated(char *pointer) {
    return ((unsigned char) pointer[1]) & 0x01;
}

/**
 * gets the size of current block
 * @param char * pointer
 * @return unsigned short
 */
unsigned short get_size(char* pointer) {
    unsigned short number = (unsigned short)(
        ((unsigned char)pointer[0]) << 8 | 
        ((unsigned char)pointer[1])
    );

    return (number >> 1);
}

/**
 * deallocates current block and sets least sig
 * bit to zero
 * @param char * pointer
 */
void de_allocate(char *pointer) {
    pointer[1] = pointer[1] & ~0x01;
}

/**
 * writes to the header block
 * writes the size and allocated bit
 * @param char * pointer
 * @param unsigned short size
 */
void create_block(char *pointer, unsigned short size) {
    size = (size << 1) | 1;
    pointer[1] = size & 0xFF;
    pointer[0] = (size >> 8) & 0xFF;
}

/**
 * Helper Functions
 */

/**
 * checks if index is withing LIST_SIZE limits
 * @param  index block number
 * @return       boolean 1|0
 */
unsigned char _is_within_list_size(int index) {
    return (index > 0 && index <= LIST_SIZE);
}

/**
 * checks to see if block id is withing bounds
 * and if the pointer is not null/0
 * @param  index blockID
 * @return       boolean 1|0
 */
unsigned char is_valid_block_id(int index) {
    if ( ! _is_within_list_size(index)) {
        printf("Block id is out of bounds\n");
        return 0;
    }

    if ( ! ALLOCATED_LIST[index]) {
        printf("Block id does not exist\n");
        return 0;
    }

    return 1;
}

/**
 * writeheap writes to a block of char c
 * @param block block id number
 * @param c     charact to repeat
 * @param num   number of times
 */
void writeheap(int block, char c, int num) {
    
    if ( ! is_valid_block_id(block)) {
        return;
    }

    char *pointer = ALLOCATED_LIST[block];

    unsigned short size = get_size(pointer);

    // todo ask TA about this
    if (num > size) {
        printf("Size goes beyond allocated blocks\n");
        return;
    }

    char *end = pointer + HEADER_SIZE + num;

    pointer += HEADER_SIZE;

    while(pointer < end) {
        *pointer = c;
        pointer++;
    }
}

void blocklist() {
    // while loop through HEAD

  printf("Size\tAllocated\tStart\tEnd\n");

  char *current_index = HEAD;
  unsigned short current_size;




  while(current_index < END_HEAP && (get_size(current_index) != 0) ){
    current_size = get_size(current_index);
    if(is_allocated(current_index))
      printf("%d \t yes \t %p \t %p \n", current_size + 2, current_index, current_index + current_size + HEADER_SIZE -1);
    else
      printf("%d \t no \t %p \t %p \n", current_size + 2, current_index, current_index + current_size + HEADER_SIZE - 1);
    current_index += current_size + HEADER_SIZE;
  }
  
}

/**
 * freeblock sets is_allocated bit to 0
 * and sets all of its blocks to 0
 * @param index current index in array of pointers
 */
void freeblock(int index) {

    // check if id is within bounds and valid
    if ( ! is_valid_block_id(index)) {
        return;
    }

    char *pointer = ALLOCATED_LIST[index];

    // size of current block
    unsigned short size = get_size(pointer);

    // pointer to the end block
    char *end = ALLOCATED_LIST[index] + HEADER_SIZE + size;

    // sets allocated bit to 0
    de_allocate(pointer);

    // shift pointer 2 places to offset header bytes
    pointer += HEADER_SIZE;

    while(pointer < end) {
        *pointer = 0;
        pointer++;
    }

    // set pointer in allocated_list to zero/null
    ALLOCATED_LIST[index] = 0;

}

/**
 * allocates n blocks of memory on the heap
 * @param n int the number of blocks
 */
void allocate(int n) {
    char *current_index = HEAD;

    // traverses
    while( ! ((is_allocated(current_index) == 0 && get_size(current_index) == 0) || 
        (get_size(current_index) >= n && is_allocated(current_index) == 0)) ) {

        if (current_index > END_HEAP) {
            printf("not enough space.\n");
            return;
        }
        current_index += 2 + get_size(current_index);
    }

    if (current_index >= END_HEAP) {
        printf("Not enough space!\n");
        return;
    }

    if (get_size(current_index) > n + HEADER_SIZE) {
        unsigned short parent_block_size = get_size(current_index);
        create_block(current_index, n);
        ALLOCATED_LIST[++COUNTER] = current_index;
        printf("%d\n", COUNTER);
        current_index += 2 + n;
        create_block(current_index, parent_block_size - 2 - n);
        de_allocate(current_index);
    } else if ( (get_size(current_index) == n && is_allocated(current_index) == 0) ||
        (is_allocated(current_index) == 0 && get_size(current_index) == 0) ) {

        //printf("[%d], [%d]", is_allocated(current_index), get_size(current_index));
        create_block(current_index, n);
        ALLOCATED_LIST[++COUNTER] = current_index;
        printf("%d\n", COUNTER);
    } else {
        printf("Not Enough Space!!\n");
    }
}

void printheap(int block, int num){

    if ( ! is_valid_block_id(block)) {
        return;
    }
  
    char *pointer = ALLOCATED_LIST[block];
    
    char *end = pointer + HEADER_SIZE + num;

        pointer += HEADER_SIZE;

        while(pointer < end && pointer != NULL) {
            printf("%c", *pointer);
            pointer++;
        }
        
    printf("\n");
  
}

void removeNewLineChar( char *s, int n )
{
    for ( int  i = 0; i < n; ++i )
        {
            if ( s[i] == '\n' )
            {
                    s[i] = '\0';
            return;
            }
        }   
}

void read_command(){

    char command[1000];
    char *token1, *token2, *token3, *token4, *token5;
  
  while(1){
    printf(">");
    fgets(command, 1000, stdin);
    removeNewLineChar( command, 1000 );
    token1 = strtok(command," \t");
    token2 = strtok(NULL," \t");
    token3 = strtok(NULL," \t");
    token4 = strtok(NULL," \t");
    token5 = strtok(NULL," \t");
    
    if ( token1 == NULL )
    continue;
 
    if ( strcmp ( token1, "allocate" ) == 0 && token2 != NULL && token3 == NULL )
    {
        int num;
        int result = sscanf(token2, "%d", &num);
        //This checks if token2 was converted to an int successfully:
        //If it does, allocate is called
        //If it does not, it does nothing to the heap and prints an error message
        //E.g.: 'allocate five' would result in an error message.  
        if ( result == 1 )
        allocate(num);
    else
        printf("\nINVALID ENTRY!!!\n");
    }
    else if ( strcmp ( token1, "free" ) == 0 && token2 != NULL && token3 == NULL )
    {   
    int num;
    int result = sscanf(token2, "%d", &num);
        if ( result == 1 )
        freeblock(num);
    else
        printf("\nINVALID ENTRY!!!\n");
    }
    else if ( strcmp ( token1, "blocklist") == 0 && token2 == NULL )   
    blocklist();
    else if ( strcmp ( token1, "writeheap" ) == 0 && token2 != NULL && token3 != NULL && token4 != NULL && token5 == NULL )
    {
    int num1, num2;
        int result1 = sscanf(token2, "%d", &num1);
        int result2 = sscanf(token4, "%d", &num2);
        
        int i = 1;
    for  ( ; token3[i] != '\0'; )
    {
        ++i;
    }
    if ( result1 == 1 && result2 == 1 && i == 1 )
            writeheap( num1, token3[0], num2);
    else
        printf("\nINVALID ENTRY!!!\n");
    }
    else if ( strcmp ( token1, "printheap" ) == 0 && token2 != NULL && token3 != NULL && token4 == NULL )
    {   
        int num1, num2;
    int result1 = sscanf(token2, "%d", &num1);
        int result2 = sscanf(token3, "%d", &num2);
    if ( result1 == 1 && result2 == 1 )
            printheap( num1, num2 );
    else
        printf("\nINVALID ENTRY!!!\n");
    }
    else if ( strcmp ( token1, "quit" ) == 0 && token2 == NULL )
    return;
    else
    printf("\nINVALID ENTRY!!!\n");
  }
  
}




int main() {
    HEAD = malloc(sizeof(char) * HEAP_SIZE);
    END_HEAP = HEAD + HEAP_SIZE;

    create_block(HEAD, 398);
    de_allocate(HEAD);

    read_command();

    free(HEAD);
    return 0;
}

