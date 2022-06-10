#pragma once

#include "IRCData.hpp"

		void				U_MODE_O( clientIterator &target )
		{
			if ( _flop == '+' && channel->isOps(target) != ( *channel->getOps() ).end())
				_servOps.push_back(target->getName());
			else if (_flop == '-')
			{
				clientIterator tmpIt;
				for (tmpIt = _servOps.begin(); tmpIt != _servOps.end(); tmpIt++)
				{
					if (*tmpIt == *target)
					{
						_servOps.erase(tmpIt);
						return ;
					}
				}
			}
		}

		void				U_MODE_I( clientIterator &target )
		{
			if ( _flop == '+')
				target->setInvisible (1);
			else if (_flop == '-')
				target->setInvisible (0);
		}

		void				C_MODE_B( channelIterator &channel )
		{
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			std::string target = getArg();
			if ( !target.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir ici code erreur argument manquant\r\n";
				_request->clear();
				sender();
				throw( IRCErr( ( *_clientIt )->getUser + " foget argument for channel mode " + _flop + "b" ) )
			}
			if ( _flop == '+' )
				channel->setBan(target , 0);
			else if (_flop == '-')
				channel->unBan(target);
		}

		void				C_MODE_K( channelIterator &channel )
		{
			if ( _flop == '-' )
				channel->unsetPass();
			else
			{
				std::string mdp = getArg();
				if ( !mdp.size() )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = "Voir ici code erreur argument manquant\r\n";
					_request->clear();
					sender();
					throw( IRCErr( ( *_clientIt )->getUser + " foget argument for channel mode " + _flop + "k" ) )
				}
				channel->setPass(mdp);
			}
		}

		void				C_MODE_M( channelIterator &channel )
		{
			if (_flop == '-')
				channel->setMod(0);
			else
				channel->setMod(1);
		}

		void				C_MODE_N( channelIterator &channel )
		{
			if (_flop == '-')
				channel->setExt(0);
			else
				channel->setExt(1);
		}

		void				C_MODE_O( channelIterator &channel )
		{
			strIt	  strIt;
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			std::string user = getArg();
			if ( !user.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir ici code erreur argument manquant\r\n";
				_request->clear();
				sender();
				throw( IRCErr( ( *_clientIt )->getUser + " foget argument for channel mode " + _flop + "o" ) )
			}
			if ( _flop == '+' && channel->isOps(target) == ( *channel->getOps() ).end() )
				channel->setOps( target );
			else if (_flop == '-' && channel->isOps(target) != ( *channel->getOps() ).end() )
				channel->unsetOps( target );
		}

		void				MODE_P( channelIterator &channel )
		{
			if ( _flop == '-' )
				channel->setPriv(0);
			else
				channel->setPriv(1);
		}

		void				C_MODE_S( channelIterator &channel )
		{
			if (_flop == '-')
				channel->setSecret(0);
			else
				channel->setSecret(1);
		}
	
		void				C_MODE_V( channelIterator &channel )
		{
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			std::string target = getArg();
			if ( !target.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir ici code erreur argument manquant\r\n";
				_request->clear();
				sender();
				throw( IRCErr( ( *_clientIt )->getUser + " foget argument for channel mode " + _flop + "v" ) )
			}
			if ( _flop == '+' && channel->isVo(target) != ( *channel->getVo() ).end())
				channel->setVo(target);
			else if (_flop == '-')
				channel->unsetVo(target);
		}

		void	wrongChannelFlag( void )
		{
			std::string flagList( "bkmnopsv" );
			strIt flagIt;
			if ( _target[0] != '#' )
				return 0;
			for ( flagIt = _flag.begin(); flagIt != _flag.end(); ++flagIt )
			{
				strIt flagListIt;
				for ( flagListIt = flagList.begin(); flagListIt != flagList.end() && *flagIt != *flagListIt; ++flagIt );
				if ( flagListIt == flagList.end() )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = "Voir code erreur channel incconu. :flag " + flag + " isn t flag of channel Mode\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
					_request->clear();
					sender();
					throw( IRCErr( "unvalid flag" ) );
				}
			}
		}

		void	wrongUserFlag( void )
		{
			std::string flagList( "io" );
			strIt flagIt;
			if ( _target[0] == '#' )
				return 0;
			for ( flagIt = _flag.begin(); flagIt != _flag.end(); ++flagIt )
			{
				strIt flagListIt;
				for ( flagListIt = flagList.begin(); flagListIt != flagList.end() && *flagIt != *flagListIt; ++flagIt );
				if ( flagListIt == flagList.end() )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = "Voir code erreur channel incconu. :flag " + flag + " isn t flag of user Mode\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
					_request->clear();
					sender();
					throw( IRCErr( "unvalid flag" ) );
				}
			}
		}

		void	USERMODE( void )
		{

		}

		void				execChannelMode( void )
		{
			std::cout << "CHANNEL MODE start" << std::endl;
			listPair::iterator	_listPairIt;

			for ( _listPairIt = _listFctn.begin(); _listPairIt != _listFctn.end() && _listPairIt->first != _cmd; ++_listPairIt );
			if ( _listPairIt != _listFctn.end() )
			{
				std::cout << _cmd << std::endl;
				ptrfct ptrFct = _listPairIt->second;
				(this->*ptrFct)();
			}
			else
				clearPostArgs();
			std::cout << "CHANNEL MODE exit" << std::endl;
		}

		void	CHANMODE( void )
		{
			channelIterator chanIt = isChannel( _target );
			if ( chanIt  == _channels.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur channel incconu\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) )
			}
			for( strIt flagIt = _flag.begin(); flagIt != _flag.end(); ++fligIt )
			{

			}
		}

		void	MODE( void )
		{
			strIt			targIt;
			channelIterator	chanIt;
			_flop = 0;
			_flag.clear();
			_target = getArg();
			for ( targIt = _request->begin(); targIt != _request->end()
				&& ( *targIt = '+' || *targIt != '-' ); ++targIt )
				_flop = *targIt;
			_request->erase( _request->begin(), targIt );
			spaceTrimer();
			_flag = getArg();
			if ( !_target.size() || ( _target.size() == 1 && _target[0] == '#' ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur target erroné\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) );
			}
			char flagErr;
			if ( !_flop || ( flagErr = wrongFlagChan() ) || ( flagErr = wrongFlagUser() ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur operateur de flag\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) );
			}
			if ( _target[0] == '#' )
				CHANMODE();
			else
				USERMODE();
		}
};