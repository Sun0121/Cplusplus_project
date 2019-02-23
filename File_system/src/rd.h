#pragma once
#include "struct.h"

/*
 *功能:删除目录
 *
 *实现:先分解出具体要删除的目录及其路径，然后根据提示信息具体删除
 *
 *path:用户输入的路径
 *
 */
void rd(char *path)
{
	dir temp_dir;
	char dirName[NAME_LEN];
	if (dividePathAndName(path, dirName, temp_dir))
		remove_dir(temp_dir, dirName);
	else
		printf("目录删除失败!\n");
}

/*
 *功能:在给定的目录temp下删除目录名为rd_name的目录
 *
 *
 *实现: 先查找temp_dir下是否存在名为rd_name的目录，然后判断要删除的目录是否是当前目录_current_dir
 *		或是_current_dir的祖先级目录，然后检查rd_name是否为空，是则直接删除，否则提示信息；
 *      若要删除则递归删除rd_name下的文件即子目录，最后更新父目录、超级块、组描述符、inode位图、数据块位图、
 *temp : 给定的具体目录
 *rd_name : 要删除的目录
 *
 */
void remove_dir(dir temp, char *rd_name)
{
	int i;			//循环控制变量
	int temp_pos = 0;	//记录找到的目录在temp中的位置
	int d_inode;	//要删除的目录的inode号
	for (i = 2; i < temp.size; i++)
	{
		if (strcmp(temp.file_entry[i].name, rd_name) == 0 &&
			inode_table[temp.file_entry[i].inode].i_mode == _DIRECTORY)	//找到该目录，记下其inode，跳出循环
		{
			temp_pos = i;
			d_inode = temp.file_entry[i].inode;
			break;
		}
	}
	if (i == temp.size) {
		printf("该目录不存在，目录删除失败!\n");
		return;
	}
	else	//找到该目录,进行删除操作
	{
		//从硬盘中读出要删除的目录
		dir r_dir;		//要删除的目录
		fd = fopen("LINUX_FILE_SYSTEM", "rb");
		if (fd == NULL)
		{
			printf("Linux_file_system create failed !\n");
			system("pause");
		}
		fseek(fd, inode_table[d_inode].i_block, SEEK_SET);
		fread(&r_dir, sizeof(dir), 1, fd);
		fclose(fd);

		if (!isDirRemove(r_dir,_current_dir))	//如果是当前目录，则删除失败
		{
			printf("另一个进程正在使用此目录，进程无法访问\n");
			printf("目录删除失败!\n");
			return;
		}

		if (r_dir.size > 2)//该目录下有文件或目录
		{
			char select = 's';	//初始化为s，无意义
			fflush(stdin);		//清除流
			do
			{
				if (select != '\n')
					printf("%s 该目录下有文件，是否还要删除(y/n)\n", r_dir.dir_name);
				select = getchar();
				if (select == 'y' || select == 'Y' || select == 'n' || select == 'N')
				{
					fflush(stdin);		//清除流
					break;
				}
			} while (1);
			if (select == 'y' || select == 'Y')
			{
				remove_dir(temp, r_dir, temp_pos);	//递归删除子目录即文件
				return;
			}
			else
			{
				printf("目录删除失败!\n");
				return;
			}
		}
		else			//该目录为空，直接删除
		{
			//(inode_table[d_inode].i_block-FIRST_DATA_ADDR)/BLOCK_SIZE为r_dir数据块在数据块位图中的起始位置
			freeBlock(inode_table[d_inode].i_blocks,
				((inode_table[d_inode].i_block - FIRST_DATA_ADDR) / BLOCK_SIZE));	//释放数据块
			freeInode(d_inode);		//释放inode节点
			//更新其父目录,把其后面的文件及目录的file_entry往前移一位，填补删除的目录的空位
			for (i = temp_pos; i < temp.size; i++)
			{
				temp.file_entry[i].inode = temp.file_entry[i + 1].inode;
				strcpy(temp.file_entry[i].name, temp.file_entry[i + 1].name);
			}
			temp.size--;
			if (temp.inode == _current_dir.inode)
				_current_dir = temp;

			//将改动的内容写回硬盘
			fd = fopen("LINUX_FILE_SYSTEM", "rb+");
			if (fd == NULL)
			{
				printf("Linux file system file creat failed !\n");
				exit(0);
			}
			fseek(fd, 0, SEEK_SET);
			fwrite(_block_group, sizeof(block_group), BLOCKS_GROUP_NUM, fd);	//将超级块和组描述符写回
			fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + d_inode), SEEK_SET);
			fwrite(inode_bitmap, sizeof(bool), 1, fd);							//将inode位图写回
			fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + (sizeof(bool) + sizeof(inode))*TOTAL_INODES_NUM
				+ (inode_table[d_inode].i_block - FIRST_DATA_ADDR) / BLOCK_SIZE), SEEK_SET);
			fwrite(block_bitmap, sizeof(bool), inode_table[d_inode].i_blocks, fd);	//将数据块位图写回
			fseek(fd, inode_table[temp.inode].i_block, SEEK_SET);
			fwrite(&temp, sizeof(dir), 1, fd);									//将父目录写回
			fclose(fd);
		}
	}
}

