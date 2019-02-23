#include "struct.h"
/*
 *功能:判断给定的目录下是否存在相同的文件名
 *
 *算法步骤:根据目录的大小循环查找对比，有同名返回true，否则返回false
 */
bool findSameName(dir temp_dir,char*dirName)
{
	for(int i=2;i<temp_dir.size;i++)
	{
		if(strcmp(temp_dir.file_entry[i].name, dirName)==0)
			return true;
	}
	return false;
}