//  $Id: KartManager.cxx,v 1.3 2004/08/24 23:28:54 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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

#include <set>
#include "Loader.h"
#include "StringUtils.h"
#include "KartManager.h"
#include "TuxkartError.h"

KartManager kart_manager;

KartManager::KartManager()
{
}

int
KartManager::getKartId(const std::string ident)
{
  int j = 0;
  for(Data::iterator i = karts.begin(); i != karts.end(); ++i)
    {
      if (i->ident == ident)
        return j;
      ++j;
    }

  throw TuxkartError("KartManager: Couldn't find kart: '" + ident + "'");
}

const KartProperties&
KartManager::getKart(const std::string ident)
{
  for(Data::iterator i = karts.begin(); i != karts.end(); ++i)
    {
      if (i->ident == ident)
        return *i;
    }

  throw TuxkartError("KartManager: Couldn't find kart: '" + ident + "'");
}

const KartProperties&
KartManager::getKartById(int i)
{
  if (i < 0 || i >= int(karts.size()))
    throw TuxkartError("KartManager: Couldn't find kart: '" + StringUtils::to_string(i) + "'");
    
  return karts[i];
}

void
KartManager::loadKartData()
{
  std::set<std::string> result;
  loader->listFiles(result, "data");

  // Findout which characters are available and load them
  for(std::set<std::string>::iterator i = result.begin(); i != result.end(); ++i)
    {
      if (StringUtils::has_suffix(*i, ".tkkf"))
        {
          karts.push_back(KartProperties("data/" + *i));
        }
    }
}

std::vector<std::string>
KartManager::getRandomKarts(int len)
{
  std::vector<std::string> all_karts;

  for(Data::iterator i = karts.begin(); i != karts.end(); ++i)
    {
      all_karts.push_back(i->ident);
    }

  std::random_shuffle(all_karts.begin(), all_karts.end());

  all_karts.resize(len);

  return all_karts;
}

/* EOF */
