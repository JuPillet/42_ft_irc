#pragma once
#include <ctime>
#include <cctype>
#include "Client.hpp"

namespace IRC
{
	int	stoi( const std::string &str, std::size_t *index = 0 )
	{
		char	posinega = 1;
		size_t	ifnull;
		int		result = 0;

		if ( !index )
			index = &ifnull;
		for ( *index = 0; *index < str.size() && std::isspace( str[*index] ); ++( *index ) );
		if ( *index < str.size() && str[*index] == '-' )
			posinega = -1;
		if ( *index < str.size() && ( str[*index] == '+' || str[*index] == '-' ) )
			++( *index );
		for ( ; *index < str.size() && std::isdigit( str[*index] ); ++( *index ) )
			result = ( result * 10 ) + ( ( str[*index] - '0' ) * posinega );
		return result;
	}

	long int	stol( const std::string &str, std::size_t *index = 0 )
	{
		char posinega = 1;
		size_t	ifnull;
		long int result = 0;

		if ( !index )
			index = &ifnull;
		for ( *index = 0; *index < str.size() && std::isspace( str[*index] ); ++( *index ) );
		if ( *index < str.size() && str[*index] == '-' )
			posinega = -1;
		if ( *index < str.size() && ( str[*index] == '+' || str[*index] == '-' ) )
			++( *index );
		for ( ; *index < str.size() && std::isdigit( str[*index] ); ++( *index ) )
			result = ( result * 10 ) + ( ( str[*index] - '0' ) * posinega );
		return result;
	}
};

typedef std::pair< std::string, time_t >	_pairBan;

class Channel
{
		bool							_moder; // is operator only mode enabled
		bool							_priv;
		bool							_secret;
		bool							_extMsg; // bloque ou non les messages externes
		bool							_invit;
		unsigned long					_limit; // limite d utilisateurs sur le channel, 0 pour pas de limite.
		const std::string				_name;
		std::string						_mods;
		std::string						_pass;
		std::string						_topic;
		std::list<Client *>				_cliCrnt;
		std::list<std::string>			_chanOps;
		std::list<std::string>			_cliVo;
		std::list<std::string>			_guests;
		std::list<_pairBan>				_chanBan;

