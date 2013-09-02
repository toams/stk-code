//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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


#include "achievements/achievements_slot.hpp"

#include "achievements/achievement_info.hpp"
#include "achievements/achievements_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"
#include "io/xml_writer.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>
// ============================================================================
AchievementsSlot::AchievementsSlot(const XMLNode * input)
{
    int fetched_user_id = input->get("user_id", &m_id);
    std::string online;
    int fetched_online = input->get("online", &online);
    if(!fetched_user_id || !fetched_online || !(online == "true" || online == "false"))
    {
        m_valid = false;
    }
    m_valid = true;
    m_online = online == "true";

    createFreshSlot();

    std::vector<XMLNode*> xml_achievements;
    input->getNodes("achievement", xml_achievements);
    for( unsigned int i=0; i<xml_achievements.size(); i++)
    {
        uint32_t achievement_id(0);
        xml_achievements[i]->get("id", &achievement_id);
        Achievement * achievement = findAchievement(achievement_id);
        if(achievement == NULL)
        {
            Log::warn("AchievementsSlot", "Found saved achievement data for a non-existent achievement. Discarding.");
            continue;
        }
        achievement->load(xml_achievements[i]);
    }
}

// ============================================================================
AchievementsSlot::AchievementsSlot(std::string id, bool online)
{
    m_valid = true;
    m_online = online;
    m_id = id;

    createFreshSlot();
}

// ============================================================================
void AchievementsSlot::createFreshSlot()
{
    m_achievements.clear();
    std::vector<AchievementInfo *> all_info = AchievementsManager::get()->getAllInfo();
    for( unsigned int i=0; i < all_info.size(); i++)
    {
        AchievementInfo * info = all_info[i];
        Achievement::AchievementType achievement_type = info->getType();
        Achievement * achievement;
        if(achievement_type == Achievement::AT_SINGLE)
        {
            achievement = new SingleAchievement(info);
        }
        else if(achievement_type == Achievement::AT_MAP)
        {
            achievement = new MapAchievement(info);
        }
        m_achievements[achievement->getID()] = achievement;
    }
}

// ============================================================================
void AchievementsSlot::save(std::ofstream & out)
{
    out << "    <slot user_id=\"" << m_id.c_str()
        << "\" online=\""           << StringUtils::boolstr(m_online)
        << "\"> \n";
    std::map<uint32_t, Achievement*>::const_iterator i;
    for(i = m_achievements.begin(); i != m_achievements.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << "    </slot>\n";
}

// ============================================================================
Achievement * AchievementsSlot::findAchievement(uint32_t id)
{
    if ( m_achievements.find(id) != m_achievements.end())
        return m_achievements[id];
    return NULL;
}