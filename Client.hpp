#pragma once
#include <string>
#include <unistd.h>
#include "IRCErr.hpp"

class Client
{
	int				_client_socket;
	std::string		_pass;
	std::string		_nick;
	std::string		_user;
	std::string		_channel;
	Client( void ):_pass(), _client_socket(0), _nick(), _user(), _channel() {};
	Client( Client const &src );
	Client	&operator=( Client const &src ) {
		_pass = src._pass;
		_client_socket = src._client_socket;
		_nick = src._nick;
		_channel = src._channel;
		return *this;
	}
	public:
							Client( int const _new_socket ): _client_socket( _new_socket ), _pass(), _nick(), _user(), _channel()
							{ return ; }
							Client( int const _new_socket, std::string const &pass, std::string const &nick, std::string const &user ):
							_client_socket( _new_socket ), _pass( pass ), _nick( nick ), _user( user ), _channel()
							{ return ; }
							~Client( void ) { close( _client_socket ); }
		void				setPass( std::string const &pass ) { _pass = pass; }
		std::string const	&getPass( void ) const { return _pass; }
		void				setSocket( int new_socket ) { _client_socket = new_socket; }
		int					getSocket( void ) const { return _client_socket; }
		void				setNick( std::string const &nick ) { _nick = nick; }
		std::string const	&getNick( void ) { return _nick; }
		void				setUser( std::string const &user ) { _user = user; }
		std::string const	&getUser( void ) { return _user; }
		void setChannel( std::string channel) {_channel = channel;}
		std::string	const	&getChannel(void) {return _channel;}

};