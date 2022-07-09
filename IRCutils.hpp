#include	<string>

namespace IRC
{
	int				stoi( const std::string &str, std::size_t *index = 0 );
	long int		stol( const std::string &str, std::size_t *index = 0 );
	std::string		ultostr( std::size_t val );
};