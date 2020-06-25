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


/********
 * Ŭ���̾�Ʈ ������ ���� ����ü ����
 * {�г���, ����, �������(���:0, ä����:1)}
 ********/
typedef struct CLNT {
	char nickname[MAXLEN];
	SOCKET sock;
	short int status;
} CLNT;


//// ��������
CLNT clients[MAX_CLNT];  // CLNT ����ü �迭
HANDLE mutex;  // MUTEX
int client_cnt;  // ���� ����� Ŭ���̾�Ʈ ����


/********
 * ���� ��¿� �Լ�
 ********/
void print_err(const char s[]) {
	fprintf(stderr, s);
	printf("\n");
}

/********
 * ���� ���� ���� ����� �г��� ����� ���ڿ��� �������ִ� �Լ�
 ********/
char* ConnectedClientList() {
	char res[MAXLEN];
	memset(res, 0, sizeof(res));

	WaitForSingleObject(mutex, INFINITE);  // Ŭ���̾�Ʈ ����� �о���� ���� Mutex ȹ�� �� �۾�
	strcat(res, "LIST ");
	for (int i = 0; i < client_cnt; i++) {
		strcat(res, clients[i].nickname);
		strcat(res, "\n");
	}
	ReleaseMutex(mutex);  // Mutex ��ȯ

	return res;
}

/********
 * �г������� Ŭ���̾�Ʈ�� ã��, ������ �������ִ� �Լ�
 * �ش� �г����� ���� ��� -1 ����
 ********/
SOCKET FindClientWithNickname(char nickname[]) {
	WaitForSingleObject(mutex, INFINITE);
	for (int i = 0; i < client_cnt; i++) {
		if (!strcmp(clients[i].nickname, nickname)) {
			ReleaseMutex(mutex);
			return clients[i].sock;
		}
	}
	ReleaseMutex(mutex);

	return -1;
}

/********
 * Ŭ���̾�Ʈ �ڵ鸵 ������ �Լ�
 ********/
