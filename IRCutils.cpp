#include	<string>
#include	"IRCutils.hpp"
int		IRC::stoi( const std::string &str, std::size_t *index)
{
	char		posinega = 1;
	std::size_t	ifnull;
	int			result = 0;

	if ( !index )
		index = &ifnull;
	for ( *index = 0; *index < str.size() && std::isspace( str[*index] ); ++( *index ) );
	if ( *index < str.size() && str[*index] == '-' )
		posinega = -1;
	if ( *index < str.size() && ( str[*index] == '+' || str[*index] == '-' ) )
		++( *index );
	for ( ; *index < str.size() && std::isdigit( str[*index] ); ++( *index ) )
		result = ( result * 10 ) + ( ( str[*index] - '0' ) * posinega );
	return result;
};

long int		IRC::stol( const std::string &str, std::size_t *index)
{
	char posinega = 1;
	std::size_t	ifnull;
	long int result = 0;

	if ( !index )
		index = &ifnull;
	for ( *index = 0; *index < str.size() && std::isspace( str[*index] ); ++( *index ) );
	if ( *index < str.size() && str[*index] == '-' )
		posinega = -1;
	if ( *index < str.size() && ( str[*index] == '+' || str[*index] == '-' ) )
		++( *index );
	for ( ; *index < str.size() && std::isdigit( str[*index] ); ++( *index ) )
		result = ( result * 10 ) + ( ( str[*index] - '0' ) * posinega );
	return result;
};

std::string		IRC::ultostr( std::size_t val )
{
	std::string valStr;
	do
	{
		char cToAdd = ( val % 10 ) + '0';
		valStr = cToAdd + valStr;
		val /= 10;
	} while( val );
	return valStr;
}