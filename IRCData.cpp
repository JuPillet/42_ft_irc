#include "IRCmsg.hpp"
#include "IRCData.hpp"

IRCData::IRCData( void ): _printedFlags(), _printedArgs(), _mods(), _modsIt(), _servIP(), _port(), _pass(), _listFctU(), _listFctC(), _listFctI(), _opt(), _master_socket(), _addrlen(), _new_socket(), _activity(), _sd(), _readfds(), _writefds(), _crntfds(), _max_sd(), _address(), _request(), _cmd(), _answer(), _target(), _clients(), _servOps(), _clientIt(), _rejectChar(), _channels(), _channelTmp(), _chanPassTmp(), _servBan(), _flop(), _flag()
{
	_clients.clear();
	for ( int i = 0; i < 1024; ++i )
		_buff[i] = 0;
}

IRCData::~IRCData( void ) 
{
	for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
		delete ( *_clientIt );
	_clients.erase( _clients.begin(), _clients.end() );
}

channelIterator				IRCData::isChannel( std::string channelName )
{
	channelIterator chanIt;

	for ( chanIt = _channels.begin(); chanIt != _channels.end() && chanIt->getName() != channelName; ++chanIt );
	return chanIt;
}

void						IRCData::spaceTrimer( void )
{
	strIt	trimIt;

	for ( trimIt = _request->begin(); trimIt != _request->end() && std::isspace( *trimIt ); ++trimIt );
	_request->erase( 0, trimIt - _request->begin() );
}

void						IRCData::clearPostArgs( void )
{
	strIt	postFct;

	for ( postFct = _request->begin(); postFct != _request->end() && *postFct != '\n'; ++postFct );
	_request->erase( 0, postFct + 1 - _request->begin() );
	spaceTrimer();
}

void						IRCData::receveRequest( void ) {
	_request = const_cast<std::string *>( ( *_clientIt )->getRequest() );
	std::cout << _request << std::endl;
	if ( *( --_request->end() ) == '\n' )
		_request->clear();
	int readvalue;
	for ( int index = 0; index != 1024; ++index )
		_buff[index] = 0;
	readvalue = recv( _sd, _buff, 1024, 0 );
	
	if ( readvalue != -1 )
	{
		_request->append( std::string( reinterpret_cast<char *>( _buff ), readvalue ) );
		std::cout << "_sd: " << _sd << " - readvalue: " << readvalue << " : " << _request->length() << " : " << std::endl << *_request << std::endl;
	}
	else
		std::cout << std::strerror( errno ) << std::endl;
}

std::string const			IRCData::getArg( void )
{
	strIt		argIt;
	for ( argIt = _request->begin(); argIt != _request->end()
		&& *argIt != '\n' && *argIt != '\r' && !std::isspace( *argIt ); ++argIt );
	std::string argTmp = std::string( *_request, 0, argIt - _request->begin() );
	_request->erase( 0, argIt - _request->begin() );
	spaceTrimer();
	return argTmp;
}

std::string const			IRCData::getLastArg( void )
{
	strIt		argIt;
	std::string argTmp;
	
	if ( ( *_request )[0] == ':' )
	{
		( *_request ).erase( ( *_request ).begin() );
		for ( argIt = _request->begin(); argIt != _request->end()
			&& *argIt != '\n' && *argIt != '\r'; ++argIt );
	}
	else
	{
		for ( argIt = _request->begin(); argIt != _request->end()
			&& *argIt != '\n' && *argIt != '\r' && !std::isspace( *argIt ); ++argIt );
	}
	argTmp = std::string( *_request, 0, argIt - _request->begin() );
	_request->erase( 0, argIt - _request->begin() );
	clearPostArgs();
	return argTmp;
}

void						IRCData::setCmd( void )
{
	strIt	cmdIt;

	for ( cmdIt = _request->begin(); cmdIt != _request->end()
		&& *cmdIt != '\n' && *cmdIt != '\r' && !std::isspace( *cmdIt ); ++cmdIt );
	_cmd = getArg();
	for ( cmdIt = _cmd.begin(); cmdIt != _cmd.end(); cmdIt++ )
		if ( std::isalpha( *cmdIt ) )
			*cmdIt = std::toupper( *cmdIt );
}

clientIterator				IRCData::isCli( std::string const &userTmp )
{
	clientIterator cliIt;

	for ( cliIt = _clients.begin(); cliIt != _clients.end() && userTmp != ( *cliIt )->getUser(); ++cliIt );
	return cliIt;
}

strListIt					IRCData::isOps( std::string userTmp )
{
	strListIt opsIt;

	for ( opsIt = _servOps.begin(); opsIt != _servOps.end() && *opsIt != userTmp; ++opsIt );
	return opsIt;
}

itBan						IRCData::isBan( std::string const &user )
{
	itBan banIt;

	for ( banIt = _servBan.begin(); banIt != _servBan.end() && banIt->first != user; ++banIt );
	if ( banIt != _servBan.end() && banIt->second && banIt->second <= std::time( nullptr ) )
	{
		_servBan.erase( banIt );
		banIt = _servBan.end();
	}
	return banIt;
}
		
void						IRCData::LIST( void )
{
	for ( channelIterator chanIt = _channels.begin(); chanIt != _channels.end(); ++chanIt )
	{
		if ( !chanIt->getSecret() || chanIt->isCli( ( *_clientIt )->getNick() ) != chanIt->getCli().end() )
		{
			std::string flags = chanIt->getFlags();
			if ( flags.size() )
				flags = "+" +flags;
			_answer = ":*." + _servIP + " 322 " + ( *_clientIt )->getNick() + " " + chanIt->getName() + " 0 :[" + flags + "]";
			if ( !chanIt->getPriv() && chanIt->getTopic().size() )
				_answer +=  " " + chanIt->getTopic();
			sender( _sd, _answer + "\r\n", 0 );
		}
	}
	sender( _sd, ":*." + _servIP + " 323 " + ( *_clientIt )->getNick() + " :End of channel list.\r\n", 0 );
}

void						IRCData::WELCOME( void )
{
	if ( ( *_clientIt )->getPass() == _pass
		&& ( *_clientIt )->getNick().size() && ( *_clientIt )->getUser().size() )
		( *_clientIt )->setAutentification( _servIP, inet_ntoa( _address.sin_addr ) );
}

void				IRCData::CAP( void )
{
	clearPostArgs();
}

void						IRCData::checkPass( void )
{
	if ( ( *_clientIt )->getPass() != _pass )
	{
		std::string	ircErr( ( ( *_clientIt )->getUser() + " Bad password" ) );
		try
		{ sender( _sd, ":*." + _servIP + " 475 " + ( *_clientIt )->getNick() + " " + _target + " :Cannot join channel ( incorrect server key or set pass in first)\r\n", &ircErr ); }
		catch ( std::string const &err )
		{
			closeEraseDeleteClient( _clientIt );
			throw err;
		}
	}
}

