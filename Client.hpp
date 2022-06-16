#pragma once
#include <string>
#include <unistd.h>
#include <list>
#include "IRCErr.hpp"

class	Client;
class	Channel;
class	IRCData;


typedef std::string::iterator					strIt;

typedef	std::list<Client*>::iterator			clientIterator;
typedef	std::list<Client*>::const_iterator		constClientIterator;
typedef std::list<Channel>::iterator			channelIterator;
typedef std::list<std::string>::iterator		strListIt;
typedef std::list<std::string>::const_iterator	conStrListIt;
typedef std::pair<std::string, time_t>			pairBan;
typedef std::list<pairBan>::iterator			itBan;
typedef void(IRCData::*ptrFct)( void );
typedef std::pair<char, ptrFct>					pairKVM;
typedef std::pair<std::string, ptrFct>			pairKVC;
typedef std::list<pairKVM>						listPairU;
typedef std::list<pairKVM>						listPairC;
typedef std::list<pairKVC>						listPairI;

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
										Client( void ): _client_socket(0), _authentified( false ), _pass(), _nick(), _user() {};
	public:
		
										Client( int const _new_socket ):
											_client_socket( _new_socket ), _authentified( false ), _pass(), _nick(), _user(), _name(), _request()
										{ return ; }

										Client( int const _new_socket, std::string const &pass, std::string const &nick, std::string const &user ):
											_client_socket( _new_socket ), _authentified( false ), _pass( pass ), _nick( nick ), _user( user ), _name(), _request()
										{ return ; }

		explicit						Client( Client const &src ): _client_socket( src.getSocket() ), _authentified( src.getAutentification() ),
											_pass( src.getPass() ), _nick( src.getNick() ), _user( src.getUser() ), _name( src.getName() ), _request( *src.getRequest() )
										{ return ; }

										~Client( void ) { close( _client_socket ); }
		Client							&operator=( Client const &src )
										{
											_client_socket = src._client_socket;
											_authentified = src._authentified;
											_pass = src._pass;
											_nick = src._nick;
											_user = src._user;
											_name = src._name;
											return *this;
										}
		void							setSocket( int new_socket ) { _client_socket = new_socket; }
		int								getSocket( void ) const { return _client_socket; }
		void							setAutentification( void ) { _authentified = true; }
		bool const						getAutentification( void ) const { return _authentified; }
		void							setPass( std::string const &pass )
										{
											if( !getAutentification() )
												_pass = pass;
										}
		std::string const				getPass( void ) const { return _pass; }
		void							setClIp( std::string new_clip ) { _clIp = new_clip; }
		std::string const				getClIp( void ) const { return _clIp; }
		void							setNick( std::string const &nick ) { _nick = nick; }
		std::string const				getNick( void ) const { return _nick; }
		void							setUser( std::string const &user ) { _user = user; }
		std::string const				getUser( void ) const { return _user; }
		void							setName( std::string const &name) { _name = name; }
		std::string const				&getName( void ) const { return _name; }
		std::string						const *getRequest( void ) const { return &_request; }
		void							setInvitation( std::string invitation ) { _invitation = invitation; }
		std::string						getInvitation( void ) const { return _invitation; }
};
