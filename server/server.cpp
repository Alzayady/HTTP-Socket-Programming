#include <stdio.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <bits/stdc++.h>
#include <fstream>
#include <mutex>
std::mutex mtx[100];

std::mutex count_mtx;
int clints = 0;

#include "../helper.cpp"
using namespace std;
int sock_fd;
void handleGet(int sock, string request)
{
    cout << request.substr(0, 50) << endl;

    string fileName = getFileName(request);
    if (!fileExist(fileName))
    {
        mtx[sock].lock();
        commuincate().sendMessage(NOT_FOUND_RES, sock);
        mtx[sock].unlock();
        return;
    }
    else
    {
        if (isImage(fileName))
        {
            mtx[sock].lock();
            commuincate().sendImage(fileName, sock, OK);
            mtx[sock].unlock();
        }
        else
        {
            string fileData = getDataFromFile(fileName);
            string header = addHeader(fileData.size());
            mtx[sock].lock();
            commuincate().sendMessage(OK + header + fileData, sock);
            mtx[sock].unlock();
        }
    }
}
void handlePost(int sock, string request)
{
    string p;
    int i = 0;
    while (request[i] != '\n')
    {
        p += request[i];
        i++;
    }
    p += request[i];
    cout << p << endl;
    string fileName = getFileName(request);
    if (isImage(fileName))
    {
        string image = getDataFromString(request);
        saveImage(image, fileName);
        mtx[sock].lock();
        commuincate().sendMessage(OK, sock);
        mtx[sock].unlock();
    }
    else
    {
        string dataFromRes = getDataFromString(request);
        saveFile(dataFromRes, fileName);
        mtx[sock].lock();
        commuincate().sendMessage(OK, sock);
        mtx[sock].unlock();
    }
    return;
}
void *handle(void *socket_desc)
{

    count_mtx.lock();
    clints++;
    double time = 1.0 * DEFUALT_TIME / clints;
    if (time < MIN_TIME)
        time = MIN_TIME;
    count_mtx.unlock();
    int sock = *(int *)socket_desc;

    while (1)
    {
        mtx[sock].lock();
        string mes = commuincate().reciveMessage(sock, time, true);
        mtx[sock].unlock();
        if (mes.size() == 0)
        {
            count_mtx.lock();
            clints--;
            count_mtx.unlock();
            return 0;
        }
        bool isPost = isPostRequest(mes);
        if (isPost)
        {
            handlePost(sock, mes);
        }
        else
        {
            handleGet(sock, mes);
        }
    }
    count_mtx.lock();
    clints--;
    count_mtx.unlock();
}

int main(int argc, char const *argv[])
{
    int new_sock, c;
    struct sockaddr_in server, client;
    int *temp;
    int port = stringToInt(argv[1]);
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == -1)
    {
        puts("error");
        return -1;
    }
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    server.sin_family = AF_INET;

    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("error 2 ");
        return -1;
    }
    int l = listen(sock_fd, 10);
    if (l < 0)
    {
        puts("error while listinng ");
        exit(1);
    }

    c = sizeof(struct sockaddr_in);

    while ((new_sock = accept(sock_fd, (struct sockaddr *)&client, (socklen_t *)&c)) > 0)
    {
        temp = &new_sock;
        pthread_t proccess_conn;
        if (pthread_create(&proccess_conn, NULL, handle, (void *)temp) < 0)
        {
            return -1;
        }
        // pthread_join(proccess_conn, NULL);
    }

    return 0;
}
