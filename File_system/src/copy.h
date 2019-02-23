#include "struct.h"

/*
 *功能:拷贝文件
 *
 *算法步骤:首先检查给出的两个路径是否正确；
 *         如果正确则找到对应的文件，读出内容到内容缓冲区中，再将内容缓冲区的内容写到目的路径中
 *         否则提示错误信息
 *src_path : 源文件所在路径
 *des_path ： 拷贝文件至此路径中
 */
void copy_file(char*src_path,char*des_path)
{
	dir temp_dir;				//保存目的目录
	char file_name[NAME_LEN];	//文件名
	char *buf;					//文件内容缓冲区
	long len = 0;				//文件长度
	char divide;				//保存分隔符

	if(isHostFile(src_path))		//从host文件系统拷贝到模拟Linux文件系统
	{
		fd = fopen(src_path,"rb");
		if(fd == NULL)
		{
			printf("文件不存在,文件拷贝失败!\n");
			return;
		}
		fseek(fd,0,SEEK_END);
		len = ftell(fd);           // 获取fd指针的当前位置
		buf = new char[len];	//申请文件内容缓冲区
		buf[len-1] = 0;
		fseek(fd,0,SEEK_SET);
		fread(buf,len-1,1,fd);	//读取文件的内容
		fclose(fd);

		divide = '\\';	//windows路径分隔符

		strcpy(file_name,strrchr(src_path,divide) +1);			//获得文件名

		if(getLastDir(des_path,temp_dir))	//找到目的目录
		{
			write_back(temp_dir,file_name,buf,len);
			return;
		}
		else
			printf("文件拷贝失败!\n");
	}
	else
	{
		if(isHostFile(des_path))		//从模拟Linux文件系统拷贝到host文件系统
		{
			if(dividePathAndName(src_path,file_name,temp_dir))
			{
				int inode;              // 源文件的i节点号
				for(int i=2;i<temp_dir.size;i++)
					if(strcmp(temp_dir.file_entry[i].name,file_name)==0)
						inode = temp_dir.file_entry[i].inode;

				buf = new char[inode_table[inode].i_size-1];	//分配缓冲区

				len = read_file(temp_dir,file_name,inode,buf);

				char *complete_path = new char[MAX_PATH_LEN];	//Windows中的完整路径
				sprintf(complete_path,"%s\\%s",des_path,file_name);
				fd = fopen(complete_path,"wb+");
				if(fd == NULL)
				{
					printf("文件拷贝失败!\n");
					delete(buf);
					delete(complete_path);
					return;
				}
				fwrite(buf,len-1,1,fd);
				fclose(fd);
				delete(complete_path);
				delete(buf);
				return;
			}
			printf("文件拷贝失败!\n");
		}
		else	//Linux文件系统内部拷贝
		{
			if(dividePathAndName(src_path,file_name, temp_dir))
			{
				int f_inode;
				for(int i=2;i<temp_dir.size;i++)
				{
					if(strcmp(temp_dir.file_entry[i].name,file_name)==0 && inode_table[temp_dir.file_entry[i].inode].i_mode == _FILE)
					{
						f_inode = temp_dir.file_entry[i].inode;
						break;
					}
				}
				buf = new char[inode_table[f_inode].i_size];	//分配缓冲区

				len = read_file(temp_dir,file_name,f_inode,buf);

				char *complete_path = new char[MAX_PATH_LEN];	//Linux中的完整路径
				sprintf(complete_path,"%s/%s",des_path,file_name);
				if(dividePathAndName(complete_path,file_name, temp_dir))
					write_back(temp_dir,file_name,buf,len);
				else
					printf("文件拷贝失败!\n");
				delete(complete_path);
				delete(buf);
			}
		}
	}
}

/*
 *功能:根据给定目录temp_dir及其下文件file_name，读取内容到buf中
 *
 *temp_dir : 给定目录
 *file_name : 文件名
 *inode : file_name的i节点号
 *buf : 接收文件内容的缓冲区
 */
