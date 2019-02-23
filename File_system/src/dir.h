#include "struct.h"

/*
 *功能:显示目录,显示当前目录或指定目录(绝对路径或相对路径)
 *
 *实现：1、若路径形式为" ../目录 "，则先返回上一级目录后再处理
 *		2、若路径形式为" ./目录 "，则直接在本级目录处理
 *		3、若路径形式为" root/目录 "，则返回根目录后再处理
 *		4、若路径形式为" 目录/第一级目录 "，则直接在本级目录处理
 *
 *path : 用户输入的目录
 */
void dirDisplay(char *path)
{
	dir temp;
	if (getLastDir(path, temp))
	{
		printf("%8s 的目录\n\n", path);
		display(temp);
	}
	else
		printf("目录显示失败!\n");
}

/*
 *功能:显示指定目录下面的文件及子目录的参数情况
 *
 *temp_dir : 待显示的目录(最后一级)
 */
void display(dir temp_dir)
{
	printf("%15s%10s%15s  %-37s\n","物理地址","类型","长度(字节)","名字");
	for(int i=0;i< temp_dir.size;i++)
	{
		printf("%15ld",inode_table[temp_dir.file_entry[i].inode].i_block);  // 物理地址
		if(inode_table[temp_dir.file_entry[i].inode].i_mode == _DIRECTORY)  // 文件类型(文件 or 目录)
		{
			printf("%10s","<DIR>");
			printf("%15s","");
		}
		else
		{
			printf("%10s","<FILE>");
			printf("%15u",inode_table[temp_dir.file_entry[i].inode].i_size);  // 显示文件大小
		}
		printf("  %-37s\n",temp_dir.file_entry[i].name);
	}
}