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


//// 전역변수
/**
 * state == 0 : 대기실에서 대기 중인 상황
 * state == 1 : 클라이언트에 연결 요청이 들어온 상황
 * state == 2 : 요청을 보낸 후 대기 중인 상황
 * state == 3 : 다른 클라이언트와 대화 중인 상황
 **/
int state;  // 현재 클라이언트의 상태.
char DEST_NICKNAME[MAXLEN];  // 현재 채팅 중인 클라이언트의 닉네임.


/********
 * 서버에 메시지를 가공해 보내는 스레드 함수
 ********/
unsigned WINAPI SendFunction(void* arg) {
	SOCKET sock = *((SOCKET*)arg);
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
				//// 사용자 리스트 요청
				// "LIST" 메시지 서버에 전송
				sprintf(request_msg, "LIST");
				send(sock, request_msg, strlen(request_msg), 0);
			}
			else if (!strcmp(input, "!R") || !strcmp(input, "!r")) {
				//// 채팅 요청
				// 대상 닉네임 입력
				memset(DEST_NICKNAME, 0, sizeof(DEST_NICKNAME));
				printf("대상 닉네임을 입력하세요.\n");
				scanf("%s", DEST_NICKNAME);

				// "REQ 사용자명" 메시지 서버에 전송
				sprintf(request_msg, "REQ %s", DEST_NICKNAME);
				send(sock, request_msg, strlen(request_msg), 0);

				// 전송 후 "요청 중..." 메시지 표시. 요청 대기 중 상태로 변경.
				state = 2;
				printf("요청 중...\n");
			}
			else if (!strcmp(input, "!Q") || !strcmp(input, "!q")) {
				//// 클라이언트 종료
				// "QUIT" 메시지 서버에 전송 후 프로그램 종료.
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
			if (!strcmp(input, "!Y") || !strcmp(input, "!y")) {
				//// 연결 수락
				// "ACCEPT 상대 닉네임" 메시지를 서버에 보내고, 채팅 중 상태로 변경.
				memset(response_msg, 0, sizeof(response_msg));
				sprintf(response_msg, "ACCEPT %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);

				state = 3;
				printf("[%s]님과 대화를 시작합니다. (!E : 종료)\n", DEST_NICKNAME);
			}
			else if (!strcmp(input, "!N") || !strcmp(input, "!n")) {
				//// 연결 거부
				// "REJECT 상대 닉네임" 메시지를 서버에 보내고, 대기실 상태로 변경.
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
			// 대기 중 상황에서는 아무런 입력도 받지 않음.
			continue;
		}
		else if (state == 3) {
			/**
			 * state == 3 : 다른 클라이언트와 대화 중인 상황
			 **/
			memset(response_msg, 0, sizeof(response_msg));
			if (!strcmp(input, "!E") || !strcmp(input, "!e")) {
				//// 대화 Disconnect 처리
				// "DIS 상대 닉네임" 메시지 서버에 전송 후, 대기실 상태로 변경.
				printf("대화방을 나갑니다.\n");
				sprintf(response_msg, "DIS %s", DEST_NICKNAME);
				send(sock, response_msg, strlen(response_msg), 0);

				state = 0;
				printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
			}
			else {
				//// 일반 문자열 처리
				// 상대방에게 보내는 메시지로 간주하고, 서버에 "SEND <상대 닉네임> <메시지>" 형태로 전송
				sprintf(response_msg, "SEND %s %s", DEST_NICKNAME, input);
				send(sock, response_msg, strlen(response_msg), 0);
			}
		}
	}

	return 0;
}


/********
 * 서버로부터 들어오는 메시지를 해석 및 처리하는 스레드 함수
 ********/