void						IRCData::PASS( void )
{
	std::string	passTmp;
	strIt		passIt;

	passTmp = getLastArg();
	std::cout << *_request << std::endl;
	if ( ( *_clientIt )->getAutentification() )
	{
		std::string ircErr( ( *_clientIt )->getUser() + " try a double registration" );
		sender( _sd, ":" + _servIP + " 462 " + " " + ( *_clientIt )->getNick() + " " + ":You may not reregister", &ircErr );
	}
	( *_clientIt )->setPass( passTmp );
	checkPass();
	if ( !( *_clientIt )->getAutentification() )
		WELCOME();
}

void						IRCData::NICK( void )
{
	std::string		nickTmp;
	strIt			nickIt;

	nickTmp.clear();
	if ( !( nickTmp = getLastArg() ).size() )
		sender( ( *_clientIt )->getSocket(), ":*." + _servIP + " 443 :no nick name given\r\n", 0 );
	nickIt = nickTmp.begin();
	if ( !std::isalpha( *nickIt ) )
	{
		std::string ircErr( "Nick format - first char not an alphabetic character" );
		try
		{ sender( _sd, ":*." + _servIP + " 432 " + nickTmp + " :Erroneous Nickname first character not an alphabetic character\r\n", &ircErr ); }
		catch ( std::string err )
		{
			closeEraseDeleteClient( _clientIt );
			throw err;
		}
	}
	for ( ; nickIt != nickTmp.end(); ++nickIt )
	{
		for ( strIt rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt )
		{
			if ( *nickIt == *rejectIt )
			{
				std::string	errLog = "Nick format, nick contain caracter '";
				errLog.push_back( *nickIt );
				errLog.push_back( '\'' );
				std::string ircErr( errLog );
				try
				{ sender( _sd, ":*." + _servIP + " 432 " + nickTmp + " :Erroneous Nickname, nick contain caracter '" + *nickIt + "'\r\n", &ircErr ); }
				catch ( std::string err )
				{
					closeEraseDeleteClient( _clientIt );
					throw err;
				}
			}
		}
	}

	clientIterator	cliIt = _clients.begin();
	for ( ; cliIt != _clients.end(); ++cliIt )
	{
		if ( ( *cliIt )->getNick() == nickTmp )
		{
			std::string ircErr( "Nick already in use" );
			try
			{ sender( _sd, ":*." + _servIP + " 433 * " + nickTmp + " :Nickname already in use\r\n", &ircErr ); }
			catch ( std::string err )
			{
				closeEraseDeleteClient( _clientIt );
				throw std::string( err );
			}
		}
	}
	for ( channelsListIt chanIt = ( *_clientIt )->getChannels().begin(); chanIt != ( *_clientIt )->getChannels().end(); ++chanIt )
	{
		if ( ( *_clientIt )->getNick() == ( *chanIt )->getOwner() )
			( *chanIt )->setOwner( nickTmp );
	}
	( *_clientIt )->setNick( nickTmp );
	checkPass();
	if ( !( *_clientIt )->getAutentification() )
		WELCOME();
}

void						IRCData::USER( void )
{
	std::string	userTmp = getArg(), modeTmp = getArg(), hostTmp = getArg(), nameTmp = getLastArg();
	strIt		userIt;

	if ( *_request->begin() == ':' )
	{
		_request->erase( _request->begin() );
		while ( _request->begin() != _request->end() && *_request->begin() != '\r' && *_request->begin() != '\n' )
			nameTmp += ( getArg() + ' ' );
		nameTmp.pop_back();
	}
	clearPostArgs();
	if ( !( userTmp.size() && modeTmp.size() && hostTmp.size() && nameTmp.size() ) )
	{
		std::string ircErr( "USER format" ) ;
		try
		{ sender( _sd, ":USER command format : USER <username> <mode> <host> <:fullname>\r\n", &ircErr ); }
		catch ( std::string err )
		{
			closeEraseDeleteClient( _clientIt );
			throw err;
		}
	}
	if ( isBan( userTmp ) != _servBan.end() )
	{
		std::string ircErr( "User " + ( *_clientIt )->getUser() + " tried to connect during his banish time." );
		try
		{ sender( _sd, ":*." + _servIP + " 466 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :You are banned from this server", &ircErr ); }
		catch ( std::string err )
		{
			closeEraseDeleteClient( _clientIt );
			throw err;
		}
	}
	( *_clientIt )->setUser( userTmp );
	checkPass();
	if ( !( ( *_clientIt )->getAutentification() ) )
		WELCOME();
}

void						IRCData::PONG( void )
{
	clearPostArgs();
	sender( _sd, ":" + _servIP + " PONG " + _servIP + " :" + _servIP + "\r\n", 0 );
}

void						IRCData::JOIN( void )
{
	_channelTmp = getArg();

	if ( _channelTmp == "-invite" )
		_channelTmp = ( *_clientIt )->getInvitation();
	if ( *_channelTmp.begin() != '#' )
		_channelTmp = "#" + _channelTmp;
	_chanPassTmp = getLastArg();
	channelIterator	chanIt;
	if ( ( chanIt = isChannel( _channelTmp ) ) == _channels.end() )
	{
		_channels.push_back( _channelTmp );
		chanIt = --_channels.end();
		chanIt->setOwner( ( *_clientIt )->getNick() );
		chanIt->setOps( ( *_clientIt )->getNick() );
	}
	if ( ( chanIt->isBan( ( *_clientIt )->getUser() ) ) != chanIt->getBan().end() )
	{
		std::string ircErr( ( *_clientIt )->getNick() + " banned from " + _target );
		sender( _sd, ":*." + _servIP + " 474 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :you are banned from channel\r\n", &ircErr );
	}
	if ( chanIt->isCli( ( *_clientIt )->getNick() ) != chanIt->getCli().end() )
	{ throw ( std::string("already in channel") ); }
	if ( chanIt->getInvit() && chanIt->isGuest( ( *_clientIt )->getNick() ) == chanIt->getGuests().end() )
	{
		std::string ircErr( "isn't invited" );
		sender( _sd, ":*." + _servIP + " 473 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :Cannot join channel ( invite only )\r\n", &ircErr );
	}
	if ( chanIt->getPass().size() && chanIt->getPass() != _chanPassTmp )
	{
		std::string ircErr( "bad password" );
		sender( _sd, ":*." + _servIP + " 475 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :Cannot join channel ( incorrect channel key )\r\n", &ircErr );
	}
	if ( chanIt->getLimit() && chanIt->getLimit() == chanIt->getCli().size() )
	{
		std::string ircErr( "limit channel full" );
		sender( _sd, ":*." + _servIP + " 471 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :Cannot join channel ( channel is full )\r\n", &ircErr );
	}
	( *_clientIt )->addChannel( _servIP, &( *chanIt ) );
}

std::list<std::string>		IRCData::KBList( const std::string& s )
{
	std::list<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while ( std::getline( tokenStream, token, ',' ) )
		tokens.push_back( token );
	return tokens;
}

