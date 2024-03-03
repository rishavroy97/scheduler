#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <tuple>

using namespace std;

/**
 * Helper functions
 */

bool hasChar(const char *str, char ch)
{
    while (*str != '\0')
    {
        if (*str == ch)
        {
            return true;
        }
        str++;
    }
    return false;
}

// int myrandom(int burst)
// {
//     return 1 + (randvals[ofs] % burst);
// }

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

class Process
{
};

class Scheduler
{
protected:
    int quantum;
    int max_priority;

public:
    virtual void add_process(Process *p, bool quant) = 0;
    virtual Process *get_next_process() = 0;
    virtual void print_runq() = 0;
    virtual string to_string() = 0;
};

class FCFSScheduler : public Scheduler
{
public:
    FCFSScheduler()
    {
        quantum = 10000;
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process;
    }

    void print_runq()
    {
        /*code*/
    }

    string to_string()
    {
        return "FCFS";
    }
};

class LCFSScheduler : public Scheduler
{
public:
    LCFSScheduler()
    {
        quantum = 10000;
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process;
    }

    void print_runq()
    {
        /*code*/
    }

    string to_string()
    {
        return "LCFS";
    }
};

class SJFScheduler : public Scheduler
{
public:
    SJFScheduler()
    {
        quantum = 10000;
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process;
    }

    void print_runq()
    {
        /*code*/
    }

    string to_string()
    {
        return "SJFS";
    }
};

class RRScheduler : public Scheduler
{
public:
    RRScheduler(int num)
    {
        quantum = num;
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process;
    }

    void print_runq()
    {
        /*code*/
    }

    string to_string()
    {
        return "RR " + std::to_string(quantum);
    }
};

class PriorityScheduler : public Scheduler
{
public:
    PriorityScheduler(int num, int maxprio)
    {
        quantum = num;
        max_priority = maxprio;
    }

    PriorityScheduler(int num)
    {
        PriorityScheduler(num, 4);
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process;
    }

    void print_runq()
    {
        /*code*/
    }

    string to_string()
    {
        return "PRIO " + std::to_string(quantum);
    }
};

class PreemptivePriorityScheduler : public Scheduler
{
public:
    PreemptivePriorityScheduler(int num, int maxprio)
    {
        quantum = num;
        max_priority = maxprio;
    }

    PreemptivePriorityScheduler(int num)
    {
        PreemptivePriorityScheduler(num, 4);
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process;
    }

    void print_runq()
    {
        /*code*/
    }

    string to_string()
    {
        return "PREPRIO " + std::to_string(quantum);
    }
};

/**
 * Get the appropriate scheduler based on the arguments
 * @param - args - string that needs to parsed to fetch the scheduler type
 *
 * @returns - the correct Scheduler based on the arguments
 */
Scheduler *getScheduler(char *args)
{
    Scheduler *scheduler;
    switch (args[0])
    {
    case 'F':
    {
        scheduler = new FCFSScheduler();
        break;
    }
    case 'L':
    {
        scheduler = new LCFSScheduler();
        break;
    }
    case 'S':
    {
        scheduler = new SJFScheduler();
        break;
    }
    case 'R':
    {
        int quantum;
        sscanf(args, "R%d", &quantum);
        if (quantum <= 0)
        {
            printf("Invalid scheduler param <%s>\n", args);
            exit(1);
        }
        scheduler = new RRScheduler(quantum);
        break;
    }
    case 'P':
    {
        int quantum;
        int maxprio;
        sscanf(args, "P%d:%d", &quantum, &maxprio);
        if (quantum <= 0 || (hasChar(args, ':') && maxprio <= 0))
        {
            printf("Invalid scheduler param <%s>\n", args);
            exit(1);
        }
        scheduler = (maxprio <= 0) ? new PriorityScheduler(quantum) : new PriorityScheduler(quantum, maxprio);
        break;
    }
    case 'E':
    {
        int quantum;
        int maxprio;
        sscanf(args, "E%d:%d", &quantum, &maxprio);
        if (quantum <= 0 || (hasChar(args, ':') && maxprio <= 0))
        {
            printf("Invalid scheduler param <%s>\n", args);
            exit(1);
        }
        scheduler = (maxprio <= 0) ? new PreemptivePriorityScheduler(quantum) : new PreemptivePriorityScheduler(quantum, maxprio);
        break;
    }
    default:
        printf("Unknown Scheduler spec: -v {FLSRPE}\n");
        exit(1);
    }
    return scheduler;
}

void parse_input(char *filename)
{
    fstream new_file;

    new_file.open(filename, ios::in);

    if (!new_file.is_open())
    {
        printf("Not a valid inputfile <%s>\n", filename);
        exit(1);
    }

    cout<<"Input file"<<filename<<endl;
}

void parse_random(char *filename)
{
    fstream new_file;
    new_file.open(filename, ios::in);

    if (!new_file.is_open())
    {
        printf("Not a valid inputfile <%s>\n", filename);
        exit(1);
    }

    cout<<"Random file"<<filename<<endl;
}

int main(int argc, char **argv)
{
    bool verbose = false;
    bool sched_details = false;
    bool event_trace = false;
    bool preemption_trace = false;
    bool single_step = false;

    Scheduler *scheduler = nullptr;

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
            scheduler = getScheduler(optarg);
            break;
        }
        default:
            print_usage(argv[0]);
            exit(1);
        }
    }

    if (argc == optind)
    {
        printf("Not a valid inputfile <(null)>\n");
        exit(1);
    }

    if (argc == optind + 1)
    {
        printf("Not a valid random file <(null)>\n");
        exit(1);
    }

    parse_random(argv[optind + 1]);
    parse_input(argv[optind]);

    if (!scheduler)
    {
        scheduler = new FCFSScheduler();
    }

    printf("%s\n", scheduler->to_string().c_str());
}