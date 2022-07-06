#pragma once
#include <string>
#include <sys/socket.h>

class	IRCErr
{
	const std::string _err;
	public :
		IRCErr( const std::string &err );
		~IRCErr();
		const std::string getError() const;
};

void	sender( int dest, std::string answer, IRCErr *err );