#include <iostream>
#include "Client.hpp"

Client::Client( Client const &src ): _client_socket( src.getSocket() ), _authentified( src.getAutentification() ),
						_pass( src.getPass() ), _nick( src.getNick() ), _user( src.getUser() ), _name( src.getName() ), _request( *src.getRequest() )
{ return; }

Client				&Client::operator=( Client const &src )	
{
	_client_socket = src._client_socket;
	_authentified = src._authentified;
	_pass = src._pass;
	_nick = src._nick;
	_user = src._user;
	_name = src._name;
	return *this;
}
		
		
void				Client::setSocket( int new_socket ) { _client_socket = new_socket; }
int					Client::getSocket( void ) const { return _client_socket; }
		
void				Client::setAutentification( std::string servIP, std::string sin_addr )
{
	_authentified = true;
	std::string answer = ":" + servIP + " 001 " + _nick + " :Welcome to the IRC_QDJ_Server " + _nick + "!" + _user + "@" + sin_addr + "\r\n";
	std::cout << "client socket: " << _client_socket << std::endl;
	sender( _client_socket, answer, 0 );
	std::cout << "Welcome message sent successfully" << std::endl;
}
bool				Client::getAutentification( void ) const { return _authentified; }
		
		
void				Client::setPass( std::string const &pass )
{
	if( !getAutentification() )
		_pass = pass;
}
std::string const	Client::getPass( void ) const { return _pass; }
		
		
void				Client::setClIp( std::string new_clip ) { _clIp = new_clip; }
std::string const	Client::getClIp( void ) const { return _clIp; }
		
void				Client::setNick( std::string const &nick )
{
	std::string holdNick = _nick;
	_nick = nick;
	sender( _client_socket, ":" + holdNick + "!~" + _user + "@" + _clIp + " NICK :" + _nick + "\r\n", 0 );
}
std::string const	Client::getNick( void ) const { return _nick; }
		
		
void				Client::setUser( std::string const &user ) { _user = user; }
std::string const	Client::getUser( void ) const { return _user; }

void				Client::setName( std::string const &name ) { _name = name; }
std::string const	&Client::getName( void ) const { return _name; }

std::string const	*Client::getRequest( void ) const { return &_request; }
		
void				Client::setInvitation( std::string invitation ) { _invitation = invitation; }
std::string			Client::getInvitation( void ) const { return _invitation; }

bool				Client::isInChannel( std::string name )
{
	for ( channelsListIt chanIt = _channels.begin(); chanIt != _channels.end(); ++chanIt )
		if ( ( *chanIt )->getName() == name )
			return true;
	return false;
}

void				Client::addChannel(std::string servIP, Channel *channel )
{
	std::string channelName = channel->getName();
	_channels.push_back( channel );
	channel->setCli( this );
	constClientIterator	chanCliIt;
	for ( chanCliIt = channel->getCli().begin(); chanCliIt != channel->getCli().end(); ++chanCliIt )
	{
		try
		{ sender( ( *chanCliIt )->getSocket(), ":" + _nick + "!~" + _user + "@" + _clIp + " JOIN :" + channelName + "\r\n", 0);	}
		catch ( std::string const &err )
		{ std::cerr << err << std::endl; }
		if ( *chanCliIt == this )
		{
			try
			{
				std::string topic = channel->getTopic();
				if ( !topic.size() )
					sender( ( *chanCliIt )->getSocket(), ":*." + servIP + " 331 " + _nick + " " + channelName + " :No topic is set" + "\r\n", 0 );
				else
					sender( ( *chanCliIt )->getSocket(), ":*." + servIP + " 332 " + _nick + " " + channelName + " :" + channel->getTopic() + "\r\n", 0 );
			}
			catch ( std::string err )
			{ std::cerr << err << std::endl; }
			try
			{
				std::string nickList = channel->getNickList();
				std::cout << ":*." + servIP + " 353 " + _nick + " = " + channelName + " :" + nickList + "\r\n" << std::endl;
				sender( ( *chanCliIt )->getSocket(), ":*." + servIP + " 353 " + _nick + " = " + channelName + " :" + nickList + "\r\n", 0 );
			}
			catch ( std::string const &err )
			{ std::cerr << std::string( "353 " ) + err << std::endl; }
			try
			{ sender( ( *chanCliIt )->getSocket(), ":*." + servIP + " 366 " + _nick + " " + channelName + " :End of NAMES list\r\n", 0 ); }
			catch ( std::string const &err )
			{ std::cerr << std::string ( "366 : " ) + err << std::endl; }
		}
	}
}

channelsList		&Client::getChannels( void ) { return _channels; }
void				Client::removeChannel( Channel *channel )
{
	channelsListIt chanIt;
	for ( chanIt = _channels.begin(); chanIt != _channels.end() && *chanIt != channel; ++chanIt );
	if ( chanIt != _channels.end() )
		_channels.erase( chanIt );
}

void				Client::removeInChannel( Channel *channel )
{
	channel->unsetVo( _nick );
	channel->unsetOps( _nick );
	channel->eraseCli( _nick );
	channel->removeGuests( _nick );
	removeChannel( channel );
}

void				Client::removeInChannel( channelIterator channel )
{
	channel->unsetVo( _nick );
	channel->unsetOps( _nick );
	channel->eraseCli( _nick );
	channel->removeGuests( _nick );
	removeChannel( &( *channel ) );
}

void				Client::removeInChannel( channelsListIt channel )
{
	( *channel )->unsetVo( _nick );
	( *channel )->unsetOps( _nick );
	( *channel )->eraseCli( _nick );
	( *channel )->removeGuests( _nick );
	removeChannel( *channel );
}

void				Client::removeInAllChannel( void )
{
	for ( channelsListIt chanIt = _channels.begin(); chanIt != _channels.end(); )
	{
		removeInChannel( chanIt );
		chanIt = _channels.begin();
	}
}