unsigned WINAPI RecvFunction(void* arg) {
	SOCKET sock = *((SOCKET*)arg);

	char input[MAXLEN];
	char msg[MAXLEN];
	char response_msg[MAXLEN];
	int msglen;

	while (1) {
		msglen = recv(sock, msg, sizeof(msg), 0);
		msg[msglen] = '\0';
		
		if (!strncmp(msg, "REQ", 3)) {
			//// 다른 클라이언트로부터 연결요청이 온 경우
			// 닉네임을 받아서 전역변수 DEST_NICKNAME에 저장
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 4);
			
			strcpy(DEST_NICKNAME, source_nickname);
			
			// 상태 변경으로 수락/거절을 기다리는 상태가 됨.
			state = 1;
			printf("[%s]님으로부터 대화요청이 도착했습니다. (!Y : 수락 / !N : 거절)\n", DEST_NICKNAME);
		}
		else if (!strncmp(msg, "LIST", 4)) {
			//// 요청했던 사용자 리스트가 서버로부터 들어온 경우
			// 리스트 문자열을 그대로 출력 (리스트는 \n으로 구분됨)
			printf("------------------------\n[사용자 리스트]\n");
			printf("%s", msg + 5);
			printf("------------------------\n");
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
		else if (!strncmp(msg, "NOTFOUND", 7)) {
			//// 연결 요청에서, 입력한 사용자를 찾을 수 없는 경우
			// 안내 후, 대기실로 돌아가게 됨
			printf("해당 사용자를 찾을 수 없습니다.\n");
			state = 0;
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
		else if (!strncmp(msg, "ACCEPT", 6)) {
			//// 연결 요청한 상대방이 연결을 수락한 경우
			// 안내 후, 채팅 상태로 진입
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 7);
			printf("[%s]님이 대화 요청을 수락했습니다.\n", source_nickname);
			state = 3;
			printf("[%s]님과 대화를 시작합니다. (!E : 종료)\n", source_nickname);
		}
		else if (!strncmp(msg, "REJECT", 6)) {
			//// 연결 요청한 상대방이 연결을 거부한 경우
			// 안내 후, 대기실로 돌아가게 됨
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 7);
			printf("[%s]님이 대화 요청을 거부했습니다.\n", source_nickname);
			state = 0;
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
		else if (!strncmp(msg, "SEND", 4)) {
			//// 채팅 상태에서, 상대방이 전송한 메시지가 도착한 경우
			// 메시지는 "SEND <상대 닉네임> <메시지 내용>" 형태로 도착함. 
			// SEND 이후의 문자열에서 닉네임을 strtok으로 분리 후, 메시지 내용을 출력.
			char trailer[MAXLEN];
			memset(trailer, 0, sizeof(trailer));
			strncpy(trailer, msg + 5, msglen - 5);

			printf("[%s] ", DEST_NICKNAME);
			char* ptr = strtok(trailer, " ");
			ptr = strtok(NULL, " ");
			while (ptr != NULL) {
				printf("%s ", ptr);  // 분리한 문자열 출력
				ptr = strtok(NULL, " ");  // 다음 문자열을 분리
			}
			printf("\n");
		}
		else if (!strncmp(msg, "DIS", 3)) {
			//// 채팅 상태에서, 상대방이 연결 종료를 입력한 경우
			// 안내 후, 대기실로 돌아가게 됨.
			char source_nickname[MAXLEN];
			memset(source_nickname, 0, sizeof(source_nickname));
			strcpy(source_nickname, msg + 4);
			printf("[%s]님이 대화방을 나갔습니다.\n", source_nickname);
			state = 0;
			printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
		}
	}
	return 0;
}


int main() {
	WSADATA wsaData; // for WSAStartup
	SOCKET sock;  // 연결 생성용 소켓
	SOCKADDR_IN serv_addr;  // 서버 연결 정보
	HANDLE SendThread, RecvThread;  // 메시지 전송, 수신 스레드
	char msg[MAXLEN];
	int msglen;

	/**
	 * 닉네임 입력받기
	 **/
	char nickname[MAXLEN];
	memset(nickname, 0, sizeof(nickname));
	printf("닉네임을 입력해 주세요. (공백 불가)\n");
	fgets(nickname, sizeof(nickname), stdin);
	nickname[strlen(nickname) - 1] = '\0';


	/**
	 * Winsock 초기화, 연결용 소켓 생성
	 **/
	printf("초기화 중입니다...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup() error!");
	}
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket() error!");
	}

	/**
	 * 서버 연결 정보 구성 및 서버와 연결
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
	 * 연결 후 대기실에 들어가기 전, 서버에 최초 1회 닉네임 전송
	 * 서버에서 닉네임 중복 체크 (ERROR를 받으면 프로그램 종료)
	 **/
	send(sock, nickname, strlen(nickname), 0);
	msglen = recv(sock, msg, sizeof(msg), 0);
	msg[msglen] = '\0';
	if (!strcmp(msg, "ERROR")) {
		printf("서버에 이미 중복된 닉네임이 있습니다.\n프로그램을 종료합니다.\n");
		exit(0);
	}
	
	/**
	 * 메시지 전송, 메시지 수신을 담당하는 스레드 2개를 생성
	 * 인자로는 서버 소켓을 전달
	 **/
	SendThread = (HANDLE)_beginthreadex(NULL, 0, SendFunction, (void*)&sock, 0, NULL);
	RecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvFunction, (void*)&sock, 0, NULL);
	WaitForSingleObject(SendThread, INFINITE);
	WaitForSingleObject(RecvThread, INFINITE);


	/**
	 * 클라이언트 프로그램 종료
	 **/
	closesocket(sock);
	WSACleanup();

	return 0;
}