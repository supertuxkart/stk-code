//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_KARTMANAGER_H
#define HEADER_KARTMANAGER_H

#include <vector>
#include "KartProperties.h"

class KartManager
{
public:
  typedef std::vector<KartProperties*> KartPropertiesVector;
  /** All available kart configurations */
  KartPropertiesVector karts;
  
  KartManager();
  ~KartManager();

  const KartProperties* getKartById(int i);
  const KartProperties* getKart(const std::string ident);
  int  getKartId(const std::string ident);
  
  void loadKartData();

  /** Return \a len random karts */
  std::vector<std::string> getRandomKarts(int len);

  /** Will the empty positions in the given vector with random karts */
  void fillWithRandomKarts(std::vector<std::string>& vec);
};

extern KartManager kart_manager;

#endif

/* EOF */
