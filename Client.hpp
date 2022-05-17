#pragma once
#include <string>
#include <unistd.h>
#include <list>
#include "IRCErr.hpp"

class Client
{
	int									_client_socket;
	bool								_authentified;
	std::string							_pass;
	std::string							_nick;
	std::string							_user;
//	std::string							_role;
	std::string							_name;
	std::list<std::string>				_channels;
										Client( void ): _client_socket(0), _authentified( false ), _pass(), _nick(), _user(), _channels() {};
	public:
										Client	&operator=( Client const &src )
										{
											_client_socket = src._client_socket;
											_authentified = src._authentified;
											_pass = src._pass;
											_nick = src._nick;
											_user = src._user;
											_name = src._name;
											for ( std::list<std::string>::const_iterator chanIt = src._channels.begin(); chanIt != src._channels.end(); ++chanIt )
												_channels.push_back( *chanIt );
											return *this;
										}
										Client( int const _new_socket ): _client_socket( _new_socket ), _authentified( false ), _pass(), _nick(), _user(), _channels()
										{ return ; }
										Client( int const _new_socket, std::string const &pass, std::string const &nick, std::string const &user ):
										 _client_socket( _new_socket ), _authentified( false ), _pass( pass ), _nick( nick ), _user( user ), _channels()
										{ return ; }
										~Client( void ) { return; }
		void							setSocket( int new_socket ) { _client_socket = new_socket; }
		int								getSocket( void ) const { return _client_socket; }
		void							setAutentification( void ) { _authentified = true; }
		bool const						getAutentification( void ) const { return _authentified; }
		void							setPass( std::string const &pass )
										{
											if( !getAutentification() )
												_pass = pass;
										}
		std::string const				&getPass( void ) const { return _pass; }
		void							setNick( std::string const &nick ) { _nick = nick; }
		std::string const				&getNick( void ) { return _nick; }
		void							setUser( std::string const &user ) { _user = user; }
		std::string const				&getUser( void ) { return _user; }
		void							setChannel( std::string channel ) { _channels.push_back( channel ); }
		std::list<std::string> const	&getChannels( void ) const { return _channels; }
		//void removeChannel
};