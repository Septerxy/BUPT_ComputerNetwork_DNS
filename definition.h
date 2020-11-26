#ifndef DEFINITION_H_INCLUDED
#define DEFINITION_H_INCLUDED

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <iomanip>
#include <WinSock2.h>
#include <windows.h>
#include <time.h>

#define BUFSIZE 1024 //最大报文缓存大小
#define PORT 53   //53端口号
#define DEF_DNS_ADDRESS "223.5.5.5"	//ipconfig/all 得知外部服务器dns地址
#define LOCAL_DNS_ADDRESS "127.0.0.1" //本地DNS服务器地址
#define AMOUNT 1500//最大ID转换表大小
#define NOTFOUND 32767 //没有找到
#define LENGTHOFURL 64 //0~63

//DNS报文首部 12字节
typedef struct DNSHeader
{
	unsigned short ID; //标志
	unsigned short Flags; //标识
	unsigned short QuestionNum;  //问题数
	unsigned short AnswerNum; //资源记录数
	unsigned short AuthorNum; //授权资源记录数
	unsigned short AdditionNum; //额外资源记录数
} DNSHDR, * pDNSHDR;

//DNS域名解析表结构
typedef struct translate
{
	char * IP;						//IP地址
	char * domain;					//域名
} Translate;

//ID转换表结构
typedef struct IDChange
{
	unsigned short oldID;			//原有ID
	BOOL done;						//标记是否完成解析
	SOCKADDR_IN client;				//请求者套接字地址
} IDTransform;



#endif // DEFINITION_H_INCLUDED
