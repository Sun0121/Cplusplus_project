#pragma once
#include "struct.h"
/*
 *功能:分配inode节点
 *
 *实现:检查i节点位图是否有空闲的i节点可用，随之更新系统信息
 *
 *返回分配的i节点号
 */
int getInode()
{
	int b_index;		//用于记录inode节点所在的数据块组，更新组描述符时用
	int i_index = -1;	//inode节点
	int temp, i, j;
	for (i = 0; i < BLOCKS_GROUP_NUM; i++)
	{
		for (j = 0; j < INODES_PER_GROUP; j++)
		{
			temp = (int)_block_group[i].group_desc.bg_inode_bitmap + j;
			if (inode_bitmap[temp] == NOT_USED)
			{
				b_index = i;
				i_index = temp;
				break;
			}
		}
		if (i_index != -1)
			break;
	}
	//更新inode位图、组描述符、超级块
	if (i_index != -1)
	{
		for (i = 0; i < BLOCKS_GROUP_NUM; i++)		//更新超级块
			_block_group[i].super_block.s_free_inodes_count -= 1;
		_block_group[b_index].group_desc.bg_free_inodes_count -= 1;	//更新组描述符
		inode_bitmap[i_index] = USED;		//更新inode位图
	}
	return i_index;     // 
}

/*
 *功能:释放inode节点
 *
 *算法步骤:根据要释放的inode号，更新超级块、组描述符、inode位图
 */
void freeInode(int inode)
{
	int i;		//循环控制变量
	for (i = 0; i < BLOCKS_GROUP_NUM; i++)		//更新超级块
		_block_group[i].super_block.s_free_inodes_count += 1;
	_block_group[inode / BLOCKS_PER_GROUP].group_desc.bg_free_inodes_count += 1;//更新组描述符(inode / BLOCKS_PER_GROUP 获得数据块组号)
	inode_bitmap[inode] = NOT_USED;		//更新inode位图
}