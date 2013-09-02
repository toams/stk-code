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

#include "achievements/achievements_manager.hpp"

#include "utils/log.hpp"
#include "utils/translation.hpp"
#include "io/file_manager.hpp"
#include "io/xml_writer.hpp"
#include "config/player.hpp"
#include "config/user_config.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

static AchievementsManager* achievements_manager_singleton(NULL);

AchievementsManager* AchievementsManager::get()
{
    if (achievements_manager_singleton == NULL)
        achievements_manager_singleton = new AchievementsManager();
    return achievements_manager_singleton;
}

void AchievementsManager::deallocate()
{
    delete achievements_manager_singleton;
    achievements_manager_singleton = NULL;
}   // deallocate

// ============================================================================
AchievementsManager::AchievementsManager()
{
    parseDataFile();
    parseConfigFile();
}

// ============================================================================
AchievementsManager::~AchievementsManager()
{
}

// ============================================================================
void AchievementsManager::parseDataFile()
{
    const std::string file_name = file_manager->getDataFile("achievements.xml");
    const XMLNode *root         = file_manager->createXMLTree(file_name);
    unsigned int num_nodes = root->getNumNodes();
    for(unsigned int i = 0; i < num_nodes; i++)
    {
        const XMLNode *node = root->getNode(i);
        std::string type("");
        node->get("type", &type);
        AchievementInfo * achievement_info;
        if(type == "single")
        {
            achievement_info = new SingleAchievementInfo(node);
        }
        else if(type == "map")
        {
            achievement_info = new MapAchievementInfo(node);
        }
        else
        {
            Log::error("AchievementsManager::parseAchievements","Non-existent achievement type. Skipping - definitely results in unwanted behaviour.");
            continue;
        }
        m_achievements_info.push_back(achievement_info);
    }
    if(num_nodes != m_achievements_info.size())
        Log::error("AchievementsManager::parseAchievements","Multiple achievements with the same id!");
}


// ============================================================================
void AchievementsManager::parseConfigFile()
{
    const std::string filename=file_manager->getConfigFile("achievements.xml");
    XMLNode* root = file_manager->createXMLTree(filename);
    if(!root || root->getName() != "achievements")
    {
        Log::info("AchievementsManager", "Achievements file '%s' will be created.",
                  filename.c_str());
        createSlotsIfNeeded();
        save();

        if (root) delete root;
        return;
    }

    std::vector<XMLNode*> xml_slots;
    root->getNodes("slot", xml_slots);
    for (unsigned int n=0; n < xml_slots.size(); n++)
    {
        AchievementsSlot * slot = new AchievementsSlot(xml_slots[n]);
        if(!slot->isValid())
        {
            Log::warn("AchievementsManager", "Found game slot with faulty or missing information. Discarding it.");
            delete slot;
            continue;
        }
        m_slots.push_back(slot);
    }

    bool something_changed = createSlotsIfNeeded();
    if (something_changed) save();

    delete root;
}   // load



//-----------------------------------------------------------------------------
/** Creates a slot for players that don't have one yet
 *  \return true if any were created
 */
bool AchievementsManager::createSlotsIfNeeded()
{
    bool something_changed = false;

    // make sure all players have at least one game slot associated
    PtrVector<PlayerProfile>& players = UserConfigParams::m_all_players;
    for (int n=0; n<players.size(); n++)
    {
        bool exists = false;

        for(unsigned int i = 0; i < m_slots.size(); i++)
        {
            if(!m_slots[i]->isOnline() && m_slots[i]->getID() == players[n].getUniqueID())
            {
                exists = true;
                break;
            }
        }

        if (!exists)
        {
            AchievementsSlot* slot = new AchievementsSlot(players[n].getUniqueID(), false);
            m_slots.push_back(slot);
            something_changed = true;
        }
    }

    return something_changed;
} // UnlockManager::createSlotsIfNeeded


// ============================================================================
void AchievementsManager::save()
{
    std::string filename = file_manager->getConfigFile("challenges.xml");

    std::ofstream achievements_file(filename.c_str(), std::ios::out);

    if (!achievements_file.is_open())
    {
        Log::warn("AchievementsManager::save",
                  "Failed to open '%s' for writing, achievements won't be saved\n",
                  filename.c_str());
        return;
    }

    achievements_file << "<?xml version=\"1.0\"?>\n";
    achievements_file << "<achievements>\n";

    for (unsigned int i = 0; i < m_slots.size(); i++)
    {
        m_slots[i]->save(achievements_file);
    }

    achievements_file << "</achievements>\n\n";
    achievements_file.close();
}
