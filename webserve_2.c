#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <errno.h>

#include "wrap.h"

//int tcp4bind(short port,const char *IP);

int send_header(int cfd, char *code, char *msg, char *fileType, int len);
int http_request(int cfd);
int send_file(int cfd, char *fileName);
int main()
{
	
    //改变当前进程工作目录
    char path[255] = {0};
    sprintf(path, "%s/%s", getenv("HOME"), "webpath");
    chdir(path);
    //创建socket--设置端口复用---bind
    int lfd = tcp4bind(9999, NULL);
    /*****
    struct sockaddr_in serv_addr;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);

    bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    ****/    
	
    //设置监听
    Listen(lfd, 128);

    //创建epoll树
    int epfd = epoll_create(1024);
    if(epfd<0)
    {
	perror("epoll_create error");
	close(lfd);
	return -1;
    }

    //将监听文件描述符lfd上树
    struct epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    int nready;
    struct epoll_event events[1024];
    int i;
    int sockfd;
    int cfd;

    while(1)
    {
	//等待事件发生
	nready = epoll_wait(epfd, events, 1024, -1);
	if(nready<0)
	{
	    if(errno==EINTR)
	    {
		continue;
	    }

	    break;
	}

	
	for(i=0; i<nready; i++)
	{
	    sockfd = events[i].data.fd;
	    //有客户端连接到来
	    if(sockfd==lfd)
	    {
		//接受新的客户端连接
		cfd = Accept(lfd, NULL, NULL);
		
		//设置cfd为非阻塞
		int flag = fcntl(cfd, F_GETFL);
		flag = O_NONBLOCK;
		fcntl(cfd, F_SETFL, flag);
		//将新的cfd上树
		ev.data.fd = cfd;
		ev.events = EPOLLIN;
		epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
	    }
	    else
	    {
		//有客户端数据发来    
		http_request(cfd);
	    }



	    //有客户端数据到来

	}
    }
}



int send_header(int cfd, char *code, char *msg, char *fileType, int len)
{
    char buf[1024] = {0};
    sprintf(buf, "HTTP/1.1 %s %s\r\n", code, msg);
    sprintf(buf+strlen(buf), "Content-Type:%s\r\n");
    if(len>0)
    {
	sprintf(buf+strlen(buf), "Content-Length:%d\r\n", len);
    }

    strcat(buf, "\r\n");
    Writen(cfd, buf, strlen(buf));

    return 0;

}

int send_file(int cfd, char *fileName)
{
    //打开文件
    int fd = open(fileName, O_RDONLY);
    if(fd<0)
    {
	perror("open error");
	return -1;
    }

    //循环读文件，然后发送
    char buf[1024];
    while(1)
    {
	int n;
	n = read(fd, buf, sizeof(buf));
	if(n<=0)
	{
	    break;   
	}
	else
	{
	    write(cfd, buf, n);
	}
    }
}


int http_request(int cfd)
{
    //读取请求行数据,分析出要请求的资源文件名
    int n;
    char buf[1024];

    memset(buf, 0x00, sizeof(buf));
    Readline(cfd, buf, sizeof(buf));
    printf("buf==[%s]\n",buf);
    //GET /hanzi.c HTTP/1.1
    char reqType[16] = {0};
    char fileName[255] = {0};
    char protocal[16] = {0};
    sscanf(buf, "%[^ ] %[^ ] %[^\r\n]", reqType, fileName, protocal);
    printf("[%s]\n", reqType);
    printf("[%s]\n", fileName);
    printf("[%s]\n", protocal);
    
    char *pFile = fileName+1;
    //循环读取完剩余的数据
    while((n=Readline(cfd, buf, sizeof(buf)))>0);   

    //判断文件是否存在
    struct stat st;
    if(stat(fileName, &st)<0)
    {
	printf("file not exist\n");
	
	//发送头部信息
	send_header(cfd, "404", "NOT FOUND", get_mine_type(pFile), st.st_size);
	
	//发送文件内容
	send_file(cfd, pFile);
	//组织应答信息：http响应消息+错误内容
    }
    else   //若文件存在
    {
	//判断文件类型
	//普通文件
	if(S_ISREG(st.st_mode))
	{
	    //发送头部信息
	    send_header(cfd, "200", "OK", get_mine_type(pFile), st.st_size);
	    
	    //发送文件内容
	    send_file(cfd, pFile);
	}

	
    }

    





}
/***
int tcp4bind(short port,const char *IP)
{
		
    struct sockaddr_in serv_addr;
    int lfd = Socket(AF_INET,SOCK_STREAM,0);
    bzero(&serv_addr,sizeof(serv_addr));
    if(IP == NULL)
    {	    
	//如果这样使用 0.0.0.0,任意ip将可以连接
	serv_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {	
	if(inet_pton(AF_INET,IP,&serv_addr.sin_addr.s_addr) <= 0)
	{			    
	    perror(IP);//转换失败
	    exit(1);
	}
    }
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_port   = htons(port);
     Bind(lfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
     return lfd;
}
****/
