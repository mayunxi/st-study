//
//  main1.cpp
//  server_select
//
//  Created by yunc on 2019/3/22.
//  Copyright © 2019年 yunc. All rights reserved.
//

//#include <cstdio>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_LEN 1024
#define SERV_PORT 6000
#define FD_SIZE 100
#define MAX_BACK 100

int main(int argc, char ** argv)
{
    int listenfd;//监听socket
    int client[FD_SIZE];//保存select监听的套接字
    fd_set allset;//select监听的集合
    struct sockaddr_in servaddr, chiaddr;//服务器地址和客户端地址
    //socket创建监听socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Create socket Error : %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    //初始化服务器地址信息
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    
    //bind绑定监听socket与服务器addr
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("Bind Error : %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    //监听
    if (listen(listenfd, MAX_BACK) == -1)
    {
        printf("Listen Error : %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    //初始化套接字数组
    for (int i = 0; i < FD_SIZE; i++)    //首先置为全-1
    {
        client[i] = -1;
    }
    
    while (1)
    {
        int nready = 0;
        int nMaxFd = listenfd;
        //select每次返回都会将集合中无信号的套接字清空，所以要重新加入
        //初始化select监听集合
        FD_ZERO(&allset);
        //服务器套接字加入select监听集合
        FD_SET(listenfd, &allset);
        //循环加入客户端套接字
        printf("重新加入allset\n");
        for (int i = 0; i < FD_SIZE; i++)
        {
            if (-1 != client[i])
            {
                if (nMaxFd < client[i])
                {
                    nMaxFd = client[i];
                }
                FD_SET(client[i], &allset);
            }
        }
        //开始select,select第一个参数要求为所有监听集合中最大fd+1
        if ((nready = select(nMaxFd + 1, &allset, NULL, NULL, NULL)) == -1)
        {
            printf("Select Erorr : %d\n", errno);
            exit(EXIT_FAILURE);
        }
        
        if (nready <= 0)            //如果所有套接字都没有通知，继续select
        {
            continue;
        }
        
        //如果是服务端套接字有通知，则有客户端建立连接
        if (FD_ISSET(listenfd, &allset))
        {
            int connfd;//保存连接上来的客户端
            socklen_t clilen = sizeof(chiaddr);
            
            printf("Start doing... \n");
            
            if ((connfd = accept(listenfd, (struct sockaddr *)&chiaddr, &clilen)) == -1)
            {                                        //!> accept 返回的还是套接字
                printf("Accept Error : %d\n", errno);
                continue;
            }
            //保存客户端套接字
            bool bSave = false;//是否保存
            for (int i = 0; i < FD_SIZE; i++)
            {
                if (-1 == client[i])
                {
                    client[i] = connfd;
                    bSave = true;
                    break;
                }
            }
            
            if (!bSave)//客户端太多了
            {
                printf("To many ... ");
                close(connfd);
                continue;
            }
            printf("connet is builded,save client fd=%d\n", connfd);
        }
        
        //循环检查哪个客户端套接字有通知
        for (int i = 0; i < FD_SIZE; i++)
        {
            int sockClient = client[i];
            if (0 > sockClient)
            {
                continue;
            }
            char buf[BUF_LEN] = { 0 };
            
            if (FD_ISSET(sockClient, &allset))
            {
                int nRead = 0;
                nRead = read(sockClient, buf, BUF_LEN);
                if (nRead < 0)
                {
                    printf("Error!\n");
                    close(sockClient);            //说明在这个请求端口上出错了！
                    client[i] = -1;
                    continue;
                }
                if (nRead == 0)
                {
                    printf("Error!\n");
                    close(sockClient);            //客户端退出
                    client[i] = -1;
                    continue;
                }
                
                printf("Server Recv: %s\n", buf);
                
                if (strcmp(buf, "q") == 0)                //!> 客户端输入“q”退出标志
                {
                    printf("Error!\n");
                    close(sockClient);            //客户端退出
                    client[i] = -1;
                    continue;
                }
                
                printf("Server send : %s\n", buf);
                write(sockClient, buf, nRead);        //!> 读出来的写进去
            }
        }
    }
    
    return 0;
}

