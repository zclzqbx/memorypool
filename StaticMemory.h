#ifndef STATICMEMORY
#define STATICMEMORY
#include"MemoryChunk.h"

typedef struct BigBlockHeader{
	size_t length;
}BigBlockHeader;

//大Block的结构
typedef struct BigBlock{
	BigBlockHeader header;
	char buffer;
}BigBlock;

//StaticMemory中存放的是不同大小的Chunk
class StaticMemory{
public:
	//Block的大小范围，最小为1字节
	enum{MAX_SIZE=1024,MIN_SIZE=8};

	StaticMemory()
	{
		count=0;//Chunk的数量
		for(size_t size=MIN_SIZE;size<MAX_SIZE;size*=2)
			++count;
		//构建Chunk的列表，内部都是指向Chunk的指针
		pChunkList=new MemoryChunk*[count];
		int index=0;
		for(size_t size=MIN_SIZE;size<MAX_SIZE;size*=2)
		{
			//每个Chunk中存放100个Block
			pChunkList[index++]=new MemoryChunk(size,100);
		}
	}

	~StaticMemory()
	{
		for(size_t index=0;index<count;++index)
		{//释放每一个Chunk指针，并调用其析构函数
			delete pChunkList[index];
		}
		delete [] pChunkList;
	}

	void *Malloc(size_t size)
	{
		if(size>MAX_SIZE)
		{//调用大Block的创建方法，大Block不在内存池内申请
			malloc(size);
		}
		else
		{
			int index=0;
			for(size_t tsize=MIN_SIZE;tsize<MAX_SIZE;tsize*=2)
			{//寻找合适大小的Block
				if(tsize>=size)
					break;
				index++;
			}
			return pChunkList[index]->malloc();//分配内存
		}
		return NULL;
	}

	void Free(void* pMem)
	{
		if(!free(pMem))
		{//如果不是大Block
			MemoryBlock* pblock;
			pblock=(MemoryBlock*)((char*)pMem-sizeof(BlockHeader));
			int index=0;
			for(size_t size=MIN_SIZE;size<MAX_SIZE;size*=2)
			{
				if((pblock->header).length==size)
					break;
				index++;
			}
			pChunkList[index]->free(pMem);
		}
	}

	size_t getCount()
	{
		return count;
	}
protected:

	void* malloc(size_t size)
	{	
		BigBlock* bblock;
		bblock=(BigBlock*)::malloc(sizeof(BigBlockHeader)*8+size);
		if(bblock)
		{
			(bblock->header).length=size;
			return &(bblock->buffer);
		}
		return NULL;
	}

	bool free(void* pMem)
	{//大Block的结构和普通block头部的结构有所区别，普通block前４字节
		//是一个指针，接下来４个字节是一个size_t类型变量
		//而大Block的头部中，只有一个size_t变量表示长度
		//所以两者头部长度是不等的，当为了判断一个block是否是大block
		//取长度时就应该注意指针的回退位置
		BigBlock* bblock;
		bblock=(BigBlock*)((char*)pMem-sizeof(BigBlockHeader));
		if((bblock->header).length>MAX_SIZE)
		{
			::free(bblock);
			return true;
		}
		return false;
	}

private:
	MemoryChunk** pChunkList;//Chunk列表
	size_t count;//不同大小的Chunk数量
};

#endif //STATICMEMORY

