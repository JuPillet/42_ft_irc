#pragma once
#include <string>
#include <exception>
#include <sys/socket.h>

void	sender( int dest, std::string answer, std::string *err );