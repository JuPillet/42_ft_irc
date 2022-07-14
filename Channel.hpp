#pragma once
#include <cctype>
#include "IRCtypedef.hpp"

class Channel
{
		bool							_mod; // is operator only mode enabled
		bool							_priv;
		bool							_secret;
		bool							_extMsg; // bloque ou non les messages externes
		bool							_invit;
		bool							_protecTopic;
		size_t							_limit; // limite d utilisateurs sur le channel, 0 pour pas de limite.
		const std::string				_name;
		std::string						_owner;
		std::string						_flags;
		std::string						_pass;
		std::string						_topic;
		std::vector<Client *>			_cliCrnt;
		std::list<std::string>			_chanOps;
		std::list<std::string>			_cliVo;
		std::list<std::string>			_guests;
		std::list<_pairBan>				_chanBan;

	public :
										Channel( void ) : _mod( false ), _priv( false ), _secret(false), _extMsg( false ), _invit( false ), _protecTopic( false ), _limit( 0 ), _name(), _flags(), _pass(), _topic(), _cliCrnt(), _chanOps(), _cliVo(), _chanBan()
										{
											_chanOps.clear();
											return;
										}
										Channel( std::string name ) : _mod( false ), _priv( false ), _secret(false), _extMsg( false ), _invit( false ), _protecTopic( false ), _limit( 0 ), _name( name ), _flags(), _pass(), _topic(), _cliCrnt(), _chanOps(), _cliVo(), _chanBan()
										{
											_chanOps.clear();
											return;
										}
										~Channel( void ) { return; }
		std::string						getName( void ) const { return _name; }
		void							setOwner( std::string const &owner ) { _owner = owner; }
		void							delOwner( void ) { _owner.clear(); }
		std::string						getOwner() const { return _owner; }

		std::string						getFlags( void ) { return _flags; }
		void							addFlag( char flag );
		void							delFlag( char flag );
		void							printModifFlags( std::string const _printedFlags ) const;

		void							setPass ( std::string str );
		void							unsetPass ( void );
		std::string						getPass( void ) const { return _pass; }

		void							setPriv( bool priv );
		bool							getPriv( void ) const { return _priv; }

		void							setSecret( bool secret );
		bool							getSecret( void ) const { return _secret; }

		void							setInvit( bool invit );
		bool							getInvit( void ) const { return _invit; }

		void							setMod( bool mod );
		bool							getMod( void ) const { return _mod; }

		void							setExt ( bool extMsg );
		bool							getExt( void ) const { return _extMsg; }

		void							setLimit ( unsigned int limit );
		unsigned int					getLimit ( void ) const { return _limit; }

		void							setProtecTopic ( bool protect );
		bool							getProtecTopic ( void ) const { return _protecTopic; }

		void							setTopic ( std::string topic, std::string servIp, std::string changerNick );
		std::string						getTopic ( void ) { return _topic; }

		strListIt						isOps( std::string nick );
		void							setOps( std::string ops );
		void							unsetOps( std::string nick );
		std::list<std::string> const	&getOps( void ) const { return _chanOps; }

		void							setCli( Client *tmp ) { _cliCrnt.push_back( tmp ); }
		void							eraseCli( std::string nick );
		clientIterator					isCli( std::string nick );
		std::vector<Client *> const		&getCli( void ) const { return _cliCrnt; }

		std::string						getNickList( void );
		std::string						getUserByNick( std::string &nickTarget );

		void							setVo( std::string tmp ) { _cliVo.push_back( tmp ); }
		void							unsetVo( std::string nick );
		strListIt						isVo( std::string nick );
		std::list<std::string>			&getVo( void ) { return _cliVo; }

		void							addGuests( std::string guest );
		void							removeGuests( std::string guest );
		strListIt						isGuest( std::string guest );
		std::list<std::string>			&getGuests( void ) { return _guests; }

		void							setBan( std::string tmp , unsigned int nb );
		void							unBan( std::string tmp );
		itBan							isBan( std::string nick );
		std::list<pairBan> const		&getBan( void ) const { return _chanBan; }

		void							WHO( clientIterator clientIt, std::string &servIP);
};

#include "Client.hpp"