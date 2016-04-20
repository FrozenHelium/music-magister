#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <ctime>
#include <cstdlib>
#include <iostream>

#include "AudioStream.h"
#include "AudioDirectory.h"
#include "Config.h"

struct mm_msgbuf {
    long mtype;
    char mtext[256];
};

const std::string usage_msg =
    "usage: mm <command>\n\n"
    "list of command\n"
    "---------------\n"
    "start\tstart the music player\n"
    "stop\tstop the music player\n"
    "next\tskip to next song in the playlist\n"
    "prev\tskip to previous song in the playlist";

void start_service();


int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << usage_msg << std::endl;
        return 0;
    }

    std::string cmd(argv[1]);

    // check for invalid command using the usage_msg
    // NOTE: cmd is invalid if "\n<cmd>\t" can't be found in the usage_msg
    if (usage_msg.find("\n"+cmd+"\t") == std::string::npos)
    {
        std::cout << usage_msg << std::endl;
        return 1;
    }

    int msqid = msgget(9900, 0666);
    // if the music player hasn't already started
    if (msqid < 0 )
    {
        if (cmd == "start") {
            start_service();
        } else {
            std::cout << "start music-magister first. ( use: mm start )" << std::endl;
        }
    }
    else
    {
        struct mm_msgbuf sbuf;
        sbuf.mtype = 1;
        strcpy(sbuf.mtext, argv[1]);
        if (msgsnd(msqid, &sbuf, strlen(sbuf.mtext) + 1, IPC_NOWAIT) < 0)
        {
            std::cout << "Some error occured!" << std::endl;
            exit(1);
        }
    }

    exit(0);
}

void start_service()
{
    pid_t pid, sid;
    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    umask(0);
    sid = setsid();
    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int msqid;
    struct mm_msgbuf rbuf;

    if ((msqid = msgget(9900, 0666|IPC_CREAT)) < 0)
    {
        exit(1);
    }

    std::srand ( unsigned ( std::time(0) ) );

    Config* mmcfg = Config::GetInstance();
    std::vector<std::string> dirs = mmcfg->GetLibDirs();
    std::vector<std::string> audiofiles;

    for(auto dir: dirs)
    {
        AudioDirectory ad;
        ad.SetPath(dir);
        std::vector<std::string> files = ad.GetFiles();
        audiofiles.insert(audiofiles.end(), files.begin(), files.end());
    }

    // Identifies the audio from the audioFiles to play.
    int index = 0;
    // Shuffle the playlist
    if(mmcfg->GetShuffle())
    {
        std::random_shuffle(audiofiles.begin(), audiofiles.end());
    }

    while(1)
    {

        pid_t childpid = fork();
        if(childpid == 0)
        {
            // child process

            // don't play cases
            if (audiofiles.size()==0 || index < 0 || index >= audiofiles.size())
                exit(0);

            auto file = audiofiles[index];
            AudioStream as;
            as.FromFile(file);
            as.Play();

            exit(0);
        }
        else if(childpid > 0)
        {
            // parent process

            int status;

            while(1)
            {
                if (msgrcv(msqid, &rbuf, 256, 1, IPC_NOWAIT) > 0)
                {
                    if( strcmp(rbuf.mtext, "next") == 0)
                    {
                        index++;
                        kill(childpid, SIGTERM);
                        wait(&status);
                        break;
                    }
                    else if( strcmp(rbuf.mtext, "prev") == 0)
                    {
                        index--;
                        kill(childpid, SIGTERM);
                        wait(&status);
                        break;
                    }
                    else if( strcmp(rbuf.mtext, "stop") == 0)
                    {
                        kill(childpid, SIGTERM);
                        wait(&status);
                        msgctl( msqid, IPC_RMID, 0);
                        exit(0);
                    }
                    else if ( strcmp(rbuf.mtext, "start") == 0)
                    {
                        // start over again
                        index = 0;
                        // NOTE: not sure if we should reshuffle everything
                        if(mmcfg->GetShuffle())
                        {
                            std::random_shuffle(audiofiles.begin(), audiofiles.end());
                        }
                        kill(childpid, SIGTERM);
                        wait(&status);
                        break;
                    }
                }

                if (waitpid(childpid, &status, WNOHANG) != 0)
                {
                    if (!mmcfg->GetRepeat() && index < 0)
                    {
                        // if in non-repeat mode, don't auto increase the index
                        // after trying to play before first song
                    }
                    else
                        index++;
                    break;
                }

                sleep(1);
            }

            // if non-repeat mode, don't let the index get past -1
            if (!mmcfg->GetRepeat() && index < 0)
                index = -1;
            // if non-repeat mode, don't let the index get past size()
            else if (!mmcfg->GetRepeat() && index >= audiofiles.size())
                index = audiofiles.size();
            // ensure index isn't out of bounds
            else if (index < 0)
                index = audiofiles.size()-1;
            // ensure index isn't out of bounds
            else if (index >= audiofiles.size())
                index = 0;

        }
        else
        {
            // fork failed
            msgctl( msqid, IPC_RMID, 0);
            exit(1);
        }
    }
    msgctl( msqid, IPC_RMID, 0);
    exit(0);
}
