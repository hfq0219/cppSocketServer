#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>


using namespace std;

#define IPADDRESS "127.0.0.1"
#define SERV_PORT 6666

int main(int argc, char *argv[]) {
    signal(SIGPIPE,SIG_IGN);
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
    char buff[1024];
    buff[0] = '\0';

    const char *p = "*-begin-*test for socket server";  

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) 
    {
        ssize_t n = write(sockfd, p, strlen(p));
        cout << "strlen(p) = " << strlen(p) << endl;
        sleep(5);
        write(sockfd, "*-end-*", 8);
        n = read(sockfd, buff, 1024);
        cout << "n=" << n << endl;
        printf("%s\n", buff);
        close(sockfd);
    }
    else 
    {
        perror("err1");
    }
    return 0;
}