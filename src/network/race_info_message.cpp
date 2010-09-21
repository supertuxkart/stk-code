//  $Id: race_info_message.cpp 2128 2008-06-13 00:53:52Z cosmosninja $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "network/race_info_message.hpp"

#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"

RaceInfoMessage::RaceInfoMessage(const std::vector<RemoteKartInfo>& kart_info) 
               : Message(Message::MT_RACE_INFO) 
{
    const GrandPrixData *cup=NULL;
    int len = 2*getCharLength()  // major, difficulty
            + getIntLength()     // minor - which is too big for a char/short!
            + getCharLength();   // num karts
    if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        cup = race_manager->getGrandPrix();
        len += getStringLength(cup->getId());
    }
    else
    {
        len += getStringLength(race_manager->getTrackName());
        len += getCharLength(); // num laps
    }
    len += getCharLength();   // kart_info.size()
    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        len += getCharLength()    // kart_info[i].getGlobalPlayerId())
             + getCharLength()    // kart_info[i].getHostId())
             + getStringLength(kart_info[i].getKartName())
             + getCharLength()    // kart_info[i].getLocalPlayerId())
             + getStringLength(kart_info[i].getPlayerName());
    }
    const std::vector<std::string>& rkl=race_manager->getAIKartList();
    len += getStringVectorLength(rkl);

    allocate(len);
    addChar(race_manager->getMajorMode()    );
    addInt (race_manager->getMinorMode()    );
    addChar(race_manager->getDifficulty()   );
    addChar(race_manager->getNumberOfKarts());
    if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
        addString(cup->getId());
    else
    {
        addString(race_manager->getTrackName());
        addChar(race_manager->getNumLaps());
    }

    addChar(kart_info.size());
    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        addChar(kart_info[i].getGlobalPlayerId());
        addChar(kart_info[i].getHostId());
        addString(kart_info[i].getKartName());
        addChar(kart_info[i].getLocalPlayerId());
        addString(kart_info[i].getPlayerName());
    }
    addStringVector(rkl);
}   // RaceInfoMessage

// ----------------------------------------------------------------------------
RaceInfoMessage::RaceInfoMessage(ENetPacket* pkt):Message(pkt, MT_RACE_INFO)
{
    race_manager->setMajorMode ( RaceManager::MajorRaceModeType(getChar()) );
    race_manager->setMinorMode ( RaceManager::MinorRaceModeType(getInt())  );
    race_manager->setDifficulty( RaceManager::Difficulty  (getChar())      );
    race_manager->setNumKarts  ( getChar()                                 );
    if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        const GrandPrixData *cup = grand_prix_manager->getGrandPrix(getString());
        race_manager->setGrandPrix(*cup);
    }
    else
    {
        race_manager->setTrack(getString());
        race_manager->setNumLaps(getChar());
    }

    std::vector<RemoteKartInfo> kart_info;
    kart_info.resize(getChar());

    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        kart_info[i].setGlobalPlayerId(getChar());
        kart_info[i].setHostId(getChar());
        kart_info[i].setKartName(getString());
        kart_info[i].setLocalPlayerId(getChar());
        kart_info[i].setPlayerName(getString());
    }

    // Set the player kart information
    race_manager->setNumPlayers(kart_info.size());
    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        race_manager->setPlayerKart(i, kart_info[i]);
    }
    std::vector<std::string> rkl=getStringVector();
    race_manager->setAIKartList(rkl);
}   // RaceInfoMessage
