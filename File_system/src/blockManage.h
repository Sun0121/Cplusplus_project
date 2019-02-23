#pragma once
#include "struct.h"
/*
 *功能: 分配数据块空间
 *
 *实现: 找到一段连续的空间能够容纳所申请的长度
 *
 *len : 所申请的数据空间的长度(盘块数目)
 *b_index : 分配的数据块在数据块位图的起始位置(返回时赋值)
 * return : 实际分配的连续区域的起始地址(物理地址)
 */
long getBlock(int len, int *b_index) {
	long addr = -1;  // 用于返回盘块的首地址

	if ((int)_block_group[0].super_block.s_free_blocks_count < len) {   // 先利用超级块中的信息判断空闲块数目
		return addr;
	}
	else
	{
		int count = 0;	//用于计算连续的空闲盘块数
		int addr_block_bitmap_index = 0;		//用于记录第一块可以用的数据块所在的数据块位图的位置
		bool block_group[BLOCKS_GROUP_NUM];	//用于记录所用到的数据块组，更新数据块组描述符时用到
		int block_group_count[BLOCKS_GROUP_NUM];	//用于记录所用到的数据块组的数据块数目(每个数据块组用了几个数据块)

		int block_group_index = 0;			//用于记录用到的第一个数据块组的位置

		int i, j;		//循环控制变量

		for (i = 0; i < BLOCKS_GROUP_NUM; i++)		//初始化block_group和block_group_count
		{
			block_group[i] = false;
			block_group_count[i] = 0;
		}
		for (i = 0; i < BLOCKS_GROUP_NUM; i++) // i表示正在对哪一个数据块进行判断
		{
			if (count == 0)	//当不为0时表示前一个组的末尾的数据块可以用，这个组可以连上一个组的数据块一起使用，所以不用检查
				if ((int)_block_group[i].group_desc.bg_free_blocks_count < len)
					continue;
			for (j = 0; j < BLOCKS_PER_GROUP; j++)		//检查是否有连续的数据块可以用
			{
				// 判断第i组的第j个数据块有没有被使用
				if (block_bitmap[(_block_group[i].group_desc.bg_block_bitmap + j)] == NOT_USED)
				{
					count++;
					block_group[i] = true;		//标明这个数据组的组描述符要修改
					block_group_count[i]++;		//统计这个数据块组用了多少块，更新组描述符时用
					if (count == 1)	//第一个可用的数据块
					{
						addr = _block_group[i].group_desc.bg_block_addr + j * BLOCK_SIZE;	//该数据块的起始地址
						addr_block_bitmap_index = i * BLOCKS_PER_GROUP + j;	//在数据块位图的位置，修改数据块位图时用
						*b_index = addr_block_bitmap_index;    // 给b_index所指向的地址中赋值
						block_group_index = i;	//block_group数组中的起始位置，用于等下更新组描述符时找第一个开始要修改的位置
					}
				}
				else	//中间有不连续的数据块则重新开始找
				{
					count = 0;
					//如果是刚换到了另一个新的数据块组，则要检查前一个数据块组是否曾经标明为使用,i-1是为了避免当该数据块组是第一个数据块组时数组下标为负
					if (j == 0 && (i - 1) >= 0 && block_group[i - 1])
					{	// 若是一个新的数据块组并重头开始
						block_group[i - 1] = false;
						block_group_count[i - 1] = 0;
					}
					block_group[i] = false;
					block_group_count[i] = 0;
				}
				if (count == len)	//如果需要的盘块数够了则跳出此循环，还要跳过外面的循环，所以有下面的另一个检查
					break;
			}
			if (count == len)			//如上解释
				break;
		}
		if (count != len)
		{
			addr = -1;
			return addr;
		}
		//更新据块位图、超级块、组描述符
		for (i = 0; i < BLOCKS_GROUP_NUM; i++)		//更新超级块
			_block_group[i].super_block.s_free_blocks_count -= len;
		j = addr_block_bitmap_index + len;
		for (i = addr_block_bitmap_index; i < j; i++)		//更新数据块位图
			block_bitmap[i] = USED;
		for (i = block_group_index; i < BLOCKS_GROUP_NUM; i++)		//更新组描述符
			if (block_group[i])
				_block_group[i].group_desc.bg_free_blocks_count -= block_group_count[i];
	}
	return addr;
}

/*
 *功能:根据给出的起始地址加上数据块数目来释放这部分数据块
 *
 *len : 所用的数据块数目
 *pos : pos为要删除的数据块在数据块位图的起始位置
 */
void freeBlock(int len, int pos)
{
	int i;					//循环控制变量
	int blk_end = len + pos;	//控制数据块位图的末尾
	unsigned int blk_group[BLOCKS_GROUP_NUM];	//用于记录数据块位图所对应的数据块组中需要更新的数据块数目 
	for (i = 0; i < BLOCKS_GROUP_NUM; i++)	//更新超级块
	{
		_block_group[i].super_block.s_free_blocks_count += len;
		blk_group[i] = 0;	//初始化blk_group
	}
	for (i = pos; i < blk_end; i++)			//更新数据块位图
	{
		block_bitmap[i] = NOT_USED;
		blk_group[i / BLOCKS_PER_GROUP]++;	//(i/BLOCKS_PER_GROUP)为第i个数据块所对应的数据块组
	}
	for (i = pos / BLOCKS_PER_GROUP; i < BLOCKS_PER_GROUP; i++)	//更新组描述符
	{
		if (blk_group[i] != 0)	//不为零表示改数据块组有数据块要释放
			_block_group[i].group_desc.bg_free_blocks_count += blk_group[i];
		else	//为零表示无数据块要释放，因为是数据块是连续分配的，所以后面的数据块组也没有数据块释放
			break;
	}
}