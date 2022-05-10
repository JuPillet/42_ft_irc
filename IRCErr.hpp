#pragma once
#include <string>

class IRCErr {
	const std::string _err;
	public :
        IRCErr( const std::string &err ):_err( "Error: " + err ){}
        ~IRCErr() {}
        const std::string getError() const { return _err; }
};