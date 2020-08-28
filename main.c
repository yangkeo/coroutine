#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "coroutine.h"

typedef struct 
{
	int _arg1;
	int _arg2;
	int* _ids;
}args_t;

int tcp_init()
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if(lfd == -1)
	{
		perror("socket");
		exit(1);
	}

	int on  = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); // 设置端口可复用
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //0.0.0.0

	int ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if(ret == -1)
	{
		perror("bind");
		exit(1);
	}
	listen(lfd, SOMAXCONN);
	return lfd;
}

//设置非阻塞IO
void set_nonblock(int fd)
{
	int flgs = fcntl(fd, F_GETFL, 0);
	flgs |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flgs);
}

void accept_conn(int lfd, schedule_t *s, int co_ids[], void* (*call_back)(schedule_t *s, void* args))
{
	while(1)
	{
		int cfd = accept(lfd, NULL, NULL);
		if(cfd > 0)
		{
			//有客户端连接
			set_nonblock(cfd);
			//int args[] = {lfd, cfd};
			args_t args;
			args._arg1 = lfd;
			args._arg2 = cfd;
			args._ids = co_ids;
			int id = coroutine_create(s, call_back, (void*)&args);
			int i;
			for(i = 0; i < CORSZ; ++i)
			{
				if(co_ids[i] == -1)
				{
					co_ids[i] = id;
					break;
				}
			}
			int count = 1;
			if(i == CORSZ)
				count = 0;
			if(count == 1)
				coroutine_running(s, id);
		}
		else
		{
			// 没有连接，唤醒其他协程
			int i;
			for(i = 0; i < CORSZ; ++i)
			{
				if(co_ids[i] != -1)
					coroutine_resume(s, co_ids[i]);
			}
		}
	}
}

void *handle(schedule_t *s, void *args)
{
	args_t* arr = (args_t*)args;
	int cfd = arr->_arg2;

	char buf[1024] = {0};
	while(1)
	{
		memset(buf, 0, 1024);
		int ret = recv(cfd, buf, 1024, 0);
		if(ret == -1)
		{
			//让出CPU
			coroutine_yield(s);
		}
		else if(ret == 0)
		{
			//对端退出
			int id = s->current_id;
			int i;
			for(i = 0; i < CORSZ; ++i)
			{
				if(arr->_ids[i] == id)
				{
					arr->_ids[i] = -1;
					break;
				}
			}
			break;
		}
		else
		{
			printf("recv:%s\n", buf);
			if(strncasecmp(buf, "exit", 4) == 0)
			{
				break;
			}
			send(cfd, buf, ret, 0);
		}
	}
}

int main()
{
	int lfd = tcp_init();
	set_nonblock(lfd);

	schedule_t *s = schedule_create();
	int co_ids[CORSZ];
	
	int i;
	for(i = 0; i < CORSZ; ++i)
	{
		co_ids[i] = -1;
	}
	accept_conn(lfd, s, co_ids, handle);
	schedule_destroy(s);
    return 0;
}
