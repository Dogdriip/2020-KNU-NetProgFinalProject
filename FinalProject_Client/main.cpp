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
	char input[MAXLEN];
	char request_msg[MAXLEN];
	char response_msg[MAXLEN];

	printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
	while (1) {
		memset(input, 0, sizeof(input));
		memset(request_msg, 0, sizeof(request_msg));
		memset(response_msg, 0, sizeof(response_msg));

		scanf("%s", input);
		if (!strcmp(input, "!L") || !strcmp(input, "!l")) {
			//// ����� ����Ʈ
			// request_msg = "LIST" ������ ����
			sprintf(request_msg, "LIST");
			send(sock, request_msg, strlen(request_msg), 0);
		}
		else if (!strcmp(input, "!R") || !strcmp(input, "!r")) {
			//// ä�� ��û
			// ��� �г��� �Է�
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			printf("��� �г����� �Է��ϼ���.\n");
			scanf("%s", dest_nickname);
			
			// request_msg = "REQ ����ڸ�" ������ ����
			sprintf(request_msg, "REQ %s", dest_nickname);
			send(sock, request_msg, strlen(request_msg), 0);
			
			// ���� �� "��û ��..." �޽��� ǥ��, �����κ��� �޽��� ���
			printf("��û ��...\n");
		}
		else if (!strcmp(input, "!Q") || !strcmp(input, "!q")) {
			//// ����
			// request_msg = "QUIT" ������ ����
			sprintf(request_msg, "QUIT");
			send(sock, request_msg, strlen(request_msg), 0);
			printf("�������� ������ ����Ǿ����ϴ�.\n");
			exit(0);
		}
		else {
			printf("�߸��� �޴��Դϴ�. �ٽ� �Է��ϼ���.\n");
		}
	}

	return 0;
}

/**
 * �����κ��� ������ �޽��� �ؼ� �� ó�� ������
 **/
unsigned WINAPI RecvMsg(void* arg) {
	SOCKET sock = *((SOCKET*)arg);

	char input[MAXLEN];
	char msg[MAXLEN];
	char res[MAXLEN];
	char response_msg[MAXLEN];
	int msglen;
	while (1) {
		msglen = recv(sock, msg, sizeof(msg), 0);
		memset(res, 0, sizeof(res));
		strncpy(res, msg, msglen);

		printf("RECEIVED: %s\n", res);
		
		if (!strncmp(res, "REQ", 3)) {
			// Ŭ���̾�Ʈ���� �����û�� ���� ���
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strncpy(source_nickname, res + 4, msglen - 4);

			printf("[%s]�����κ��� ��ȭ��û�� �����߽��ϴ�. (!Y : ����, !N : ����)\n", source_nickname);
			while (1) {
				memset(input, 0, sizeof(input));
				scanf("%s", input);
				if (!strcmp(input, "!Y") || !strcmp(input, "!y")) {
					memset(response_msg, 0, sizeof(response_msg));
					sprintf(response_msg, "ACCEPT");
					send(sock, response_msg, strlen(response_msg), 0);
				}
				else if (!strcmp(input, "!N") || !strcmp(input, "!n")) {
					memset(response_msg, 0, sizeof(response_msg));
					sprintf(response_msg, "REJECT");
					send(sock, response_msg, strlen(response_msg), 0);
				}
				else {
					printf("�߸��� �޴��Դϴ�. �ٽ� �Է��ϼ���.\n");
				}
			}
			
		}
		else if (!strncmp(res, "LIST", 4)) {
			// �����κ��� ���� �������� ���е� ����Ʈ ���ڿ� ���� �� ���
			printf("------------------------\n[����� ����Ʈ]\n");
			printf("%s", res + 5);
			printf("------------------------\n");
		}
		else if (!strncmp(res, "NOTFOUND", 7)) {
			printf("�ش� ����ڸ� ã�� �� �����ϴ�.\n");
		}
		else if (!strncmp(res, "ACCEPT", 6)) {
			printf("����");
		}
		else if (!strncmp(res, "REJECT", 6)) {

		}
		else if (!strncmp(res, "SEND", 4)) {

		}

		/*
			else if (!strncmp(response_msg, "ACCEPT", strlen)) {
				printf("[%s]���� ��ȭ ��û�� �����߽��ϴ�. (!E : ����)", dest_nickname);
				while (1) {

				}


			}
			else if (!strncmp(response_msg, "REJECT", strlen)) {
				printf("[%s]���� ��ȭ ��û�� �ź��߽��ϴ�.\n", dest_nickname);
			}*/



		else {
			printf("�����κ��� �߸��� ������ �����߽��ϴ�.\n");
		}
	}
	return 0;
}









int main() {
	/////////////////////////////
	WSADATA wsaData; // for WSAStartup
	SOCKET sock;  // ���� ������ ����
	SOCKADDR_IN serv_addr;  // ���� ���� ����
	HANDLE WaitingRoomThread;
	HANDLE SendThread, RecvThread;
	/////////////////////////////
	
	/////////////////////////////

	/**
	 * �г��� �Է�
	 **/
	char nickname[MAXLEN];
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
	// SendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&sock, 0, NULL);
	RecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sock, 0, NULL);

	WaitForSingleObject(WaitingRoomThread, INFINITE);
	// WaitForSingleObject(SendThread, INFINITE);
	WaitForSingleObject(RecvThread, INFINITE);


	

	closesocket(sock);
	WSACleanup();
	


	return 0;
}