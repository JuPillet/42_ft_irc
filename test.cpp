#include <ctime>
#include <iostream>
int main ( void )
{
	time_t _time = std::time( 0 );
	std::cout << _time << std::endl;
}

 