#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
 
using namespace std;
 
int main(int ac, char **av) {
    try {
        char			selfhost[256];
	    struct hostent	*host;

        for (int i = 0; i < strlen(av[1]); i++)
        {
            if (!isdigit(av[1][i]))
                throw(std::exception());
        }
        string server = ""; // network address
        int port = atoi(av[1]); // server port
        string nick = "NICK Awesomebot\n"; // NICK raw
        string user = "USER Awesomebot * * Awesomebot\n"; // USER raw
        string pass = "PASS test\n";
        string join = "JOIN #";
        string msg = "PRIVMSG #";
        msg.append(av[2]);
        msg.append(" : I am a bot and I send a message every 30 seconds.\n");
        join.append(av[2]);
        join.append("\n");
        /** Structs that hold the socket information **/

        struct sockaddr_in addr;
        /** Get an ip address from the network to connect to **/
        gethostname( selfhost, sizeof( selfhost ) );
        host = gethostbyname(selfhost);


        /** Fill the members of the socket structs required to connect **/
 
        addr.sin_addr.s_addr = *(unsigned long*)host->h_addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((unsigned short)port);
        int sockd = socket(AF_INET, SOCK_STREAM, 0);

        /** Connect to address **/
        connect(sockd, (struct sockaddr *)&addr, sizeof(addr));
    
        send(sockd, pass.c_str(), pass.size(), 0);
        send(sockd, nick.c_str(), nick.size(), 0); // Converts nick string to c-array and sends it to server
        send(sockd, user.c_str(), user.size(), 0); // Converts user string to c-array and sends it to server
        usleep(3000000);
        send(sockd, join.c_str(), join.size(), 0);
        while (1) { 
            usleep(30000000);
            send(sockd, msg.c_str(), msg.size(), 0);
        }
        return 0;
    }
    catch (std::exception &e){
        std::cout << "Please send a correct port and a channel for the bot to join." << std::endl;
    }
}