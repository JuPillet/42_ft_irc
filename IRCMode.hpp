#pragma once

#include "IRCData.hpp"

		void				MODE_B( channelIterator &channel )
		{
			strIt	   strIt;
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			for ( strIt = _request->begin(); strIt != _request->end()
				&& *strIt != '\n' && *strIt != '\r' && *strIt != ' '; ++strIt );
			std::string target( _request->begin(), strIt );
			_request->erase( _request->begin(), strIt );
			spaceTrimer();
			if ( _flop == '+' )
				channel->setBan(target , 0);
			else if (_flop == '-')
				channel->unBan(target);
		}

		bool	isFlagChan( void )

		bool	isFlagUser( void )
		void	CHANMODE( void )
		{
			for ( chanIt = _channels.begin(); chanIt != _channels.end() && _target != chanIt->getName(); ++chanIt )
			if ( chanIt == _channels.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur channel incconu\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) )
			}
			for( strIt flagIt = _flag.begin(); flagIt != _flag.end(); fligIt )
		}

		void	MODE( void )
		{
			strIt			targIt;
			channelIterator	chanIt;
			_flop = 0;
			_flag.clear();
			for ( targIt = _request->begin(); targIt != _request->end()
				&& *targIt != '\n' && *targIt != '\r' && *targIt != ' '; ++targIt );
			_target = std::string( *_request, 0, targIt - _request->begin() );
			_request->erase( _request->begin(), targIt );
			spaceTrimer();
			for ( targIt = _request->begin(); targIt != _request->end()
				&& ( *targIt = '+' || *targIt != '-' ); ++targIt )
				_flop = *targIt;
			_request->erase( _request->begin(), targIt );
			spaceTrimer();
			for ( targIt = _request->begin(); targIt != _request->end()
				&& *targIt != '\n' && *targIt != '\r' && *targIt != ' '; ++targIt );
			_flag = std::string( _request->begin(), targIt );
			if ( !_target.size() || ( _target.size() == 1 && _target[0] == '#' ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur target erroné\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) )
			}
			if ( !_flop || _flage )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur flag erroné\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) )
			}
			if ( _target[0] == '#' )
				CHANMODE();
			else
				USERMODE();
		}
};