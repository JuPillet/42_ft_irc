#include <iostream>
#include "IRCutils.hpp"
#include "Channel.hpp"

void			Channel::setTopic ( std::string topic, std::string servIp, std::string changerNick )
{
	_topic = topic;
	for ( clientIterator userIt = _cliCrnt.begin(); userIt != _cliCrnt.end(); ++userIt )
		sender( ( *userIt )->getSocket() , ":*." + servIp + " 332 " + changerNick + " " + _name +" "+ _topic + "\r\n", 0 );
}

void			Channel::addFlag( char flag )
{
	strIt	flagIt;
	for ( flagIt = _flags.begin(); flagIt != _flags.end() && *flagIt != flag ; ++flagIt );
	if ( flagIt == _flags.end() )
		_flags.push_back( flag );
}

void			Channel::delFlag( char flag )
{
	strIt	flagIt;
	for ( flagIt = _flags.begin(); flagIt != _flags.end() && *flagIt != flag ; ++flagIt );
	if ( flagIt != _flags.end() )
		_flags.erase( flagIt );
}

void							Channel::printModifFlags( std::string const _printed ) const
{
	constClientIterator userIt;
	for ( userIt = _cliCrnt.begin(); userIt != _cliCrnt.end() ; ++userIt )
		sender( ( *userIt )->getSocket(),  _printed, 0 );
}

void			Channel::setPass ( std::string str )
{
	_pass = str;
	addFlag( 'k' );
}
void			Channel::unsetPass ( void )
{
	_pass.clear();
	delFlag( 'k' );
}
void			Channel::setPriv( bool priv ) { ( _priv = priv ) ? addFlag( 'p' ) : delFlag( 'p' ); }
void			Channel::setSecret( bool secret ) { ( _secret = secret ) ? addFlag( 's' ) : delFlag( 's' ); }
void			Channel::setInvit( bool invit ) { ( _invit = invit ) ? addFlag( 'i' ) : delFlag( 'i' ); }
void			Channel::setMod( bool mod ) { ( _mod = mod ) ? addFlag( 'm' ) : delFlag( 'm' ); }
void			Channel::setExt ( bool extMsg ) { ( _extMsg = extMsg ) ? addFlag( 'n' ) : delFlag( 'n' ); }
void			Channel::setLimit ( unsigned int limit ) { ( _limit = limit ) ? addFlag( 'l' ) : delFlag( 'l' ); }
void			Channel::setProtecTopic ( bool protect ) { ( _protecTopic = protect )? addFlag( 't' ) : delFlag( 't' ); }

void			Channel::setOps( std::string ops )
{
	if ( isOps( ops ) == _chanOps.end() )
		_chanOps.push_back( ops );
}
void			Channel::unsetOps( std::string nick )
{
	strListIt opsIt = isOps( nick );
	if ( opsIt != _chanOps.end() )
		_chanOps.erase( opsIt );
}
strListIt		Channel::isOps( std::string nick )
{
	strListIt opsIt;
	for ( opsIt = _chanOps.begin(); opsIt != _chanOps.end() && nick != *opsIt ; ++opsIt );
	return ( opsIt );
}

void			Channel::eraseCli( std::string nick )
{
	clientIterator clienTarget = isCli( nick );
	if ( clienTarget != _cliCrnt.end() )
		_cliCrnt.erase( clienTarget );
}
clientIterator	Channel::isCli( std::string nick )
{
	clientIterator cliIt;
	for ( cliIt = _cliCrnt.begin(); cliIt != _cliCrnt.end() && nick != ( *cliIt )->getNick(); ++cliIt );
	return ( cliIt );
}