void						IRCData::KICK( channelIterator chanIt, std::list<std::string> kbList, std::string reason)
{
	strListIt KBIt;
	for ( KBIt = kbList.begin(); KBIt != kbList.end(); ++KBIt )
	{
		std::string KBstr = *KBIt;
		clientIterator userIt = chanIt->isCli( KBstr );
		try
		{
			if ( userIt == chanIt->getCli().end() )
			{
				kbList.erase( KBIt-- );
				std::string ircErr( "KICK: nick target inexistant in channel" );
				sender( _sd, ":*." + _servIP + " 401 " + ( *_clientIt )->getNick() + " " + KBstr + " :No such nick\r\n", &ircErr );
			}
			for ( constClientIterator chanCliIt = chanIt->getCli().begin(); chanCliIt != chanIt->getCli().end(); ++chanCliIt )
			{
				try
				{ sender( ( *chanCliIt )->getSocket(), ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _target + " " + KBstr + reason + "\r\n", 0 ); }
				catch ( std::string const &err )
				{ std::cerr << err << std::endl; }
			}
			( *userIt )->removeInChannel( chanIt );
		}
		catch ( std::string err )
		{ std::cout << err << std::endl; }
	}
}

void						IRCData::BAN( channelIterator chanIt, std::list<std::string> kbList, std::string reason )
{
	strListIt KBIt;
	for ( KBIt = kbList.begin(); KBIt != kbList.end(); ++KBIt )
	{
		std::string KBstr = *KBIt;
		if ( KBstr[0] == '*' && KBstr[1] == '!' && KBstr[2] == '*')
			KBstr = KBstr.substr(3, KBstr.find('@'));
		clientIterator userIt = isCli( KBstr );
		try
		{
			if ( userIt == _clients.end() )
			{
				std::string ircErr( "BAN: nick target inexistant in channel" );
				sender( _sd, ":*." + _servIP + " 401 " + ( *_clientIt )->getNick() + " " + KBstr + " :No such nick\r\n", &ircErr );
			}
			if ( _cmd == "BAN" )
			{
				for ( constClientIterator chanCliIt = chanIt->getCli().begin(); chanCliIt != chanIt->getCli().end(); ++chanCliIt )
				{
					try
					{ sender( ( *chanCliIt )->getSocket(), ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _target + " " + KBstr + reason + "\r\n", 0 ); }
					catch ( std::string const &err )
					{ std::cerr << err << std::endl; }
				}
			}
			if ( chanIt->isBan( KBstr ) == chanIt->getBan().end() )
				chanIt->setBan( KBstr, 0 );
			chanIt->setBan( KBstr, 0 );
		}
		catch ( std::string err )
		{ std::cout << err << std::endl; }
	}
}

void						IRCData::KICKBAN( void )
{
	_target = getArg();
	std::list<std::string> kbList = KBList( getArg() );
	std::string reason;
	if ( ( reason = " :" + getLastArg() ) == " :" )
		reason.clear();
	channelIterator chanIt = isChannel( _target );
	if ( chanIt == _channels.end() )
	{
		std::string ircErr( _cmd + " : channel inexistant" );
		sender( _sd, ":*." + _servIP + " 403 " + ( *_clientIt )->getNick() + " " + _target + " :No such channel\r\n", &ircErr );
	}
	if ( chanIt->isOps( ( *_clientIt )->getNick() ) == chanIt->getOps().end() )
	{
		std::string ircErr( "Not channel operator" );
		sender( ( *_clientIt )->getSocket(), ":*." + _servIP + " 481 " + ( *_clientIt )->getNick() + " :You must have channel op access or above to ban someone\r\n", &ircErr );
	}
	if ( !kbList.size() || ( *kbList.begin() ).front() == ':' )
	{
		std::string ircErr( _cmd + "Not enough parameters given" );
		sender( _sd, ":*." + _servIP + " 461 " + ( *_clientIt )->getNick() + " " + _cmd + " " + _target + " :Not enough parameters given\r\n", &ircErr );
	}
	if ( _cmd == "KICK" || _cmd == "KICKBAN" )
		KICK( chanIt, kbList, reason );
	if ( _cmd == "BAN" || _cmd == "KICKBAN" )
		BAN( chanIt, kbList, reason );
}

void						IRCData::KILL( void )
{
	std::string		_target = getArg();
	std::string		reason;
	strIt			argIt;
	clientIterator	kickIt;
	if ( ( reason = " :" + getLastArg() ) == " :" )
		reason.clear();
	if ( isOps( ( *_clientIt )->getUser() ) == _servOps.end() )
	{
		_request->clear();
		std::string ircErr( "Not server operator" );
		sender( ( *_clientIt )->getSocket(), ":*." + _servIP + " 481 " + ( *_clientIt )->getNick() + " :You must have channel op access or above to kick some one\r\n", &ircErr );
	}
	kickIt = isCli( _target );
	if ( kickIt == _clients.end() )
	{
		std::string ircErr( "KICK: nick target inexistant in channel" );
		sender( _sd, ":*." + _servIP + " 401 " + ( *_clientIt )->getNick() + " " + _target + " :No such nick\r\n", &ircErr );
	}
	closeEraseDeleteClient( kickIt );
}

void						IRCData::OPENMSG( void )
{
	channelIterator	chanIt = isChannel( _target );
	std::string		privMsg = getLastArg();

	if ( chanIt == _channels.end() )
	{
		std::string ircErr( "No such channel" );
		sender( _sd, ":*." + _servIP + " 403 " + ( *_clientIt )->getNick() + " " + _target + " :No such channel\r\n", &ircErr );
	}
	if ( ( chanIt->isBan( ( *_clientIt )->getUser() ) ) != chanIt->getBan().end() )
	{
		std::string ircErr( ( *_clientIt )->getNick() + " banned from " + _target );
		sender( _sd, ":*." + _servIP + " 474 " + ( *_clientIt )->getNick() + " " + _target + " :you are banned from channel\r\n", &ircErr );
	}
	if ( chanIt->getExt() && chanIt->isCli( ( *_clientIt )->getUser() ) == chanIt->getCli().end() )
	{
		std::string ircErr( "PRIVMSG - Channel " + _target + " external restriction" );
		sender( _sd, ":*." + _servIP + " 404 " + ( *_clientIt )->getNick() + " " + _target + " You cannot send messages to this channel whilst the +n ( noextmsg ) mode is set.\r\n", &ircErr );
	}
	if ( chanIt->getMod()
		&& chanIt->isOps( ( *_clientIt )->getUser() ) == chanIt->getOps().end()
		&& chanIt->isVo( ( *_clientIt )->getUser() ) == chanIt->getVo().end() )
	{
		std::string ircErr( "PRIVMSG - Channel " + _target + " moderation restriction" );
		sender( _sd, ":*." + _servIP + " 404 " + ( *_clientIt )->getNick() + " " + _target + " You cannot send messages to this channel whilst the +m ( moderated ) mode is set.\r\n", &ircErr );
	}

	_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _target + " " + privMsg + "\r\n"; // A voir formatage pour envoyer un message
	constClientIterator	userIt;
	for ( userIt = chanIt->getCli().cbegin() ; userIt != chanIt->getCli().cend(); ++userIt )
	{
		if ( ( *userIt )->getNick() != ( *_clientIt )->getNick() )
		{
			try
			{ sender( ( *userIt )->getSocket(), _answer, 0 ); } //essaye d envoyer le message a l utilisateur
			catch( std::string const &err )
			{ std::cerr << err << std::endl; } //affiche ici si le message n a pas ete envoye a un utilisateur
		}
	}
}



