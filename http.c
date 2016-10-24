#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#define _SIZE_ 1024

void set_nonblock(int fd)
{
	int fl=fcntl(fd,F_GETFL);
	fcntl(fd,F_SETFL,fl|O_NONBLOCK);
}
static void usage(const char *proc)
{
	printf("Usage: %s [ip][port]\n",proc);
}
int get_line(int sock,char buf[],int len)
{
//	printf("ccccccccccccccccccc\n");
	if(!buf||len<0)
	{
		return -1;
	}
	char c='\0';
	int n=0,i=0;
	while((i<len-1)&&c!='\n'){
		n=recv(sock,&c,1,0);
		if(n>0){
			if(c=='\r'){
			   n=recv(sock,&c,1,MSG_PEEK);
			   if(n>0&&c=='\n'){
				recv(sock,&c,1,0);
			   }else{
				   c='\n';
			   }
			}
			buf[i++]=c;
		}else{
			c='\n';
		}
	}
	buf[i]='\0';
//	printf("dddddddddddddddddddddd\n");
	return i;
}//change '\n'
int start_up(const char* _ip,int _port)
{
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		exit(2);
	}
	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	struct sockaddr_in local;
	local.sin_family=AF_INET;//IPV4
	local.sin_port=htons(_port);
	local.sin_addr.s_addr=inet_addr(_ip);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
	{
		perror("bind");
		exit(3);
	}
	if(listen(sock,5)<0)
	{
		perror("listen");
		exit(4);
	}
	return sock;
}
static void clear_header(int sock)
{
	int ret=-1;
	char buf[_SIZE_];
	do{
		ret=get_line(sock,buf,sizeof(buf));
	}while(ret>0&&(strcmp(buf,"\n")!=0));//unitil  \n
}
static void echo_errno(int sock)
{};
void exec_cgi(int sock,const char *path,const char *method,\
		const char*query_string)//tao jiezi  ,path ,fangfa ,?->
{
	printf("begin cgi\n");
	char buf[_SIZE_];
	int ret=-1;
	int cgi_input[2]={0,0};
	int cgi_output[2]={0,0};
	int content_length=-1;
	char method_env[_SIZE_];
	char query_string_env[_SIZE_/10];
	char content_length_env[_SIZE_];
	close(cgi_output[0]);
	printf("method is %s\n",method);
	printf("query_string is %s\n",query_string);
	printf("path is %sn",path);

	if(strcasecmp(method,"GET")==0){
		clear_header(sock);
	}else{//post
		printf("if  method is post\n");
		do{
			ret=get_line(sock,buf,sizeof(buf));
			if(ret>0&&(strncasecmp(buf,"Content-Length: ",16)==0)){
				content_length=atoi(&buf[16]);
				printf("content length is :%d \n",content_length);
			}
		}while((ret>0)&&(strcmp(buf,"\n")!=0));
		if(content_length==-1){
			echo_errno(sock);
			return;
		}
	}
	memset(buf,'\0',sizeof(buf));
	strcpy(buf,"HTTP/1.0 200 OK \r\n\r\n");//huixian
	send(sock,buf,strlen(buf),0);
	//create pipe
	printf("begin create pipe\n");
	if(pipe(cgi_input)<0){
		perror("pipe");
		return;
	}	
	if(pipe(cgi_output)<0){
		perror("pipe");
		return;
	}
	pid_t id=fork();
	if(id==0){//chiled
		printf("hello i am child\n");
		close(cgi_output[0]);
		close(cgi_input[1]);
		//dup2(cgi_input[0],0);
		//dup2(cgi_output[1],1);
		//	printf("aaaaaaaaa\n");
		memset(method_env,'\0',sizeof(method_env));
		memset(query_string_env,'\0',sizeof(query_string_env));
		memset(content_length_env,'\0',sizeof(content_length_env));
		sprintf(method_env,"REQUEST_METHOD=%s",method);
		putenv(method_env);
		//	printf("method is %s\n",method_env);
		if(strcasecmp(method,"GET")==0)//method
		{
			sprintf(query_string_env,"QUERY_STRING=%s",query_string);
			putenv(query_string_env);
		}else{//POST
			sprintf(content_length_env,"CONTENT_LENGTH=%d",content_length);
			putenv(content_length_env);
		}
	//	printf("path is %s\n",path);
		//execl(path,path,NULL);
		printf("chiled is done\n");
		exit(1);
	}else if(id>0){//father
		printf("hello i am father\n");
		close(cgi_input[0]);
		close(cgi_output[1]);
		int i=0;
		char c=0;
		if(strcasecmp(method,"POST")==0){
			for(;i<content_length;i++){
				recv(sock,&c,1,0);
				printf("%c ",c);
				write(cgi_input[1],&c,1);
			}
		}
		printf("\n");
		int ret=0;
		printf("sever begin to send\n");
		while((ret=read(cgi_output[0],&c,1))>0){
			send(sock,&c,1,0);
		}
		close(cgi_input[1]);
		close(cgi_output[0]);
		waitpid(id,NULL,0);


	
	}
	}




