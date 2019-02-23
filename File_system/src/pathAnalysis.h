#pragma once
#include"struct.h"


/*功能：解析出最终的那个路径("root/目录/第一个目录"通过此函数解析得到"第一个目录")
 *
 *path : 待解析的路径
 *temp_dir ： 可理解为返回值，将解析到的目的路径赋值给temp_dir
 */
bool getLastDir(char *path, dir &temp_dir)
{
	temp_dir = _current_dir;
	if (strcmp(path, ".") == 0) {                  // 当前目录
		return true;

	}else if (strcmp(path, "..") == 0)            //  上一级目录
	{
		fd = fopen("LINUX_FILE_SYSTEM", "rb");
		if (fd == NULL)
		{
			printf("Linux_file_system create failed !\n");
			system("pause");
		}
		fseek(fd, inode_table[temp_dir.file_entry[1].inode].i_block, SEEK_SET);
		fread(&temp_dir, sizeof(dir), 1, fd);
		fclose(fd);
		return true;
	}
	else {
		int pos = 1;           // 指定路径中的第几层目录
		char dirName[NAME_LEN];   // 保存分解出的目录名
		getDirName(path, pos, dirName);     // 查找某一层级的目录名
		fd = fopen("LINUX_FILE_SYSTEM", "rb");
		if (fd == NULL)
		{
			printf("Linux_file_system create failed !\n");
			system("pause");
		}

		if (strcmp(dirName, "root") == 0)
		{
			fseek(fd, inode_table[0].i_block, SEEK_SET);
			fread(&temp_dir, sizeof(dir), 1, fd);		//先跳转到根目录
			pos++;
			if (findFinalDir(path, pos, dirName, temp_dir)) {
				fclose(fd);
				return true;
			}
			else {
				fclose(fd);
				return false;
			}
		}
		else if (strcmp(dirName, "..") == 0) {
			fseek(fd, inode_table[temp_dir.file_entry[1].inode].i_block, SEEK_SET);
			fread(&temp_dir, sizeof(dir), 1, fd);         // 先转到上一级目录
			pos++;
			if (findFinalDir(path, pos, dirName, temp_dir)) {
				fclose(fd);
				return true;
			}
			else {
				fclose(fd);
				return false;
			}
		}
		else if (strcmp(dirName, ".") == 0) {
			pos++;
			if (findFinalDir(path, pos, dirName, temp_dir)) {
				fclose(fd);
				return true;
			}
			else {
				fclose(fd);
				return false;
			}
		}
		else {
			if (findFinalDir(path, pos, dirName, temp_dir)) {
				fclose(fd);
				return true;
			}
			else {
				fclose(fd);
				return false;
			}
		}
	}
}

/*
 *功能:查找出完整的路径 (例如根据 "/第一个目录" 查找出 "root/目录/第一个目录 ")
 *
 *算法步骤:递归查找上一级目录，直到找到根目录为止
 *
 *temp_dir : 需要被查找的目录
 */
void findFullPath(dir temp_dir) {
	dir temp = temp_dir;
	 
	if (temp_dir.inode != 0) {          // 0为根目录的i节点
		fd = fopen("LINUX_FILE_SYSTEM", "rb");
		if (fd == NULL)
		{
			printf("Linux_file_system create failed !\n");
			system("pause");
		}
		fseek(fd, inode_table[temp_dir.file_entry[1].inode].i_block, SEEK_SET);
		fread(&temp_dir, sizeof(dir), 1, fd);
		fclose(fd);
		findFullPath(temp_dir);
	}
	if (temp.inode == 0) {
		strcpy(current_path, temp.dir_name);
		if(temp.inode == _current_dir.inode)  // 此时_current_dir是目标目录
			sprintf(current_path, "%s>", current_path);
	}
	else
	{
		sprintf(current_path, "%s/%s", current_path, temp.dir_name);
		if (temp.inode == _current_dir.inode)
			sprintf(current_path, "%s>", current_path);
	}
}

/*
 *功能：根据pos查找某一层级的目录名(例如"root/一层/二层" 若pos = 1,则找到root,若pos为2,则找到一层，类推...)
 *
 *算法：利用strtok函数进行字符串分割
 *
 *path : 待分解的路径
 *pos :  分解级数
 *dirName : 可理解为返回值，将查找到的结果赋值给dirName
 */
bool getDirName(char *path, int pos, char *dirName) {
	char *pathCopy = new char[MAX_PATH_LEN];
	strcpy(pathCopy, path);

	char dir_name[NAME_LEN];

	char *singleDir = strtok(pathCopy, "/");
	strcpy(dir_name, singleDir);

	singleDir = strtok(NULL, "/");
	for (int i = 1; i < pos; ++i) {
		if (singleDir) {
			
			strcpy(dir_name, singleDir);
			singleDir = strtok(NULL, "/");
		}
		else {
			return false;
		}
	}
	strcpy(dirName, dir_name);
	return true;
}

/*
 *功能：不断查找路径中给定的pos级目录并验证其正确性，直到查找到最后一级,即最终目录
 *
 *path : 待解析路径
 *pos : 标志当前处于第几级目录
 *dirName : 查找到pos级别的目录名赋值给dirName
 *temp_dir ： 查找到的路径中的最后一级目录赋值给temp_dir
 */
bool findFinalDir(char *path, int &pos, char *dirName, dir &temp_dir) {
	while (getDirName(path, pos, dirName)) {
		int temp_dir_size = temp_dir.size;         // 此时temp_dir是dirName的父目录
		int i;
		for (i = 2; i < temp_dir_size; ++i) {     // temp_dir的第一个目录分别是.和..
			if (strcmp(dirName, temp_dir.file_entry[i].name) == 0 &&
				inode_table[temp_dir.file_entry[i].inode].i_mode == _DIRECTORY) {
				fseek(fd, inode_table[temp_dir.file_entry[i].inode].i_block, SEEK_SET);
				fread(&temp_dir, sizeof(dir), 1, fd);		//temp_dir已经改变!!!
				break;
			}
		}
		if (i < temp_dir_size) {
			pos++;
		}
		else {
			printf("给出的路径错误!\n");
			return false;
		}
	}
	return true;
}

/*
 *功能：分离出路径与目录(例如 "root/a/b/c" 分离出"root>a>b" 和 "c" ,c是待创建的目录或文件)
 *
 *实现：利用strrchr函数
 *
 *path : 待分解的路径
 *dirName : 分解得到的名字结果赋值给dirName
 *temp_dir：分解得到的目录结果赋值给temp_dir
 */
bool dividePathAndName(char *path,char *dirName, dir &temp_dir) {
	char *str;
	temp_dir = _current_dir;

	char *copyPath = new char[MAX_PATH_LEN];
	strcpy(copyPath, path);

	str = strrchr(path, '/');
	if (str) {
		//分析给出的路径和目录名
		int divide_pos = str - path;	//最后一个 '/' 所在的位置
		char *dividePath = new char[divide_pos + 1];			//要打开、删除或创建的文件或目录所在的路径
		int i;		//循环变量
		for (i = divide_pos + 1; i < strlen(copyPath) + 1; ++i)				//分解出要创建的文件名
			dirName[i - (divide_pos + 1)] = copyPath[i];
		dirName[i] = 0;
		for (i = 0; i < divide_pos; i++)						//分解出要打开、删除或创建的文件或目录所在的路径
			dividePath[i] = copyPath[i];
		dividePath[i] = 0;
		
		if (!getLastDir(dividePath, temp_dir))
			return false;

		delete(dividePath);

		return true;
	}
	else {
		strcpy(dirName, path);
		return true;
	}
}