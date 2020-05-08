#include <ncurses.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <cstdio>
#include <list>

const int PROC_DELAY = 1;

using namespace std;

int main(int argc, char** argv) {
    int reciver = 0;
    char userChoise = '\0';
    pid_t pid;
    list<pid_t> pidList;

    initscr();
    cbreak();
    noecho();

    sigset_t waitSet;
    sigemptyset(&waitSet);
    sigaddset(&waitSet, SIGUSR2);
    sigprocmask(SIG_BLOCK, &waitSet, NULL);

    while (true) {
        userChoise = getch();
        switch (userChoise) {
            case '+':
                pid = fork();
                switch (pid) {
                    case -1:
                        cout << "Erorr while creating child process! (fork)" << endl << endl;
                        exit(EXIT_FAILURE);
                    case 0:
                        execv("/home/Boss/systemProgrammingCourse/laba_2/childProcess", argv);
                        cout << "Error while loading child process (excec)!" << endl << endl;
                        exit(127);
                    default:
                        pidList.push_back(pid);
                        sleep(PROC_DELAY);
                        break;
                }
                break;

            case '-':
                if (pidList.empty()) {
                    cout << "There are no chidren to delete!" << endl;
                } else {
                    kill(pidList.back(), SIGKILL);
                    pidList.pop_back();
                }
                break;

            case 'q':
                if (!pidList.empty()) {
                    for (auto &childPid : pidList) {
                        kill(childPid, SIGKILL);
                    }
                    pidList.clear();
                }
                return 0;
            default:
                continue;
        }

        cin.ignore();

        for (auto &childPid: pidList) {
            kill(childPid, SIGUSR1);
            sigwait(&waitSet, &reciver);
        }
    }
}