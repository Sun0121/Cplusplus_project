#pragma once
#include "struct.h"
/*
 *功能:打开文件
 *
 *实现:检查路径及文件是否正确，正确则取到文件的inode，申请和文件一样大小的缓冲区，
 *         把文件内容读到缓冲区并输出即可；否则提示错误信息
 *
 *path : 用户输入的路径
 */
void cat(char*path)
{
	dir temp_dir;
	char file_name[NAME_LEN];
	if (dividePathAndName(path, file_name, temp_dir))
		open_file(temp_dir, file_name);
	else
		printf("文件打开失败!\n");
}

/*
 *功能:打开文件
 *实现:检查文件是否存在，然后输出其中内容
 *
 *temp_dir : 要打开的文件的具体所在的目录
 *file_name : 具体要打开的文件的文件名
 */
void open_file(dir temp_dir, char *file_name)
{
	int i;			//循环控制变量
	int inode;		//要打开文件的inode
	for (i = 2; i < temp_dir.size; i++)		//在temp_dir中找该文件
	{
		//检查是否为有同名文件且类型为文件
		if (strcmp(temp_dir.file_entry[i].name, file_name) == 0 &&
			inode_table[temp_dir.file_entry[i].inode].i_mode == _FILE)
		{
			inode = temp_dir.file_entry[i].inode;
			break;
		}
	}
	if (i == temp_dir.size)
		printf("目录 %s 中不存在文件 %s ，文件打开失败!\n", temp_dir.dir_name, file_name);
	else
	{
		//把文件内容读到缓冲区buffer
		char *buffer = new char[inode_table[inode].i_size];		//文件内容缓冲区
		fd = fopen("LINUX_FILE_SYSTEM", "rb");
		if (fd == NULL)
		{
			printf("Linux_file_system create failed !\n");
			system("pause");
		}
		fseek(fd, inode_table[inode].i_block, SEEK_SET);
		fread(buffer, inode_table[inode].i_size, 1, fd);
		fclose(fd);
		buffer[inode_table[inode].i_size - 1] = 0;
		printf("文件 %s 的内容如下:\n\n", file_name);
		printf("%s\n", buffer);
		delete(buffer);
	}
}
