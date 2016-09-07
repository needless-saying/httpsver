#include<stdio.h>
#include<pthread.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>

#define SIZE 1024

void usage(const char *proc)
{
	printf("Usage:%s [ip] [port]\n",proc);
}

int get_line(int sock, char buf[], int buflen)
{
	if(!buf || buflen < 0){
		return -1;
	}

	int i = 0;
	char c = '\0';
	int ret = 0;
	//\n->\n, \r->\n, \r\n->\n
	while((i < buflen-1) && c != '\n'){
		ret=recv(sock, &c, 1, 0);
		if( ret > 0){
			if( c == '\r'){
				ret=recv(sock, &c, 1, MSG_PEEK);
				if(ret > 0 && c == '\n'){
					recv(sock, &c, 1, 0);
				}else{
					c = '\n';
				}
			}
			buf[i++] = c;
		}else{
			c = '\n';
		}
	}

	buf[i] = '\0';
	return i;
}

void echo_error(int sock)
{
}

static void echo_www(int sock, const char *path,ssize_t size)
{
	int fd = open(path,O_RDOMLY);
	if(fd < 0){
		echo_errno(sock);
		return ;
	}

	printf("get a new client :%d -> %s\n",sock, path);

	char statua_line[SIZE];
	sprintf(status_line, "HTTP/1.0 200 OK\r\n\r\n");
	send(sock, status_line, strlen(status_line), 0);

	if(sendfile(sock, fd, NULL, size) < 0){
		echo_errno(sock);
		return;
	}

	close(fd);
}


void clear_header(int sock)
{
	char buf[SIZE];
	int len = SIZE;
	int ret = -1;
	do{
		ret = get_line(sock, buf, len);
	}while((ret > 0) && strcmp(buf, "\n") != 0);
}

static void exe_cgi(int sock, const char *method,\
		const char *path, const char *query_string)
{
	char buf[SIZE];
	int content_length = -1;
	int ret = -1;
	int cgi_input[2];
	int cgi_output[2];
	char method_env[SIZE];
	char query_string_env[SIZE];
	char content_length_env[SIZE];

	printf("method: %s\n",method);

	if( strcasecmp(method, "GET") == 0){
		clear_header(sock);
	}else{
		do{
			ret = get_line(sock, buf, sizeof(buf));
			if(strncasecmp(buf, "Content-Length: ",16) == 0){
				content_length = atoi(&buf[16]);
				//break;
			}
		}while((ret > 0) &&  (strcmp(buf, '\n') != 0));
		if( content_length == -1){
			echo_errno(sock);
		return;
		}
	}

	sprintf(buf,"Http/1.0 200 OK\r\n\r\n");
	send(sock, buf, strlen(buf),0);

	if(pipe(cgi_input) < 0){
		echo_errno(sock);
		return;
	}

	if(pipe(cgi_output) < 0){
		echo_errno(sock);
		return;
	}

	pid_t id = fork();
	if(id == 0){//child
		close(cgi_input[1]);
		close(cgi_output[0]);

		dup2(cgi_input[0],0);
		dup2(cgi_output[0],0);

		sprintf(method_env, "REQUEST_METHOD=$s",method);
		putenv(method_env);

		if(strcasecmp(method,"GET") == 0){
			sprint(query_string_env,"QUERY_STRING=%s",query_string);
			putenv(query_string_env);

		}else{
			sprint(content_length_env,"CONTENT_LENGTH=%s",content_length);
			putenv(content_length_env);
		}
		execl(path, path, NULL);
		exit(1);
	}else{//father
		close(cgi_input[0]);
		close(cgi_output[1]);

		char c = '\0';
		int i = 0;
		if(strcasecmp(method, "POST") == 0){
			for(; i < content_length; i++){
				recv(sock, &c, 1, 0);
				printf("%c ",c);
				write(cgi_input[1], &c, 1);
			}
		}

		printf("\n");

		int ret = 0;
		while((ret = read(cgi_output[0], &c, 1)) > 0){
			send(sock, &c, 1, 0);
		}

		waitpid(id,NULL,0);
	}
}

static void* accept_request(void *arg)
{
	int sock = (int)arg;
	char buf[SIZE];
	int len = sizeof(buf)/sizeof(buf[0]);

	char method[size/10];
	char url[SIZE];
	char path[SIZE];

	int i,j;
	int cgi = 0;
	char *query_string = NULL;

	int ret= -1;
#ifdef _DEBUG_
	do{
		ret = get_line(sock, buf, len);
		printf("%s",buf);
		fflush(stdout);
	}while((ret > 0) && strcmp(buf,'\n') !=0);
#endif

	//request line -> nethod   url http->version
	ret = get_line(sock, buf, len);
	if(ret <= 0){
		echo_errno(sock);
		return (void*)1;
	}

	i = 0; //method index
	j = 0; //buf index
	while((i<sizeof(method)-1) && (j < sizeof(buf)) &&\
			(!isspace(buf[j]))){
		method[i] = buf[j];
		i++;
		j++;
	}
	method[i] = '\0';

	if(strcasecmp(method,"GET") != 0 &&\
			strcasecmp(method, "POST") != 0){
		echo_errno(sock);
		return (void*)2;
	}

	while(isspace(buf[j])){
		j++;
	}

	i = 0;
	while((i<sizeof(url)-1) && (j<sizeof(buf)) &&\
				(!isspace(buf[j]))){
		url[i] = buf[j];
		i++;
		j++;
	}
	url[i] = '\0';

	if(strcasecmp(method, "POST") == 0){
		cgi = 1;
	}

	if(strcasecmp(method, "GET") == 0){
		query_string = url;
		while(*query_string != '\0' && *query_string != '?'){
			query_string++;
		}
		if(*query_string == '?'){
			cgi = 1;
			*query_string = '\0';
			query_string++;
		}
	}

	sprintf(path, "htdoc%s",url); //htdoc/
	if(path[strlen(path)-1] == '/'){
		strcat(path, "index.html");
	}

	//method, query_string, cgi, path
	struct stat st;//  /aa/bb/cc/dir
	if( stat(path, &st) < 0){
		echo_errno(sock);
		return (void*)3;
	}else{
		if(S_ISDIR(st.st_mode)){
			strcat(path,"index.html");
		}else if((st.st_mode & S_IXUSR) ||\
				 (st.st_mode & S_IXGRP) ||\
				 (st.st_mode & S_IXOTH)){
			cgi = 1;
		}else{
		}

		if(cgi){
			exe_cgi(sock, method, path, query_string);
		}else{
			clear_header(sock);//
			echo_www(sock, path, st.st_size);
		}
	}

	close(sock);
	return (void*)0;
}

static int startup(const char *_ip,int _port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		exit(2);
	}

	int opt = 1;
	setsockopt(sock, SQL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family AF_INET;
	local.sin_port = htons(_port);
	local.sin_addr.s_addr = inet_addr(_ip);

	if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0){
		perror("bind");
		exit(3);
	}

	if(listen(sock,5) < 0){
		perror("listen");
		exit(4);
	}

	return sock;
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		usage(argv[0]);
		exit(1);
	}

	struct sockaddr_in peer;
	socklen_t len = sizeof(peer);

	int listen_sock = startup(argv[1], stoi(argv[2]));

	int done = 0;
	while(!done){
		int new_sock = accept(listen_sock,\
				(struct sockaddr*)&peer,&len);
		if(new_sock > 0){
			pthread_t id;
			pthread_create(&id, NULL, accept_request,\
					(void*)new_sock);
			pthread_detach(id);
			//pthread_join();
		}
	}
	return 0;
}
