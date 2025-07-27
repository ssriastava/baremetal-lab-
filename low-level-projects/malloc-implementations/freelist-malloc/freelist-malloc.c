#include <stdio.h>
#include <sys/mman.h>

struct Block {
    size_t size;
    char isused;
    struct Block* next;
};

struct Block* head = NULL;
void* checkSpace(size_t size);

void* allocate(size_t size) {
    struct Block* block = (struct Block*)checkSpace(size);
    if (block != NULL) {
        block->size = size;
        block->isused = 1;
        return (void*)(block + 1);
    }

    size_t blocksize = sizeof(struct Block) + size;
    void* ptr = mmap(NULL, blocksize, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("Error while allocating new memory");
        return NULL;
    }

    struct Block* blockptr = (struct Block*)ptr;
    blockptr->size = size;
    blockptr->isused = 1;
    blockptr->next = NULL;

    if (head == NULL) {
        head = blockptr;
    } else {
        struct Block* curr = head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = blockptr;
    }

    void* user_ptr = (void*)(blockptr + 1);
    printf("Allocated block at: %p | user_ptr: %p\n", (void*)blockptr, user_ptr);
    return user_ptr;
}

void my_free(void* ptr) {
    if (!ptr) return;
    struct Block* block = (struct Block*)((char*)ptr - sizeof(struct Block));
    block->isused = 0;
}

void* checkSpace(size_t size) {
    struct Block* curr = head;
    while (curr != NULL) {
        if (curr->isused == 0 && curr->size >= size) {
            return (void*)curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void traverseAllBlocks() {
    struct Block* curr = head;
    while (curr != NULL) {
        if (curr->isused == 1) {
            printf("Block size: %zu\n", curr->size);
            int* data = (int*)((char*)curr + sizeof(struct Block));
            printf("Data: %d\n", *data);
        }
        curr = curr->next;
    }
}

int main() {
    int* all1 = (int*)allocate(sizeof(int));
    int* all2 = (int*)allocate(sizeof(int));

    *all1 = 24354;
    *all2 = 63651;

    printf("Before free:\n");
    traverseAllBlocks();

    my_free(all1);  // free one block

    printf("After free and new allocation:\n");
    int* all3 = (int*)allocate(sizeof(int));  // should reuse all1's block
    *all3 = 9999;

    traverseAllBlocks();
    return 0;
}
