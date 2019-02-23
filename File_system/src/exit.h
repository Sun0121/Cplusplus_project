#include "struct.h"

/*
 *功能：退出系统
 */
void exit()
{
	//写入文件
	fd = fopen("LINUX_FILE_SYSTEM","rb+");
	if(fd == NULL)
	{
		printf("linux_file_system file creat failed !\n");
		exit(0);
	}
	fseek(fd,0,SEEK_SET);
	fwrite(&_block_group[0],sizeof(block_group),BLOCKS_GROUP_NUM,fd);	//把数据块组写到文件上
	fwrite(&inode_bitmap[0],sizeof(bool),TOTAL_INODES_NUM,fd);			//把inode 位图写到文件上
	fwrite(&inode_table[0],sizeof(inode),TOTAL_INODES_NUM,fd);			//把inode 表写到文件上
	fwrite(&block_bitmap[0],sizeof(bool),TOTAL_BLOCKS_NUM,fd);			//把数据块位图写到文件上
	fseek(fd,inode_table[_current_dir.inode].i_block,SEEK_SET);
	fwrite(&_current_dir,sizeof(dir),1,fd);								//把当前目录写回文件
	fclose(fd);
	exit(0);
}