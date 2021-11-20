#include <stdio.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>
#include "sstream"
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include "../helper.cpp"
using namespace std;
std::mutex mtx;

struct Request
{
    int sock;
    string request;
};
void handlePostRequest(string request, int sock_fd);
void handleGetRequest(string data, int sock_fd);
void *handleRequest(void *r)
{
    mtx.lock();
    Request *d = (Request *)r;
    int sock_fd = d->sock;
    string str = d->request;
    bool isPost = isPostRequest(str);
    if (isPost)
    {
        handlePostRequest(trim(str), sock_fd);
    }
    else
    {
        handleGetRequest(trim(str), sock_fd);
    }
    mtx.unlock();
}
void handleGetRequest(string data, int sock_fd)
{

    commuincate().sendMessage(data, sock_fd);
    string fileName = getFileName(data);
    if (isImage(fileName))
    {
        string res = commuincate().reciveMessage(sock_fd);

        if (!isOk(res))
        {
            cout << "get request , NOT FOUND : " << data.substr(0, 100) << endl;

            return;
        }
        string image = getDataFromString(res);
        saveImage(image, fileName);
    }
    else
    {
        string res = commuincate().reciveMessage(sock_fd);

        if (!isOk(res))
        {
            return;
        }
        string dataFromRes = getDataFromString(res);
        saveFile(dataFromRes, fileName);
    }
    cout << "get request is success : " << data.substr(0, 100) << endl;

    return;
}

void handlePostRequest(string request, int sock_fd)
{
    string fileName = getFileName(request);
    if (!fileExist(fileName))
    {
        cout << "post request ,file not here  : " << request.substr(0, 100) << endl;
        return;
    }
    else
    {
        if (isImage(fileName))
        {

            commuincate().sendImage(fileName, sock_fd, request);
        }
        else
        {
            string fileData = getDataFromFile(fileName);
            string header = addHeader(fileData.size());
            commuincate().sendMessage(request + header + fileData, sock_fd);
        }
        string res = commuincate().reciveMessage(sock_fd);

        if (isOk(res))
        {
            cout << "post request is success : " << request.substr(0, 100) << endl;
        }
        else
        {
            puts("error while sending ");
        }
    }
    return;
}
int defulatPort = 80;
int main(int argc, char const *argv[])
{

    freopen("input.txt", "r", stdin);
    const char *serverIp;
    int port;
    serverIp = argv[1];
    if (argc <= 1)
    {
        port = defulatPort;
    }
    else
    {
        port = stringToInt(argv[2]);
    }
    printf("server is %s and the port number is %d \n", serverIp, port);
    fflush(stdout);
    int sock_fd;
    struct sockaddr_in server;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1)
    {
        puts("error");
        return -1;
    }
    server.sin_addr.s_addr = inet_addr(serverIp);
    server.sin_port = htons(port);
    server.sin_family = AF_INET;

    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("error ");
        return -1;
    }
    string line;
    vector<pthread_t> allThreads;
    while (getline(std::cin, line))
    {
        if (IsEmptyLine(line))
        {
            continue;
        }
        else
        {

            Request *r = new Request();
            r->sock = sock_fd;
            r->request = line;
            pthread_t proccess_conn;
            if (pthread_create(&proccess_conn, NULL, handleRequest, (void *)r) < 0)
            {
                puts("error last");
                return -1;
            }
            allThreads.push_back(proccess_conn);
            // pthread_join(proccess_conn, NULL);
        }
    }
    for (auto it : allThreads)
    {
        pthread_join(it, NULL);
    }
    // while (1)
    // {
    // }
    close(sock_fd);
    return 0;
}
