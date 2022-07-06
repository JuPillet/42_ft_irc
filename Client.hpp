#pragma once

#include <list>
#include <unistd.h>
#include "IRCtypedef.hpp"
#include "IRCmsg.hpp"
#include "Channel.hpp"

class Client
{
	int									_client_socket;
	bool								_authentified;
	std::string							_clIp;
	std::string							_pass;
	std::string							_nick;
	std::string							_user;
	std::string							_name;
	std::string							_request;
	std::string							_invitation;
	channelsList						_channels;
	
										Client( void ): _client_socket( 0 ), _authentified( false ), _pass(), _nick(), _user() {};
	public:
		
										Client( int const _new_socket ): _client_socket( _new_socket ), _authentified( false ), _pass(), _nick(), _user(), _name(), _request()
										{ return ; }

										Client( int const _new_socket, std::string const &pass, std::string const &nick, std::string const &user ):
											_client_socket( _new_socket ), _authentified( false ), _pass( pass ), _nick( nick ), _user( user ), _name(), _request()
										{ return ; }

		explicit						Client( Client const &src );

										~Client( void ) { close( _client_socket ); }
		Client							&operator=( Client const &src );

		void							setSocket( int new_socket );
		int								getSocket( void ) const;

		void							setAutentification( std::string servIP, std::string sin_addr );
		bool const						getAutentification( void ) const;

		void							setPass( std::string const &pass );
		std::string const				getPass( void ) const;
		
		void							setClIp( std::string new_clip );
		std::string const				getClIp( void ) const;
		
		void							setNick( std::string const &nick );
		std::string const				getNick( void ) const;
		
		void							setUser( std::string const &user );
		std::string const				getUser( void )const;
		
		void							setName( std::string const &name );
		std::string const				&getName( void )const;
		
		std::string						const *getRequest( void )const;
		
		void							setInvitation( std::string invitation );
		std::string						getInvitation( void )const;
		
		bool							isInChannel( std::string name );

		void							addChannel(std::string servIP, Channel *channel );
		channelsList					getChannels( void );
		
		void							removeChannel( Channel *channel );
		void							removeInChannel( Channel *channel );
		void							removeInChannel( channelIterator channel );
		void							removeInChannel( channelsListIt channel );
		void							removeInAllChannel( void );
};