void						IRCData::PRIVMSG( void )
{
	std::string		privMsg = getLastArg();
	clientIterator	userIt = isCli( _target );

	if ( privMsg == " :" )
	{
		std::string ircErr( _cmd + "Not enough parameters given" );
		sender( _sd, ":*." + _servIP + " 461 " + ( *_clientIt )->getNick() + " " + _cmd + " " + _target + " :Not enough parameters given\r\n", &ircErr );
	}
	if ( userIt != _clients.end() )
		sender( ( *userIt )->getSocket(), ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _target + " :" + privMsg + "\r\n", 0 );
}

void						IRCData::MSG( void )
{
	_target = getArg();
	if ( ( *_request )[0] != ':' )
		*_request = ":" + *_request;
	if ( _target[0] == '#' )
		OPENMSG();
	else
		PRIVMSG();
}

void						IRCData::PART( void )
{
	_target = getLastArg();
	channelIterator chanIt = isChannel( _target );
	if ( chanIt == _channels.end() )
	{
		std::string ircErr( "Channel doesnt exist." );
		sender( _sd, ":*." + _servIP + " 403 " + ( *_clientIt )->getNick() + " " + _target + " :No such channel\r\n", &ircErr );
	}
	try
	{
		if ( !( *_clientIt )->isInChannel( _target ) )
		{
			std::string ircErr( "Client not on the channel." );
			sender( ( *_clientIt )->getSocket(), ":*." + _servIP + " 442 " + ( *_clientIt )->getNick() + " " + _target + " :You're not on that channel\r\n", &ircErr );
		}
		( *_clientIt )->removeInChannel( chanIt );
		sender( _sd, ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " PART " + _target + " :left away\r\n", 0 );
		if ( chanIt->getCli().size() )
		{
			for ( constClientIterator chanCliIt = chanIt->getCli().begin(); chanCliIt != chanIt->getCli().end(); ++chanCliIt )
			{
				try
				{ sender( ( *chanCliIt )->getSocket(), ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " PART " + _target + " :left away\r\n", 0 ); }
				catch ( std::string const &err )
				{ std::cerr << err << std::endl; }
			}
		}
		else
		{
			std::cout << chanIt->getCli().size() << std::endl;
			_channels.erase( chanIt );
		}
	}
	catch ( std::string const &err )
	{ throw( err ); }
}

void						IRCData::QUIT( void )
{
	std::string		quitMsg = getLastArg();
	if ( !quitMsg.size() )
		quitMsg = "left away";
	for ( channelsListIt chanIt = ( *_clientIt )->getChannels().begin(); chanIt != ( *_clientIt )->getChannels().end(); ++chanIt )
	{
		for ( constClientIterator chanCliIt = ( *chanIt )->getCli().begin(); chanCliIt != ( *chanIt )->getCli().end(); ++chanCliIt )
		{
			try
			{ sender( ( *chanCliIt )->getSocket(), ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " PART " + _target + " :" + quitMsg + "\r\n", 0 ); }
			catch ( std::string const &err )
			{ std::cerr << err << std::endl; }
		}
	}
	closeEraseDeleteClient( _clientIt );
}

void						IRCData::TOPIC( void )
{
	_target = getArg();
	std::string topic = getLastArg();
	channelIterator channelIt = isChannel( _target );

	if ( channelIt == _channels.end() )
	{
		std::string ircErr( "TOPIC: channel inexistant" );
		sender( _sd, ":*." + _servIP + " 403 " + ( *_clientIt )->getNick() + " " + _target + " :No such channel\r\n", &ircErr );
	}
	if ( topic.size() && topic != " :" )
	{
		if ( channelIt->getProtecTopic() && channelIt->isOps( ( *_clientIt )->getNick() ) == channelIt->getOps().end() )
		{
			_request->clear();
			std::string ircErr( ( *_clientIt )->getNick() + " not '" + _channelTmp + "' channel operator" );
			sender( _sd, ":*." + _servIP + " 482 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :You must be a channel half-operator\r\n", &ircErr );
		}
		channelIt->setTopic( topic, _servIP, ( *_clientIt )->getNick() );
	}
	else
		sender( _sd, ":*." + _servIP + " 332 " + ( *_clientIt )->getNick() + " " + _target + " " + channelIt->getTopic() + "\r\n", 0 );
}

void						IRCData::INVITE( void )
{
	std::string user = ( *_clientIt )->getNick();
	_target = getArg();
	clientIterator cliTarget = isCli( _target );
	_channelTmp = getLastArg();
	channelIterator channel = isChannel( _channelTmp );
	if ( cliTarget == _clients.end() )
	{
		_request->clear();
		std::string ircErr( _target + " not server client" );
		sender( _sd, ":*." + _servIP + " 401 " + ( *_clientIt )->getNick() + " " + _target + " :No such nick\r\n", &ircErr );
	}
	if ( channel == _channels.end() )
	{
		std::string ircErr( "TOPIC: channel inexistant" );
		sender( _sd, ":*." + _servIP + " 403 " + ( *_clientIt )->getNick() + " " + _target + " :No such channel\r\n", &ircErr );
	}
	if ( channel->isOps( user ) == channel->getOps().end() )
	{
		_request->clear();
		std::string ircErr( user + " not '" + _channelTmp + "' channel operator" );
		sender( _sd, ":*." + _servIP + " 482 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :You must be a channel half-operator\r\n", &ircErr );
	}
	if ( channel->isCli( _target ) != channel->getCli().end() )
	{
		_request->clear();
		std::string ircErr( _target + " allready on '" + _channelTmp + "' channel" );
		sender( _sd, ":*." + _servIP + " 443 " + user + " " + _target + " :" + _channelTmp + " :is already on channel\r\n", &ircErr );
	}
	channel->addGuests( _target );
	sender( ( *cliTarget )->getSocket(), ":" + _target + "!~" + ( *cliTarget )->getUser() + "@" + ( *cliTarget )->getClIp() + " " + _cmd + " " + ( *cliTarget )->getUser() + " :" + _channelTmp + "\r\n", 0 );
	( *cliTarget )->setInvitation( _channelTmp );
	sender( _sd, ":*." + _servIP + " 341 " + user + " " + _target + " :" + _channelTmp + "\r\n", 0 );
}

void						IRCData::setAddress( void )
{
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons( _port );
}

void						IRCData::closeEraseDeleteClient( clientIterator clientIt )
{
	_request->clear();
	if ( FD_ISSET( ( *clientIt )->getSocket(), &_crntfds ) )
		FD_CLR( ( *clientIt )->getSocket(), &_crntfds );
	if ( FD_ISSET( ( *clientIt )->getSocket(), &_readfds ) )
		FD_CLR( ( *clientIt )->getSocket(), &_readfds );
	if ( FD_ISSET( ( *clientIt )->getSocket(), &_writefds ) )
		FD_CLR( ( *clientIt )->getSocket(), &_writefds );
	//Close the socket and mark as 0 in list for reuse
	( *clientIt )->removeInAllChannel();
	for( channelIterator chanIt = _channels.begin(); chanIt != _channels.end(); ++chanIt )
	{
		if ( !chanIt->getCli().size() )
			_channels.erase( chanIt-- );
	}
	delete ( *clientIt );
	if ( clientIt <= _clientIt )
		_clientIt--;
	_clients.erase( clientIt );
}

