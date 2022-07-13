
#include "IRCmsg.hpp"

		
void				sender( int dest, std::string answer, std::string *err )
{
	if ( send( dest, answer.c_str(), answer.length(), 0 ) == -1 )
		throw	std::string( "Error: send" );
	if ( err )
		throw	std::string( "Error: " + *err );
}