unsigned WINAPI HandleClient(void* arg) {
	CLNT client = *((CLNT*)arg);
	char msg[MAXLEN];  // Ŭ���̾�Ʈ�κ��� ���ʷ� �޴� �޽���.
	int msglen;
	char request_msg[MAXLEN];  // Ŭ���̾�Ʈ�� ��û ������ �޽���.
	char response_msg[MAXLEN]; // Ŭ���̾�Ʈ�κ��� ��û���� �޽���. (���� �������� �κ��� ������ ���ڿ�)

	/**
	 * Ŭ���̾�Ʈ�κ��� ���� �޽��� ó��
	 * �̸� ����� �������ݴ�� �޽����� ������ ��
	 **/
	while ((msglen = recv(client.sock, msg, sizeof(msg), 0)) != 0) {
		if (!strncmp(msg, "LIST", 4)) {
			//// ����� ����Ʈ ��û
			printf("[%s] : ����� ����Ʈ ��û.\n", client.nickname);
			char* list_str = ConnectedClientList();
			send(client.sock, list_str, strlen(list_str), 0);
			printf("[%s] : ����� ����Ʈ ���� �Ϸ�.\n", client.nickname);
		}
		else if (!strncmp(msg, "REQ", 3)) {
			//// ���� ��û
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			strncpy(dest_nickname, msg + 4, msglen - 4);
			printf("[%s] : ���� ��û -> %s\n", client.nickname, dest_nickname);

			// �ش� �г����� ������ �г������� �˻�
			SOCKET dest_sock = FindClientWithNickname(dest_nickname);
			if (dest_sock != -1) {
				// �ش� �г����� ������ ã��. �ش� ���Ͽ� ���� ���� ����.
				printf("[%s] : %s ����ڸ� ã��. ���� ���� ��� ��...\n", client.nickname, dest_nickname);
				memset(request_msg, 0, sizeof(request_msg));
				sprintf(request_msg, "REQ %s", client.nickname);
				send(dest_sock, request_msg, strlen(request_msg), 0);
			}
			else {
				// �ش� �г����� ������ ã�� �� ����.
				printf("[%s] : %s ����ڸ� ã�� �� ����.\n", client.nickname, dest_nickname);
				memset(response_msg, 0, sizeof(response_msg));
				sprintf(response_msg, "NOTFOUND");
				send(client.sock, response_msg, strlen(response_msg), 0);
			}
		}
		else if (!strncmp(msg, "ACCEPT", 6)) {
			//// ���� ������ ���� ������ ���
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			strncpy(dest_nickname, msg + 7, msglen - 7);
			printf("[%s] : ���� ���� -> %s\n", client.nickname, dest_nickname);

			// ������ ��û�� Ŭ���̾�Ʈ ���� ���� ������ �˸�.
			// �� Ŭ���̾�Ʈ�� �� Ŭ���̾�Ʈ ���� ���� ä�� ���°� ��.
			SOCKET dest_sock = FindClientWithNickname(dest_nickname);
			memset(request_msg, 0, sizeof(request_msg));
			sprintf(request_msg, "ACCEPT %s", client.nickname);
			send(dest_sock, request_msg, strlen(request_msg), 0);
		}
		else if (!strncmp(msg, "REJECT", 6)) {
			//// ���� ������ ���� �ź��� ���
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			strncpy(dest_nickname, msg + 7, msglen - 7);
			printf("[%s] : ���� �ź� -> %s\n", client.nickname, dest_nickname);

			// ������ ��û�� Ŭ���̾�Ʈ ���� ���� �źθ� �˸�.
			// �� Ŭ���̾�Ʈ�� �� Ŭ���̾�Ʈ ���� ���� ���Ƿ� ���ư��� ��.
			SOCKET dest_sock = FindClientWithNickname(dest_nickname);
			memset(request_msg, 0, sizeof(request_msg));
			sprintf(request_msg, "REJECT %s", client.nickname);
			send(dest_sock, request_msg, strlen(request_msg), 0);
		}
		else if (!strncmp(msg, "SEND", 4)) {
			//// ������ ������ ���¿���, �޽����� �ְ�޴� ���
			char trailer[MAXLEN];
			memset(trailer, 0, sizeof(trailer));
			strncpy(trailer, msg + 5, msglen - 5);
			
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			short int nickname_set = 0;

			char* ptr = strtok(trailer, " ");
			strcpy(dest_nickname, ptr);

			printf("[%s] : �޽��� -> %s\n", client.nickname, dest_nickname);

			// ���� ���� �޽����� ����.
			SOCKET dest_sock = FindClientWithNickname(dest_nickname);
			msg[msglen] = '\0';
			memset(request_msg, 0, sizeof(request_msg));
			sprintf(request_msg, msg);
			send(dest_sock, request_msg, strlen(request_msg), 0);
		}
		else if (!strncmp(msg, "DIS", 3)) {
			//// ������ ������ ���¿���, �� ���� ���� ����(�� ������)�� �Է��� ���
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			strncpy(dest_nickname, msg + 4, msglen - 4);
			printf("[%s] : ���� ���� -> %s\n", client.nickname, dest_nickname);

			// ���� ���� ���� ���Ḧ �˸���, �� Ŭ���̾�Ʈ�� ���Ƿ� ���ư��� ��.
			SOCKET dest_sock = FindClientWithNickname(dest_nickname);
			memset(request_msg, 0, sizeof(request_msg));
			sprintf(request_msg, "DIS %s", client.nickname);
			send(dest_sock, request_msg, strlen(request_msg), 0);
		}
		else if (!strncmp(msg, "QUIT", 4)) {
			//// Ŭ���̾�Ʈ�� ���� ��û
			// �������� Ż����, �Ʒ��� "���� ���� �� ó��"�� ������ ��.
			printf("[%s] : ���� ��û.\n", client.nickname);
			break;
		}
		else {
			//// �������ݿ� ��߳��� �޽��� (�̷� ���� �Ͼ�� ����)
			printf("[%s] : ������ �� ���� �޽��� (%s)\n", client.nickname, msg);
		}
	}

	/**
	 * ���� ���� �� ó��
	 **/
	// Ŭ���̾�Ʈ ��� �۾� ���� Mutex ȹ�� �� �۾�
	WaitForSingleObject(mutex, INFINITE);
	for (int i = 0; i < client_cnt; i++) {
		if (client.sock == clients[i].sock) {
			// ���� ���� ������ Ŭ���̾�Ʈ�� clients[]���� ����, �ϳ��� ������ ���
			while (i++ < client_cnt - 1) {
				clients[i].sock = clients[i + 1].sock;
				memset(clients[i].nickname, 0, sizeof(clients[i].nickname));
				strcpy(clients[i].nickname, clients[i + 1].nickname);
			}
			break;
		}
	}

	// client_cnt �ϳ� ����
	client_cnt--;

	// Mutex ����
	ReleaseMutex(mutex);
	
	closesocket(client.sock);
	return 0;
}

int main() {
	WSADATA wsaData; // for WSAStartup
	SOCKET serv_sock;  // ���� ����� ����
	SOCKADDR_IN serv_addr, clnt_addr;  // Ŭ���̾�Ʈ, ���� �ּ�����
	HANDLE hThread;  // Ŭ���̾�Ʈ ������

	char nickname_tmp[MAXLEN];

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

		// ������ ������ (������ tmp_sock���� ����) Mutex ȹ�� �� �۾�
		WaitForSingleObject(mutex, INFINITE);
		
		// �г��� �Է¹ޱ�
		memset(nickname_tmp, 0, sizeof(nickname_tmp));
		int strlen = recv(tmp_sock, nickname_tmp, sizeof(nickname_tmp), 0);

		// �޾ƿ� �г���, ������ clients(CLNT ����ü �迭)�� ���� �� client_cnt ����
		strcpy(clients[client_cnt].nickname, nickname_tmp);
		clients[client_cnt].sock = tmp_sock;
		client_cnt++;
		
		// Mutex ��ȯ
		ReleaseMutex(mutex);
		
		// �α� ��, Ŭ���̾�Ʈ �����ϴ� ������ �ϳ� ����
		printf("Ŭ���̾�Ʈ �����: {%s, %s} \n", clients[client_cnt - 1].nickname, inet_ntoa(clnt_addr.sin_addr));
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clients[client_cnt - 1], 0, NULL);
	}


	/**
	 * ���� ���α׷� ����
	 **/
	closesocket(serv_sock);
	WSACleanup();

	return 0;
}