int							IRCData::getMasterSocket( void ) const { return _master_socket; }
fd_set	const				IRCData::getCrntFds( void ) const { return _crntfds; }
fd_set	const				*IRCData::getPtrCrntFds( void ) const { return &_crntfds; }
fd_set	const				IRCData::getReadFds( void ) const { return _readfds; }
fd_set	const				*IRCData::getPtrReadFds( void ) const { return &_readfds; }
fd_set	const				IRCData::getWriteFds( void ) const { return _writefds; }
fd_set	const				*IRCData::getPtrWriteFds( void ) const { return &_writefds; }
struct sockaddr_in const	&IRCData::getAddress( void ) const { return _address; }

void						IRCData::nbArgs( const int ac )
{
	if( ac != 3 )
		throw( std::string( "Server need 2 arguments : port and password." ) );
}
void						IRCData::MODE_GET_ARG( void )
{
	bool minus = _modsIt->flop == '-' && ( _modsIt->flag == 'o' || _modsIt->flag == 'b' || _modsIt->flag == 'k' || _modsIt->flag == 'v' );
	if ( _modsIt->flop == '+' || minus )
	{
		std::string arg = getArg();
		_modsIt->arg = arg;
	}		
}

void						IRCData::initFct( void )
{
	////	listPtrFctnIRC
	_listFctI.push_back( pairKVI( "CAP", &IRCData::CAP ) );
	_listFctI.push_back( pairKVI( "PASS", &IRCData::PASS ) );
	_listFctI.push_back( pairKVI( "NICK", &IRCData::NICK ) );
	_listFctI.push_back( pairKVI( "USER", &IRCData::USER ) );
	_listFctI.push_back( pairKVI( "WHO", &IRCData::WHO ) );
	_listFctI.push_back( pairKVI( "PING", &IRCData::PONG ) );
	_listFctI.push_back( pairKVI( "JOIN", &IRCData::JOIN ) );
	_listFctI.push_back( pairKVI( "PRIVMSG", &IRCData::MSG ) );
	_listFctI.push_back( pairKVI( "NOTICE", &IRCData::MSG ) );
	_listFctI.push_back( pairKVI( "INVITE", &IRCData::INVITE ) );
	_listFctI.push_back( pairKVI( "LIST", &IRCData::LIST ) );
	_listFctI.push_back( pairKVI( "BAN", &IRCData::KICKBAN ) );
	_listFctI.push_back( pairKVI( "KICK", &IRCData::KICKBAN ) );
	_listFctI.push_back( pairKVI( "KICKBAN", &IRCData::KICKBAN ) );
	_listFctI.push_back( pairKVI( "KILL", &IRCData::KILL ) );
	_listFctI.push_back( pairKVI( "PART", &IRCData::PART ) );
	_listFctI.push_back( pairKVI( "TOPIC", &IRCData::TOPIC ) );
	_listFctI.push_back( pairKVI( "QUIT", &IRCData::QUIT ) );
	_listFctI.push_back( pairKVI( "MODE", &IRCData::MODE ) );
	////	listPtrFctnModeChannel
	_listFctC.push_back( pairKVM( 'o', pairFctsM( &IRCData::C_MODE_O, &IRCData::MODE_GET_ARG ) ) ); // ADD operator
	_listFctC.push_back( pairKVM( 'p', pairFctsM( &IRCData::C_MODE_P, 0 ) ) );						// Hide Topic.
	_listFctC.push_back( pairKVM( 's', pairFctsM( &IRCData::C_MODE_S, 0 ) ) );						// Hide channel.
	_listFctC.push_back( pairKVM( 'i', pairFctsM( &IRCData::C_MODE_I, 0 ) ) );						// Invite only.
	_listFctC.push_back( pairKVM( 't', pairFctsM( &IRCData::C_MODE_T, 0 ) ) );						// Topic modif. only by ops.
	_listFctC.push_back( pairKVM( 'n', pairFctsM( &IRCData::C_MODE_N, 0 ) ) );						// No external message.
	_listFctC.push_back( pairKVM( 'm', pairFctsM( &IRCData::C_MODE_M, 0 ) ) );						// moderated channel
	_listFctC.push_back( pairKVM( 'l', pairFctsM( &IRCData::C_MODE_L, &IRCData::MODE_GET_ARG ) ) );	// user limit.
	_listFctC.push_back( pairKVM( 'b', pairFctsM( &IRCData::C_MODE_B, &IRCData::MODE_GET_ARG ) ) ); // display banned list or ban target 
	_listFctC.push_back( pairKVM( 'v', pairFctsM( &IRCData::C_MODE_V, &IRCData::MODE_GET_ARG ) ) ); // add target to voice authorization on modded channel
	_listFctC.push_back( pairKVM( 'k', pairFctsM( &IRCData::C_MODE_K, &IRCData::MODE_GET_ARG ) ) ); // define a password.
	////	listPtrFctnModeUser
	_listFctU.push_back( pairKVM( 'o', pairFctsM( &IRCData::U_MODE_O, 0 ) ) ); // add server ops.
}

void						IRCData::init( std::string port, std::string password, char **ep )
{
	size_t			lastchar;
	char			selfhost[256];
	struct hostent	*host_entry;

	signal( SIGPIPE, SIG_IGN );
	_clients.clear();
	_opt = 1;
	_rejectChar = "~!@#$%&*()+=:;\"\',<.>?/";
	_port = IRC::stoi( port, &lastchar );
	if ( port[lastchar] || _port < 0 || _port > 65535 )
		throw( std::string( "Bad port value : enter port to 0 at 65 535" ) );
	_pass = password;
	int	var;
	for ( var = 0; ep[var] && std::string( ep[var] ).compare( 0, 5, "USER=" ) ; ++var );
	if ( !ep[var] )
		throw ( std::string( "no user found for serverOPs" ) );
	_servOps.push_back( ep[var] + 5 );
	if ( ( _master_socket = socket( AF_INET, SOCK_STREAM, 0 ) ) == 0 )
		throw( std::string( "socket failed" ) );
	if ( setsockopt( _master_socket, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<char *>( &_opt ), sizeof( _opt ) ) < 0 )
		throw( std::string( "setsock_opt" ) );
	setAddress();
	if ( bind( _master_socket, reinterpret_cast<struct sockaddr *>( &_address ), sizeof( _address ) ) < 0 )
		throw( std::string( "bind failed" ) );
	_addrlen = sizeof( _address );
	if ( listen( _master_socket, 0 ) < 0 )
	{
		std::cout << std::string( "listen" ) << std::endl;
		exit( EXIT_FAILURE );
	}
	FD_ZERO( &_crntfds );
	FD_SET( _master_socket, &_crntfds );
	gethostname( selfhost, sizeof( selfhost ) );
	host_entry = gethostbyname( selfhost );
	_servIP = inet_ntoa( *( reinterpret_cast<struct in_addr*>( host_entry->h_addr_list[0] ) ) );
	close( 4 );
	close( 5 );
	std::cout << _servIP << std::endl;
	initFct();
	std::cout << "Waiting for connections ..." << std::endl;
}

