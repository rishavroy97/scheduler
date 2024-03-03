#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <tuple>

// int myrandom(int burst)
// {
//     return 1 + (randvals[ofs] % burst);
// }

using namespace std;

class Process
{
};

class Scheduler
{
public:
    virtual void add_process(Process *p, bool quant) = 0;
    virtual Process *get_next_process() = 0;
    virtual void print_runq() = 0;
};

class FCFSScheduler : Scheduler
{
};

auto getScheduler(char *args)
{
    return make_tuple('A', 1, "qwerty");
}

void print_usage(char *filename)
{
    printf("Usage: %s [-v] [-t] [-e] [-p] [-i] [-s sched] inputfile randomfile\n"
           "-v enables verbose\n"
           "-t enables scheduler details\n"
           "-e enables event tracing\n"
           "-p enables E scheduler preempton tracing\n"
           "-i single steps event by event\n",
           filename);
}

int main(int argc, char **argv)
{
    bool verbose = false;
    bool sched_details = false;
    bool event_trace = false;
    bool preemption_trace = false;
    bool single_step = false;

    Scheduler *scheduler;
    int quantum;
    int max_priority;

    int option;
    while ((option = getopt(argc, argv, "vtepis:")) != -1)
    {
        switch (option)
        {
        case 'v':
            verbose = true;
            break;
        case 't':
            sched_details = true;
            break;
        case 'e':
            event_trace = true;
            break;
        case 'p':
            preemption_trace = true;
            break;
        case 'i':
            single_step = true;
            break;
        case 's':
        {
            auto result = getScheduler(optarg);
            printf("Result: %c, %d, %s\n", get<0>(result), get<1>(result), get<2>(result));
            break;
        }
        default:
            print_usage(argv[0]);
            exit(2);
        }
    }
}