std::string		Channel::getNickList( void )
{
	std::string	clients;
	for ( clientIterator clientIt = _cliCrnt.begin(); clientIt != _cliCrnt.end(); ++clientIt )
	{
		if ( isOps( ( *clientIt )->getNick() ) != _chanOps.end() )
			clients += "@";
		else if ( isVo( ( *clientIt )->getNick() ) != _cliVo.end() )
			clients += "+";
		clients += ( ( *clientIt )->getNick() + " " );
	}
	if ( clients.size() )
		clients.erase( --clients.end() );
	return clients;
}

std::string		Channel::getUserByNick( std::string &nickTarget ) 
{
	clientIterator clientIt;
	for ( clientIt = _cliCrnt.begin(); clientIt != _cliCrnt.end(); ++clientIt )
		if ( ( *clientIt )->getNick() == nickTarget )
			return ( *clientIt )->getUser();
	return "";
}

void			Channel::unsetVo( std::string nick )
{
	strListIt voIt = isVo( nick );
	if ( voIt != _cliVo.end() )
		_cliVo.erase( voIt );
}

strListIt		Channel::isVo( std::string nick )
{
	strListIt voIt;
	for ( voIt = _cliVo.begin(); voIt != _cliVo.end() && nick != *voIt; ++voIt );
	return voIt;
}

void			Channel::addGuests( std::string guest )
{
	if ( isGuest( guest ) == _guests.end() )
		_guests.push_back( guest );
}

void			Channel::removeGuests( std::string guest )
{
	strListIt guestIt = isGuest( guest );
	if ( guestIt != _guests.end() )
		_guests.erase( guestIt );
}

strListIt		Channel::isGuest( std::string guest )
{
	strListIt guestIt;
	for ( guestIt = _guests.begin(); guestIt != _guests.end() && *guestIt != guest; ++guestIt );
	return guestIt;
}

void			Channel::setBan( std::string tmp , unsigned int nb )
{
	itBan tmpIt = isBan( tmp );

	if ( tmpIt == _chanBan.end() )
	{
		if ( nb == 0 )
			_chanBan.push_back( _pairBan( tmp, 0 ) );
		else
			_chanBan.push_back( _pairBan( tmp, std::time( nullptr ) + nb ) );
	}
	else
	{
		if ( nb == 0 )
			tmpIt->second = nb;
		else
			tmpIt->second = std::time( nullptr ) + nb;
	}
}

void			Channel::unBan( std::string tmp )
{
	itBan tmpIt ( isBan( tmp ) );
	if ( isBan( tmp ) == _chanBan.end() )
		throw ( std::string( "User isnt banned." ) );
	_chanBan.erase( tmpIt );
}

itBan			Channel::isBan( std::string nick )
{
	itBan banIt;
	for ( banIt = _chanBan.begin(); banIt != _chanBan.end() && banIt->first != nick; ++banIt );
	if ( banIt != _chanBan.end() && banIt->second && banIt->second <= std::time( nullptr ) )
	{
			_chanBan.erase( banIt );
			banIt = _chanBan.end();
	}
	return banIt;
}

void			Channel::WHO( clientIterator clientIt, std::string &servIP )
{
	for ( clientIterator userIt = _cliCrnt.begin(); userIt != _cliCrnt.end(); ++userIt )
	{
		std::cout << "WHO passed" << std::endl;
		std::string answer = ":*." + servIP + " 352 " + ( *clientIt )->getNick() + " " + _name + " ";
		if ( ( *userIt )->getNick() == _owner )
			answer.push_back( '~' );
		answer += ( *userIt )->getUser() + " " + servIP + " " + servIP + " ";
		if ( isOps( (*userIt)->getNick() ) != _chanOps.end() )
			answer.push_back( '@' );
		answer += " :0 " + ( *userIt )->getName() + "\r\n";
		try
		{ sender( ( *clientIt )->getSocket(), answer , 0); }
		catch (std::string err)
		{ std::cerr << err << std::endl; }
	}
	sender ( ( *clientIt )->getSocket(), ":*." + servIP + " 315 " + ( *clientIt )->getNick() + " " + _name + " :End of /WHO list.\r\n" , 0 );
}