long read_file(dir temp_dir,char *file_name,int inode,char *buf)
{
	fd = fopen("LINUX_FILE_SYSTEM","rb");
	if (fd == NULL)
	{
		printf("Linux_file_system create failed !\n");
		system("pause");
	}
	fseek(fd,inode_table[inode].i_block,SEEK_SET);
	fread(buf,inode_table[inode].i_size-1,1,fd);
	fclose(fd);
	buf[inode_table[inode].i_size-1] = 0;
	return inode_table[inode].i_size;
}

/*
 *功能:将创建的文件更新并写回硬盘
 * 
 *实现：先检查目标目录下是否存在同名文件，然后更新系统信息
 *
 *temp : 待创建文件所在的目录
 *file_name:待创建的文件名
 *buf : 中间缓存，存储文件内容
 *len : 文件的长度(字节数)
 */
void write_back(dir temp,char *file_name,char *buf,int len)
{
	int b_index = -1;		//该文件分配的数据块在数据块位图中的位置
	long addr = -1;			//该文件分配的数据块首地址
	int blocks_num = 0;		//该文件所占的数据块数目
	int f_inode = -1;		//该文件的inode

	blocks_num = len/BLOCK_SIZE;	//计算文件所占的数据块数目
	if(len % BLOCK_SIZE != 0)
		blocks_num += 1;

	if (findSameName(temp, file_name))
	{
		printf("%s 目录下已存在名为 %s 的文件，操作失败!\n", temp.dir_name, file_name);
		return;
	}

	addr = getBlock(blocks_num,&b_index);	//为文件分配数据块空间		
	if(addr<0)
	{
		printf("文件太大，硬盘空间不够，文件创建失败!\n");
		return;
	}
	f_inode = getInode();						//为文件分配inode
	//更新文件inode
	inode_table[f_inode].i_block = addr;
	inode_table[f_inode].i_blocks = blocks_num;
	inode_table[f_inode].i_change = false;
	inode_table[f_inode].i_mode = _FILE;
	inode_table[f_inode].i_size = len ;
	inode_table[f_inode].i_uid = uid;
	//更新其所属目录
	temp.file_entry[temp.size].inode = f_inode;
	strcpy(temp.file_entry[temp.size].name,file_name);
	temp.size++;
	if(temp.inode == _current_dir.inode)
		_current_dir = temp;
	//写回硬盘
	fd = fopen("LINUX_FILE_SYSTEM","rb+");
	if (fd == NULL)
	{
		printf("Linux_file_system create failed !\n");
		system("pause");
	}
	fseek(fd,0,SEEK_SET);
	fwrite(&_block_group[0],sizeof(block_group),BLOCKS_GROUP_NUM,fd);	//把数据块组写回文件
	fseek(fd,(sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*f_inode),SEEK_SET);
	fwrite(&inode_bitmap[f_inode],sizeof(bool),1,fd);					//把inode 位图写回文件
	fseek(fd,(sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*TOTAL_INODES_NUM
		+ sizeof(inode)*f_inode),SEEK_SET);
	fwrite(&inode_table[f_inode],sizeof(inode),1,fd);					//把inode 表写回文件
	fseek(fd,(sizeof(block_group)*BLOCKS_GROUP_NUM + sizeof(bool)*TOTAL_INODES_NUM
		+ sizeof(inode)*TOTAL_INODES_NUM + sizeof(bool)*b_index),SEEK_SET);
	fwrite(&block_bitmap[b_index],sizeof(bool),blocks_num,fd);	//把修改的数据块位图写回文件
	fseek(fd,addr,SEEK_SET);
	fwrite(buf,1,len,fd);				//把文件写入硬盘
	fseek(fd,inode_table[temp.inode].i_block,SEEK_SET);
	fwrite(&temp,sizeof(dir),1,fd);		//把父目录写回文件
	fclose(fd);
}

/*
 *功能:检查是否从host文件系统中拷贝
 *
 *算法步骤:检查路径的第一个字段是否为<host>,是则把路径从path中提取出来，并拷贝到path中
 *         否则返回false
 */
bool isHostFile(char*path)
{
	char host[] = "<host>";
	char *divide;
	divide = strstr(path, host);
	if (divide == path)
	{
		strcpy(path, divide + sizeof(host) - 1); // 赋值从源地址开始，到碰到'\0'结束
		return true;
	}
	return false;
}