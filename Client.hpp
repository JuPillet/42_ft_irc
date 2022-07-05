#pragma once

#include <list>
#include "IRCmsg.hpp"

class	Client;
class	Channel;
class	IRCData;


typedef std::string::iterator					strIt;

typedef	std::list<Client*>::iterator			clientIterator;
typedef	std::list<Client*>::const_iterator		constClientIterator;
typedef std::list<Channel*>						channelsList;
typedef channelsList::iterator					channelListIt;
typedef std::list<Channel>::iterator			channelIterator;
typedef std::list<std::string>::iterator		strListIt;
typedef std::list<std::string>::const_iterator	conStrListIt;
typedef std::pair<std::string, time_t>			pairBan;
typedef std::list<pairBan>::iterator			itBan;
typedef void( IRCData::*ptrFct )( void );
typedef std::pair<ptrFct, ptrFct>				pairFctsM;
typedef std::pair<char, pairFctsM>				pairKVM;
typedef std::pair<std::string, ptrFct>			pairKVI;
typedef std::list<pairKVM>						listPairM;
typedef std::list<pairKVI>						listPairI;

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
		void							setAutentification( std::string servIP, std::string sin_addr )
										{
											_authentified = true;
											std::string answer = ":" + servIP + " 001 " + _nick + " :Welcome to the IRC_QJ_Server " + _nick + "!" + _user + "@" + sin_addr + "\r\n";
											std::cout << "client socket: " << _client_socket << std::endl;
											sender( _client_socket, answer, 0 );
											std::cout << "Welcome message sent successfully" << std::endl;
										}
		bool const						getAutentification( void ) const { return _authentified; }
		void							setPass( std::string const &pass )
										{
											if( !getAutentification() )
												_pass = pass;
										}
		std::string const				getPass( void ) const { return _pass; }
		void							setClIp( std::string new_clip ) { _clIp = new_clip; }
		std::string const				getClIp( void ) const { return _clIp; }
		void							setNick( std::string const &nick )
										{
											std::string holdNick = _nick;
											_nick = nick;
											sender( _client_socket, ":" + holdNick + "!~" + _user + "@" + _clIp + " NICK :" + _nick + "\r\n", 0 );
										}
		std::string const				getNick( void ) const { return _nick; }
		void							setUser( std::string const &user ) { _user = user; }
		std::string const				getUser( void ) const { return _user; }
		void							setName( std::string const &name ) { _name = name; }
		std::string const				&getName( void ) const { return _name; }
		std::string						const *getRequest( void ) const { return &_request; }
		void							setInvitation( std::string invitation ) { _invitation = invitation; }
		std::string						getInvitation( void ) const { return _invitation; }
		void							addChannel(std::string servIP, Channel *channel )
										{
											channelListIt channelIt;
											for ( channelIt = _channels.begin(); channelIt != _channels.end() && *channelIt != channel; ++channel );
											if ( channelIt == _channels.end() )
											{
												std::string channelName = channel->getName();
												_channels.push_back( channel );
												channel->setCli( this );

												constClientIterator	chanCliIt;
												std::string			answer;
												for ( chanCliIt = channel->getCli()->begin(); chanCliIt != channel->getCli()->end(); ++chanCliIt )
												{
													try
													{ sender( _client_socket, ":" + _nick + "!~" + _user + "@" + _clIp + " JOIN :" + channelName + "\r\n", 0);	}
													catch ( IRCErr err )
													{ std::cerr << err.getError() << std::endl; }
													if ( *chanCliIt == this )
													{
														try
														{
															if ( channel->getTopic().size() )
																sender( _client_socket, ":*." + servIP + " 332 " + _nick + " " + channelName + " :" + channel->getTopic() + "\r\n", 0 );
														}
														catch ( IRCErr err )
														{ std::cerr << err.getError() << std::endl; }
														try
														{ sender( _client_socket, ":*." + servIP + " 353 " + _nick + " = " + channelName + " :" + channel->getNickList() + "\r\n", 0 );	}
														catch ( IRCErr err )
														{ std::cerr << err.getError() << std::endl; }
														try
														{ sender( _client_socket, ":*." + servIP + " 366 " + _nick + " = " + channelName + " :End of /NAMES list.\r\n", 0 );	}
														catch ( IRCErr err )
														{ std::cerr << err.getError() << std::endl; }
													}
												}
											}
										}
		channelsList					getChannels( void ) { return _channels; }
		void							removeChannel( Channel *channel )
		{
			channelListIt chanIt;
			for ( chanIt = _channels.begin(); chanIt != _channels.end() && *chanIt != channel; ++chanIt );
			if ( chanIt != _channels.end() )
				_channels.erase( chanIt );
		}

		void							removeInChannel( Channel *channel )
		{
			channel->unsetVo( _nick );
			channel->unsetOps( _nick );
			channel->eraseCli( _nick );
			channel->removeGuests( _nick );
			removeChannel( channel );
		}

		void							removeInChannel( channelIterator channel )
		{
			channel->unsetVo( _nick );
			channel->unsetOps( _nick );
			channel->eraseCli( _nick );
			channel->removeGuests( _nick );
			removeChannel( &( *channel ) );
		}

		void							removeInChannel( channelListIt channel )
		{
			( *channel )->unsetVo( _nick );
			( *channel )->unsetOps( _nick );
			( *channel )->eraseCli( _nick );
			( *channel )->removeGuests( _nick );
			removeChannel( *channel );
		}
			
		void							removeInAllChannel( void )
		{
			for ( channelListIt chanIt = _channels.begin(); chanIt != _channels.end(); )
			{
				removeInChannel( chanIt );
				chanIt = _channels.begin();
			}
		}
};