void						IRCData::addClearedMasterSocket( void )
{
	_max_sd = _master_socket;
	for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
	{
		_sd = ( *_clientIt )->getSocket();
		if( _sd > _max_sd )
			_max_sd = _sd;
	}
}

void						IRCData::activityListener( void )
{
	_readfds = _writefds = _crntfds;
	_activity = select( _max_sd + _clients.size() + 1, &_readfds, &_writefds, NULL, NULL );
	if ( ( _activity < 0 ) )  
		throw std::string( "select error" );
}

void						IRCData::execFct( void )
{
	listPairI::iterator	_listPairIt;

	if ( !( *_clientIt )->getAutentification() && _cmd != "PING" && _cmd != "PONG" && _cmd != "CAP" && _cmd != "PASS" && _cmd != "NICK" && _cmd != "USER" )
	{
		_request->clear();
		std::string ircErr( "Client try to use other command than PASS, NICK or USER without authentification\r\n" );
		sender( _sd, ":*." + _servIP + " 451 " + ( *_clientIt )->getNick() + " :Not register\r\n", &ircErr );
	}
	for ( _listPairIt = _listFctI.begin(); _listPairIt != _listFctI.end() && _listPairIt->first != _cmd; ++_listPairIt );
	if ( _listPairIt != _listFctI.end() )
	{
		ptrFct ptrFct = _listPairIt->second;
		( this->*ptrFct )();
	}
	else
		clearPostArgs();
}

void						IRCData::newClient( void )
{
	if ( ( _new_socket = accept( _master_socket, reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ) ) < 0 )
		throw std::string( "accept" );
	FD_SET( _new_socket, &_crntfds );
	_clients.push_back( new Client( _new_socket, inet_ntoa( _address.sin_addr ) ) );
	std::cout << "New connection, socket fd is " << _new_socket << ", ip is : " << inet_ntoa( _address.sin_addr ) << ", port : " << ntohs( _address.sin_port ) << std::endl;
}


bool FD_IS_ANY_SET(fd_set const *fdset)
{
    static fd_set empty;     // initialized to 0 -> empty
    return memcmp(fdset, &empty, sizeof(fd_set)) != 0;
}

void						IRCData::IOListener( void )
{
	for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
	{
		try
		{
			if ( FD_ISSET( ( *_clientIt )->getSocket(), &_readfds ) )
			{
				_sd = ( *_clientIt )->getSocket();
				//Check if it was for closing, and also read the 
				//incoming message
				receveRequest();
				if ( _request->length() <= 0 )
					closeEraseDeleteClient( _clientIt );
				else if ( *( _request->end() - 1 ) == '\n' )
				{
					spaceTrimer();
					while ( _request->size() )
					{
						setCmd();
						execFct();
					}
				}
			}
		}
		catch ( std::string const &err )
		{
			_request->clear();
			std::cerr << err << std::endl;
		}
	}
}

void	IRCData::printNotChanOps( void )
{
	_request->clear();
	std::string ircErr( "Not channel operator" );
	sender( ( *_clientIt )->getSocket(), ":*." + _servIP + " 482 " + ( *_clientIt )->getNick() + " " + _target + " :You must have channel op access or above to set channel mode\r\n", &ircErr );
}

void	IRCData::printNotServOps( void )
{
	_request->clear();
	std::string ircErr( "Not channel operator" );
	sender( ( *_clientIt )->getSocket(), ":*." + _servIP + " 481 " + ( *_clientIt )->getNick() + " :You must have server op access or above to kick some one\r\n", &ircErr );
}

void						IRCData::U_MODE_O( void )
{
	if ( isOps( ( *_clientIt )->getUser() ) == _servOps.end() )
		printNotServOps();

	if ( !_target.size() )
	{
		std::string ircErr( "MODE user +o targeted user forgotten" );
		sender( _sd, ":" + _servIP + " 461 " + ( *_clientIt )->getNick()  + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " MODE : MODE user +o targetted user forgotten\r\n", &ircErr );
	}
	if ( isCli( _target ) == _clients.end() )
	{
		std::string ircErr( "No such nick " + _target );
		sender( _sd, ":*." + _servIP + " 401 " + ( *_clientIt )->getNick() + " " + _target + " : No such nick\r\n", &ircErr );
	}
	strListIt opsTarget = isOps( _target );
	if ( ( _modsIt->flop == '+' && opsTarget == _servOps.end() ) || ( _modsIt->flop == '-' && opsTarget != _servOps.end() ) )
	{
		if ( _modsIt->flop == '+' )
			_servOps.push_back( _target );
		else
			_servOps.erase( opsTarget );
		_printedFlags += " " + _target;
	}
}

void						IRCData::C_MODE_L( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	if ( _modsIt->flop == '+' )
	{
		if ( !_modsIt->arg.size() )
			_modsIt->arg = "*";
		size_t limit = IRC::stol( _modsIt->arg );
		if ( !limit )
		{
			std::string ircErr( " :Invalid limit mode parameter" );
			if ( _modsIt->arg[0] < '0' || _modsIt->arg[0] < '0' )
				sender( _sd, ":*." + _servIP + " 696 " + ( *_clientIt )->getNick() + " " + _target + " l " + _modsIt->arg + " : Invalid limit mode parameter. Syntax: <limit>.\r\n", &ircErr );
			sender( _sd, ":*." + _servIP + " 696 " + ( *_clientIt )->getNick() + " " + _target + " l 0 : Invalid limit mode parameter. Syntax: <limit>.\r\n", &ircErr );
		}
		_printedArgs += " " + IRC::ultostr( limit );
		_modsIt->chanIt->setLimit( limit );
	}
	else
		_modsIt->chanIt->setLimit( false );
}

void						IRCData::C_MODE_B( void )
{
	//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
	if ( _modsIt->arg[0] == '*' && _modsIt->arg[1] == '!' && _modsIt->arg[2] == '*')
	{
		_modsIt->arg.erase( 0, 3 );
		strIt subIt;
		for ( subIt = _modsIt->arg.begin(); subIt != _modsIt->arg.end() && *subIt != '@'; ++subIt );
		_modsIt->arg.erase( subIt, _modsIt->arg.end() );
	}
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() && _modsIt->arg.size() )
		printNotChanOps();
	if ( !_modsIt->arg.size() )
	{
		std::list<pairBan> const	&channelBan = _modsIt->chanIt->getBan();
		for ( std::list<pairBan>::const_iterator chanBanIt = channelBan.begin(); chanBanIt != channelBan.end(); ++chanBanIt )
			sender( _sd, ":*." + _servIP + " 367 " + ( *_clientIt )->getNick() + " " + _target + " " + chanBanIt->first + "!*@*\r\n", 0 );
		sender( _sd, ":*." + _servIP + " 368 " + ( *_clientIt )->getNick() + " " + _target + " :End of channel ban list\r\n", 0 );
	}
	else
	{
		/*itBan opsTarget =*/ _modsIt->chanIt->isBan( _target );
		if ( _modsIt->flop == '+' )
			_modsIt->chanIt->setBan( _modsIt->arg, 0 );
		else
			_modsIt->chanIt->unBan( _modsIt->arg );
		_printedArgs += " " + _modsIt->arg + "!*@*";
	}
}

