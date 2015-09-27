#include<iostream>
#include"StaticMemory.h"

using namespace std;

int main()
{
	StaticMemory memoryPool;
	int *ival=(int*)memoryPool.Malloc(sizeof(int)*8);
	
	*ival=9;
	cout<<*ival<<endl;
	MemoryBlock* pblock;
	pblock=(MemoryBlock*)((char*)ival-sizeof(BlockHeader));
	cout<<(pblock->header).length<<endl;
	memoryPool.Free(ival);
	return 0;
}
