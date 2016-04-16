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

void start_service();

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "usage: mm <command>" << std::endl;
        std::cout << "\nlist of command" << std::endl;
        std::cout <<   "---------------" << std::endl;
        std::cout << "start\tstarts the music player" << std::endl;
        std::cout << "stop\tstops the music player" << std::endl;
        std::cout << "next\tskip to next song in the playlist" << std::endl;
        return 0;
    }

    std::string cmd(argv[1]);
    if(cmd == "start")
    {
        if (msgget(9900, 0666) < 0)
        {
            start_service();
        }
        else
        {
            std::cout << "already running" << std::endl;
            exit(1);
        }
    }
    else
    {
        int msqid;
        if((msqid = msgget(9900, 0666)) < 0)
        {
            std::cout << "start the music-magister first ( use: mm start )" << std::endl;
        }
        else
        {
            struct mm_msgbuf sbuf;
            sbuf.mtype = 1;
            strcpy(sbuf.mtext, argv[1]);
            if (msgsnd(msqid, &sbuf, strlen(sbuf.mtext) + 1, IPC_NOWAIT) < 0)
            {
                exit(1);
            }
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

    if(mmcfg->GetShuffle())
    {
        std::random_shuffle(audiofiles.begin(), audiofiles.end());
    }

    for(auto file: audiofiles)
    {
        pid_t childpid = fork();
        if(childpid == 0)
        {
            // child process

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
                }

                if (waitpid(childpid, &status, WNOHANG) != 0)
                {
                    break;
                }

                sleep(1);
            }
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
