#pragma once
#include <string>
#include <unistd.h>
#include <list>
#include "IRCErr.hpp"

class Client
{
	int							_client_socket;
	std::string					_pass;
	std::string					_nick;
	std::string					_user;
	std::list<std::string>		_channels;
	Client( void ):_pass(), _client_socket(0), _nick(), _user(), _channels() {};
	Client( Client const &src );
	Client	&operator=( Client const &src ) {
		_pass = src._pass;
		_client_socket = src._client_socket;
		_nick = src._nick;
		for ( std::list<std::string>::iterator channelIt = static_cast<Client>(src)._channels.begin(); channelIt != src._channels.end(); ++channelIt )
			_channels.push_back( *channelIt );
		return *this;
	}
	public:
							Client( int const _new_socket ): _client_socket( _new_socket ), _pass(), _nick(), _user(), _channels()
							{ return ; }
							Client( int const _new_socket, std::string const &pass, std::string const &nick, std::string const &user ):
							_client_socket( _new_socket ), _pass( pass ), _nick( nick ), _user( user ), _channels()
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
		void setChannel( std::string channel ) { _channels.push_back( channel ); }
		std::list<std::string> const &getChannels( void ) const { return _channels; }
		//void removeChannel
};