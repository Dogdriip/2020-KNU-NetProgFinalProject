/**
 * FinalProject_Client
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
#define SERV_ADDR "127.0.0.1"
#define SERV_PORT 12345


using namespace std;




/**
 * 대기실 스레드
 **/
unsigned WINAPI WaitingRoom(void* arg) {
	SOCKET sock = *((SOCKET*)arg);  // server socket
	char input[MAXLEN];
	char request_msg[MAXLEN];
	char response_msg[MAXLEN];

	printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
	while (1) {
		memset(input, 0, sizeof(input));
		memset(request_msg, 0, sizeof(request_msg));
		memset(response_msg, 0, sizeof(response_msg));

		scanf("%s", input);
		if (!strcmp(input, "!L") || !strcmp(input, "!l")) {
			//// 사용자 리스트
			// request_msg = "LIST" 서버에 전송
			sprintf(request_msg, "LIST");
			send(sock, request_msg, strlen(request_msg), 0);
		}
		else if (!strcmp(input, "!R") || !strcmp(input, "!r")) {
			//// 채팅 요청
			// 대상 닉네임 입력
			char dest_nickname[MAXLEN];
			memset(dest_nickname, 0, sizeof(dest_nickname));
			printf("대상 닉네임을 입력하세요.\n");
			scanf("%s", dest_nickname);
			
			// request_msg = "REQ 사용자명" 서버에 전송
			sprintf(request_msg, "REQ %s", dest_nickname);
			send(sock, request_msg, strlen(request_msg), 0);
			
			// 전송 후 "요청 중..." 메시지 표시, 서버로부터 메시지 대기
			printf("요청 중...\n");
		}
		else if (!strcmp(input, "!Q") || !strcmp(input, "!q")) {
			//// 종료
			// request_msg = "QUIT" 서버에 전송
			sprintf(request_msg, "QUIT");
			send(sock, request_msg, strlen(request_msg), 0);
			printf("서버와의 연결이 종료되었습니다.\n");
			exit(0);
		}
		else {
			printf("잘못된 메뉴입니다. 다시 입력하세요.\n");
		}
	}

	return 0;
}

/**
 * 서버로부터 들어오는 메시지 해석 및 처리 스레드
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
			// 클라이언트에게 연결요청이 들어온 경우
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strncpy(source_nickname, res + 4, msglen - 4);

			printf("[%s]님으로부터 대화요청이 도착했습니다. (!Y : 수락, !N : 거절)\n", source_nickname);
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
					printf("잘못된 메뉴입니다. 다시 입력하세요.\n");
				}
			}
			
		}
		else if (!strncmp(res, "LIST", 4)) {
			// 서버로부터 오는 공백으로 구분된 리스트 문자열 수신 후 출력
			printf("------------------------\n[사용자 리스트]\n");
			printf("%s", res + 5);
			printf("------------------------\n");
		}
		else if (!strncmp(res, "NOTFOUND", 7)) {
			printf("해당 사용자를 찾을 수 없습니다.\n");
		}
		else if (!strncmp(res, "ACCEPT", 6)) {
			printf("성공");
		}
		else if (!strncmp(res, "REJECT", 6)) {

		}
		else if (!strncmp(res, "SEND", 4)) {

		}

		/*
			else if (!strncmp(response_msg, "ACCEPT", strlen)) {
				printf("[%s]님이 대화 요청을 수락했습니다. (!E : 종료)", dest_nickname);
				while (1) {

				}


			}
			else if (!strncmp(response_msg, "REJECT", strlen)) {
				printf("[%s]님이 대화 요청을 거부했습니다.\n", dest_nickname);
			}*/



		else {
			printf("서버로부터 잘못된 응답을 수신했습니다.\n");
		}
	}
	return 0;
}









int main() {
	/////////////////////////////
	WSADATA wsaData; // for WSAStartup
	SOCKET sock;  // 연결 생성용 소켓
	SOCKADDR_IN serv_addr;  // 서버 연결 정보
	HANDLE WaitingRoomThread;
	HANDLE SendThread, RecvThread;
	/////////////////////////////
	
	/////////////////////////////

	/**
	 * 닉네임 입력
	 **/
	char nickname[MAXLEN];
	memset(nickname, 0, sizeof(nickname));
	printf("닉네임을 입력해 주세요.\n>> ");
	scanf("%s", nickname);


	/**
	 * WSAStartup() - Winsock 초기화 
	 * socket() - 연결용 소켓 생성
	 **/
	printf("초기화 중입니다...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup() error!");
	}
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket() error!");
	}

	/**
	 * 서버 연결 정보 구성
	 **/
	printf("서버와 연결 중입니다...\n");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_ADDR);
	serv_addr.sin_port = htons(SERV_PORT);

	if (connect(sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == -1) {
		fprintf(stderr, "connect() error!");
	} else {
		printf("서버와 연결되었습니다.\n");
	}

	/**
	 * 연결 후 대기실에 들어가기 전, 최초 1회 닉네임 전송
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