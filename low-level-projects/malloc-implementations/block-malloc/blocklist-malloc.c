#include <stdio.h>
#include <sys/mman.h>

#define POOL_SIZE (1024 * 1024) 

struct Block {
    size_t size;   
    char isused;   
};

void* bloc = NULL;
size_t currentsize = 0;

void* mergeWithPrevious(void* ptr, int prevsize);


void* sbrkmalloc(size_t size) {
    if (bloc == NULL) {
        bloc = mmap(NULL, POOL_SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (bloc == MAP_FAILED) {
            perror("Error while allocating new memory");
            return NULL;
        }
    }

    if ((currentsize + sizeof(struct Block) + size) > POOL_SIZE) {
        perror("Heap space is now exhausted");
        return NULL;
    }

    size_t curr = 0;
    while (curr < currentsize) {
        struct Block* currentblock = (struct Block*)((char*)bloc + curr);
        size_t leftspace = currentblock -> size-size;
        if (currentblock->isused == 0 && currentblock->size >= size && leftspace > sizeof(struct Block)) {
            int prevsize=currentblock->size;
            currentblock->isused = 1;
            currentblock->size = size;
            allocateNextBlockAfterCurrent(currentblock, prevsize);
            return (void*)(currentblock + 1);  
        }
        curr += sizeof(struct Block) + currentblock->size;  
    }

    struct Block* newblock = (struct Block*)((char*)bloc + currentsize);
    newblock->size = size;
    newblock->isused = 1;
    currentsize += sizeof(struct Block) + size;

    return (void*)(newblock + 1);
}

void allocateNextBlockAfterCurrent(void* current, int prevsize) {
    struct Block* currentblock = (struct Block*)current;
    struct Block* nextblock = (struct Block*)((char*)(currentblock+1)+currentblock->size);
    nextblock -> size = (prevsize-(currentblock->size)-sizeof(struct Block));
    nextblock -> isused=0;

    size_t new_end = (char*)nextblock + sizeof(struct Block) + nextblock->size - (char*)bloc;
    if (new_end > currentsize) {
        currentsize = new_end;
    }
}

void sbrkfree(void* ptr) {
    printf("pointer to delete %d\n", ptr);
    size_t curr = 0;
    int prevsize=0;
    int currsize=0;
    while (curr < currentsize) {
        struct Block* currentblock = (struct Block*)((char*)bloc + curr);
        currsize=currentblock->size;
        void* user_ptr = (void*)(currentblock + 1);
        if (user_ptr == ptr) {
            currentblock->isused = 0;
            void* newptr = mergeWithPrevious((char*)currentblock, prevsize);
            mergeWithNext(newptr);
            return;
        }
        prevsize=currsize;
        curr += sizeof(struct Block) + currentblock->size;
    }
}

void* mergeWithPrevious(void* ptr, int prevsize) {
    void* prevptr = ptr-prevsize-sizeof(struct Block);
    struct Block* lastblockptr = (struct Block*)prevptr;
    struct Block* currblockptr = (struct Block*)ptr;
    if(lastblockptr->isused==0){
        lastblockptr->size=lastblockptr->size+sizeof(struct Block)+currblockptr->size;
        return prevptr;
    }
    return ptr;
}

void mergeWithNext(void* ptr) {
    struct Block* currentblock = (struct Block*)ptr;

    void* nextptr = (char*)(currentblock + 1) + currentblock->size;

    size_t offset = (char*)nextptr - (char*)bloc;
    if (offset >= currentsize || offset + sizeof(struct Block) > POOL_SIZE) {
        return; 
    }

    struct Block* nextblockptr = (struct Block*)nextptr;

    if (nextblockptr->isused == 0) {
        currentblock->size += sizeof(struct Block) + nextblockptr->size;
    }
}


void printAllBlocks() {
    int i = 0;
    size_t curr = 0;
    while (curr < currentsize) {
        struct Block* currentblock = (struct Block*)((char*)bloc + curr);
        printf("Block %d: Address=%p, Size=%zu, %s\n", i,
               (void*)currentblock,
               currentblock->size,
               currentblock->isused ? "Used" : "Free");
        i++;
        curr += sizeof(struct Block) + currentblock->size;
    }
}

int main() {
    printf("%d \n", sizeof(struct Block));
    int* data1 = (int*)sbrkmalloc(sizeof(int));
    int* data2 = (int*)sbrkmalloc(sizeof(int));
    int* data3 = (int*)sbrkmalloc(sizeof(int));
    int *data4 = (int*) sbrkmalloc(8 * sizeof(int));
    int* data5 = (int*)sbrkmalloc(sizeof(int));
    int* data6 = (int*)sbrkmalloc(sizeof(int));
    int* data7 = (int*)sbrkmalloc(sizeof(int));
    int* data8 = (int*)sbrkmalloc(sizeof(int));
    char* char1 = (char*)sbrkmalloc(sizeof(char));
    char* char2 = (char*)sbrkmalloc(sizeof(char));
    char* char3 = (char*)sbrkmalloc(sizeof(char));
    char* char4 = (char*)sbrkmalloc(sizeof(char));
    char* char5 = (char*)sbrkmalloc(sizeof(char));

    printAllBlocks();
    sbrkfree(data4);
    sbrkfree(char2);

    printAllBlocks();
    // int* data9 = (int*)sbrkmalloc(sizeof(int));
    // char* data7 = (char*)sbrkmalloc(sizeof(char));

    // printf("---------------------------------------- \n");
    sbrkfree(data6);
    sbrkfree(data8);
    sbrkfree(data7);

     printAllBlocks();

    return 0;
}
