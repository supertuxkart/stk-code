#include <fstream>
#include <map>

#ifndef STK_PROTO_LOGGING_GLOBAL_H_
#define STK_PROTO_LOGGING_GLOBAL_H_

enum class GlobalLogTypes { POS_LOG, GOAL_LOG };

class GlobalLog
{
    public:
        static void writeLog(std::string text, GlobalLogTypes log_name);
        static void openLog(GlobalLogTypes log_name);
        static void closeLog(GlobalLogTypes log_name);

        // Player names are normalized by adding a ? prefix for offline accounts, and replacing spaces by tabulators.
        static void addIngamePlayer(unsigned int world_kart_id, std::string player_name, bool offline_account);
        static void removeIngamePlayer(unsigned int world_kart_id);
        static std::string getPlayerName(unsigned int world_kart_id);
        static void resetIngamePlayers();
    private:
        static std::ofstream outfile_posLog;
        static std::ofstream outfile_goalLog;
        static std::map<unsigned int, std::string> ingame_players;
};

#endif /* STK_PROTO_LOGGING_GLOBAL_H_ */
