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

#include "race_info_message.hpp"
#include "grand_prix_manager.hpp"
#include "race_manager.hpp"

RaceInfoMessage::RaceInfoMessage(const std::vector<RemoteKartInfo>& kart_info) 
               : Message(Message::MT_RACE_INFO) 
{
    const GrandPrixData *cup=NULL;
    int len = getLength(race_manager->getMajorMode() )
            + getLength(race_manager->getMinorMode() )
            + getLength(race_manager->getDifficulty())
            + getLength(race_manager->getNumKarts()  );
    if(race_manager->getMajorMode()==RaceManager::RM_GRAND_PRIX)
    {
        cup = race_manager->getGrandPrix();
        len += getLength(cup->getId());
    }
    else
    {
        len += getLength(race_manager->getTrackName());
        len += getLength(race_manager->getNumLaps());
    }
    len += getLength(kart_info.size());
    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        len += getLength(kart_info[i].getGlobalPlayerId())
             + getLength(kart_info[i].getHostId())
             + getLength(kart_info[i].getKartName())
             + getLength(kart_info[i].getLocalPlayerId())
             + getLength(kart_info[i].getPlayerName());
    }
    const std::vector<std::string>& rkl=race_manager->getRandomKartList();
    len += getLength(rkl);

    allocate(len);
    add(race_manager->getMajorMode() );
    add(race_manager->getMinorMode() );
    add(race_manager->getDifficulty());
    add(race_manager->getNumKarts()  );
    if(race_manager->getMajorMode()==RaceManager::RM_GRAND_PRIX)
        add(cup->getName());
    else
    {
        add(race_manager->getTrackName());
        add(race_manager->getNumLaps());
    }

    add(kart_info.size());
    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        add(kart_info[i].getGlobalPlayerId());
        add(kart_info[i].getHostId());
        add(kart_info[i].getKartName());
        add(kart_info[i].getLocalPlayerId());
        add(kart_info[i].getPlayerName());
    }
    add(rkl);
}   // RaceInfoMessage

// ----------------------------------------------------------------------------
RaceInfoMessage::RaceInfoMessage(ENetPacket* pkt):Message(pkt, MT_RACE_INFO)
{
    race_manager->setMajorMode ( RaceManager::RaceModeType(getInt()) );
    race_manager->setMinorMode ( RaceManager::RaceModeType(getInt()) );
    race_manager->setDifficulty( RaceManager::Difficulty  (getInt()) );
    race_manager->setNumKarts  ( getInt()                            );
    if(race_manager->getMajorMode()==RaceManager::RM_GRAND_PRIX)
    {
        GrandPrixData cup;
        grand_prix_manager->getGrandPrix(getString());
        race_manager->setGrandPrix(cup);
    }
    else
    {
        race_manager->setTrack(getString());
        race_manager->setNumLaps(getInt());
    }

    std::vector<RemoteKartInfo> kart_info;
    kart_info.resize(getInt());

    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        kart_info[i].setGlobalPlayerId(getInt());
        kart_info[i].setHostId(getInt());
        kart_info[i].setKartName(getString());
        kart_info[i].setLocalPlayerId(getInt());
        kart_info[i].setPlayerName(getString());
    }

    // Set the player kart information
    race_manager->setNumPlayers(kart_info.size());
    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        race_manager->setPlayerKart(i, kart_info[i]);
    }
    std::vector<std::string> rkl=getStringVector();
    race_manager->setRandomKartList(rkl);
}   // RaceInfoMessage
