#include "IRCData.hpp"

int main( int ac, char **av, char **ep )
{
	IRCData server;

	try
	{
		server.nbArgs( ac );
		server.init( av[1], av[2], ep );
	}
	catch ( std::string const &err )
	{
		std::cerr << err << std::endl;
		std::exit( EXIT_FAILURE );
	}

	while ( true )
	{
		server.addClearedMasterSocket(); //OK
		try
		{ server.activityListener(); }
		catch ( std::string const &err )
		{
			std::cerr << err << std::endl;
			continue;
		}
		try
		{
			if ( FD_ISSET( server.getMasterSocket(), server.getPtrReadFds() ) )
				server.newClient();
		}
		catch ( std::string const &err )
		{ std::cerr << err << std::endl; }
		try { server.IOListener(); }
		catch ( std::string const &err )
		{ std::cerr << err << std::endl; }
		//usleep( 500000 );
		//std::cout << "END OF IOListener" << std::endl;
	}
	return 0;
}
