#pragma once
#ifndef _STRUCT_H                 // 防止头文件重复包含
#define _STRUCT_H

#define MAX_PATH_LEN        1000    // 最大路径长度

/*
 * 文件系统的基本信息
 */
#define BLOCK_SIZE			1024	//盘块大小为 1KB
#define BLOCKS_GROUP_NUM	100		//数据块组的数目
#define BLOCKS_PER_GROUP	1024	//每组的盘块数
#define INODES_PER_GROUP	1024	//每组的inode数
#define TOTAL_BLOCKS_NUM	(BLOCKS_PER_GROUP*BLOCKS_GROUP_NUM)		//总的盘块数
#define TOTAL_INODES_NUM	(INODES_PER_GROUP*BLOCKS_GROUP_NUM)		//总的inode数

/* FIRST_DATA_ADDR : 第一个数据块的首地址
 * 在文件系统中并不是每个数据块组独立存放，而是每个区域连续存放完后存下一个区域
 * 具体存放顺序是  (超级块 + 组描述符) + i节点位图 + 数据盘块位图 + i节点 + 数据区
 */
#define FIRST_DATA_ADDR		(sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*(TOTAL_INODES_NUM + TOTAL_BLOCKS_NUM) + sizeof(inode)*TOTAL_INODES_NUM)

#define DIR_SIZE			(sizeof(dir)/BLOCK_SIZE+1)	  //一个目录占的盘块数

#define NAME_LEN			128		//文件名的最大长度
#define FILE_NUM			256		//一个目录下可以拥有的文件(包括目录)的最大数目
#define MAX_USER_NUM		8		//该文件系统最大的用户数目

#define READ_ONLY			1		//只读
#define WRITE_ONLY			2		//只写
#define RW					3		//读写

#define NOT_USED			false	//未被使用
#define USED				true	//被使用

#define ADMIN				1		//系统管理员
#define DEFULT				0		//普通用户

#define _FILE				1		//文件
#define _DIRECTORY			2		//目录

/*
 *超级块
 */
struct super_block
{
	unsigned int s_inodes_count;		//inode 的总数
	unsigned int s_blocks_count;		//盘块的总数
	unsigned int s_log_block_size;		//盘块的大小
	unsigned int s_free_blocks_count;	//空闲块的总数
	unsigned int s_free_inodes_count;	//空闲inode的总数
	unsigned int s_first_data_block;	//第一个数据块
	unsigned int s_blocks_per_group;	//每组的盘块数
	unsigned int s_inodes_per_group;	//每组的inode数
};

/*
 *组描述符
 */
struct group_desc
{
	unsigned long bg_block_addr;		//本组数据块在数据区的首地址
	unsigned int bg_block_bitmap;		//本组数据块位图所在的块号
	unsigned int bg_inode_bitmap;		//本组inode 位图所在的块号
	unsigned int bg_inode_table;		//本组inode 表所在的块号
	unsigned int bg_free_blocks_count;	//组中空闲块的数目
	unsigned int bg_free_inodes_count;	//组中空闲inode 的数目
};


/*
 *inode
 */
struct inode
{
	unsigned short i_mode;				//模式
	unsigned int i_uid;					//文件的用户ID
	unsigned int i_size;				//文件的大小
	unsigned int i_blocks;				//分配给该文件的磁盘块的数目
	long i_block;						//指向磁盘块的起始地址
	bool i_change;						//表示该文件是否修改过,true表示修改过，false表示没有
};


/*
 * 数据块组
 */
struct block_group
{
	struct super_block super_block;		//超级块
	struct group_desc group_desc;		//组描述符
};

/*
 *文件,包括文件名(普通文件或者目录)和inode号
 */
struct file_entry
{
	char name[NAME_LEN];				//文件名
	int inode;							//inode号,即在inode_table中的数组下标
};


/*
 *目录,用于存储指定具体目录所包含文件(包括目录)的数目和具体的文件内容(即dir_entry)
 */
struct dir
{
	char dir_name[NAME_LEN];					//目录名
	int inode;									//inode号
	int size;									//记录该目录下包含多少个文件(包括目录)
	file_entry file_entry[FILE_NUM];		//具体的文件内容
};


/*
 *用户
 */
struct user
{
	unsigned int mode;					//用户的模式(权限)
	int uid;							//用户的ID
	char password[20];					//密码
};


/*
 *命令
 */
struct command
{
	char order[10];
};

extern struct block_group _block_group[BLOCKS_GROUP_NUM];		//数据块组
extern struct inode inode_table[TOTAL_INODES_NUM];				//inode表
extern struct user _user[MAX_USER_NUM];							//用户
extern struct dir _current_dir;									//目录
extern bool block_bitmap[TOTAL_BLOCKS_NUM];						//数据块位图
extern bool inode_bitmap[TOTAL_INODES_NUM];						//inode 位图
extern FILE *fd;												//系统文件
extern unsigned int uid;										//使用文件系统的用户ID
extern char pwd[20];											//密码
extern command cmd[13];											//命令
extern char current_path[1000];									//保存当前路径

extern void initialize();
extern void login();
extern void help();
extern void info();

extern void display(dir _current_dir);

extern void remove_dir(dir temp,char*rd_name);
extern void remove_dir(dir father_dir,dir r_dir,int index);
extern void delete_file(char*path_and_file_name,int path_len);
extern void delete_file(dir _dir,char *file_name);
extern void create_file(dir temp_dir,char *file_name);
extern void open_file(dir temp_dir,char*file_name);
extern void copy_file(char*src_path,int src_len,char*des_path,int des_len);
extern void write_back(dir temp,char*file_name,char *buf,int len);
extern long read_file(dir temp_dir,char*file_name,int inode,char*buf);
extern void check();

// 修改函数 pathAnalysis
extern bool getLastDir(char *path, dir &temp_dir);
extern void findFullPath(dir temp_dir);
extern bool getDirName(char *path, int pos, char *dirName);
extern bool findFinalDir(char *path, int &pos, char *dirName, dir &temp_dir);
extern bool dividePathAndName(char *path, char *dirName, dir &temp_dir);
//  md
extern void makeDir(dir temp_dir, char *dirName);

// findSameName
extern bool findSameName(dir temp_dir, char*dir_name);

// blockManager
extern long getBlock(int len, int *b_index);
extern void freeBlock(int len, int pos);

// inodeManage
extern int getInode();
extern void freeInode(int inode);

// rd
extern void rd(char*path_and_dir_name);
extern bool isDirRemove(dir rdDir, dir currentDir);

// copy
extern bool isHostFile(char*path);

#endif