#include "struct.h"
/*
 *登陆文件系统
 */
void login()
{

	printf("-----班级: 16计科2班-----\n");
	printf("-----姓名: 孙斌----------\n");
	printf("-----学号: 201630599319--\n\n");
	printf("请登录系统...\n");

	bool isLogin = false;
	while (!isLogin) {
		int loginID;
		char loginPassword[100];
		printf("请输入用户ID: ");
		fflush(stdin);
		scanf("%dL", &loginID);
		printf("请输入密码: ");
		fflush(stdin);
		scanf("%s", loginPassword);
		fflush(stdin);
		for (int i = 0; i < MAX_USER_NUM; i++)
		{
			if (loginID == _user[i].uid && strcmp(loginPassword, _user[i].password) == 0)
			{
				uid = loginID;
				isLogin = true;
				return;
			}
		}
		printf("用户名或密码错误，请重新输入\n\n");
	}
}
