

// Implementation for the member-functions that are defined in BoundaryTag.hpp
// would be added here.


#include "BoundaryTag.hpp"
#include <cstdlib>
#include <cstring>

BoundaryTag::BoundaryTag() {
    std::memset(memory, 0, sizeof(memory));
    memory[0] = SIZE - FREE_OVERHEAD;
    memory[1] = 0;
    freeIdx = 0;
    iterIdx = 0;
}

void BoundaryTag::addToFreeList(void* ptrToMem, int size) {
    int* memBlock = reinterpret_cast<int*>(ptrToMem);
    memBlock[0] = size;
    memBlock[1] = freeIdx;
    freeIdx = (memBlock - memory);
}

void BoundaryTag::deleteFromFreeList(void* ptrToMem) {
    int* memBlock = reinterpret_cast<int*>(ptrToMem);
    int* prevBlock = memory + memBlock[1];
    prevBlock[1] += memBlock[0] + 2;
    memBlock[1] = prevBlock - memory;
}

void* BoundaryTag::allocate( int numBytes ) {
    // Round up the requested size to be a multiple of BYTES_PER_WORD.
    int numWords = ( numBytes + BYTES_PER_WORD - 1 ) / BYTES_PER_WORD;
    numWords = ( numWords + 1 ) / 2 * 2;  // make sure numWords is even
    numBytes = numWords * BYTES_PER_WORD;

    // Traverse the free list, looking for a block of sufficient size.
    int* prevBlock = 0;
    int* currBlock = memory + freeIdx;
    while( currBlock != memory && ( currBlock[0] < numWords || currBlock[0] < 0 ) ) {
        prevBlock = currBlock;
        currBlock = memory + currBlock[1];
    }
    if( currBlock[0] < numWords || currBlock[0] < 0 ) {
        return 0;  // insufficient memory available
    }

    // Split the block into two blocks.
    int remainingWords = currBlock[0] - numWords;
    if( remainingWords > 0 ) {
        int* newBlock = currBlock + numWords;
        newBlock[0] = remainingWords;
        newBlock[1] = currBlock[1];
        currBlock[1] = ( newBlock - memory );
        addToFreeList( newBlock, remainingWords );
    }

    // Mark the block as allocated and return a pointer to the user.
    if( prevBlock ) {
        prevBlock[1] = currBlock[1];
    } else {
        freeIdx = currBlock[1];
    }
    currBlock[0] = numWords;
    return currBlock + 2;
}

void BoundaryTag::free( void* ptrToMem ) {
    // Delete the block from the list of allocated blocks.
    deleteFromFreeList( ptrToMem );

    // Add the block to the list of free blocks.
    int* memBlock = reinterpret_cast<int*>( ptrToMem );
    addToFreeList( memBlock, memBlock[-2] );
}

void BoundaryTag::start() {
    iterIdx = 2;
}

void* BoundaryTag::next() {
    if( iterIdx >= SIZE ) {
        return 0;
    }
    int* memBlock = memory + iterIdx;
    iterIdx += std::abs(memBlock[0]) + 2;
    return ( memBlock[0] > 0 ) ? memBlock + 2 : 0;
}


bool BoundaryTag::isFree(void* ptrToMem) {
    int* currBlockPtr = (int*)ptrToMem - 1;
    return (*currBlockPtr) < 0;
}

int BoundaryTag::size(void* ptr) {
    int* currBlockPtr = (int*)ptr - 1;
    return (*currBlockPtr) & (~1);
}