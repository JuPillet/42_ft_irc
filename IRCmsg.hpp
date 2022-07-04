#pragma once
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

class IRCErr {
	const std::string _err;
	public :
		IRCErr( const std::string &err ):_err( "Error: " + err ){}
		~IRCErr() {}
		const std::string getError() const { return _err; }
};

void	sender( int dest, std::string answer, IRCErr *err )
{
	if ( send( dest, answer.c_str(), answer.length(), 0 ) == -1 )
		throw( IRCErr( "send" ) );
	if ( err )
		throw( *err );
}