	public :
										Channel( void ) : _moder( false ), _priv( false ), _secret(false), _extMsg( false ), _invit( false ), _limit( 0 ), _name(), _mods(), _pass(), _topic(), _cliCrnt(), _chanOps(), _cliVo(), _chanBan() { return; }
										Channel( std::string name ) : _moder( false ), _priv( false ), _secret(false), _extMsg( false ), _invit( false ), _limit( 0 ), _name( name ), _mods(), _pass(), _topic(), _cliCrnt(), _chanOps(), _cliVo(), _chanBan() { return; }
										~Channel( void ) {}
		std::string						getName( void ) const { return _name; }
//		void							
		std::string						getMods( void ) { return _mods; }
		void							setPass ( std::string str ) { _pass = str; }
		void                            unsetPass ( void ) { _pass.clear(); }
		std::string						getPass( void ) const { return _pass; }
		void							setPriv( bool priv ) { _priv = priv; }
		bool							getPriv( void ) const { return _priv; }
		void							setSecret( bool secret ) { _secret = secret; }
		bool							getSecret( void ) const { return _secret; }
		void							setInvit( bool invit ) { _invit = invit; }
		bool							getInvit( void ) const { return _invit; }
		void							setMod( bool mod ) { _moder = mod; }
		bool							getMod( void ) const { return _moder; }
		void							setExt ( bool extMsg ) { _extMsg = extMsg; }
		bool							getExt( void ) const { return _extMsg; }
		void							setLimit ( unsigned int tmp ) { _limit = tmp;}
		unsigned int					getLimit ( void ) const { return _limit; }
		void							setTopic ( std::string topic ) { _topic = topic; }
		std::string						getTopic ( void ) { return _topic; }
		strListIt						isOps( std::string user ) {
			strListIt opsIt;
			for ( opsIt = _chanOps.begin(); opsIt != _chanOps.end() && user != *opsIt ; ++opsIt )
			return ( opsIt );
		}
		void                            setOps( std::string ops ) {
			if ( isOps( ops ) == _chanOps.end() )
				_chanOps.push_back( ops );
		}
		void                            unsetOps( std::string user )
        {
            strListIt opsIt = isOps( user );
            if ( opsIt != _chanOps.end() )
				_chanOps.erase( opsIt );
        }
		std::list<std::string> const	*getOps( void ) const { return &_chanOps; }
		void							setCli( Client *tmp ) { _cliCrnt.push_back( tmp ); }
		std::list<Client *> const		*getCli( void ) const { return &_cliCrnt; }
		std::string						getNickList( void )
		{
			std::string	clients;
			for ( clientIterator clientIt = _cliCrnt.begin(); clientIt != _cliCrnt.end(); ++clientIt )
				clients.append( ( *clientIt )->getNick() + " " );
			if ( clients.size() )
				clients = std::string( clients, clients.size() - 1 );
			return clients;
		}
		std::string						getUserByNick( std::string &nickTarget ) 
		{
			clientIterator clientIt;
			for ( clientIt = _cliCrnt.begin(); clientIt != _cliCrnt.end(); ++clientIt );
				if ( ( *clientIt )->getNick() == nickTarget )
					return ( *clientIt )->getUser();
			return "";
		}
		clientIterator					isCli( std::string user ) {
			clientIterator cliIt;
			for ( cliIt = _cliCrnt.begin(); cliIt != _cliCrnt.end() && user != ( *cliIt )->getUser(); ++cliIt );
			return ( cliIt );
		}
		void							eraseCli( std::string user )
		{
			clientIterator cliIt ( isCli( user ) );
			if ( cliIt == _cliCrnt.end() )
				throw( IRCErr( "Client isnt in the channel: " + _name ) );
			_cliCrnt.erase( cliIt );
		}

		void							setVo( std::string tmp ) { _cliVo.push_back( tmp ); }
		std::list<std::string>			*getVo( void ) { return &_cliVo; }
		strListIt							isVo( std::string user ) {
			strListIt voIt;
			for ( voIt = _cliVo.begin(); voIt != _cliVo.end() && user != *voIt; ++voIt );
			return voIt;
		}

		void                            unsetVo( std::string user )
        {
			strListIt voIt = isVo( user );
            if ( voIt != _cliVo.end() )
				_cliVo.erase( voIt );
        }

		strListIt						isGuest( std::string guest )
		{
			strListIt guestIt;
			for ( guestIt = _guests.begin(); guestIt != _guests.end() && *guestIt != guest; ++guestIt );
			return guestIt;
		}

		void							addGuests( std::string guest )
		{
			if ( isGuest( guest ) == _guests.end() )
				_guests.push_back( guest );
		}

		void							removeGuests( std::string guest )
		{
			strListIt guestIt = isGuest( guest );
			if ( guestIt != _guests.end() )
				_guests.erase( guestIt );
		}

		std::list<std::string>			getGuests( void ) { return _guests; }

		itBan							isBan( std::string user ) {
			itBan banIt;
			for ( banIt = _chanBan.begin(); banIt != _chanBan.end() && banIt->first != user; ++banIt );
			if ( banIt != _chanBan.end() && banIt->second && banIt->second <= std::time( nullptr ) )
			{
					_chanBan.erase( banIt );
					banIt = _chanBan.end();
			}
			return banIt;
		}

		void						setBan( std::string tmp , unsigned int nb )
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

		void						unBan( std::string tmp )
		{
			itBan tmpIt ( isBan( tmp ) );
			if ( isBan( tmp ) == _chanBan.end() )
				throw ( IRCErr( "User isnt banned." ) );
			_chanBan.erase( tmpIt );
		}

		std::list<pairBan> const	*getBan( void ) const { return &_chanBan; }
};