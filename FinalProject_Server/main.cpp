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
	strcat(res, "LIST ");
	for (int i = 0; i < client_cnt; i++) {
		strcat(res, clients[i].nickname);
		strcat(res, "\n");
	}
	ReleaseMutex(mutex);

	return res;
}

/********
 * 닉네임으로 클라이언트를 찾아, 소켓을 리턴해주는 함수
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
 * 클라이언트 핸들링 스레드 함수
 ********/
unsigned WINAPI HandleClient(void* arg) {
	CLNT client = *((CLNT*)arg);
	int msglen;
	char msg[MAXLEN];  // 클라이언트로부터 최초로 오는 메시지.
	char request_msg[MAXLEN];  // 클라이언트에 요청 보내는 메시지.
	char response_msg[MAXLEN]; // 클라이언트로부터 요청 받는 메시지.

	/**
	 * 클라이언트로부터 오는 메시지 처리
	 * 미리 지정된 프로토콜 (LIST, REQ, SEND, QUIT)
	 **/
	while ((msglen = recv(client.sock, msg, sizeof(msg), 0)) != 0) {
		if (!strncmp(msg, "LIST", 4)) {
			//// 사용자 리스트 요청
			printf("[%s] : 사용자 리스트 요청.\n", client.nickname);

			char* list_str = ConnectedClientList();
			send(client.sock, list_str, strlen(list_str), 0);

			printf("[%s] : 사용자 리스트 전송 완료.\n", client.nickname);
		}
		else if (!strncmp(msg, "REQ", 3)) {
			//// 연결 요청
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			strncpy(dest_nickname, msg + 4, msglen - 4);
			printf("[%s] : 연결 요청 -> %s\n", client.nickname, dest_nickname);

			SOCKET dest_sock = FindClientWithNickname(dest_nickname);
			if (dest_sock != -1) {
				// 해당 닉네임의 소켓을 찾음. 해당 소켓에 연결 여부 질의.
				memset(request_msg, 0, sizeof(request_msg));
				sprintf(request_msg, "REQ %s", client.nickname);
				send(dest_sock, request_msg, strlen(request_msg), 0);
			}
			else {
				// 해당 닉네임의 소켓을 찾을 수 없음.
				printf("[%s] : %s 사용자를 찾을 수 없음.\n", client.nickname, dest_nickname);
				memset(response_msg, 0, sizeof(response_msg));
				sprintf(response_msg, "NOTFOUND");
				send(client.sock, response_msg, strlen(response_msg), 0);
			}

		}
		else if (!strncmp(msg, "ACCEPT", 6)) {
			// 연결 수락
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			strncpy(dest_nickname, msg + 7, msglen - 7);
			printf("[%s] : 연결 수락 -> %s\n", client.nickname, dest_nickname);

			SOCKET dest_sock = FindClientWithNickname(dest_nickname);
			memset(request_msg, 0, sizeof(request_msg));
			sprintf(request_msg, "ACCEPT");
			send(dest_sock, request_msg, strlen(request_msg), 0);
		}
		else if (!strncmp(msg, "REJECT", 6)) {
			// 연결 거부

		}

		else if (!strncmp(msg, "SEND", 4)) {

		}
		else if (!strncmp(msg, "QUIT", 4)) {
			printf("[%s] : 종료 요청.\n", client.nickname);
			break;
		}
		else {
			printf("[%s] : 이해할 수 없는 메시지 (%s)\n", client.nickname, msg);
		}
	}

	/**
	 * 연결 종료 시 처리
	 **/
	//// Mutex로 보호
	WaitForSingleObject(mutex, INFINITE);
	for (int i = 0; i < client_cnt; i++) {
		if (client.sock == clients[i].sock) {
			// 현재 연결 종료한 클라이언트를 clients[]에서 제거, 하나씩 앞으로 당김
			while (i++ < client_cnt - 1) {
				clients[i].sock = clients[i + 1].sock;
				memset(clients[i].nickname, 0, sizeof(clients[i].nickname));
				strcpy(clients[i].nickname, clients[i + 1].nickname);
			}
			break;
		}
	}

	// client_cnt 하나 감소
	client_cnt--;

	//// Mutex 해제
	ReleaseMutex(mutex);
	
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