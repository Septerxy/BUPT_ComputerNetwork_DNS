#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <process.h>
#include "definition.h"

#pragma  comment(lib, "Ws2_32.lib") //加载 ws2_32.dll

extern Translate DNSTable[AMOUNT];		//DNS域名解析表
extern IDTransform IDTransTable[AMOUNT];	//ID转换表
extern int IDcount;					//转换表中的条目个数
extern char Url[LENGTHOFURL];					//域名
extern SYSTEMTIME TimeOfSys;                     //系统时间
extern int Day, Hour, Minute, Second, Milliseconds;//保存系统时间的变量

//加载本地txt文件
int InitialDNSTable(char* path)
{
	int i = 0, j = 0;
	int num = 0;
	char* Temp[AMOUNT];//char型指针1500数组
	FILE* fp = fopen(path, "ab+");
	if (!fp)
	{
		printf("Open file failed.\n");
		exit(-1);
	}
	char* reac;
	while (i < AMOUNT - 1)//实现把每一行分开的操作
	{
		Temp[i] = (char*)malloc(sizeof(char)*200);
		//Temp[200];
		//fscanf(fp, "%*c%*[^\n]", IPTemp[i]);
		if (fgets(Temp[i],1000,fp)== NULL)//如果错误或者读到结束符，就返回NULL；
			break;
		else
		{
			//reac = strchr(Temp[i], '\n');          //查找换行符
			//if (reac)                            //如果find不为空指针
			//	*reac = '\0';
			;//printf("%s", Temp[i]);
		}
		i++;
	}
	if (i == AMOUNT - 1)
		printf("The DNS record memory is full.\n");


	for (j; j < i; j++)//用来把刚分好的TEMP【i】再次切割成IP和domain
	{
		char* ex1 = strtok(Temp[j], " ");
		char* ex2 = strtok(NULL, " ");
		if (ex2 == NULL)
		{
			printf("The record is not in a correct format.\n");
		}
		else
		{

			DNSTable[j].IP = ex1;
			DNSTable[j].domain = ex2;
			//DNSTable[j].IP[strlen(DNSTable[j].IP) - 1] = 0;

			//DNSTable[j].domain[strlen(DNSTable[j].domain) - 1] = '\0';
			//printf("%d\n%s\n%s\n",j, DNSTable[j].IP, DNSTable[j].domain);
			//printf("%d %s %s\n", j,  Temp[j], DNSTable[j].domain);
			num++;
		}
	}

	//printf("%d\n", num);
    //
	fclose(fp);
	printf("Load records success.\n");
	return num;
}

//获取DNS请求中的域名
void GetUrl(char* recvbuf, int recvnum)
{
	char urlname[LENGTHOFURL];
	int i = 0, j, k = 0;

	memset(Url, 0, LENGTHOFURL); //全用0初始化
	memcpy(urlname, &(recvbuf[sizeof(DNSHDR)]), recvnum - 12);	//获取请求报文中的域名表示，要去掉DNS报文首部的12字节

	int len = strlen(urlname);

	//域名转换
	while (i < len)
	{
		if (urlname[i] > 0 && urlname[i] <= 63)
			for (j = urlname[i], i++; j > 0; j--, i++, k++)
				Url[k] = urlname[i];

		if (urlname[i] != 0)
		{
			Url[k] = '.';
			k++;
		}
	}

	Url[k] = '\0';
}

//判断能不能在本中找到DNS请求中的域名，找到返回下标
int IsFind(char* url, int num)
{
	int find = NOTFOUND;
	char* domain;

	char* NUrl;
	NUrl = (char*)malloc(sizeof(char)*210);
	strcpy(NUrl, url);
	strcat(NUrl, "\r\n");

	//printf("URL: %sabab\n", url);
	//printf("%sabab\n", DNSTable[20].domain);

	for (int i = 0; i < num+2; i++)
	{
		if (DNSTable[i].domain)
		{
			domain = DNSTable[i].domain;
			/*printf("%s", domain);
			printf("%s", NUrl);
			printf("%d\n", strcmp(domain, NUrl));
			printf("wwwwwwwwwwwwwwwwwwwwwwwwwwww\n");*/

			if (strcmp(domain, NUrl) == 0)
			{
				//printf("!~!FOUND!~!\n");
				find = i;
				break;
			}
		}
	}

	//printf("find:%d\n", find);

	return find;
}

//将请求ID转换为新的ID，并将信息写入ID转换表中
unsigned short ReplaceNewID(unsigned short OldID, SOCKADDR_IN temp, BOOL ifdone)
{
	srand(time(NULL)); //随机数种子
	IDTransTable[IDcount].oldID = OldID;
	IDTransTable[IDcount].client = temp;
	IDTransTable[IDcount].done = ifdone;
	IDcount++; //ID转换表数目要更新~

	return (unsigned short)(IDcount - 1);	//以表中下标作为新的ID
}

//打印 时间 newID 功能 域名 IP
void PrintInfo(unsigned short newID, int find)
{
	//打印时间
	GetLocalTime(&TimeOfSys);
	//输出指定长度的字符串, 超长时不截断, 不足时左对齐:
	//printf("%-ns", str);            --n 为指定长度的10进制数值
	int Btime;
	int Ltime;
	Btime = ((((TimeOfSys.wDay - Day) * 24 + TimeOfSys.wHour - Hour) * 60 + TimeOfSys.wMinute - Minute) * 60) + TimeOfSys.wSecond - Second;
	Ltime = abs(TimeOfSys.wMilliseconds - Milliseconds);
	printf("%d.%d   %d", Btime, Ltime, newID);
	printf("    ");

	//在表中没有找到DNS请求中的域名
	if (find == NOTFOUND)
	{
		//中继功能
		printf("中继");
		printf("    ");
		//打印域名
		printf("%s",Url);
		printf("    ");
		//打印IP
		printf("\n");
	}

	//在表中找到DNS请求中的域名
	else
	{
		if (strcmp(DNSTable[find].IP, "0.0.0.0") == 0)  //不良网站拦截
		{
			//屏蔽功能
			printf("屏蔽");
			printf("    ");
			//打印域名(加*)
			//打印域名
			printf("***%s", Url);
			printf("    ");
			//打印IP
			printf("%s\n", DNSTable[find].IP);
		}

		//检索结果为普通IP地址，则向客户返回这个地址
		else
		{
			//服务器功能
			printf("Local服务器");
			printf("    ");
			//打印域名
			printf("***%s", Url);
			printf("    ");
			//打印IP
			printf("%s\n", DNSTable[find].IP);
		}
	}
}
