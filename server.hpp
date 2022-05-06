#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <string>
#include <iostream>
#include <vector>

typedef struct t_user {

    bool pass;
    char *nick;
    char *channel;

} user;

typedef struct t_params {

    std::vector<std::string> channels;
    std::vector<user> users;

} params;

class server {
    public :

        server() {
            max_clients = 30;
            opt = 1;
            message = (char *)"Bienvenue sur le serveur IRC de Quentin. \r\nVeuillez saisir votre mot de passe.\r\n";
        }

        ~server() {}

        int port;
        char *password;
        int opt;
        int master_socket , addrlen , new_socket , client_socket[30] , 
          max_clients , activity, i , valread , sd;
        int max_sd;
        struct sockaddr_in address;
        char buffer[1025];
        fd_set readfds; //stock les file descriptors
        char *message;   // simple message
        params p;


        int checkArgs (int ac, char **args){
            char *tmp = args[1];
            char *tmp2 = args[2];

            if (ac != 3)
                return (0);
            for (int i = 0; tmp[i]; i++)
            {
                if (!std::isdigit(tmp[i]))
                    return (0);
            }
            if (std::strlen(tmp2) <= 0 || std::strlen(tmp) <= 0)
                return (0);
            password = tmp2;
            port = std::atoi(tmp);
            if (port < 0 || port > 65535)
                return (0);
            return (1);
        }


        void init(){
            for (int i = 0; i < max_clients; i++)  // initiliase a zero.
            {  
                client_socket[i] = 0;
            }
            master_socket = socket(AF_INET , SOCK_STREAM , 0); // socket principale qui va assurer le traffic. AF_INET pour ipv4, SOCK_STREAM pour du TPC.
            
            setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)); //setup master socker pour recevoir plusieurs connexions.

            address.sin_family = AF_INET;  // ipv4.
            address.sin_addr.s_addr = INADDR_ANY; // Accepte toutes les adresses de la machine.
            address.sin_port = htons( port ); // defini le port sur 8888.
            
            bind(master_socket, (struct sockaddr *)&address, sizeof(address)); // bind la socket sur le localhost au port 8888.

            listen(master_socket, 3); // donne le nombre de 3 connections en attente simultanement.

            addrlen = sizeof(address);  
           // puts("Waiting for connections ...");
            while (1)
            {
                FD_ZERO(&readfds); // Clear les sockets.

                FD_SET(master_socket, &readfds); // Ajoute le master_socket dans le set.s
                max_sd = master_socket;  
            

                for (int i = 0 ; i < max_clients ; i++)  // Ajoute les autres sockets
                {  
                    sd = client_socket[i];  // socket descriptor actuel.
                 
                    if(sd > 0)  
                        FD_SET( sd , &readfds);  // Si valide on le rajoute a la liste.
                 
                    if(sd > max_sd)  // plus gros numero de fd , utilise par select.
                        max_sd = sd;  
                } 

                activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  // En attente d activite sur un socket de la liste.


                // Nouvelle activite
                if (FD_ISSET(master_socket, &readfds)) // verifie que le fd est bien integre a la liste.
                {
                    new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen); // accepte la nouvelle socket

                    send(new_socket, message, strlen(message), 0); // envoie le message de bienvenue.
                    
                    for (i = 0; i < max_clients; i++)  // Ajoute la nouvelle socket a la liste.
                    {  
                        // si la position est vide.
                        if(client_socket[i] == 0 )  
                        {
                            client_socket[i] = new_socket;
                            break;  
                        }  
                    }  
                }
                for (i = 0; i < max_clients; i++)
                {
                    sd = client_socket[i];
                    
                    if (FD_ISSET( sd , &readfds))
                    {
                        if ((valread = read( sd , buffer, 1024)) == 0) // Verifie si il s agit d une deconnexion et lit le message recu.
                        {
                            close( sd );
                            client_socket[i] = 0;
                        }
                     /*   else // termine par NULL le string et le renvoie.
                        {
                            buffer[valread] = '\0';  
                            send(sd , buffer , strlen(buffer) , 0 );  
                        }*/
                    }
                }
            }
        }
};