void						IRCData::C_MODE_K( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	if ( _modsIt->flop == '-' )
		_modsIt->arg = _modsIt->chanIt->getPass();
	if ( !_modsIt->arg.size() )
	{
		std::string ircErr( ( *_clientIt )->getUser() + " forget argument for channel mode " + _modsIt->flop + "k" );
		sender( _sd, ":*." + _servIP + " 696 " + ( *_clientIt )->getNick() + _target + " k * :You must specify a parameter for the key mode. Syntax: <key>.\r\n", &ircErr );
	}
	if ( _modsIt->flop == '-' )
		_modsIt->chanIt->unsetPass();
	else
	{
		if ( _modsIt->chanIt->getPass().size() )
		{
			_answer = ":*." + _servIP + " 467 " + ( *_clientIt )->getNick() + _target + " k * :Channel key already set.\r\n";
			std::string ircErr( ( *_clientIt )->getUser() + " password already exist for channel mode " + _modsIt->flop + "k" );
			sender( _sd, _answer = ":*." + _servIP + " 467 " + ( *_clientIt )->getNick() + _target + " k * :Channel key already set.\r\n", &ircErr );
		}
		_modsIt->chanIt->setPass( _modsIt->arg );
	}
	_printedArgs += " " + _modsIt->arg + "!*@*";
}

void						IRCData::C_MODE_M( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	bool mode = ( _modsIt->flop == '+' );
	if ( _modsIt->chanIt->getMod() != mode )
		_modsIt->flop == '+' ? _modsIt->chanIt->setMod( true ) : _modsIt->chanIt->setMod( false );
}

void						IRCData::C_MODE_T( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	bool mode = ( _modsIt->flop == '+' );
	if ( _modsIt->chanIt->getProtecTopic() != mode )
		_modsIt->flop == '+' ? _modsIt->chanIt->setProtecTopic( true ) : _modsIt->chanIt->setProtecTopic( false );
}

void						IRCData::C_MODE_N( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	bool mode = ( _modsIt->flop == '+' );
	if ( _modsIt->chanIt->getExt() != mode )
		_modsIt->flop == '+' ? _modsIt->chanIt->setExt( true ) : _modsIt->chanIt->setExt( false );
}

void						IRCData::C_MODE_O( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	strIt	  strIt;
	//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
	if ( !_modsIt->arg.size() )
	{
		std::string ircErr (( *_clientIt )->getUser() + " forgotten argument <nick> for channel mode o");
		sender( _sd, ":*." + _servIP + " 696 " + ( *_clientIt )->getNick() + _target + " o * :You must specify a parameter for the op mode. Syntax: <nick>.\r\n", &ircErr );
	}
	if ( isCli( _modsIt->arg ) == _clients.end() )
	{
		std::string ircErr( "No such nick " + _target );
		sender( _sd, ":*." + _servIP + " 401 " + ( *_clientIt )->getNick() + " " + _target + " : No such nick\r\n", &ircErr );
	}
	if ( _modsIt->chanIt->isCli( _modsIt->arg ) == _modsIt->chanIt->getCli().end() )
	{
		std::string ircErr( ( *_clientIt )->getUser() + " not on the channel" );
		sender( ( *_clientIt )->getSocket(), ":*." + _servIP + " 442 " + ( *_clientIt )->getNick() + " " + _target + " :You're not on that channel\r\n", &ircErr );
	}
	_modsIt->flop == '+' ? _modsIt->chanIt->setOps( _modsIt->arg ) : _modsIt->chanIt->unsetOps( _modsIt->arg );
	_printedArgs += " " + _modsIt->arg + "!*@*";
}

void						IRCData::C_MODE_P( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	bool mode = ( _modsIt->flop == '+' );
	if ( _modsIt->chanIt->getPriv() != mode )
		_modsIt->flop == '+' ? _modsIt->chanIt->setPriv( true ) : _modsIt->chanIt->setPriv( false );
}

void						IRCData::C_MODE_S( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	bool mode = ( _modsIt->flop == '+' );
	if ( _modsIt->chanIt->getSecret() != mode )
		_modsIt->flop == '+' ? _modsIt->chanIt->setSecret( true ) : _modsIt->chanIt->setSecret( false );
}

void						IRCData::C_MODE_I( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	bool mode = ( _modsIt->flop == '+' );
	if ( _modsIt->chanIt->getInvit() != mode )
		_modsIt->flop == '+' ? _modsIt->chanIt->setInvit( true ) : _modsIt->chanIt->setInvit( false );
}

void						IRCData::C_MODE_V( void )
{
	if ( _modsIt->chanIt->isOps( ( *_clientIt )->getUser() ) == _modsIt->chanIt->getOps().end() )
		printNotChanOps();

	if ( !_modsIt->arg.size() )
	{
		_request->clear();
		std::string ircErr( ( *_clientIt )->getUser() + " forget argument for channel mode " + _modsIt->flop + "v" );
		sender( _sd, ":*." + _servIP + " 696 " + ( *_clientIt )->getNick() + _target + " * :You must specify a parameter for the voice mode. Syntax: <nick>.\r\n", &ircErr );
	}
	bool plus = _modsIt->flop == '+' && _modsIt->chanIt->isVo( _modsIt->arg ) == _modsIt->chanIt->getVo().end();
	bool minus = _modsIt->flop == '-' && _modsIt->chanIt->isVo( _modsIt->arg ) != _modsIt->chanIt->getVo().end();
	if ( ( plus || minus ) && _modsIt->chanIt->isCli( _modsIt->arg ) != _modsIt->chanIt->getCli().end() )
	{
		plus ? _modsIt->chanIt->setVo( _modsIt->arg ) : _modsIt->chanIt->unsetVo( _modsIt->arg );
		_printedArgs += " " + _modsIt->arg + "!*@*";
	}
}

void						IRCData::wrongUserFlag( void )
{
	strIt flagIt;
	for ( flagIt = _flag.begin(); flagIt != _flag.end(); ++flagIt )
	{
		if ( *flagIt != 'o' )
		{
			std::string stErr;
			stErr.push_back( *flagIt );
			stErr += " :is not a recognised channel mode.";
			std::string ircErr( stErr );
			sender( _sd, ":*." + _servIP + " 472 " + ( *_clientIt )->getNick() + " " + *flagIt + " :is not a recognised channel mode.\r\n", &ircErr );
		}
	}
}

void						IRCData::wrongFlag( void ) { sender( _sd, ":*." + _servIP + " 472 " + ( *_clientIt )->getNick() + " " + _modsIt->arg + "' :is not a recognised channel mode.\r\n", 0 ); }

