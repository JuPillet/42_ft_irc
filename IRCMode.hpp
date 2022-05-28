#pragma once

#include "IRCData.hpp"

		void				MODE_B( chanelIterator &chanel )
		{
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			std::string target = getArg();
			if ( !target.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir ici code erreur argument manquant\r\n";
				_request->clear();
				sender();
				throw( IRCErr( ( *_clientIt )->getUser + " foget argument for chanel mode " + _flop + "b" ) )
			}
			if ( _flop == '+' )
				chanel->setBan(target , 0);
			else if (_flop == '-')
				chanel->unBan(target);
		}

		void				MODE_K( chanelIterator &chanel )
		{
			if ( _flop == '-' )
				chanel->unsetPass();
			else
			{
				std::string mdp = getArg();
				if ( !mdp.size() )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = "Voir ici code erreur argument manquant\r\n";
					_request->clear();
					sender();
					throw( IRCErr( ( *_clientIt )->getUser + " foget argument for chanel mode " + _flop + "k" ) )
				}
				chanel->setPass(mdp);
			}
		}

		void				MODE_M( chanelIterator &chanel )
		{
			if (_flop == '-')
				chanel->setMod(0);
			else
				chanel->setMod(1);
		}

		void				MODE_N( chanelIterator &chanel )
		{
			if (_flop == '-')
				chanel->setExt(0);
			else
				chanel->setExt(1);
		}

		void				MODE_O( chanelIterator &chanel )
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
				throw( IRCErr( ( *_clientIt )->getUser + " foget argument for chanel mode " + _flop + "o" ) )
			}
			if ( _flop == '+' && chanel->isOps(target) == ( *chanel->getOps() ).end() )
				chanel->setOps( target );
			else if (_flop == '-' && chanel->isOps(target) != ( *chanel->getOps() ).end() )
				chanel->unsetOps( target );
		}

		void				MODE_P( chanelIterator &chanel )
		{
			if ( _flop == '-' )
				chanel->setPriv(0);
			else
				chanel->setPriv(1);
		}

		void				MODE_S( chanelIterator &chanel )
		{
			if (_flop == '-')
				chanel->setSecret(0);
			else
				chanel->setSecret(1);
		}
	
		void				MODE_V( chanelIterator &chanel )
		{
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			std::string target = getArg();
			if ( !target.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir ici code erreur argument manquant\r\n";
				_request->clear();
				sender();
				throw( IRCErr( ( *_clientIt )->getUser + " foget argument for chanel mode " + _flop + "v" ) )
			}
			if ( _flop == '+' && chanel->isVo(target) != ( *chanel->getVo() ).end())
				chanel->setVo(target);
			else if (_flop == '-')
				chanel->unsetVo(target);
		}

		void	wrongFlagChan( void )
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
					_answer = "Voir code erreur chanel incconu. :flag " + flag + " isn t flag of chanel Mode\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
					_request->clear();
					sender();
					throw( IRCErr( "unvalid flag" ) );
				}
			}
		}

		void	wrongFlagUser( void )
		{
			std::string flagList( "acghiosw" );
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
					_answer = "Voir code erreur chanel incconu. :flag " + flag + " isn t flag of user Mode\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
					_request->clear();
					sender();
					throw( IRCErr( "unvalid flag" ) );
				}
			}
		}

		void	CHANMODE( void )
		{
			wrongFlagChan();
			chanelIterator chanIt = isChanel( _target );
			if ( chanIt  == _chanels.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur chanel incconu\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) )
			}
			for( strIt flagIt = _flag.begin(); flagIt != _flag.end(); fligIt )

		}

		void	MODE( void )
		{
			strIt			targIt;
			chanelIterator	chanIt;
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