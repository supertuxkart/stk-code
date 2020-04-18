#!/bin/sh
#
# (C) 2018 Dawid Gan, under the GPLv3
#
# A script that manages STK servers
#

export SELF_PID=$$
export BASENAME="$(basename "$0")"
export DIRNAME="$(dirname "$(readlink -f "$0")")"
export DATETIME="$(date +%Y%m%d%H%M%S)"

############## General info ##############

# Usage:
#
# Start all servers and close the script:
#     run_server.sh start
#
# Start all servers and keep the script running and testing if servers are 
# alive:
#     run_server.sh startdaemon
#
# Stop all servers and close the running daemon:
#     run_server.sh stop
#
# By default the script works with following directories structure
# --- stk-server/
# ----- data/
# ----- supertuxkart
# ----- run_server.sh


################# Config #################

### General ###

# Server name, make sure that it's unique
export SERVER_NAME="STK Server"

# Login for STK account
export LOGIN="xxx"

# Password for STK account
export PASS="yyy"

### Paths ###

# A path for STK server binary file
export CMD="$DIRNAME/supertuxkart"

# A path in which "data" directory is placed
export SUPERTUXKART_DATADIR="$DIRNAME"

# A path for STK assets
export SUPERTUXKART_ASSETS_DIR="$DIRNAME/data/"

# A path to config template for additional options
export CONFIG_FILE="$DIRNAME/config_template.xml"

# A path to server config template for additional options
export SERVER_CONFIG="$DIRNAME/server_config_template.xml"

# A path for configuration files
export HOME="/tmp/stk-server/.config"

# A path where logs will be saved
export STDOUT_DIR="/tmp/stk-server/"

### Daemon mode ###

# How often the script should check if servers are alive
export SLEEP_TIME=300

# How many times the script should try to recreate servers
export MAX_CREATION_RETRIES=100

# Determines if the script should parse stdout.log files to see if servers are 
# alive. Set it to 0 to disable.
export CHECK_SERVERS=0

# A path to the application that can be used to show GUI messages when error 
# ocurred, server crashed etc. Atm. it will only work with xmessage/gxmessage. 
# Zenity and other apps need additional args.
export MESSAGE_CMD="/usr/bin/xmessage"

# Max number of messages that can be showed at the same time. Set it to 0 to 
# disable.
export MAX_MESSAGES=3

##########################################

show_message()
{
    export MESSAGE="$1"

    if [ -z "$MESSAGE" ]; then
        return
    fi

    echo "$MESSAGE"

    if [ ! -x "$MESSAGE_CMD" ]; then
        return
    fi
    
    if [ ! -z "$TERM" ] && [ "$TERM" != "dumb" ]; then
        return
    fi

    if [ $(pidof -x "$MESSAGE_CMD" | wc -w) -ge $MAX_MESSAGES ]; then
        return
    fi

    "$MESSAGE_CMD" "$MESSAGE" &
}

