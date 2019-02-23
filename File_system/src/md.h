#pragma once
#include "struct.h"


/*功能：创建目录
 *
 *实现：先分解出创建的路径与待创建的目录名
 *		1、若路径形式为" ../目录 "，则先返回上一级目录后再处理
 *		2、若路径形式为" ./目录 "，则直接在本级目录处理
 *		3、若路径形式为" root/目录 "，则返回根目录后再处理
 *		4、若路径形式为" 目录/第一级目录 "，则直接在本级目录处理
 *
 *path : 用户所输入的路径
 */
void md(char *path) {
	
	dir finalDir;
	char dirName[NAME_LEN];
	if (dividePathAndName(path, dirName, finalDir)) {
		makeDir(finalDir, dirName);
	}else 
		printf("目录创建失败!\n");
}

/*
 *功能: 根据指定的具体目录temp_dir，再其中创建新目录dir_name,供md函数调用
 *
 *temp_dir ：待创建目录的父目录
 *dir_name : 待创建的目录名
 */
void makeDir(dir temp_dir, char *dirName) {

	long addr = -1;   // 分配目录在数据区的首地址
	int _inode = -1;  // 分配目录的i节点号
	int b_index;      // 分配的数据块在数据块位图的起始位置

	if (findSameName(temp_dir, dirName))
	{
		printf("%s 目录下已存在名为 %s 的文件，文件创建失败!\n", temp_dir.dir_name, dirName);
		return;
	}
	// 分配空间
	addr = getBlock(DIR_SIZE, &b_index);
	if (addr < 0) {
		printf("硬盘空间不够，目录创建失败!\n");
		return;
	}
	else {
		_inode = getInode();       //  i节点分配
		if (_inode < 0) {
			printf("i节点分配失败，目录创建失败!\n");
			return;
		}

		// 创建并初始化目录(包含本级目录'.'和父级目录'..')
		dir make_dir;
		strcpy(make_dir.dir_name, dirName);
		make_dir.inode = (unsigned int)_inode;
		make_dir.size = 2;
		make_dir.file_entry[0].inode = _inode;
		strcpy(make_dir.file_entry[0].name,".");
		make_dir.file_entry[1].inode = temp_dir.inode;
		strcpy(make_dir.file_entry[1].name, "..");
		// 修改本目录对应的inode
		inode_table[_inode].i_block = addr;
		inode_table[_inode].i_blocks = DIR_SIZE;
		inode_table[_inode].i_change = false;
		inode_table[_inode].i_mode = _DIRECTORY;
		inode_table[_inode].i_size = sizeof(dir);
		inode_table[_inode].i_uid = uid;
		// 修改其父目录
		temp_dir.file_entry[temp_dir.size].inode = (unsigned int)_inode;
		strcpy(temp_dir.file_entry[temp_dir.size].name, dirName);
		temp_dir.size++;

		if (temp_dir.inode == _current_dir.inode)	//如果是在当前目录下创建则要更新当前目录
			_current_dir = temp_dir;

		// 将修改写回硬盘
		fd = fopen("LINUX_FILE_SYSTEM", "rb+");
		if (fd == NULL)
		{
			printf("Linux file system file creat failed !\n");
			exit(0);
		}
		fseek(fd, 0, SEEK_SET);
		fwrite(&_block_group[0], sizeof(block_group), BLOCKS_GROUP_NUM, fd); // 把数据块组写回文件
		fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*_inode), SEEK_SET);
		fwrite(&inode_bitmap[_inode], sizeof(bool), 1, fd);		//把对应的inode位图中的那一位写回文件
		fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*TOTAL_INODES_NUM + sizeof(inode)*_inode), SEEK_SET);
		fwrite(&inode_table[_inode], sizeof(inode), 1, fd);		//把对应的inode表中的那一个inode写回文件
		fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*TOTAL_INODES_NUM
			+ sizeof(inode)*TOTAL_INODES_NUM + sizeof(bool)*b_index), SEEK_SET);
		fwrite(&block_bitmap[b_index], sizeof(bool), DIR_SIZE, fd);        // 将修改的数据块位图写回文件
		fseek(fd, addr, SEEK_SET);
		fwrite(&make_dir, sizeof(dir), 1, fd);		//把创建的目录写入文件
		fseek(fd, inode_table[temp_dir.inode].i_block, SEEK_SET);
		fwrite(&temp_dir, sizeof(dir), 1, fd);		//把父目录写回文件
		fclose(fd);
	}
}