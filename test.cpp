#include "IRCData.hpp"

int main(int ac, char **av)
{
	IRCData data;
	try
	{
		data.init( ac, std::atoi( av[1] ), av[2] );
	}
	catch (IRCErr const &err)
	{
		std::cerr << err.getMessage() << std::endl;
		throw ;
	}
	while (1)
	{
		data.setAddressLen();
		data.setNewSock();
		data.readAndPrint();
	}

	return 0;
}