run_servers()
{
    echo "Info: Run servers"

    "$CMD" --ranked                            \
           --owner-less                        \
           --disable-polling                   \
           --max-players=8                     \
           --min-players=2                     \
           --difficulty=3                      \
           --mode=0                            \
           --port=2760                         \
           --wan-server="$SERVER_NAME Ranked"  \
           --stdout="$DATETIME-normal.log"     \
           --stdout-dir="$STDOUT_DIR"          \
           --no-console-log                    \
           --no-firewalled-server              \
           --log=0                               &> /dev/null &

    sleep 5

    #~ "$CMD" --no-ranked                        \
           #~ --owner-less                       \
           #~ --disable-polling                  \
           #~ --max-players=8                    \
           #~ --min-players=2                    \
           #~ --difficulty=2                     \
           #~ --mode=3                           \
           #~ --soccer-goals                     \
           #~ --port=2761                        \
           #~ --wan-server="$SERVER_NAME Soccer" \
           #~ --stdout="$DATETIME-soccer.log"    \
           #~ --stdout-dir="$STDOUT_DIR"         \
           #~ --no-console-log                   \
           #~ --no-firewalled-server             \
           #~ --log=0                              &> /dev/null &

    #~ sleep 5

    #~ "$CMD" --no-ranked                        \
           #~ --owner-less                       \
           #~ --disable-polling                  \
           #~ --max-players=8                    \
           #~ --min-players=2                    \
           #~ --difficulty=2                     \
           #~ --mode=2                           \
           #~ --battle-mode=0                    \
           #~ --port=2762                        \
           #~ --wan-server="$SERVER_NAME FFA"    \
           #~ --stdout="$DATETIME-ffa.log"       \
           #~ --stdout-dir="$STDOUT_DIR"         \
           #~ --no-console-log                   \
           #~ --no-firewalled-server             \
           #~ --log=0                              &> /dev/null &

    #~ sleep 5

    #~ "$CMD" --no-ranked                        \
           #~ --owner-less                       \
           #~ --disable-polling                  \
           #~ --max-players=8                    \
           #~ --min-players=2                    \
           #~ --difficulty=2                     \
           #~ --mode=2                           \
           #~ --battle-mode=1                    \
           #~ --port=2763                        \
           #~ --wan-server="$SERVER_NAME CTF"    \
           #~ --stdout="$DATETIME-ctf.log"       \
           #~ --stdout-dir="$STDOUT_DIR"         \
           #~ --no-console-log                   \
           #~ --no-firewalled-server             \
           #~ --log=0                              &> /dev/null &

    #~ sleep 5
    
    "$CMD" --no-ranked                        \
           --no-owner-less                    \
           --disable-polling                  \
           --max-players=8                    \
           --min-players=2                    \
           --difficulty=2                     \
           --mode=3                           \
           --soccer-goals                     \
           --port=2761                        \
           --wan-server="$SERVER_NAME Custom" \
           --stdout="$DATETIME-custom.log"    \
           --stdout-dir="$STDOUT_DIR"         \
           --no-console-log                   \
           --no-firewalled-server             \
           --log=0                              &> /dev/null &

    sleep 5

    "$CMD" --no-ranked                          \
           --no-owner-less                      \
           --disable-polling                    \
           --max-players=8                      \
           --min-players=2                      \
           --difficulty=2                       \
           --mode=2                             \
           --battle-mode=1                      \
           --port=2762                          \
           --wan-server="$SERVER_NAME Custom 2" \
           --stdout="$DATETIME-custom2.log"     \
           --stdout-dir="$STDOUT_DIR"           \
           --no-console-log                     \
           --no-firewalled-server               \
           --log=0                              &> /dev/null &

    sleep 5
}

init_servers()
{
    echo "Info: Init servers"

    mkdir -p "$STDOUT_DIR"

    "$CMD" --init-user                   \
           --login="$LOGIN"              \
           --password="$PASS"            \
           --stdout="$DATETIME-init.log" \
           --stdout-dir="$STDOUT_DIR"    \
           --no-console-log              \
           --log=0                         &> /dev/null

    sleep 5

    find "$HOME/.config/supertuxkart" -mindepth 1 -maxdepth 1 -type d -exec cp "$CONFIG_FILE" "{}/config.xml" \;
    find "$HOME/.config/supertuxkart" -mindepth 1 -maxdepth 1 -type d -exec cp "$SERVER_CONFIG" "{}/server_config.xml" \;
}

stop_servers()
{
    echo "Info: Stop servers"

    for PID in $(pidof -x "$CMD"); do
        echo "Info: Killing the STK server $PID"
        kill -15 $PID
    done

    sleep 10

    for PID in $(pidof -x "$CMD"); do
        echo "Info: Force killing the STK server $PID"
        kill -9 $PID
    done
}

