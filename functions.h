#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#pragma once

int InitialDNSTable(char* path); //���ر���txt�ļ�
void GetUrl(char* recvbuf, int recvnum); //��ȡDNS�����е�����
int IsFind(char* url, int num);//�ж��ܲ����ڱ����ҵ�DNS�����е��������ҵ������±�
unsigned short ReplaceNewID(unsigned short OldID, SOCKADDR_IN temp, BOOL ifdone); //������IDת��Ϊ�µ�ID��������Ϣд��IDת������
void PrintInfo(unsigned short newID, int find); //��ӡ ʱ�� newID ���� ���� IP



#endif // FUNCTIONS_H_INCLUDED
