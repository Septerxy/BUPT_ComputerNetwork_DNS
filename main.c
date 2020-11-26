//#define _CRT_SECURE_NO_WARNINGS
//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <time.h>
#include <process.h>
#include "definition.h"
#include "functions.h"
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib") //���� ws2_32.dll

Translate DNSTable[AMOUNT];		//DNS����������
IDTransform IDTransTable[AMOUNT];	//IDת����
int IDcount = 0;					//ת�����е���Ŀ����
char Url[LENGTHOFURL];					//����
SYSTEMTIME TimeOfSys;                     //ϵͳʱ��
int Day, Hour, Minute, Second, Milliseconds;//����ϵͳʱ��ı���


int main()
{

	//�ο���https://wenku.baidu.com/view/ed7d64c852d380eb62946df4.html

    //��ʼ�� DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //�����׽���
    SOCKET servSock = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET localSock = socket(AF_INET, SOCK_DGRAM, 0);

    //���׽ӿڶ�����Ϊ������
    int unBlock = 1;
    ioctlsocket(servSock, FIONBIO, (u_long FAR*) &unBlock);//���ⲿ�׽ֿ�����Ϊ������
    ioctlsocket(localSock, FIONBIO, (u_long FAR*) &unBlock);//�������׽ֿ�����Ϊ������

    //���׽���
    SOCKADDR_IN serverName, clientName, localName;	//����DNS���ⲿDNS����������������׽��ֵ�ַ
    localName.sin_family = AF_INET;
    localName.sin_port = htons(PORT);
    localName.sin_addr.s_addr = inet_addr(LOCAL_DNS_ADDRESS);
    serverName.sin_family = AF_INET;
    serverName.sin_port = htons(PORT);
    serverName.sin_addr.s_addr = inet_addr(DEF_DNS_ADDRESS);

    //�󶨱��ط�������ַ
    if (bind(localSock, (SOCKADDR*)&localName, sizeof(localName)))
    {
        printf("Bind 53 port failed.\n");
        exit(-1);
    }
    else
        printf("Bind 53 port success.\n");

    char sendBuf[BUFSIZE]; //���ͻ���
    char recvBuf[BUFSIZE]; //���ջ���
    char* Path;
    Path=(char*)malloc(sizeof(char)*100);
    int recordNum; //txt�ļ���Ч����
    int iLen_cli, iSend, iRecv;

    strcpy(Path, "C:\\Users\\mrw29\\Desktop\\dnsrelay.txt");
    recordNum = InitialDNSTable(Path);
    //����ϵͳʱ��
    GetLocalTime(&TimeOfSys);
    Day = TimeOfSys.wDay;
    Hour = TimeOfSys.wHour;
    Minute = TimeOfSys.wMinute;
    Milliseconds = TimeOfSys.wMilliseconds;

	int find;
	unsigned short NewID;
	unsigned short* pID;

    //�����Ƿ������ľ������
	while (1)
	{
		iLen_cli = sizeof(clientName);
		memset(recvBuf, 0, BUFSIZE); //�����ջ�������Ϊȫ0

		//����DNS����
		//������int recvfrom(int s, void* buf, int len, unsigned int flags, struct sockaddr* from, int* fromlen);
		//����˵����recv()��������Զ��������ָ����socket ����������, �������ݴ浽�ɲ���buf ָ����ڴ�ռ�, ����len Ϊ�ɽ������ݵ���󳤶�.
		//����flags һ����0, ������ֵ������ο�recv().����from ����ָ�������͵������ַ, �ṹsockaddr ��ο�bind().����fromlen Ϊsockaddr �Ľṹ����.
		iRecv = recvfrom(localSock, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&clientName, &iLen_cli);
		//������
		if (iRecv == SOCKET_ERROR)
		{
			//printf("Recvfrom Failed: %s\n", strerror(WSAGetLastError()));
			continue; //ǿ�ƿ�ʼ��һ��ѭ��
		}
		else if (iRecv == 0)
		{
			break; //û����������ѭ��0
		}
		else
		{
			GetUrl(recvBuf, iRecv);				//��ȡ����
			find = IsFind(Url, recordNum);		//�������������в���

			//printf("We have get the url: %s\n", Url);

			//printf("%d\n", find);

			//��ʼ���������
			//��������������û���ҵ�
			if (find == NOTFOUND)
			{
				//printf("We dont find this url, will get a new ID and forward to SERVER.\n");
				//IDת��
				//pID = new (unsigned short);
				pID = (unsigned short*)malloc(sizeof(unsigned short*));
				memcpy(pID, recvBuf, sizeof(unsigned short)); //����ǰ���ֽ�ΪID
				NewID = htons(ReplaceNewID(ntohs(*pID), clientName, FALSE));
				memcpy(recvBuf, &NewID, sizeof(unsigned short));

				//��ӡ ʱ�� newID ���� ���� IP
				PrintInfo(ntohs(NewID), find);

				//��recvbufת����ָ�����ⲿDNS������
				iSend = sendto(servSock, recvBuf, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
				if (iSend == SOCKET_ERROR)
				{
					//printf("sendto Failed: %s\n", strerror(WSAGetLastError()));
					continue;
				}
				else if (iSend == 0)
					break;

				//delete pID; //�ͷŶ�̬������ڴ�
				free(pID);
				clock_t start, stop; //��ʱ
				double duration = 0;

				//���������ⲿDNS����������Ӧ����
				start = clock();
				iRecv = recvfrom(servSock, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&clientName, &iLen_cli);
				while ((iRecv == 0) || (iRecv == SOCKET_ERROR))
				{
					iRecv = recvfrom(servSock, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&clientName, &iLen_cli);
					stop = clock();
					duration = (double)(stop - start) / CLK_TCK;
					if (duration > 5)
					{
						printf("Long Time No Response From Server.\n");
						goto ps;
					}
				}
				//IDת��
				pID = (unsigned short*)malloc(sizeof(unsigned short*));
				memcpy(pID, recvBuf, sizeof(unsigned short)); //����ǰ���ֽ�ΪID
				int GetId = ntohs(*pID); //ntohs�Ĺ��ܣ��������ֽ���ת��Ϊ�����ֽ���
				unsigned short oID = htons(IDTransTable[GetId].oldID);
				memcpy(recvBuf, &oID, sizeof(unsigned short));
				IDTransTable[GetId].done = TRUE;

				//char* urlname;
				//memcpy(urlname, &(recvBuf[sizeof(DNSHDR)]), iRecv - 12);	//��ȡ�������е�������ʾ��Ҫȥ��DNS�����ײ���12�ֽ�
				//char* NewIP;

				//��ӡ ʱ�� newID ���� ���� IP
				PrintInfo(ntohs(NewID), find);

				//��IDת�����л�ȡ����DNS�����ߵ���Ϣ
				clientName = IDTransTable[GetId].client;

				//printf("We get a answer from SERVER, now we give it back to client.\n");

				//��recvbufת���������ߴ�
				iSend = sendto(localSock, recvBuf, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR)
				{
					//printf("sendto Failed: %s\n\n", strerror(WSAGetLastError()));
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID); //�ͷŶ�̬������ڴ�
			}

			//���������������ҵ�
			else
			{
				//printf("We have find this url.\n");
				//��ȡ�����ĵ�ID
				pID = (unsigned short*)malloc(sizeof(unsigned short*));
				memcpy(pID, recvBuf, sizeof(unsigned short));

				//ת��ID
				unsigned short nID = ReplaceNewID(ntohs(*pID), clientName, FALSE);

				//printf("We have get a new ID, now we will create an answer.\n");

				//��ӡ ʱ�� newID ���� ���� IP
				PrintInfo(nID, find);
				//�ο���https://blog.csdn.net/weixin_34192993/article/details/87949701
				//������Ӧ����ͷ
				memcpy(sendBuf, recvBuf, iRecv); //����������
				unsigned short AFlag = htons(0x8180); //htons�Ĺ��ܣ��������ֽ���ת��Ϊ�����ֽ��򣬼����ģʽ(big-endian) 0x8180ΪDNS��Ӧ���ĵı�־Flags�ֶ�
				memcpy(&sendBuf[2], &AFlag, sizeof(unsigned short)); //�޸ı�־��,�ƿ�ID�����ֽ�

				//�޸Ļش�����
				if (strcmp(DNSTable[find].IP, "0.0.0.0") == 0)
					AFlag = htons(0x0000);	//���ι��ܣ��ش���Ϊ0
				else
					AFlag = htons(0x0001);	//���������ܣ��ش���Ϊ1
				memcpy(&sendBuf[6], &AFlag, sizeof(unsigned short)); //�޸Ļش��¼�����ƿ�ID���ֽڡ�Flags���ֽڡ������¼�����ֽ�

				int curLen = 0; //���ϸ��µĳ���

				//����DNS��Ӧ����
				//�ο���http://c.biancheng.net/view/6457.html
				char answer[16];
				unsigned short Name = htons(0xc00c);
				memcpy(answer, &Name, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short TypeA = htons(0x0001);
				memcpy(answer + curLen, &TypeA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short ClassA = htons(0x0001);
				memcpy(answer + curLen, &ClassA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				//TTL���ֽ�
				unsigned long timeLive = htonl(0x7b);
				memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
				curLen += sizeof(unsigned long);

				unsigned short RDLength = htons(0x0004);
				memcpy(answer + curLen, &RDLength, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned long IP = (unsigned long)inet_addr(DNSTable[find].IP); //inet_addrΪIP��ַת������
				memcpy(answer + curLen, &IP, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
				curLen += iRecv;


				//�����ĺ���Ӧ���ֹ�ͬ���DNS��Ӧ���Ĵ���sendbuf
				memcpy(sendBuf + iRecv, answer, curLen);

				//printf("Create Over, give it to client.\n");

				clock_t Nstart, Nstop; //clock_tΪclock()�������صı�������
				double Nduration;


				//����DNS��Ӧ����
				Nstart = clock();
				iSend = sendto(localSock, sendBuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				//if (iSend == SOCKET_ERROR)
				//{
				//	//printf("sendto Failed: %s\n", strerror(WSAGetLastError()));
				//	Nstop = clock();
				//	Nduration = (double)(Nstop - Nstart) / CLK_TCK;
				//	if (Nduration > 1)
				//		goto ps;
				//	else
				//		continue;
				//}
				//else if (iSend == 0)
				//	break;

				free(pID); //�ͷŶ�̬������ڴ�

				//printf("\nThis loop is over, thanks.\n\n");
			}
		}
	ps:;
	}

	closesocket(servSock);
	closesocket(localSock);
	WSACleanup();				//�ͷ�ws2_32.dll��̬���ӿ��ʼ��ʱ�������Դ

	system("pause");
	return 0;
}
