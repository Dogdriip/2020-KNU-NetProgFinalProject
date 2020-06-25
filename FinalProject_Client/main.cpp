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

int state;  // 0: ����, 1: ä�� ��û�� ���԰ų� ä�� ��
char DEST_NICKNAME[MAXLEN];

HANDLE mutex;

/********
 * ?? ������ �Լ�
 ********/
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
		
		
		fgets(input, sizeof(input), stdin);
		input[strlen(input) - 1] = '\0';
		
		if (!state) {
			/**
			 * state == 0 : ���ǿ��� ��� ���� ��Ȳ
			 **/
			
			if (!strcmp(input, "!L") || !strcmp(input, "!l")) {
				//// ����� ����Ʈ
				// request_msg = "LIST" ������ ����
				sprintf(request_msg, "LIST");
				send(sock, request_msg, strlen(request_msg), 0);
			}
			else if (!strcmp(input, "!R") || !strcmp(input, "!r")) {
				//// ä�� ��û
				// ��� �г��� �Է�
				memset(DEST_NICKNAME, 0, sizeof(DEST_NICKNAME));
				printf("��� �г����� �Է��ϼ���.\n");
				scanf("%s", DEST_NICKNAME);

				// request_msg = "REQ ����ڸ�" ������ ����
				sprintf(request_msg, "REQ %s", DEST_NICKNAME);
				send(sock, request_msg, strlen(request_msg), 0);

				// ���� �� "��û ��..." �޽��� ǥ��, �����κ��� �޽��� ���
				printf("��û ��...\n");
				state = 2;
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
		else if (state == 1) {
			/**
			 * state == 1 : Ŭ���̾�Ʈ�� ���� ��û�� ���� ��Ȳ
			 **/
			// scanf("%s", input);
			if (!strcmp(input, "!Y") || !strcmp(input, "!y")) {
				//// ���� ����
				// ������ ���� �г��Ӱ� �Բ� ACCEPT ������, state = 3�� ����
				memset(response_msg, 0, sizeof(response_msg));
				sprintf(response_msg, "ACCEPT %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);
				state = 3;
				printf("[%s]�԰� ��ȭ�� �����մϴ�. (!E : ����)\n", DEST_NICKNAME);
			}
			else if (!strcmp(input, "!N") || !strcmp(input, "!n")) {
				//// ���� �ź�
				// ������ ���� �г��Ӱ� �Բ� REJECT ������, state = 0���� ����
				memset(response_msg, 0, sizeof(response_msg));
				sprintf(response_msg, "REJECT %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);
				printf("[%s]���� ��û�� �ź��߽��ϴ�.\n", DEST_NICKNAME);
				state = 0;
				printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
			}
			else {
				printf("�߸��� �޴��Դϴ�. �ٽ� �Է��ϼ���.\n");
			}
		}
		else if (state == 2) {
			/**
			 * state == 2 : ��û�� ���� �� ��� ���� ��Ȳ
			 **/
			continue;
		}
		else if (state == 3) {
			/**
			 * state == 3 : �ٸ� Ŭ���̾�Ʈ�� ��ȭ ���� ��Ȳ
			 **/
			memset(response_msg, 0, sizeof(response_msg));
			if (!strcmp(input, "!E") || !strcmp(input, "!e")) {
				//// ��ȭ Disconnect ó��
				// ������ DIS �޽��� ���� ��, 
				printf("��ȭ���� �����ϴ�.\n");
				sprintf(response_msg, "DIS %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);

				state = 0;
				printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
			}
			else {
				//// �Ϲ� ���ڿ� ó��
				// �޽����� �����ϰ�, ������ SEND �޽��� ����
				sprintf(response_msg, "SEND %s %s", DEST_NICKNAME, input);
				printf("%s\n", response_msg);
				send(sock, response_msg, strlen(response_msg), 0);
			}
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
	char response_msg[MAXLEN];
	int msglen;
	while (1) {
		msglen = recv(sock, msg, sizeof(msg), 0);
		msg[msglen] = '\0';
		
		if (!strncmp(msg, "REQ", 3)) {
			// Ŭ���̾�Ʈ���� �����û�� ���� ���
			// REQ ���� �г����� �޾Ƽ� �������� DEST_NICKNAME�� ����
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 4);
			
			strcpy(DEST_NICKNAME, source_nickname);
			
			state = 1;
			printf("[%s]�����κ��� ��ȭ��û�� �����߽��ϴ�. (!Y : ���� / !N : ����)\n", DEST_NICKNAME);
		}
		else if (!strncmp(msg, "LIST", 4)) {
			// �����κ��� ���� �������� ���е� ����Ʈ ���ڿ� ���� �� ���
			printf("------------------------\n[����� ����Ʈ]\n");
			printf("%s", msg + 5);
			printf("------------------------\n");
			printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
		}
		else if (!strncmp(msg, "NOTFOUND", 7)) {
			printf("�ش� ����ڸ� ã�� �� �����ϴ�.\n");
			state = 0;
			printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
		}
		else if (!strncmp(msg, "ACCEPT", 6)) {
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 7);
			printf("[%s]���� ��ȭ ��û�� �����߽��ϴ�.\n", source_nickname);
			state = 3;
			printf("[%s]�԰� ��ȭ�� �����մϴ�. (!E : ����)\n", source_nickname);
		}
		else if (!strncmp(msg, "REJECT", 6)) {
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 7);
			printf("[%s]���� ��ȭ ��û�� �ź��߽��ϴ�.\n", source_nickname);
			state = 0;
			printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
		}
		else if (!strncmp(msg, "SEND", 4)) {
			char trailer[MAXLEN];
			memset(trailer, 0, sizeof(trailer));
			strncpy(trailer, msg + 5, msglen - 5);

			printf("[%s] ", DEST_NICKNAME);
			char* ptr = strtok(trailer, " ");
			ptr = strtok(NULL, " ");
			while (ptr != NULL) {
				printf("%s ", ptr);          // �ڸ� ���ڿ� ���
				ptr = strtok(NULL, " ");      // ���� ���ڿ��� �߶� �����͸� ��ȯ
			}
			printf("\n");
		}
		else if (!strncmp(msg, "DIS", 3)) {
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 4);
			printf("[%s]���� ��ȭ���� �������ϴ�.\n", source_nickname);
			state = 0;
			printf("[����] �޴��� �����ϼ���. (!L : ����� ����Ʈ / !R : ä�� ��û / !Q : ����)\n");
		}
		/*
		else {
			printf("�����κ��� �߸��� ������ �����߽��ϴ�.\n");
		}
		*/
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
	printf("�г����� �Է��� �ּ���. (���� �Ұ�)\n");
	fgets(nickname, sizeof(nickname), stdin);
	nickname[strlen(nickname) - 1] = '\0';


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