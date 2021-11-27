#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <errno.h>
//#include "pub.h""
//#include "wrap.c"

//int tcp4bind(short port,const char *IP);
int http_request(int cfd);

int main()
{
	
    //创建socket--设置端口复用---bind
    //int lfd = tcp4bind(9999, NULL);
    
    struct sockaddr_in serv_addr;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);

    bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
	
    //设置监听
    listen(lfd, 128);

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
		cfd = accept(lfd, NULL, NULL);

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

int http_request(int cfd)
{
    //读取请求行数据,分析出要请求的资源文件名
    int n;
    char buf[1024];

    memset(buf, 0x00, sizeof(buf));
    readline(cfd, buf, sizeof(buf));
    //GET /hanzi.c HTTP/1.1
    char reqType[16] = {0};
    char fileName[255] = {0};
    char protocal[16] = {0};
    sscanf(buf, "%[^ ]%[^ ]%[^\r\n]", reqType, fileName, protocal);
    printf("[%s]\n", reqType);
    printf("[%s]\n", fileName);
    printf("[%s]\n", protocal);

    //循环读取完剩余的数据
    //while((n=Readline(cfd, buf, sizeof(buf)))>0);   

    //判断文件是否才能在
    

    //若文件不存在

    //若文件存在




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
