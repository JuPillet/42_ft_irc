 #include "server.hpp"
 
int main (int argc, char **argv)
{
    server serv;
    if (serv.checkArgs(argc, argv) == 0)
    {
        std::cout << "BAD ARGUMENTS, SEND PORT AS FIRST ARGUMENT WITH NUMBERS ONLY, PASSWORD AS SECOND ARGUMENT, PORT AS TO BE IN A RANGE FROM 1 TO 65535" << std::endl;
        return (EXIT_FAILURE);
    }
    serv.init();

    return (0);
}