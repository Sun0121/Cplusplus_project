#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "pathAnalysis.h"
#include "blockManage.h"
#include "inodeManage.h"
#include "findSameName.h"
#include "struct.h"
#include "init.h"
#include "login.h"
#include "help.h"
#include "dir.h"
#include "copy.h"
#include "info.h"
#include "check.h"
#include "exit.h"
#include "cd.h"
#include "md.h"
#include "rd.h"
#include "newfile.h"
#include "cat.h"
#include "del.h"

unsigned int uid;										//使用文件系统的用户ID
char pwd[20];											//密码
command cmd[13];										//命令
block_group _block_group[BLOCKS_GROUP_NUM];		//数据块组(超级块 + 组描述符)
inode inode_table[TOTAL_INODES_NUM];			//inode表
user _user[MAX_USER_NUM];						//用户
dir _current_dir;								//目录
bool block_bitmap[TOTAL_BLOCKS_NUM];					//数据块位图
bool inode_bitmap[TOTAL_INODES_NUM];					//inode 位图
char current_path[MAX_PATH_LEN];								//保存当前路径
FILE *fd;												//系统文件

void main()
{
	load();login(); 
	printf("\n*****************************************************************************\n");
	printf("*                  欢迎使用模拟Linux文件系统                                *\n");
	printf("*****************************************************************************\n");
	while(1)
	{
		char control[100];					//用于输入命令
		char path1[MAX_PATH_LEN],path2[MAX_PATH_LEN];		//用于路径或名字输入
		int i=0;							//循环控制变量
		fflush(stdin);						//清除流
		printf("\n%s",current_path);
		scanf("%s",control);
		for(i=0;i<13;i++)
			if(strcmp(cmd[i].order,control)==0)
				break;
		switch(i)
		{
		case 0:		//init
			initialize();
			break;
		case 1:		//info
			info();
			break;
		case 2:		//cd
			scanf("%s",path1);
			cd(path1);
			break;
		case 3:		//dir
			scanf("%s",path1);
			dirDisplay(path1);
			break;
		case 4:		//md
			scanf("%s",path1);
			md(path1);
			break;
		case 5:		//rd
			scanf("%s",path1);
			rd(path1);
			break;
		case 6:		//newfile
			scanf("%s",path1);
			newfile(path1);
			break;
		case 7:		//cat
			scanf("%s",path1);
			cat(path1);
			break;
		case 8:		//copy
			scanf("%s",path1);
			scanf("%s",path2);
			copy_file(path1,path2);
			break;
		case 9:		//del
			scanf("%s",path1);
			del(path1);
			break;
		case 10:	//check
			check();
			break;
		case 11:	//exit
			exit();
			break;
		case 12:	//help
			help();
			break;
		default:
			printf("没有此命令!如需帮助请输入 hlep\n");
		}
	}
}