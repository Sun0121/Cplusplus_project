#pragma once
#include "struct.h"

/*功能：改变当前的工作目录
 *
 *实现：1、若路径形式为" ../目录 "，则先返回上一级目录后再处理
 *		2、若路径形式为" ./目录 "，则直接在本级目录处理
 *		3、若路径形式为" root/目录 "，则返回根目录后再处理
 *		4、若路径形式为" 目录/第一级目录 "，则直接在本级目录处理
 *
 *path : 用户所输入的路径
 */
void cd(char *path)
{
	dir temp;
	if (getLastDir(path, temp))
		_current_dir = temp;
	else {
		printf("目录切换失败!\n");
		return;
	}
	findFullPath(_current_dir);
}