void						IRCData::WHO( void )
{
	_target = getLastArg();
	if ( !_target.size() )
	{
		std::string ircErr( "WHO no target given" );
		sender( _sd, ":" + _servIP + " 461 " + ( *_clientIt )->getNick()  + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " WHO : WHO no target given\r\n", &ircErr );
	}
	channelIterator chanIt = isChannel( _target ) ;
	if ( chanIt == _channels.end() )
	{
		std::string ircErr( "No such channel" );
		sender( _sd, ":*." + _servIP + " 403 " + ( *_clientIt )->getNick() + " " + _target + " :No such channel\r\n", &ircErr );
	}
	chanIt->WHO( _clientIt, _servIP );
}

void						IRCData::execMode( void )
{
	channelIterator chanIt = isChannel( _target );
	bool display = ( _target[0] == '#' && chanIt->isOps( ( *_clientIt )->getNick() ) != chanIt->getOps().end() ) || ( _target[0] != '#' && isOps( ( *_clientIt )->getNick() ) != _servOps.end() );

	for ( _modsIt = _mods.begin(); _modsIt != _mods.end(); ++_modsIt )
	{
		try
		{ ( this->*_modsIt->fctn )(); }
		catch( std::string const &err )
		{ std::cerr << err << std::endl; }
	}
	if ( _target[0] == '#' )
	{
		if ( display )
			chanIt->printModifFlags( ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + _servIP + " MODE " + _target + " :" + _printedFlags + _printedArgs + "\r\n" );
	}
	else
	{
		if ( display )
		{
			clientIterator targetIt = isCli( _target );
			if ( targetIt != _clientIt )
				sender( ( *isCli( _target ) )->getSocket(), ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + _servIP + " MODE " + _target + " :" + _printedFlags + _printedArgs + "\r\n", 0 );
			sender( ( *_clientIt )->getSocket(), ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + _servIP + " MODE " + _target + " :" + _printedFlags + _printedArgs + "\r\n", 0 );
		}
	}
}

void						IRCData::setListFlagCmdU( void )
{
	char	printedFlop = 0;
	_flop = 0;
	_mods.clear();
	for( strIt flagIt = _flag.begin(); flagIt != _flag.end(); )
	{
		_flop = *( flagIt++ );
		for( ; flagIt != _flag.end() && *flagIt != '+' && *flagIt != '-'; ++flagIt)
		{
			_mods.push_back( Mode() );
			_modsIt = --( _mods.end() );
			_modsIt->flop = _flop;
			_modsIt->flag = *flagIt;
			listPairM::iterator	_listPairIt;
			for ( _listPairIt = _listFctU.begin(); _listPairIt != _listFctU.end() && _listPairIt->first != *flagIt; ++_listPairIt );
			if ( _listPairIt != _listFctU.end() )
			{
				if ( printedFlop != _modsIt->flop )
				{
					printedFlop = _modsIt->flop;
					_printedFlags.push_back( printedFlop );
				}
				_printedFlags.push_back( *flagIt );
				_modsIt->client = _target;
				_modsIt->fctn = _listPairIt->second.first;
				if ( _listPairIt->second.second )
					( this->*_listPairIt->second.second )();
			}
			else
			{
				_modsIt->fctn = &IRCData::wrongFlag;
				_modsIt->arg = "user MODE ";
				_modsIt->arg.push_back( *flagIt );
			}
		}
	}
}

void						IRCData::setListFlagCmdC( channelIterator &chanIt )
{
	char	printedFlop = 0;
	_flop = 0;
	_mods.clear();
	
	for( strIt flagIt = _flag.begin(); flagIt != _flag.end(); )
	{
		_flop = *( flagIt++ );
		for( ; flagIt != _flag.end() && *flagIt != '+' && *flagIt != '-'; ++flagIt )
		{
			_mods.push_back( Mode() );
			_modsIt = --( _mods.end() );
			_modsIt->flop = _flop;
			_modsIt->flag = *flagIt;
			listPairM::iterator	_listPairIt;
			for ( _listPairIt = _listFctC.begin(); _listPairIt != _listFctC.end() && _listPairIt->first != *flagIt; ++_listPairIt );
			if ( _listPairIt != _listFctC.end() )
			{
				if( printedFlop != _modsIt->flop )
				{
					printedFlop = _modsIt->flop;
					_printedFlags.push_back( printedFlop );
				}
				_printedFlags.push_back( *flagIt );
				_modsIt->chanIt = chanIt;
				_modsIt->fctn = _listPairIt->second.first;
				if ( _listPairIt->second.second	)
					( this->*_listPairIt->second.second )();
			}
			else
			{
				_modsIt->fctn = &IRCData::wrongFlag;
				_modsIt->arg = "channel MODE ";
				_modsIt->arg.push_back( _modsIt->flop );
				_modsIt->arg.push_back( *flagIt );
			}
		}
	}
}

void						IRCData::USERMODE( void )
{
	clientIterator clientIt = isCli( _target );
	if ( clientIt == _clients.end() )
	{
		_request->clear();
		std::string ircErr( "unknowned nick" );
		sender( _sd, ":*." + _servIP + " 401 " + ( *_clientIt )->getNick() + " " + _target + " :No such nick\r\n", &ircErr );
	}
	if ( !_flag.size() )
		sender( ( *_clientIt )->getSocket(), ":" + _servIP + " 501 " + ( *_clientIt )->getNick() + " " + _target + " :[+|-]flag forget\r\n", 0 ); //si pas de flag affiche les modes actif du channel
	else
		setListFlagCmdU();
}

void						IRCData::CHANMODE( void )
{
	channelIterator chanIt = isChannel( _target );
	if ( chanIt == _channels.end() )
	{
		_request->clear();
		std::string ircErr( "unknowned channel" );
		sender( _sd, ":*." + _servIP + " 403 " + ( *_clientIt )->getNick() + " " + _target + " :No such channel\r\n", &ircErr );
	}
	if ( !_flag.size() )
		sender( ( *_clientIt )->getSocket(), ":" + _servIP + " 324 " + ( *_clientIt )->getNick() + " " + _target + " :+" + chanIt->getFlags() + "\r\n", 0 ); //si pas de flag affiche les modes actif du channel
	else
		setListFlagCmdC( chanIt );
}

void						IRCData::MODE( void )
{
	strIt			flopIt;

	_flag.clear();
	_target = getArg();
	_flag = getArg();
	if ( !_target.size() || _target[0] == '+' || _target[0] == '-' )
	{
		std::string ircErr( "MODE : <channel|user> target forgotten" );
		sender( _sd, ":" + _servIP + " 461 " + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " MODE : <channel|user> target forgotten\r\n", &ircErr );
	}
	if ( !_flag.size() || ( _flag[0] != '+' && _flag[0] != '-' ) )
	{
		std::string ircErr( "MODE :operator [+|-] for flag Mode forgotten" );
		sender( _sd, ":" + _servIP + " 400 " + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " MODE :operator [+|-] for flag Mode forgotten\r\n", &ircErr );
	}
	_printedFlags.clear();
	_printedArgs.clear();
	if ( _target[0] == '#' )
		CHANMODE();
	else
		USERMODE();
	execMode();
}
