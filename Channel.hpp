#pragma once
#include <ctime>
#include <cctype>
#include "Client.hpp"

namespace IRC
{
	int	stoi( const std::string &str, std::size_t *index = 0 )
	{
		char posinega = 1;
		std::string::const_iterator strIt;
		int result = 0;
		for ( strIt = str.begin(); strIt != str.end() && std::isspace( *strIt ); ++strIt );
		if ( *strIt == '-' )
			posinega = -1;
		if ( *strIt == '+' || *strIt == '-' )
			++strIt;
		for ( strIt = str.begin(); strIt != str.end() && std::isdigit( *strIt ); ++strIt );
			result = ( result * 10 ) + ( ( *strIt - '0' ) * posinega );
		*index = *strIt;
		return result;
	}

	long int	stol( const std::string &str, std::size_t *index = 0 )
	{
		char posinega = 1;
		std::string::const_iterator strIt;
		long int result = 0;
		for ( strIt = str.begin(); strIt != str.end() && std::isspace( *strIt ); ++strIt );
		if ( *strIt == '-' )
			posinega = -1;
		if ( *strIt == '+' || *strIt == '-' )
			++strIt;
		for ( strIt = str.begin(); strIt != str.end() && std::isdigit( *strIt ); ++strIt );
			result = ( result * 10 ) + ( ( *strIt - '0' ) * posinega );
		*index = *strIt;
		return result;
	}
};

typedef std::pair< std::string, time_t >	_pairBan;

class Channel
{
		bool							_mod; // is operator only mode enabled
		bool							_priv;
		bool							_secret;
		bool							_extMsg; // bloque ou non les messages externes
		unsigned int					_limit; // limite d utilisateurs sur le channel, 0 pour pas de limite.
		const std::string				_name;
		std::string						_mdp;
		std::list<Client *>				_cliCrnt;
		std::list<std::string>			_chanOps;
		std::list<std::string>			_cliVo;
		std::list< _pairBan >			_chanBan;

	public :
										Channel( void ) : _mod (0), _priv (0), _secret(0), _limit (0), _extMsg(0) {}
										Channel( std::string str ) : _name( str ), _mod (0), _priv(0), _secret(0), _limit(0), _extMsg(0) {}
										~Channel( void ) {}
		std::string						getName( void ) const { return _name; }
		void							setPass ( std::string str ) { _mdp = str; }
		std::string						getPass( void ) const { return _mdp; }
		void							setPriv( int sw ) { _priv = sw; }
		bool							getPriv( void ) const { return _priv;}
		void							setMod( int sw ) { _mod = sw; }
		bool							getMod( void ) const { return _mod; }
		void							setSecret ( int sw ) { _secret = sw; }
		bool							getSecret( void ) const { return _secret; }
		void							setExt ( int sw ) { _extMsg = sw; }
		bool							getExt( void ) const { return _extMsg; }
		void							setLimit (unsigned int tmp) { _limit = tmp;}
		unsigned int					getLimit ( void ) const { return _limit; }

		bool							isOps( Client *tmp ) {
			for ( itStr tmpIt = _chanOps.begin(); tmpIt != _chanOps.end(); ++tmpIt ) {
				if ( *tmpIt == tmp->getUser() )
					return (1);
			}
			return (0);
		}
		void							setOps( Client *tmp ) { _chanOps.push_back( tmp->getUser() ); }
		std::list<std::string> const	*getOps( void ) const { return &_chanOps; }

		bool							isCli( Client *tmp ) {
			for ( clientIterator tmpIt = _cliCrnt.begin(); tmpIt != _cliCrnt.end(); ++tmpIt ) {
				if ( *tmpIt == tmp )
					return (1);
			}
			return (0);
		}

		void							setCli( Client *tmp ) { _cliCrnt.push_back(tmp); }
		std::list<Client *> const		*getCli( void ) const { return &_cliCrnt; }

		bool							isVo( Client *tmp ) {
			for ( itStr tmpIt = _cliVo.begin(); tmpIt != _cliVo.end(); ++tmpIt ){
				if ( *tmpIt == tmp->getUser() )
					return (1);
			}
			return (0);
		}
		void							setVo( Client *tmp ) { _cliVo.push_back( tmp->getUser() ); }
		std::list<std::string>			*getVo( void ) { return &_cliVo; }

		bool							isBan( Client const *tmp ) {
			for ( itBan tmpIt = _chanBan.begin(); tmpIt != _chanBan.end(); ++tmpIt )
			{
				if ( tmpIt->first == tmp->getUser() )
				{
					if (tmpIt->second && tmpIt->second <= std::time(nullptr))
					{
						_chanBan.erase(tmpIt);
						break;
					}
					else
						return 1;
				}
			}
			return (0);
		}

//		bool					isBan (Client *cli)
//		{
//			itBan banIt;
//			for ( banIt = _cliBan.begin(); banIt != _cliBan.end() && banIt->first != cli->getUser(); ++banIt );
//
//			if ( banIt == _cliBan.end() )
//				return 0;
//
//			if ( !banIt->second || banIt->second > std::time(nullptr) )
//				return 1;
//
//			_cliBan.erase( banIt );
//			return 0;
//		}

		void					setBan( Client *tmp , unsigned int nb )
		{
			if (!isBan(tmp))
				_chanBan.push_back( _pairBan(tmp->getUser(), std::time(nullptr) + nb ) );
		}
		std::list<pairBan> const		*getBan( void ) const { return &_chanBan; }
};