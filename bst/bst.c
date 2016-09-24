#include <stdio.h>
#include <stdlib.h>

struct node {
  struct node* left;
  struct node* right;
  int value;
};

struct node* root;

struct node* create_node(int value) {
  struct node* n = calloc(1, sizeof(struct node));
  n->value = value;

  printf("%d", n->value);
  printf("fact: %p\n", n);
  return n;
}

void free_node(struct node* node) {
  
}

void insert(struct node** tree, int value) {

  printf("abc\n");
  if (*tree == NULL) {
    printf("tree: %p\n", tree);
    struct node* n = create_node(value);
    *tree = n;
    printf("tree: %p\n", tree);
    return;
  }

  printf(" tree outside if: %p\n", tree);
  if ((*tree)->value > value) {
    insert(&(*tree)->left, value);
    return;
  } else {
    insert(&(*tree)->right, value);
    return;
  }

}

void delete(struct node* node) {

}

int main() {

  printf("root: %p\n", root);
  //root = 
  insert(&root, 10);

  printf("---\n");
  printf("root: %p\n", root);
  printf("value: %d\n", root->value);
  printf("hello world\n");
  return 0;
}