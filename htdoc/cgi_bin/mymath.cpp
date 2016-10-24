#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"bigdata.h"
static void mymath(char *arg)
{
//	printf("bbbbbbbbbbbb\n");
	char *data[3];
	data[2]=NULL;
	int i=1;
//	printf("%s\n",arg);
	char *end=arg+strlen(arg)-1;
	while(end>arg)
	{
		if(*end=='='){
			data[i--]=end+1;
		}
		if(*end=='&'){
			*end='\0';
		}
		end--; 
	}
//printf("data[0] is %s\n",data[0]);
//printf("data[1] is %s\n",data[1]);
BigData data1(data[0]);
BigData data2(data[1]);
printf("<html>\n");
printf("<h1>");
std::cout<<data[0]<<"+"<<data[1]<<"="<<data1+data2<<endl;
printf("</h1>\n");
printf("</html>\n");
}
int main()
{
//	printf("mathaaaaaaaaaaaaaaaaaa\n");
	char method[1024];
	char arg[1024];
	char content_len[1024];
	int len=-1;
	if(getenv("REQUEST_METHOD")){
		strcpy(method,getenv("REQUEST_METHOD"));
	}
//	printf("method is %s\n",method);
	if(strcasecmp(method,"GET")==0)
	{
		if(getenv("QUERY_STRING")){
			strcpy(arg,getenv("QUERY_STRING"));
		}
	}else{
		if(getenv("CONTENT_LENGTH")){
			strcpy(content_len,getenv("CONTENT_LENGTH"));
			len=atoi(content_len);
		}
		int i=0;
		for(;i<len;i++){
			read(0,&arg[i],1);
		}
		arg[i]='\0';
	}
	mymath(arg);
	return 0;
	

}
