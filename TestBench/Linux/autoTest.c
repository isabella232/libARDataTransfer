/**
 * @file autoTest.c
 * @brief libARDataTransfer autoTest c file.
 **/


#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <libARSAL/ARSAL_Print.h>

#define ARACADEMY_AUTOTEST_TAG          "autoTest"

extern void test_manager(const char *tmp, int opt);
extern void test_manager_checking_running(const char *tmp);
extern void test_manager_checking_running_signal();

void sigIntHandler(int sig)
{
    printf("SIGINT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    test_manager_checking_running_signal();
}

void sigAlarmHandler(int sig)
{
    printf("SIGALRM !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}

int main(int argc, char *argv[])
{
    int opt = 0;
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARACADEMY_AUTOTEST_TAG, "options <-s, -a>, -s: sync tests, -a: async tests");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARACADEMY_AUTOTEST_TAG, "autoTest Starting");
    
    if (argc > 1)
    {
        if (strcmp(argv[1], "-s") == 0)
        { 
            opt = 1;
        }
        else if (strcmp(argv[1], "-a") == 0)
        { 
            opt = 2;
        }
    }

    //http://people.cs.pitt.edu/~alanjawi/cs449/code/shell/UnixSignals.htm
    signal(SIGINT, sigIntHandler);
    signal(SIGALRM, sigAlarmHandler);

    char *tmp = getenv("HOME");
    char tmpPath[512];
    strcpy(tmpPath, tmp);
    strcat(tmpPath, "/");

    //test_manager_checking_running(tmpPath);
    test_manager(tmpPath, opt);

    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARACADEMY_AUTOTEST_TAG, "autoTest Completed");
    return 0;
}
