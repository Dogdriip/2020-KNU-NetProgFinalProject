/**
 * FinalProject_Server
 * 2018112749 전현승
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

CLNT clients[MAX_CLNT];  // 클라이언트 {닉네임, 소켓} 구조체 배열
HANDLE mutex;  // MUTEX
int client_cnt;  // 현재 연결된 클라이언트 개수




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
			printf("[%s] : 사용자 리스트 요청.\n", client.nickname, msg);

			char* list_str = ConnectedClientList();
			send(client.sock, list_str, strlen(list_str), 0);

			printf("[%s] : 사용자 리스트 전송 완료. \"%s\"\n", client.nickname, list_str);
		}
		else if (!strncmp(msg, )) {

		}

		else if (!strncmp(msg, "QUIT", msglen)) {

		}
		else {
			printf("[%s] : 이해할 수 없는 메시지 (%s)\n", client.nickname, msg);
		}
	}

	// 연결 종료와 관련


	closesocket(client.sock);
	return 0;
}

int main() {
	/////////////////////////////
	WSADATA wsaData; // for WSAStartup
	SOCKET serv_sock;  // 서버 연결용 소켓
	
	SOCKADDR_IN serv_addr, clnt_addr;  // 클, 서 주소정보
	
	HANDLE hThread;

	

	/////////////////////////////
	char nickname_tmp[MAXLEN];


	/////////////////////////////




	/**
	 * Winsock 초기화, Mutex 생성, 연결용 소켓 생성
	 **/
	printf("초기화 중입니다... ");
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		print_err("WSAStartup() error!");
	}
	mutex = CreateMutex(NULL, FALSE, NULL);
	if ((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		print_err("socket() error!");
	}
	printf("완료.\n");

	/**
	 * 서버 구성
	 **/
	printf("서버 설정 중입니다... ");
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
	printf("완료.\n");


	/**
	 * 클라이언트 연결 대기 루프
	 **/
	printf("클라이언트의 연결을 대기합니다.\n");
	while (1) {
		// 연결 대기
		int clnt_addr_sz = sizeof(clnt_addr);
		SOCKET tmp_sock = accept(serv_sock, (SOCKADDR*)&clnt_addr, &clnt_addr_sz);

		//// 연결이 들어오면 (연결은 tmp_sock으로 수신) Mutex 획득 후 작업
		WaitForSingleObject(mutex, INFINITE);
		
		// 닉네임 받아오기
		memset(nickname_tmp, 0, sizeof(nickname_tmp));
		int strlen = recv(tmp_sock, nickname_tmp, sizeof(nickname_tmp), 0);

		// 받아온 닉네임, 소켓 저장 후 client_cnt 증가
		strcpy(clients[client_cnt].nickname, nickname_tmp);
		clients[client_cnt].sock = tmp_sock;
		client_cnt++;
		
		//// Release Mutex
		ReleaseMutex(mutex);
		
		// 로그 후 클라이언트 관리하는 스레드 하나 생성
		printf("클라이언트 연결됨: {%s, %s} \n", clients[client_cnt - 1].nickname, inet_ntoa(clnt_addr.sin_addr));
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clients[client_cnt - 1], 0, NULL);
	}









	closesocket(serv_sock);
	WSACleanup();

	return 0;
}