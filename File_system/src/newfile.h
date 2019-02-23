#pragma once
#include "struct.h"

/*
 *功能:建立文件
 *
 *实现:同样先检查路径，然后进行系列能否创建文件的判断，可建后修改相应的磁盘信息
 *
 *path : 用户输入的路径 
 */
void newfile(char *path)
{
	dir temp_dir;
	char dirName[NAME_LEN];
	if (dividePathAndName(path, dirName, temp_dir))
		create_file(temp_dir, dirName);
	else
		printf("文件创建失败!\n");
}

/*
*功能:在给定的目录temp_dir下创建文件名为file_name的文件
*
*算法步骤:检查temp_dir下是否已经存在名为file_name的文件，如果有则创建失败
*         否则输入文件内容，直到输入回车，分配数据块和inode，如果分配成功则，修改超级块、
*         组描述符、数据块位图、inode位图、文件所在的目录、数据区等，否则提示错误信息
*temp_dir: 在此目录下建立文件
*file_name : 文件的名字
*/
void create_file(dir temp_dir, char *file_name)
{
	//检查是否有同名文件
	if (findSameName(temp_dir, file_name))
	{
		printf("%s 目录下已存在名为 %s 的文件，文件创建失败!\n", temp_dir.dir_name, file_name);
		return;
	}

	//读取文件内容
	char content;		//文件内容中的单个字符
	int i, size = 5;		//i为循环控制变量，size为文件内容buffer的大小，当输入大于size时，size成倍增加
	char *buffer = new char[size];		//用于存取文件内容，随size增加而增大
	char *temp;			//用于当buffer变大时，用于临时存取buffer的内容
	int count = 0;		//用于统计读取的文件内容的大小

	for (i = 0; i < size; i++)
		buffer[i] = 0;
	printf("请输入文件内容，按 (# + Enter) 结束输入:\n\n");
	while ((content = getchar()) != '#')	//直到输入回车结束
	{
		buffer[count] = content;
		count++;
		if (count >= size - 1)			//当输入内容超过缓冲区大小时，缓冲区大小翻倍
		{
			temp = new char[size];
			strcpy(temp, buffer);	//将原来缓冲区内容保存到temp中
			delete(buffer);
			size = size * 2;			//文件内容缓冲区成倍增加
			buffer = new char[size];//重新分配缓冲区大小
			for (i = 0; i < size; i++)
				buffer[i] = 0;
			strcpy(buffer, temp);	//将原来的内容放回缓冲区
			delete(temp);
		}
	}

	//分配文件数据块空间和inode
	int block_num = 0;		//存储文件需要的数据块数目
	long addr = -1;			//存储文件的数据块首地址
	int b_bitmap_index = -1;//存储文件的数据块在数据块位图中的首地址
	int f_inode = -1;

	if ((count + 1) % BLOCK_SIZE == 0)
		block_num = (count + 1) / BLOCK_SIZE;
	else
		block_num = (count + 1) / BLOCK_SIZE + 1;
	addr = getBlock(block_num, &b_bitmap_index);
	if (addr < 0)
	{
		printf("文件太大，硬盘空间不够，文件创建失败!\n");
		return;
	}
	f_inode = getInode();

	//更新文件inode内容
	inode_table[f_inode].i_block = addr;
	inode_table[f_inode].i_blocks = block_num;
	inode_table[f_inode].i_change = false;
	inode_table[f_inode].i_mode = _FILE;
	inode_table[f_inode].i_size = count + 1;
	inode_table[f_inode].i_uid = uid;
	//更新文件所在目录的内容
	temp_dir.file_entry[temp_dir.size].inode = f_inode;
	strcpy(temp_dir.file_entry[temp_dir.size].name, file_name);
	temp_dir.size++;
	if (temp_dir.inode == _current_dir.inode)
		_current_dir = temp_dir;
	//写回硬盘
	fd = fopen("LINUX_FILE_SYSTEM", "rb+");
	if (fd == NULL)
	{
		printf("Linux file system file creat failed !\n");
		exit(0);
	}
	fseek(fd, 0, SEEK_SET);
	fwrite(&_block_group[0], sizeof(block_group), BLOCKS_GROUP_NUM, fd);	//把数据块组写回文件
	fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*f_inode), SEEK_SET);
	fwrite(&inode_bitmap[f_inode], sizeof(bool), 1, fd);					//把inode 位图写回文件
	fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*TOTAL_INODES_NUM
		+ sizeof(inode)*f_inode), SEEK_SET);
	fwrite(&inode_table[f_inode], sizeof(inode), 1, fd);					//把inode 表写回文件
	fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*TOTAL_INODES_NUM
		+ sizeof(inode)*TOTAL_INODES_NUM + sizeof(bool)*b_bitmap_index), SEEK_SET);
	fwrite(&block_bitmap[b_bitmap_index], sizeof(bool), block_num, fd);	//把修改的数据块位图写回文件
	fseek(fd, addr, SEEK_SET);
	fwrite(buffer, 1, count, fd);				//把文件写入硬盘
	fseek(fd, inode_table[temp_dir.inode].i_block, SEEK_SET);
	fwrite(&temp_dir, sizeof(dir), 1, fd);		//把父目录写回文件
	fclose(fd);

	delete(buffer);
}