check_servers()
{
    export SUCCESS=1

    for FILE in $(find "$STDOUT_DIR" -type f -name "$DATETIME-*.log"); do
        echo "Info: Check file: $FILE"

        FILE_BEGIN=$(cat "$FILE" | head -n100)

        if [ $(echo $FILE_BEGIN | grep -c "Done saving user, leaving") -gt 0 ]; then
            echo "Info: Check server: Servers successfully initialized"
        elif [ $(echo $FILE_BEGIN | grep "Server" | grep -c "is now online.") -gt 0 ]; then
            echo "Info: Check server: Servers successfully created"
        elif [ $(echo $FILE_BEGIN | grep -c "Specified server already exists.") -gt 0 ]; then
            show_message "Error: Check server: Specified server already exists"
            SUCCESS=0
        else
            show_message "Error: Check server: Unknown error"
            SUCCESS=0
        fi

        FILE_END=$(cat "$FILE" | tail -n50)

        if [ $(echo $FILE_END | grep -c "Session not valid. Please sign in.") -gt 0 ]; then
            show_message "Error: Check server: Session not valid"
            SUCCESS=0
#        elif [ $(echo $FILE_END | grep curl_easy_perform | grep -c "Timeout was reached") -gt 0 ]; then
#            show_message "Error: Check server: Timeout was reached"
#            SUCCESS=0
        fi
    done

    return $SUCCESS
}

start()
{
    if [ ! -z $(pidof -x "$DIRNAME/$BASENAME" -o $SELF_PID) ]; then
        show_message "Error: The script is already started"
        exit
    fi

    if [ ! -z $(pidof -s -x "$CMD") ]; then
        show_message "Error: Some servers are already running"
        exit  
    fi
    
    if [ ! -f "$CMD" ]; then
        show_message "Error: Couldn't find STK executable in CMD: $CMD"
        exit    
    fi
    
    if [ ! -d "$SUPERTUXKART_DATADIR/data" ]; then
        show_message "Error: Couldn't find data directory in SUPERTUXKART_DATADIR: $SUPERTUXKART_DATADIR"
        exit    
    fi
    
    if [ ! -d "$SUPERTUXKART_ASSETS_DIR/tracks" ]; then
        show_message "Error: Couldn't find assets directories in SUPERTUXKART_ASSETS_DIR: $SUPERTUXKART_ASSETS_DIR"
        exit    
    fi

    init_servers
    run_servers

    if [ $CHECK_SERVERS -eq 1 ]; then
        check_servers
    fi

    echo "Info: Servers started"
}

startdaemon()
{
    start

    export SERVERS_COUNT=$(pidof -x "$CMD" | wc -w)
    export SERVER_OK=1
    export LOOP=0

    while [ $LOOP -lt $MAX_CREATION_RETRIES ]; do
        if [ $(pidof -x "$CMD" | wc -w) -lt $SERVERS_COUNT ]; then
            SERVER_OK=0
        fi

        if [ $SERVER_OK -eq 1 ] && [ $CHECK_SERVERS -eq 1 ]; then
            check_servers
            SERVER_OK=$?
        fi
        
        if [ $SERVER_OK -eq 0 ]; then
            show_message "Error: Some servers don't work, restart is needed"
            stop_servers
            
            DATETIME="$(date +%Y%m%d%H%M%S)"

            init_servers
            run_servers

            SERVERS_COUNT=$(pidof -x "$CMD" | wc -w)
            SERVER_OK=1
            LOOP=$(($LOOP + 1))
        fi

        sleep $SLEEP_TIME
    done

    $MESSAGE_CMD "Error: Closing STK server"
}

stop()
{
    for PID in $(pidof -x "$DIRNAME/$BASENAME" -o $SELF_PID); do
        echo "Info: Killing the $BASENAME script $PID"
        kill -9 $PID
    done

    stop_servers
}


if [ "$1" = "startdaemon" ] && [ "$2" != "disown" ]; then
    sleep 5 && "$DIRNAME/$BASENAME" "$1" disown &
    exit
fi

if [ "$1" = "start" ]; then
    start
elif [ "$1" = "startdaemon" ]; then
    startdaemon
elif [ "$1" = "stop" ]; then
    stop
else
    show_message "Error: The script must be started with start/startdaemon/stop command"
fi
