#ifndef MEMORY_CHUNK
#define MEMORY_CHUNK

#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

struct MemoryBlock;

typedef struct BlockHeader{//头部
	MemoryBlock *next;
	size_t length;
}BlockHeader;

typedef struct BlockData{//数据
	char buffer;
}BlockData;

//Block的定义，本质上是一个链表结构
typedef struct MemoryBlock{
	BlockHeader header;//头部
	BlockData data;//数据部分
}MemoryBlock;

//Chunk是一系列Block的管理者
class MemoryChunk{//维护block及回收释放操作
public:
	//初始化里构建一定大小和数量的Block
	MemoryChunk(size_t size,size_t num)
	{
		pthread_mutex_init(&mutex,NULL);//互斥量初始化
		blockSize=size;//Block大小的初始化
		phead=NULL;//头指针
		MemoryBlock* pblock;
		blockNum=0;//Block块数大小的初始化，分配成功里再计数
		while(blockNum<num)
		{
			if((pblock=createBlock())!=NULL)
			{//分配成功，将Block加入到链表当中
				(pblock->header).next=phead;//下一指针
				(pblock->header).length=blockSize;//Block大小初始化
				phead=pblock;
				blockNum++;//Block块数增１
			}
			else
			{
				break;
			}
		}
	}

	~MemoryChunk()
	{
		MemoryBlock* pblock;
		while(phead)
		{
			pblock=phead;
			phead=(phead->header).next;
			::free(pblock);//将申请的内存归还给系统
			pblock=NULL;
			blockNum--;
		}
		assert(blockNum==0);//确保内存释放完全
		pthread_mutex_destroy(&mutex);
	}

	void* malloc()//分配内存
	{
		MemoryBlock* pBlock=NULL;
		pthread_mutex_lock(&mutex);
		if(phead)
		{//Chunk中有足够的Block，则直接分配
			pBlock=phead;
			phead=(phead->header).next;
			blockNum--;//总数量减１
		}
		else
		{//重新申请一块Block
			pBlock=createBlock();
		}
		pthread_mutex_unlock(&mutex);
		return &((pBlock->data).buffer);
	}

	void free(void* pmem)//回收内存
	{
		MemoryBlock* pblock;
		//对内存区域的操作，很重要
		pblock=(MemoryBlock*)((char*)pmem-sizeof(BlockHeader));
		free(pblock);
	}
	
	void free(MemoryBlock* pblock)
	{//回收到内存池当中
		pthread_mutex_lock(&mutex);
		(pblock->header).next=phead;
		phead=pblock;
		blockNum++;//Block问题增１
		pthread_mutex_unlock(&mutex);
	}

	size_t getBlockSize()//获取Chunk中block的大小
	{
		return blockSize;
	}

	size_t getBlockNum()//获取chunk中block的数量
	{
		return blockNum;
	}

protected:
	MemoryBlock* createBlock()//从系统当中获取内存
	{
		MemoryBlock* result;
		//sizeof返回的是字节数，blockSize存放的是位数
		result=(MemoryBlock*)::malloc(sizeof(BlockHeader)*8+blockSize);
		return result;
	}

private:
	MemoryBlock* phead;//头指针
	size_t blockSize;//Block大小
	size_t blockNum;//Block的数量
	pthread_mutex_t mutex;//互斥量
};

#endif //MEMORY_CHUNK
