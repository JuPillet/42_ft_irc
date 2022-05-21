#pragma once
#include <ctime>
#include "Client.hpp"

typedef std::pair< std::string, time_t >	_pairBan;

class Channel
{
		bool _mod; // is operator only mode enabled
		bool _priv;
		bool _secret;
		bool _extMsg; // bloque ou non les messages externes
		unsigned int _limit; // limite d utilisateurs sur le channel, 0 pour pas de limite.
		const std::string _name;
		std::string _mdp;
		std::list<std::string> _chanOps;
		std::list<Client *> _cli;
		std::list<std::string> _vo;

	public :
		Channel( void ) : _mod (0), _priv (0), _secret(0), _limit (0), _extMsg(0) {}
		Channel( std::string str ) : _name( str ), _mod (0), _priv(0), _secret(0), _limit(0), _extMsg(0) {}
		~Channel( void ) {}
		std::string getName( void ) const { return _name; }
		std::string getPass( void ) const { return _mdp; }
		void		setPass ( std::string str ) { _mdp = str; }
		bool		getPriv( void ) const { return _priv;}
		void		setPriv( int sw ) { _priv = sw; }
		bool		getMod( void ) const { return _mod; }
		void		setMod( int sw ) { _mod = sw; }
		bool		getSecret( void ) const { return _secret; }
		void		setSecret ( int sw ) { _secret = sw; }
		bool		getExt( void ) { return _extMsg; }
		void		setExt ( int sw ) { _extMsg = sw; }
		unsigned int getLimit ( void ) { return _limit; }
		void		setLimit (unsigned int tmp) { _limit = tmp;}

		bool		isOps (Client *tmp) {
			for ( itStr tmpIt = _chanOps.begin(); tmpIt != _chanOps.end(); ++tmpIt ) {
				if ( *tmpIt == tmp->getUser() )
					return (1);
			}
			return (0);
		}
		void		setOps( Client *tmp ) { _chanOps.push_back( tmp->getUser() ); }

		bool		isCli ( Client *tmp ) {
			for ( clientIterator tmpIt = _cli.begin(); tmpIt != _cli.end(); ++tmpIt ) {
				if ( *tmpIt == tmp )
					return (1);
			}
			return (0);
		}
		void		setCli( Client *tmp ) { _cli.push_back(tmp); }

		bool		isVo (Client *tmp) {
			for ( itStr tmpIt = _vo.begin(); tmpIt != _vo.end(); ++tmpIt ){
				if ( *tmpIt == tmp->getUser() )
					return (1);
			}
			return (0);
		}
		void		setVo(Client *tmp) { _vo.push_back( tmp->getUser() ); }
};