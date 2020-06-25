/**
 * FinalProject_Server
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
#define MAX_CLNT 101
#define SERV_PORT 12345


using namespace std;

typedef struct CLNT {
	char nickname[MAXLEN];
	SOCKET sock;
} CLNT;

CLNT clients[MAX_CLNT];  // Ŭ���̾�Ʈ {�г���, ����} ����ü �迭
HANDLE mutex;  // MUTEX
int client_cnt;  // ���� ����� Ŭ���̾�Ʈ ����




void print_err(const char s[]) {
	fprintf(stderr, s);
	printf("\n");
}




char* ConnectedClientList() {
	char res[MAXLEN];
	memset(res, 0, sizeof(res));

	WaitForSingleObject(mutex, INFINITE);
	for (int i = 0; i < client_cnt; i++) {
		strcat(res, clients[i].nickname);
		if (i < client_cnt - 1) {
			strcat(res, " ");
		}
	}
	ReleaseMutex(mutex);

	return res;
}

unsigned WINAPI HandleClient(void* arg) {
	CLNT client = *((CLNT*)arg);
	int msglen;
	char msg[MAXLEN];

	while ((msglen = recv(client.sock, msg, sizeof(msg), 0)) != 0) {
		// SendMsg(msg, strLen);
		/*
		for (int i = 0; i < msglen; i++) {
			printf("%c", msg[i]);
		}
		*/

		if (!strncmp(msg, "LIST", msglen)) {
			printf("[%s] : ����� ����Ʈ ��û.\n", client.nickname, msg);

			char* list_str = ConnectedClientList();
			send(client.sock, list_str, strlen(list_str), 0);

			printf("[%s] : ����� ����Ʈ ���� �Ϸ�. \"%s\"\n", client.nickname, list_str);
		}
		else if (!strncmp(msg, )) {

		}

		else if (!strncmp(msg, "QUIT", msglen)) {

		}
		else {
			printf("[%s] : ������ �� ���� �޽��� (%s)\n", client.nickname, msg);
		}
	}

	// ���� ����� ����


	closesocket(client.sock);
	return 0;
}

int main() {
	/////////////////////////////
	WSADATA wsaData; // for WSAStartup
	SOCKET serv_sock;  // ���� ����� ����
	
	SOCKADDR_IN serv_addr, clnt_addr;  // Ŭ, �� �ּ�����
	
	HANDLE hThread;

	

	/////////////////////////////
	char nickname_tmp[MAXLEN];


	/////////////////////////////




	/**
	 * Winsock �ʱ�ȭ, Mutex ����, ����� ���� ����
	 **/
	printf("�ʱ�ȭ ���Դϴ�... ");
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		print_err("WSAStartup() error!");
	}
	mutex = CreateMutex(NULL, FALSE, NULL);
	if ((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		print_err("socket() error!");
	}
	printf("�Ϸ�.\n");

	/**
	 * ���� ����
	 **/
	printf("���� ���� ���Դϴ�... ");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);
	
	if (bind(serv_sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
		print_err("bind() error!");
	}
	if (listen(serv_sock, MAX_CLNT) == SOCKET_ERROR) {
		print_err("listen() error!");
	}
	printf("�Ϸ�.\n");


	/**
	 * Ŭ���̾�Ʈ ���� ��� ����
	 **/
	printf("Ŭ���̾�Ʈ�� ������ ����մϴ�.\n");
	while (1) {
		// ���� ���
		int clnt_addr_sz = sizeof(clnt_addr);
		SOCKET tmp_sock = accept(serv_sock, (SOCKADDR*)&clnt_addr, &clnt_addr_sz);

		//// ������ ������ (������ tmp_sock���� ����) Mutex ȹ�� �� �۾�
		WaitForSingleObject(mutex, INFINITE);
		
		// �г��� �޾ƿ���
		memset(nickname_tmp, 0, sizeof(nickname_tmp));
		int strlen = recv(tmp_sock, nickname_tmp, sizeof(nickname_tmp), 0);

		// �޾ƿ� �г���, ���� ���� �� client_cnt ����
		strcpy(clients[client_cnt].nickname, nickname_tmp);
		clients[client_cnt].sock = tmp_sock;
		client_cnt++;
		
		//// Release Mutex
		ReleaseMutex(mutex);
		
		// �α� �� Ŭ���̾�Ʈ �����ϴ� ������ �ϳ� ����
		printf("Ŭ���̾�Ʈ �����: {%s, %s} \n", clients[client_cnt - 1].nickname, inet_ntoa(clnt_addr.sin_addr));
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clients[client_cnt - 1], 0, NULL);
	}









	closesocket(serv_sock);
	WSACleanup();

	return 0;
}