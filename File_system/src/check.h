#include "struct.h"

/*
 *功能:检测并回复文件系统
 *实现:检查每个数据块组的组描述符与该组数据块与inode的记录是否相符，
 *	   检查超级块中的系统使用记录是否与实际相符(与数据块位图与i节点位图作对比)
 */
void check()
{
	unsigned int f_b_num = 0;	//空闲数据块数目
	unsigned int f_i_num = 0;	//空闲inode数目
	unsigned int total_f_b_num=0;//总的空闲数据块数目
	unsigned int total_f_i_num=0;//总的空闲inode数目

	int i,j;			//循环控制变量
	int start;			//记录数据块位图及inode位图的起始位置
	bool change = false;//标明是否有修改过
	printf("检查文件系统中...\n");

	for(i=0;i<BLOCKS_GROUP_NUM;i++)
	{
		f_b_num = 0;
		f_i_num = 0;
		start = i*BLOCKS_PER_GROUP;
		for(j=0;j<BLOCKS_PER_GROUP;j++)
		{
			if(block_bitmap[start+j] == NOT_USED)	//统计每数据块组空闲数据块的数目
				f_b_num++;
			if(inode_bitmap[start+j] == NOT_USED)	//统计每数据块组空闲inode的数目
				f_i_num++;
		}
		if(_block_group[i].group_desc.bg_free_blocks_count != f_b_num)	//不相等则修改组描述符
		{
			_block_group[i].group_desc.bg_free_blocks_count = f_b_num;
			change = true;
		}
		if(_block_group[i].group_desc.bg_free_inodes_count != f_i_num)	//不相等则修改组描述符
		{
			_block_group[i].group_desc.bg_free_inodes_count = f_i_num;
			change = true;
		}
		total_f_b_num += _block_group[i].group_desc.bg_free_blocks_count;	//统计总的空闲数据块数
		total_f_i_num += _block_group[i].group_desc.bg_free_inodes_count;	//统计总的空闲inode数
	}
	if(_block_group[0].super_block.s_free_blocks_count != total_f_b_num)	//不相等则修改超级块
	{
		for(i=0;i<BLOCKS_PER_GROUP;i++)
			_block_group[0].super_block.s_free_blocks_count = total_f_b_num;
		change = true;
	}
	if(_block_group[0].super_block.s_free_inodes_count != total_f_i_num)	//不相等则修改超级块
	{
		for(i=0;i<BLOCKS_PER_GROUP;i++)
			_block_group[0].super_block.s_free_inodes_count = total_f_i_num;
		change = true;
	}
	if(!change)
		printf("系统没有异常\n");
	else	//将修改写回
	{
		printf("系统出现异常，正在修复中...\n");
		fd = fopen("LINUX_FILE_SYSTEM","rb+");
		if(fd == NULL)
		{
			printf("linux_file_system file create failed !\n");
			system("pause");
		}
		fseek(fd,0,SEEK_SET);
		fwrite(&_block_group[0],sizeof(block_group),BLOCKS_GROUP_NUM,fd);	//把数据块组写到文件上
		fwrite(&inode_bitmap[0],sizeof(bool),TOTAL_INODES_NUM,fd);			//把inode 位图写到文件上
		fwrite(&inode_table[0],sizeof(inode),TOTAL_INODES_NUM,fd);			//把inode 表写到文件上
		fwrite(&block_bitmap[0],sizeof(bool),TOTAL_BLOCKS_NUM,fd);			//把数据块位图写到文件上
		printf("修复成功!\n");
	}
}