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

enum Transitions
{
    TRANS_TO_READY,
    TRANS_TO_PREEMPT,
    TRANS_TO_RUN,
    TRANS_TO_BLOCK
};

class Process
{
private:
    static int process_count;
    int pid;

public:
    int arrival_time, total_cpu_time, cpu_burst, io_burst;
    int state_ts;
    int static_priority;
    int dynamic_priority;

    Process(string args)
    {
        pid = Process::process_count++;
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
        arrival_time = nums[0];
        total_cpu_time = nums[1];
        cpu_burst = nums[2];
        io_burst = nums[3];
    }

    int get_pid()
    {
        return pid;
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

class Event
{
public:
    Process *process;
    int timestamp;
    Transitions transition;
};

class DES_Layer
{
private:
    deque<Event *> eventQ;

public:
    Event *get_event()
    {
        return new Event();
    }

    int get_next_event_time()
    {
        return 0;
    }
};

/**
 * Global variables
 */

int RANDVALS[100000];           // initialize a list of random numbers
vector<Process *> PROCESSES;    // initialize a list of processes
int Process::process_count = 0; // set process count static data to 0
int CURRENT_TIME = 0;           // current CPU time
bool CALL_SCHEDULER = false;
Scheduler *SCHEDULER = nullptr;
Process *CURRENT_RUNNING_PROCESS = nullptr;
DES_Layer *DESPATCHER = nullptr;

bool VERBOSE = false;
bool SHOW_SCHED_DETAILS = false;
bool SHOW_EVENT_TRACE = false;
bool SHOW_PREEMPTION_TRACE = false;
bool SHOW_SINGLE_STEP = false;

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
    return 1 + (RANDVALS[ofs] % burst);
}

/**
 * Print error message for incorrect input arguments
 * @param - filename - executable file's name
 */
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

/**
 * Parse the numbers from the random-number file
 * @param - filename - random-number file
 */
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
        RANDVALS[c++] = stoi(line);
    }
}

/**
 * Parse the process information from the input file
 * @param - filename - input file
 */
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
        PROCESSES.push_back(new Process(line));
    }
}

/**
 * Start simulation
 */
void Simulation()
{
    Event *evt;
    while ((evt = DESPATCHER->get_event()))
    {
        Process *proc = evt->process; // this is the process the event works on
        CURRENT_TIME = evt->timestamp;
        int transition = evt->transition;
        int timeInPrevState = CURRENT_TIME - proc->state_ts; // for accounting
        delete evt;
        evt = nullptr; // remove cur event obj and don’t touch anymore
        switch (transition)
        { // encodes where we come from and where we go
        case TRANS_TO_READY:
            // must come from BLOCKED or CREATED
            // add to run queue, no event created
            CALL_SCHEDULER = true;
            break;
        case TRANS_TO_PREEMPT: // similar to TRANS_TO_READY // must come from RUNNING (preemption)
            // add to runqueue (no event is generated)
            CALL_SCHEDULER = true;
            break;
        case TRANS_TO_RUN:
            // create event for either preemption or blocking
            break;
        case TRANS_TO_BLOCK:
            // create an event for when process becomes READY again
            CALL_SCHEDULER = true;
            break;
        }
        if (CALL_SCHEDULER)
        {
            if (DESPATCHER->get_next_event_time() == CURRENT_TIME)
                continue;           // process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            if (CURRENT_RUNNING_PROCESS == nullptr)
            {
                CURRENT_RUNNING_PROCESS = SCHEDULER->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr)
                    continue;
                // create event to make this process runnable for same time.
            }
        }
    }
}

/**
 * Read command-line arguments and assign values to global variables
 *
 * @param - argc - total argument count
 * @param - argv - array of arguments
 */
void read_arguments(int argc, char **argv)
{
    int option;
    while ((option = getopt(argc, argv, "vtepis:")) != -1)
    {
        switch (option)
        {
        case 'v':
            VERBOSE = true;
            break;
        case 't':
            SHOW_SCHED_DETAILS = true;
            break;
        case 'e':
            SHOW_EVENT_TRACE = true;
            break;
        case 'p':
            SHOW_PREEMPTION_TRACE = true;
            break;
        case 'i':
            SHOW_SINGLE_STEP = true;
            break;
        case 's':
        {
            SCHEDULER = getScheduler(optarg);
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
}

int main(int argc, char **argv)
{
    read_arguments(argc, argv);

    if (!SCHEDULER)
    {
        SCHEDULER = new FCFSScheduler();
    }

    printf("%s\n", SCHEDULER->to_string().c_str());

    parse_randoms(argv[optind + 1]);
    parse_input(argv[optind]);

    for (Process *p : PROCESSES)
    {
        printf("%04d: %4d %4d %4d %4d |\n", p->get_pid(), p->arrival_time, p->total_cpu_time, p->cpu_burst, p->io_burst);
    }

    DESPATCHER = new DES_Layer;
}