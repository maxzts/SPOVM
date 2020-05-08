#include <iostream>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>

#define PID_STRING "Hello, I'm child! My PID is "
#define DELAY 1



using namespace std;

int main() {
    int reciver = 0;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGKILL);
    sigprocmask(SIG_BLOCK, &set, NULL);

    while (true) {
        sigwait(&set, &reciver);

        cout << PID_STRING << static_cast<int>(getpid()) << endl;
        kill(getppid(), SIGUSR2);
    }
    return 1;
}