void echo_www(int sock,const char *path,int size)
{
	int fd=open(path,O_RDONLY);
	if(fd<0){
		return;
	}
	printf("server path : %s\n",path);
	char buf[_SIZE_];
	sprintf(buf,"HTTP/1.0 200 OK \r\n\r\n");
	send(sock,buf,strlen(buf),0);
	if(sendfile(sock,fd,NULL,size))
	{
		//return
	}
	close(fd);
}


void *accept_request(void *arg)
{
	int sock=(int)arg;
	char buf[_SIZE_];
	char method[_SIZE_];
	char url[_SIZE_];
	char path[_SIZE_];
	memset(buf,'\0',sizeof(buf));
	memset(method,'\0',sizeof(method));
	memset(url,'\0',sizeof(url));
	memset(path,'\0',sizeof(path));
	int cgi=0;
	int ret=-1;
	char *query_string=NULL;
//	do{
//		ret=get_line(sock,buf,sizeof(buf));
//		printf("%s",buf);
//		fflush(stdout);
//	}while((ret>0)&&(strcmp(buf,"\n")!=0));
	ret=get_line(sock,buf,sizeof(buf));
	if(ret<0){
		return (void*)1;
	}
	printf("%s\n",buf);
	//GET method
	int i=0;//method index
	int j=0;//buf index
	while((i<sizeof(method)-1) && (j < sizeof(buf) )&&\
			(!isspace(buf[j]))){
		method[i]=buf[j];
		i++,j++;
	}
	method[i]='\0';
//	printf("method is :%s\n",method);
	while(isspace(buf[j])){
		j++;
	}
	if((strcasecmp(method,"GET")!=0)&&strcasecmp(method,"POST")!=0)
	{
		return (void*)2;
	}
	printf("method id %s\n ",method);
	if(strcasecmp(method,"POST")==0){
		cgi=1;
	}
	i=0;//url index
	while((i<sizeof(url)-1)&&(j<sizeof(buf))&&(!isspace(buf[j]))){
		url[i]=buf[j];
		i++,j++;
	}
	printf("url:%s\n",url);
	if(strcasecmp(method,"GET")==0){
		query_string=url;
		while(*query_string!='\0'&&*query_string!='?'){
			query_string++;
		}
		if(*query_string=='?'){
			cgi=1;
			*query_string='\0';
			query_string++;
		}
	}
	sprintf(path,"htdoc%s",url);//htdoc/
	if(path[strlen(path)-1]=='/'){
		strcat(path,"index.html");
	}
	printf("path:%s\n",path);
	//
	struct stat st;
	if(stat(path,&st)<0){
		return (void*)3;
	}else{
		if(S_ISDIR(st.st_mode)){
			strcpy(path,"htdoc/index.html");
		}else if((st.st_mode&S_IXUSR)||st.st_mode&S_IXGRP||st.st_mode&S_IXOTH){
			cgi=1;
			printf("hei i can run\n");
		}else{
		}
		if(cgi){
			exec_cgi(sock,path,method,query_string);
		}else{
			clear_header(sock);
			echo_www(sock,path,st.st_size);	
		}
	}
	close(sock);
	return (void*)0;
}

int main(int argc,char *argv[])
{
	if(argc!=3)
	{
		usage(argv[0]);
		exit(-1);
	}
	int listen_socket=start_up(argv[1],atoi(argv[2]));
	int done=0;
	struct sockaddr_in peer;
	socklen_t len=sizeof(peer);
	//while(!done)
	//{
	//	int new_sock=accept(listen_socket,(struct sockaddr*)&peer,&len);
	//	if(new_sock>0)
	//	{
	//		printf("debug :client socket: %s:%d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
	//		pthread_t tid;
	//		pthread_create(&tid,NULL,accept_request,(void*)new_sock);
	//		pthread_detach(tid);
	//	}
	//}
	int epfd=epoll_create(256);//max =256
	struct epoll_event _ev;
	_ev.events=EPOLLIN;
	_ev.data.fd=listen_socket;
	epoll_ctl(epfd,EPOLL_CTL_ADD,listen_socket,&_ev);
	struct epoll_event revs[128];
	int timeout=-1;//block
	int num=0;
	while(1)
	{
		switch((num=epoll_wait(epfd,revs,128,timeout)))
		{
			case 0:
				printf("timeout\n");
				break;
			case -1:
				perror("epoll_wait");
				break;
			default:
				{
					struct sockaddr_in peer;
					socklen_t len=sizeof(peer);
					int i=0;
					for(;i<num;i++)
					{
						int rsock=revs[i].data.fd;
						if(rsock==listen_socket&&(revs[i].events&EPOLLIN))
						{
							int new_fd=accept(listen_socket,(struct sockaddr*)&peer,&len);
							if(new_fd>0)
							{
								printf("get a new client:%s:%d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
								set_nonblock(new_fd);
								_ev.events=EPOLLIN | EPOLLET;
								_ev.data.fd=new_fd;
								epoll_ctl(epfd,EPOLL_CTL_ADD,new_fd,&_ev);
							}
							else printf("accept failure\n");
						}
						else if(revs[i].events&EPOLLIN){
								accept_request((void*)rsock);
								epoll_ctl(epfd,EPOLL_CTL_DEL,rsock,&_ev);
								printf("client is ok\n");
							}
						else{};
					}
				}break;
		}
	}
	return 0;
}
