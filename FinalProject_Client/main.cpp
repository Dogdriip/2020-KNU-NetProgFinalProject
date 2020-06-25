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
	printf("[대기실] 메뉴를 선택하세요. (!L : 사용자 리스트 / !R : 채팅 요청 / !Q : 종료)\n");
	while (1) {
		memset(input, 0, sizeof(input));
		memset(request_msg, 0, sizeof(request_msg));
		memset(response_msg, 0, sizeof(response_msg));

		printf(">> ");
		scanf("%s", input);
		if (!strcmp(input, "!L") || !strcmp(input, "!l")) {
			//// 사용자 리스트
			// request_msg = "LIST" 서버에 전송
			strcpy(request_msg, "LIST");
			send(sock, request_msg, strlen(request_msg), 0);

			// 서버로부터 오는 공백으로 구분된 리스트 문자열 수신 후 출력
			int strlen = recv(sock, response_msg, sizeof(response_msg), 0);
			printf("[사용자 리스트]\n");
			char* ptr = strtok(response_msg, " "); 
			while (ptr != NULL) {
				printf("%s\n", ptr);          // 자른 문자열 출력
				ptr = strtok(NULL, " ");      // 다음 문자열을 잘라서 포인터를 반환
			}


		}
		else if (!strcmp(input, "!R") || !strcmp(input, "!r")) {
			printf("채팅 요청\n");
		}
		else if (!strcmp(input, "!Q") || !strcmp(input, "!q")) {
			printf("종료\n");
		}
		else {
			printf("잘못된 메뉴입니다. 다시 입력하세요.\n");
			continue;
		}
	}



	return 0;
}








int main() {
	/////////////////////////////
	WSADATA wsaData; // for WSAStartup
	SOCKET sock;  // 연결 생성용 소켓
	SOCKADDR_IN serv_addr;  // 
	HANDLE WaitingRoomThread;
	HANDLE hSndThread, hRcvThread;
	/////////////////////////////
	char nickname[MAXLEN];
	/////////////////////////////

	/**
	 * 닉네임 입력
	 **/
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
	WaitForSingleObject(WaitingRoomThread, INFINITE);

	// WaitForSingleObject(hSndThread, INFINITE);
	// WaitForSingleObject(hRcvThread, INFINITE);






	closesocket(sock);
	WSACleanup();
	


	return 0;
}