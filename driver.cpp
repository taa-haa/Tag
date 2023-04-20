#include "BoundaryTag.hpp"
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<vector>

using namespace std;

enum { SIZE = 4096, BYTES_PER_WORD = sizeof(int), FREE_OVERHEAD = 4 }; 

class MemoryBlock {
public:
  MemoryBlock( void *memory, int bytes ): mPtr( memory ), sizeInBytes( bytes ) {}
  void *pointerToMemoryOfThisBlock() { return mPtr; }
  int size() { return sizeInBytes; }

private:
  void *mPtr;
  int sizeInBytes;
};


class BlockCollection {
public:
  BlockCollection() { collection.clear(); }
  void add( void* ptr, int n  ) { collection.push_back( new MemoryBlock( ptr, n ) ); }
  bool empty() { return collection.size() == 0; }
  MemoryBlock *getAMemoryBlock() {
    int idx = rand() % collection.size();
    MemoryBlock* toReturn = collection[ idx ];
    collection[ idx ] = collection.back();
    collection.pop_back();
    return toReturn;
  }

  int numBlocks() { return collection.size(); }

  void clearMemoryBlocks() {
    for( int i = 0; i < collection.size(); ++i )
      memset( collection[i]->pointerToMemoryOfThisBlock(), 0, collection[i]->size() );
  }

private:
  vector<MemoryBlock *> collection;
};

void assertMemorySize( BoundaryTag *memory, int where )
{
    memory->start();
    int numBytes = 0;
    while( void *ptr = memory->next() )
        numBytes += memory->size( ptr );
    if( numBytes != SIZE * BYTES_PER_WORD ) {
        std::cout << "Total number of bytes is " << numBytes << " instead of " << 
            SIZE * BYTES_PER_WORD << " at " << where << "\n";
        exit( 1 );
    }
}

int main( )
{
    BoundaryTag *memory = new BoundaryTag();
    srand( 13 );  // initialize the random number generator so we get
                  // the same sequence each time we run the program to
                  // help debugging.
    
    assertMemorySize( memory, 1 );
    std::cout << "Up to this point, you have earned 20 points.\n";

    BlockCollection *collection = new BlockCollection();

    int allocatePercentage = 55;
    int maxSize = 150;

    void* block;
    int numIterations = 0;
    // If your program dies in this loop, 
    // you will not be able to make more than 40 out of 100.
    while( true ) {
        numIterations++;
        if( rand() % 100 < allocatePercentage ) {
	  int n = rand() % maxSize + 1;
	  block = memory->allocate( n );
	  if( block == 0 ) // out of memory?
	    break;
	  collection->add( block, n );
        } else if( ! collection->empty() ) {
	  MemoryBlock *mb = collection->getAMemoryBlock();
	  memory->free( mb->pointerToMemoryOfThisBlock() );
	  delete mb;
	}
	collection->clearMemoryBlocks();
        assertMemorySize(memory, 2);
    }

    std::cout << "You have earned 65 points.\n";

    std::cout << "Iterated " << numIterations << " times. There are " << 
        collection->numBlocks() << " free blocks.\n";

    while( ! collection->empty() ) {
	  MemoryBlock *mb = collection->getAMemoryBlock();
	  memory->free( mb->pointerToMemoryOfThisBlock() );
	  delete mb;
    }

    assertMemorySize(memory, 3);
    std::cout << "You have earned 75 points.\n";

    if( memory->allocate( SIZE * BYTES_PER_WORD - 100 ) == 0 ) {
        std::cout << "Memory is not being coalesced!  This feature is worth 15 out of 100 points!\n";
        exit( 2 );
    }

    assertMemorySize(memory, 4);
    std::cout << "Well done!  You have earned 90 points.  \n";

    delete memory;
    delete collection;
    return 0;
}
