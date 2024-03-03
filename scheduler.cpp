#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <tuple>

using namespace std;

class Process
{
public:
    int at, tc, cb, io;

    Process(string args)
    {
        char *buffer = new char[args.length() + 1];
        vector<int> nums;
        strcpy(buffer, args.c_str());

        char delims[] = " \t\n";

        char *token = strtok(buffer, delims);
        int num = stoi(token);
        nums.push_back(num);

        while (token != nullptr)
        {
            token = strtok(nullptr, delims);
            if (token != nullptr)
            {
                num = stoi(token);
                nums.push_back(num);
            }
        }
        at = nums[0];
        tc = nums[1];
        cb = nums[2];
        io = nums[3];
    }
};

class Scheduler
{
protected:
    int quantum;
    int max_priority;

public:
    Scheduler(int q = 10000, int maxprio = 4)
    {
        quantum = q;
        max_priority = maxprio;
    }

    virtual void add_process(Process *p, bool quant) = 0;
    virtual Process *get_next_process() = 0;
    virtual void print_runq() = 0;
    virtual string to_string() = 0;
};

class FCFSScheduler : public Scheduler
{
public:
    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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
    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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
    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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
    RRScheduler(int num) : Scheduler(num) {}

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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
    PriorityScheduler(int num, int maxprio) : Scheduler(num, maxprio)
    {
        if (max_priority <= 0)
            max_priority = 4;
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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
    PreemptivePriorityScheduler(int num, int maxprio) : Scheduler(num, maxprio)
    {
        if (max_priority <= 0)
            max_priority = 4;
    }

    void add_process(Process *p, bool quant)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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

class Event {

};

class DES_Layer {
private:
    deque<Event*> eventQ;
};

/**
 * Global variables
 */

int randvals[100000]; // variable to store random numbers
vector<Process> processes;
int ofs = 0; // offset

/**
 * Helper functions
 */

/**
 * check if a char* string has a character
 * @param str - actual string
 * @param ch - character to be searched
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

/**
 * Get random number from randvals
 * @param - burst - the corresponding CPU or IO burst
 *
 * @returns - random value in the range of 1,..,burst
 */
int myrandom(int burst)
{
    return 1 + (randvals[ofs] % burst);
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

/**
 * Get the appropriate scheduler based on the arguments
 * @param - args - string that needs to parsed to fetch the scheduler type
 *
 * @returns - the correct Scheduler based on the arguments
 */
Scheduler *getScheduler(char *args)
{
    switch (args[0])
    {
    case 'F':
        return new FCFSScheduler();
    case 'L':
        return new LCFSScheduler();
    case 'S':
        return new SJFScheduler();
    case 'R':
    {
        int quantum;
        sscanf(args, "R%d", &quantum);
        if (quantum <= 0)
        {
            printf("Invalid scheduler param <%s>\n", args);
            exit(1);
        }
        return new RRScheduler(quantum);
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
        return new PriorityScheduler(quantum, maxprio);
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
        return new PreemptivePriorityScheduler(quantum, maxprio);
    }
    default:
        printf("Unknown Scheduler spec: -v {FLSRPE}\n");
        exit(1);
    }
}

void parse_randoms(char *filename)
{
    fstream rand_file;
    rand_file.open(filename, ios::in);

    if (!rand_file.is_open())
    {
        printf("Not a valid inputfile <%s>\n", filename);
        exit(1);
    }

    string line;
    int c = 0;
    while (getline(rand_file, line))
    {
        randvals[c++] = stoi(line);
    }
}

void parse_input(char *filename)
{
    fstream input_file;

    input_file.open(filename, ios::in);

    if (!input_file.is_open())
    {
        printf("Not a valid inputfile <%s>\n", filename);
        exit(1);
    }

    string line;
    int c = 0;
    while (getline(input_file, line))
    {
        processes.push_back(Process(line));
    }
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

    parse_randoms(argv[optind + 1]);
    parse_input(argv[optind]);

    for (Process p : processes)
    {
        cout << p.at << " " << p.tc << " " << p.cb << " " << p.io << endl;
    }

    if (!scheduler)
    {
        scheduler = new FCFSScheduler();
    }

    printf("%s\n", scheduler->to_string().c_str());
}