/*
 *功能，判断要删除的目录是否是当前正在工作的目录或当前正在工作目录的祖先级目录
 *
 *实现：递归向上级查找，看是否有当前要被删除目录的祖先级目录
 *
 */
bool isDirRemove(dir rdDir,dir currentDir) {
	if (rdDir.inode == 0)
		return false;                 // 根目录不能删除
	if (currentDir.inode != 0)        // 0是根目录的i节点号
	{
		if (currentDir.inode == rdDir.inode)
			return false;                     // 不能删除
		else {
			dir fatherDir;
			fd = fopen("LINUX_FILE_SYSTEM", "rb");
			if (fd == NULL)
			{
				printf("Linux_file_system create failed !\n");
				system("pause");
			}
			fseek(fd, inode_table[currentDir.file_entry[1].inode].i_block, SEEK_SET);
			fread(&fatherDir, sizeof(dir), 1, fd);
			fclose(fd);
			return isDirRemove(rdDir, fatherDir);
		}
	}
	return true;
}


/*
 *功能:递归删除目录及其下的文件
 *
 *father_dir : 被删除目录的父级目录
 *r_dir : 当前要被删除的目录
 *index : 被删除的目录在父级目录中的位置
 */
void remove_dir(dir father_dir, dir r_dir, int index)
{
	int i;		//循环控制变量
	for (i = 2; i < r_dir.size; i++)
	{
		if (inode_table[r_dir.file_entry[i].inode].i_mode == _DIRECTORY)
		{
			//从硬盘读出其子目录
			dir sub_dir;	//要删除目录的子目录
			fd = fopen("LINUX_FILE_SYSTEM", "rb");
			if (fd == NULL)
			{
				printf("Linux file system file creat failed !\n");
				exit(0);
			}
			fseek(fd, inode_table[r_dir.file_entry[i].inode].i_block, SEEK_SET);
			fread(&sub_dir, sizeof(dir), 1, fd);
			fclose(fd);

			remove_dir(r_dir, sub_dir, i);//递归删除子目录
		}
		else
			delete_file(r_dir, r_dir.file_entry[i].name);		//删除文件
	}
	freeBlock(inode_table[r_dir.inode].i_blocks,
		((inode_table[r_dir.inode].i_block - FIRST_DATA_ADDR) / BLOCK_SIZE));	//释放数据块
	freeInode(r_dir.inode);				//释放inode节点
	//更新其父目录,把其后面的文件及目录的file_entry往前移一位，填补删除的目录的空位
	for (i = index; i < father_dir.size; i++)
	{
		father_dir.file_entry[i].inode = father_dir.file_entry[i + 1].inode;
		strcpy(father_dir.file_entry[i].name, father_dir.file_entry[i + 1].name);
	}
	father_dir.size--;
	if (father_dir.inode == _current_dir.inode)
		_current_dir = father_dir;

	//将改动的内容写回硬盘
	fd = fopen("LINUX_FILE_SYSTEM", "rb+");
	if (fd == NULL)
	{
		printf("Linux file system file creat failed !\n");
		exit(0);
	}
	fseek(fd, 0, SEEK_SET);
	fwrite(_block_group, sizeof(block_group), BLOCKS_GROUP_NUM, fd);	//将超级块和组描述符写回
	fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + r_dir.inode), SEEK_SET);
	fwrite(inode_bitmap, sizeof(bool), 1, fd);							//将inode位图写回
	fseek(fd, (sizeof(block_group)*BLOCKS_GROUP_NUM + (sizeof(bool) + sizeof(inode))*TOTAL_INODES_NUM
		+ (inode_table[r_dir.inode].i_block - FIRST_DATA_ADDR) / BLOCK_SIZE), SEEK_SET);
	fwrite(block_bitmap, sizeof(bool), inode_table[r_dir.inode].i_blocks, fd);	//将数据块位图写回
	fseek(fd, inode_table[father_dir.inode].i_block, SEEK_SET);
	fwrite(&father_dir, sizeof(dir), 1, fd);									//将父目录写回
	fclose(fd);
}
