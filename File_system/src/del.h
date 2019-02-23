#pragma once
#include "struct.h"
/*
 *功能:删除文件
 *实现：检查给定的路径，找出要删除的文件所在的目录；删除文件，修改系统信息
 *
 *path : 用户所输入的路径
 *         
 */
void del(char *path)
{
	dir temp_dir;
	char file_name[NAME_LEN];
	if (dividePathAndName(path, file_name, temp_dir))
		delete_file(temp_dir, file_name);
	else
		printf("文件删除失败!\n");
}

/*
 *功能:删除给定目录下的指定文件
 *实现:根据给定的目录temp_dir，查找是否存在名为file_name的文件，有则删除文件，
 *         释放数据块和inode节点，修改所属目录temp_dir，超级块、组描述符、数据块位图、inode位图等
 *         不存在则提示错误信息
 *temp_dir : 要删除的文件所在的目录
 *file_name : 要删除的文件的名字
 */
void delete_file(dir temp_dir, char *file_name)
{
	int i;			//循环控制变量
	int f_inode;	//文件的inode
	int temp_pos;	//在temp_dir中file_entry的位置
	for (i = 2; i < temp_dir.size; i++)
	{
		if (strcmp(temp_dir.file_entry[i].name, file_name) == 0 &&
			inode_table[temp_dir.file_entry[i].inode].i_mode == _FILE)
		{
			f_inode = temp_dir.file_entry[i].inode;
			temp_pos = i;
			break;
		}
	}
	if (i == temp_dir.size)	//没找到文件
	{
		printf("目录 %s 中不存在文件 %s,文件删除失败!\n", temp_dir.dir_name, file_name);
		return;
	}
	freeBlock(inode_table[f_inode].i_blocks,
		((inode_table[f_inode].i_block - FIRST_DATA_ADDR) / BLOCK_SIZE));	//释放数据块
	freeInode(f_inode);	//释放文件inode
	//更新其所属目录,把其后面的文件及目录的file_entry往前移一位，填补删除的文件的空位
	for (i = temp_pos; i < temp_dir.size; i++)
	{
		temp_dir.file_entry[i].inode = temp_dir.file_entry[i + 1].inode;
		strcpy(temp_dir.file_entry[i].name, temp_dir.file_entry[i + 1].name);
	}
	temp_dir.size--;
	if (temp_dir.inode == _current_dir.inode)
		_current_dir = temp_dir;

	//将改动的内容写回硬盘
	fd = fopen("linux_file_system", "rb+");
	if (fd == NULL)
	{
		printf("Linux file system file creat failed !\n");
		exit(0);
	}
	fseek(fd, 0, SEEK_SET);
	fwrite(_block_group, sizeof(block_group), BLOCKS_GROUP_NUM, fd);	//将超级块和组描述符写回
	fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + f_inode), SEEK_SET);
	fwrite(inode_bitmap, sizeof(bool), 1, fd);							//将inode位图写回
	fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + (sizeof(bool) + sizeof(inode))*TOTAL_INODES_NUM
		+ (inode_table[f_inode].i_block - FIRST_DATA_ADDR) / BLOCK_SIZE), SEEK_SET);
	fwrite(block_bitmap, sizeof(bool), inode_table[f_inode].i_blocks, fd);	//将数据块位图写回
	fseek(fd, inode_table[temp_dir.inode].i_block, SEEK_SET);
	fwrite(&temp_dir, sizeof(dir), 1, fd);									//将父目录写回
	fclose(fd);
}