#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")

struct folder { char name[32], path[32]; };
struct file { char name[32], path[32], sum; long long len, cur; };

struct state { 
	struct file files[123];
	struct folder folders[123];
	char data[123456];
	long long data_cur;
	int files_cur, folders_cur;
};
struct state s;

FILE* f;
WIN32_FIND_DATAA fd;
HANDLE hFind;
ULARGE_INTEGER fileSize;
long long totalBytes;

#define PORT            80
#define ROOT_DIR        "www"    
#define BUFFER_SIZE     4096
#define MAX_PATH_LEN    1024

char* get_mime_type(const char* filename) {
	const char* ext = strrchr(filename, '.');
	if (!ext) return "application/octet-stream";
	if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
	if (strcmp(ext, ".css") == 0)  return "text/css";
	if (strcmp(ext, ".js") == 0)   return "application/javascript";
	if (strcmp(ext, ".json") == 0) return "application/json";
	if (strcmp(ext, ".png") == 0)  return "image/png";
	if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
	if (strcmp(ext, ".gif") == 0)  return "image/gif";
	if (strcmp(ext, ".svg") == 0)  return "image/svg+xml";
	if (strcmp(ext, ".ico") == 0)  return "image/x-icon";
	if (strcmp(ext, ".txt") == 0)  return "text/plain";
	if (strcmp(ext, ".pdf") == 0)  return "application/pdf";
	if (strcmp(ext, ".zip") == 0)  return "application/zip";
	return "application/octet-stream";
}

int main() { 
	f = fopen("store/grass.png", "rb");
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);

	fread(&s.data, len, 1, f);
	fclose(f);
	for (int i = 0; i < len; i++) {
		if (!s.data[i]) s.data[i] = 222;
		if (s.data[i]==7) s.data[i] = 223;
	}

	char path[32] = "store";
	char path2[32] = "store";
	sprintf(path2, "%s%s", path, "/*");
	hFind = FindFirstFileA(path2, &fd);
	do
	{
		if (strcmp(fd.cFileName, ".") == 0 ||
			strcmp(fd.cFileName, "..") == 0)
			continue;
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			s.folders[s.folders_cur] = (struct folder) { fd.cFileName, path };
			s.folders_cur++;
			printf("[DIR]  %s P %s\n", fd.cFileName, path);
		}
		else
		{
			fileSize.LowPart = fd.nFileSizeLow;
			fileSize.HighPart = fd.nFileSizeHigh;
			totalBytes = fileSize.QuadPart;
			char full[32];
			sprintf(full, "%s/%s", path, fd.cFileName);

			s.files[s.files_cur] = 
				(struct file){ "", "", 0, totalBytes, s.data_cur};
			strcpy(s.files[s.files_cur].path, full);

			s.files_cur++;
			printf("[FILE] %s %lld p %s\n", fd.cFileName, totalBytes, path);



			f = fopen(full, "rb");
			fseek(f, 0, SEEK_END);
			long len = ftell(f);
			fseek(f, 0, SEEK_SET);

			fread(&s.data[s.data_cur], len, 1, f);
			//s.data[len + 1] = 0;
			fclose(f);

			s.data_cur += len;
		}

	} while (FindNextFileA(hFind, &fd));

	for (int i = 0; i < s.data_cur; i++) {
		if (!s.data[i]) s.data[i] = 222;
		if (s.data[i] == 7) s.data[i] = 223;
		if (s.data[i] == 27) s.data[i] = 221;
	}

	FindClose(hFind);
	printf("SIZE %lld FL%i DIR%i\n", s.data_cur, s.files_cur, s.folders_cur);
//	printf("%s\n", s.data);
	for (int i = 0; i < 6; i++) {
		printf("%.5s\n", s.data+s.files[i].cur);

	}

	WSADATA wsa;
	struct sockaddr_in addr;
	struct sockaddr_in client_addr;
	int client_addr_len;
	SOCKET client;

	WSAStartup(MAKEWORD(2, 2), &wsa);
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);
	bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr));
	listen(listen_sock, SOMAXCONN);
	printf("HTTP server listening on port %d, serving '%s'\n", PORT, ROOT_DIR);
	client_addr_len = sizeof(client_addr);
	int resp = 0;

	while (1) {
		client = accept(listen_sock, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			continue;
		}
		char host[NI_MAXHOST];
		char service[NI_MAXSERV];
		getnameinfo(client_addr, client_addr_len,
			host, sizeof(host),
			service, sizeof(service),
			NI_NUMERICHOST | NI_NUMERICSERV);

		char request[BUFFER_SIZE];
		int recv_len = recv(client, request, sizeof(request) - 1, 0);
		request[recv_len] = '\0';
		printf("asd!!! %i[%s]\n %s\n", resp, host, request);

		char h[512] ="HTTP/1.0 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 3483\r\n"
			"Server: snrv\r\n"

			"\r\n"
			;
		send(client, h, 512, 0);

		send(client, &s.data[s.files[3].cur], 3483, 0);
		closesocket(client);
		resp++;
		//handle_client(client);
	}
	return 0; }