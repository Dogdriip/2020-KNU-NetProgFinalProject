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

int state;  // 0: 대기실, 1: 채팅 요청이 들어왔거나 채팅 중
char DEST_NICKNAME[MAXLEN];

HANDLE mutex;

/********
 * ?? 스레드 함수
 ********/
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
		
		
		fgets(input, sizeof(input), stdin);
		input[strlen(input) - 1] = '\0';
		
		if (!state) {
			/**
			 * state == 0 : 대기실에서 대기 중인 상황
			 **/
			
			if (!strcmp(input, "!L") || !strcmp(input, "!l")) {
				//// 사용자 리스트
				// request_msg = "LIST" 서버에 전송
				sprintf(request_msg, "LIST");
				send(sock, request_msg, strlen(request_msg), 0);
			}
			else if (!strcmp(input, "!R") || !strcmp(input, "!r")) {
				//// 채팅 요청
				// 대상 닉네임 입력
				memset(DEST_NICKNAME, 0, sizeof(DEST_NICKNAME));
				printf("대상 닉네임을 입력하세요.\n");
				scanf("%s", DEST_NICKNAME);

				// request_msg = "REQ 사용자명" 서버에 전송
				sprintf(request_msg, "REQ %s", DEST_NICKNAME);
				send(sock, request_msg, strlen(request_msg), 0);

				// 전송 후 "요청 중..." 메시지 표시, 서버로부터 메시지 대기
				printf("요청 중...\n");
				state = 2;
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
		else if (state == 1) {
			/**
			 * state == 1 : 클라이언트에 연결 요청이 들어온 상황
			 **/
			// scanf("%s", input);
			if (!strcmp(input, "!Y") || !strcmp(input, "!y")) {
				//// 연결 수락
				// 서버에 상대방 닉네임과 함께 ACCEPT 보내고, state = 3로 변경
				memset(response_msg, 0, sizeof(response_msg));
				sprintf(response_msg, "ACCEPT %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);
				state = 3;
				printf("[%s]님과 대화를 시작합니다. (!E : 종료)\n", DEST_NICKNAME);
			}
			else if (!strcmp(input, "!N") || !strcmp(input, "!n")) {
				//// 연결 거부
				// 서버에 상대방 닉네임과 함께 REJECT 보내고, state = 0으로 변경
				memset(response_msg, 0, sizeof(response_msg));
				sprintf(response_msg, "REJECT %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);
				printf("[%s]님의 요청을 거부했습니다.\n", DEST_NICKNAME);
				state = 0;
				printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
			}
			else {
				printf("잘못된 메뉴입니다. 다시 입력하세요.\n");
			}
		}
		else if (state == 2) {
			/**
			 * state == 2 : 요청을 보낸 후 대기 중인 상황
			 **/
			continue;
		}
		else if (state == 3) {
			/**
			 * state == 3 : 다른 클라이언트와 대화 중인 상황
			 **/
			memset(response_msg, 0, sizeof(response_msg));
			if (!strcmp(input, "!E") || !strcmp(input, "!e")) {
				//// 대화 Disconnect 처리
				// 서버에 DIS 메시지 전송 후, 
				printf("대화방을 나갑니다.\n");
				sprintf(response_msg, "DIS %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);

				state = 0;
				printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
			}
			else {
				//// 일반 문자열 처리
				// 메시지로 간주하고, 서버에 SEND 메시지 전송
				sprintf(response_msg, "SEND %s %s", DEST_NICKNAME, input);
				printf("%s\n", response_msg);
				send(sock, response_msg, strlen(response_msg), 0);
			}
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
	char response_msg[MAXLEN];
	int msglen;
	while (1) {
		msglen = recv(sock, msg, sizeof(msg), 0);
		msg[msglen] = '\0';
		
		if (!strncmp(msg, "REQ", 3)) {
			// 클라이언트에게 연결요청이 들어온 경우
			// REQ 뒤의 닉네임을 받아서 전역변수 DEST_NICKNAME에 저장
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 4);
			
			strcpy(DEST_NICKNAME, source_nickname);
			
			state = 1;
			printf("[%s]님으로부터 대화요청이 도착했습니다. (!Y : 수락 / !N : 거절)\n", DEST_NICKNAME);
		}
		else if (!strncmp(msg, "LIST", 4)) {
			// 서버로부터 오는 공백으로 구분된 리스트 문자열 수신 후 출력
			printf("------------------------\n[사용자 리스트]\n");
			printf("%s", msg + 5);
			printf("------------------------\n");
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
		else if (!strncmp(msg, "NOTFOUND", 7)) {
			printf("해당 사용자를 찾을 수 없습니다.\n");
			state = 0;
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
		else if (!strncmp(msg, "ACCEPT", 6)) {
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 7);
			printf("[%s]님이 대화 요청을 수락했습니다.\n", source_nickname);
			state = 3;
			printf("[%s]님과 대화를 시작합니다. (!E : 종료)\n", source_nickname);
		}
		else if (!strncmp(msg, "REJECT", 6)) {
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 7);
			printf("[%s]님이 대화 요청을 거부했습니다.\n", source_nickname);
			state = 0;
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
		else if (!strncmp(msg, "SEND", 4)) {
			char trailer[MAXLEN];
			memset(trailer, 0, sizeof(trailer));
			strncpy(trailer, msg + 5, msglen - 5);

			printf("[%s] ", DEST_NICKNAME);
			char* ptr = strtok(trailer, " ");
			ptr = strtok(NULL, " ");
			while (ptr != NULL) {
				printf("%s ", ptr);          // 자른 문자열 출력
				ptr = strtok(NULL, " ");      // 다음 문자열을 잘라서 포인터를 반환
			}
			printf("\n");
		}
		else if (!strncmp(msg, "DIS", 3)) {
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 4);
			printf("[%s]님이 대화방을 나갔습니다.\n", source_nickname);
			state = 0;
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
		/*
		else {
			printf("서버로부터 잘못된 응답을 수신했습니다.\n");
		}
		*/
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
	printf("닉네임을 입력해 주세요. (공백 불가)\n");
	fgets(nickname, sizeof(nickname), stdin);
	nickname[strlen(nickname) - 1] = '\0';


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