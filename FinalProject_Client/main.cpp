/**
 * FinalProject_Client
 * 2018112749 ������
 **/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <winsock2.h>
#include <windows.h>
#include <process.h> 

#pragma comment(lib, "Ws2_32.lib")

#define MAXLEN 1001
#define SERV_ADDR "127.0.0.1"
#define SERV_PORT 12345


using namespace std;




/**
 * ���� ������
 **/
unsigned WINAPI WaitingRoom(void* arg) {
	SOCKET sock = *((SOCKET*)arg);  // server socket

	/*
	char nameMsg[NAME_SIZE + BUF_SIZE];
	while (1) {
		fgets(msg, BUF_SIZE, stdin);
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			closesocket(hSock);
			exit(0);
		}
		sprintf(nameMsg, "%s %s", name, msg);
		send(hSock, nameMsg, strlen(nameMsg), 0);
	}

	*/

	char input[MAXLEN];
	char request_msg[MAXLEN];
	char response_msg[MAXLEN];
	printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
	while (1) {
		memset(input, 0, sizeof(input));
		memset(request_msg, 0, sizeof(request_msg));
		memset(response_msg, 0, sizeof(response_msg));

		printf(">> ");
		scanf("%s", input);
		if (!strcmp(input, "!L") || !strcmp(input, "!l")) {
			//// ����� ����Ʈ
			// request_msg = "LIST" ������ ����
			strcpy(request_msg, "LIST");
			send(sock, request_msg, strlen(request_msg), 0);

			// �����κ��� ���� �������� ���е� ����Ʈ ���ڿ� ���� �� ���
			int strlen = recv(sock, response_msg, sizeof(response_msg), 0);
			printf("[����� ����Ʈ]\n");
			char* ptr = strtok(response_msg, " "); 
			while (ptr != NULL) {
				printf("%s\n", ptr);          // �ڸ� ���ڿ� ���
				ptr = strtok(NULL, " ");      // ���� ���ڿ��� �߶� �����͸� ��ȯ
			}


		}
		else if (!strcmp(input, "!R") || !strcmp(input, "!r")) {
			printf("ä�� ��û\n");
		}
		else if (!strcmp(input, "!Q") || !strcmp(input, "!q")) {
			printf("����\n");
		}
		else {
			printf("�߸��� �޴��Դϴ�. �ٽ� �Է��ϼ���.\n");
			continue;
		}
	}



	return 0;
}








int main() {
	/////////////////////////////
	WSADATA wsaData; // for WSAStartup
	SOCKET sock;  // ���� ������ ����
	SOCKADDR_IN serv_addr;  // 
	HANDLE WaitingRoomThread;
	HANDLE hSndThread, hRcvThread;
	/////////////////////////////
	char nickname[MAXLEN];
	/////////////////////////////

	/**
	 * �г��� �Է�
	 **/
	memset(nickname, 0, sizeof(nickname));
	printf("�г����� �Է��� �ּ���.\n>> ");
	scanf("%s", nickname);


	/**
	 * WSAStartup() - Winsock �ʱ�ȭ 
	 * socket() - ����� ���� ����
	 **/
	printf("�ʱ�ȭ ���Դϴ�...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup() error!");
	}
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket() error!");
	}

	/**
	 * ���� ���� ���� ����
	 **/
	printf("������ ���� ���Դϴ�...\n");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_ADDR);
	serv_addr.sin_port = htons(SERV_PORT);

	if (connect(sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == -1) {
		fprintf(stderr, "connect() error!");
	} else {
		printf("������ ����Ǿ����ϴ�.\n");
	}

	/**
	 * ���� �� ���ǿ� ���� ��, ���� 1ȸ �г��� ����
	 **/
	send(sock, nickname, strlen(nickname), 0);
	

	WaitingRoomThread = (HANDLE)_beginthreadex(NULL, 0, WaitingRoom, (void*)&sock, 0, NULL);
	WaitForSingleObject(WaitingRoomThread, INFINITE);

	// WaitForSingleObject(hSndThread, INFINITE);
	// WaitForSingleObject(hRcvThread, INFINITE);






	closesocket(sock);
	WSACleanup();
	


	return 0;
}