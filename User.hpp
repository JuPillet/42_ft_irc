#pragma once
#include <string>

class User
{
	int				_client_socket;
	std::string		_pass;
	std::string		_nick;
	std::string		_user;
	std::string		_channel;
	User( void ):_pass(), _client_socket(0), _nick(), _user(), _channel() {};
	User( User const &src );
	User	&operator=( User const &src ) {
		_pass = src._pass;
		_client_socket = src._client_socket;
		_nick = src._nick;
		_channel = src._channel;
		return *this;
	}
	public:
							User( int const _new_socket, std::string const &pass, std::string const &nick, std::string const &user ): _client_socket( _new_socket ), _pass( pass ), _nick( nick ), _user( user ), _channel() {};
							~User( void ) { close( _client_socket ); }
		void				setPass( std::string const &pass ) { _pass = pass; }
		std::string const	&getPass( void ) const { return _pass; }
		void				setClientSocket( int new_socket ) { _client_socket = new_socket; }
		int					getClientSocket( void ) const { return _client_socket; }

};