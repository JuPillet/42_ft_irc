
#include "IRCmsg.hpp"

IRCErr::IRCErr( const std::string &err ) :_err( "Error: " + err ) { return; }

IRCErr::~IRCErr() { return; }
		
const std::string 	IRCErr::getError() const { return _err; }

void				sender(int dest, std::string answer, IRCErr *err )
{
	if ( send( dest, answer.c_str(), answer.length(), 0 ) == -1 )
		throw( IRCErr( "send" ) );
	if ( err )
		throw( *err );
}