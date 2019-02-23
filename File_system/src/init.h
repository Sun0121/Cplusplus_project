#pragma once
#include "struct.h"

void load()
{
	//初始化命令 
	strcpy(cmd[0].order, "init");
	strcpy(cmd[1].order, "info");
	strcpy(cmd[2].order, "cd");
	strcpy(cmd[3].order, "dir");
	strcpy(cmd[4].order, "md");
	strcpy(cmd[5].order, "rd");
	strcpy(cmd[6].order, "newfile");
	strcpy(cmd[7].order, "cat");
	strcpy(cmd[8].order, "copy");
	strcpy(cmd[9].order, "del");
	strcpy(cmd[10].order, "check");
	strcpy(cmd[11].order, "exit");
	strcpy(cmd[12].order, "help");

	//初始化路径
	strcpy(current_path, "root>");

	//注册用户
	_user[0].mode = ADMIN;
	_user[0].uid = 201630599319;
	strcpy(_user[0].password, "599319");

	fd = fopen("LINUX_FILE_SYSTEM", "rb");
	if (fd == NULL)
	{
		fflush(stdin);
		initialize();
		return;
	}

	// 读入文件系统中内容到内存
	fread(_block_group, sizeof(block_group), BLOCKS_GROUP_NUM, fd);		//读入超级块和组描述符
	fread(inode_bitmap, sizeof(bool), TOTAL_INODES_NUM, fd);				//读入inode位图
	fread(inode_table, sizeof(inode), TOTAL_INODES_NUM, fd);				//读入inode表
	fread(block_bitmap, sizeof(bool), TOTAL_BLOCKS_NUM, fd);				//读入数据块位图
	fread(&_current_dir, sizeof(dir), 1, fd);								//读入根目录

}

void initialize()
{
	printf("正在进行磁盘划分和初始化系统...\n");
	int i;

	//分配空间
	long file_size = FIRST_DATA_ADDR + BLOCK_SIZE * TOTAL_BLOCKS_NUM;
	char *buf = new char[file_size];
	//写入磁盘
	fd = fopen("LINUX_FILE_SYSTEM", "wb+");
	if (fd == NULL)
	{
		printf("Linux_file_system create failed !\n");
		system("pause");
	}
	fseek(fd, 0, SEEK_SET);
	fwrite(buf, file_size, 1, fd);
	fclose(fd);
	delete(buf);

	//初始化数据块组
	for (i = 0; i < BLOCKS_GROUP_NUM; i++)
	{
		//初始化超级块
		_block_group[i].super_block.s_inodes_count = TOTAL_INODES_NUM;
		_block_group[i].super_block.s_blocks_count = TOTAL_BLOCKS_NUM;
		_block_group[i].super_block.s_log_block_size = BLOCK_SIZE;
		_block_group[i].super_block.s_free_blocks_count = TOTAL_BLOCKS_NUM - DIR_SIZE;	//一个用于根目录
		_block_group[i].super_block.s_free_inodes_count = TOTAL_INODES_NUM - 1;			//一个用于根目录
		_block_group[i].super_block.s_first_data_block = FIRST_DATA_ADDR;
		_block_group[i].super_block.s_blocks_per_group = BLOCKS_PER_GROUP;
		_block_group[i].super_block.s_inodes_per_group = INODES_PER_GROUP;

		//初始化组描述符
		_block_group[i].group_desc.bg_block_addr = FIRST_DATA_ADDR + i * BLOCKS_PER_GROUP*BLOCK_SIZE;
		_block_group[i].group_desc.bg_block_bitmap = i * BLOCKS_PER_GROUP;
		_block_group[i].group_desc.bg_inode_bitmap = i * INODES_PER_GROUP;
		_block_group[i].group_desc.bg_inode_table = i * INODES_PER_GROUP;
		_block_group[i].group_desc.bg_free_blocks_count = BLOCKS_PER_GROUP;
		_block_group[i].group_desc.bg_free_inodes_count = INODES_PER_GROUP;
	}

	//初始化数据块位图,所有数据块均未被使用
	for (i = 0; i < TOTAL_BLOCKS_NUM; i++)
		block_bitmap[i] = NOT_USED;

	//初始化inode位图,所有inode均未被使用
	for (i = 0; i < TOTAL_INODES_NUM; i++)
		inode_bitmap[i] = NOT_USED;

	//初始化inode表
	for (i = 0; i < TOTAL_INODES_NUM; i++)
	{
		inode_table[i].i_mode = READ_ONLY;
		inode_table[i].i_uid = 0;
		inode_table[i].i_size = 0;
		inode_table[i].i_blocks = 0;
		inode_table[i].i_block = -1;
		inode_table[i].i_change = false;
	}

	/*设置第0个数据块为根目录的一系列信息*/
	//初始化根目录所在数据块组的组描述符
	_block_group[0].group_desc.bg_free_blocks_count = BLOCKS_PER_GROUP - DIR_SIZE;	//一个用于根目录
	_block_group[0].group_desc.bg_free_inodes_count = INODES_PER_GROUP - 1;			//一个用于根目录
	//初始化根目录的数据块位图
	for (i = 0; i < DIR_SIZE; i++)
		block_bitmap[i] = USED;
	//初始化根目录的inode 位图
	inode_bitmap[0] = USED;

	//初始化根目录的inode
	inode_table[0].i_mode = _DIRECTORY;			//模式为目录
	inode_table[0].i_uid = 201630599319;				//管理员的uid
	inode_table[0].i_size = sizeof(dir);		//根目录的大小
	inode_table[0].i_blocks = DIR_SIZE;			//一个目录分配DIR_SIZE个数据块
	inode_table[0].i_block = FIRST_DATA_ADDR;	//根目录在文件的起始位置
	inode_table[0].i_change = false;
	
	//往根目录项中添加内容
	strcpy(_current_dir.dir_name, "root");			//根目录名为root
	_current_dir.inode = 0;							//根目录的inode 号为0
	_current_dir.size = 2;							//一个当前目录，一个上一级目录
	strcpy(_current_dir.file_entry[0].name, ".");	//当前目录
	_current_dir.file_entry[0].inode = 0;			//即还是根目录
	strcpy(_current_dir.file_entry[1].name, "..");	//上一级目录
	_current_dir.file_entry[1].inode = 0;			//根目录的上一级目录还是自己

	//内存中的数据写入文件
	fd = fopen("LINUX_FILE_SYSTEM", "rb+");
	if (fd == NULL)
	{
		printf("Linux_file_system create failed !\n");
		system("pause");
	}
	fseek(fd, 0, SEEK_SET);
	fwrite(&_block_group[0], sizeof(block_group), BLOCKS_GROUP_NUM, fd);	//把数据块组写到文件上
	fwrite(&inode_bitmap[0], sizeof(bool), TOTAL_INODES_NUM, fd);			//把inode 位图写到文件上
	fwrite(&inode_table[0], sizeof(inode), TOTAL_INODES_NUM, fd);			//把inode 表写到文件上
	fwrite(&block_bitmap[0], sizeof(bool), TOTAL_BLOCKS_NUM, fd);			//把数据块位图写到文件上
	fwrite(&_current_dir, sizeof(dir), 1, fd);								//把根目录写到文件上
	fclose(fd);
}