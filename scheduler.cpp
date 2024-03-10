#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <map>

using namespace std;

enum Transitions
{
    TRANS_TO_BLOCK,
    TRANS_TO_DONE,
    TRANS_TO_PREEMPT,
    TRANS_TO_READY,
    TRANS_TO_RUN,
};

enum Proc_State
{
    BLOCKED,
    CREATED,
    PREEMPT,
    READY,
    RUNNING
};

class Process
{
private:
    static int process_count;
    int pid;

public:
    int arrival_time, total_cpu_time, cpu_burst, io_burst;
    int state_start_time;   // used to calculate time spent in state
    int curr_cpu_burst;     // time remaining in current cpu burst
    int remaining_cpu_time; // time remaining in total cpu time
    int finishing_time;     // time when the process finished
    int io_time;            // time spent in blocked state
    int cpu_wait_time;      // time spent in ready state
    int static_priority;    // static priority
    int dynamic_priority;   // dynamic priority
    Proc_State state;       // current

    Process(string args)
    {
        pid = Process::process_count++;
        char *buffer = new char[args.length() + 1];
        strcpy(buffer, args.c_str());

        sscanf(buffer, "%d %d %d %d", &arrival_time, &total_cpu_time, &cpu_burst, &io_burst);

        /** default initialization **/
        state = CREATED;
        state_start_time = arrival_time;
        remaining_cpu_time = total_cpu_time;
        io_time = 0;
        cpu_wait_time = 0;
        finishing_time = arrival_time;
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

    virtual void add_process(Process *p) = 0;
    virtual Process *get_next_process() = 0;
    virtual string to_string() = 0;

    int get_maxprio()
    {
        return max_priority;
    }

    int get_quant()
    {
        return quantum;
    }
};

class FCFSScheduler : public Scheduler
{
private:
    deque<Process *> runQ;

public:
    void add_process(Process *p)
    {
        runQ.push_front(p);
    }

    Process *get_next_process()
    {
        if (runQ.empty())
        {
            return nullptr;
        }
        Process *p = runQ.back();
        runQ.pop_back();
        return p;
    }

    string to_string()
    {
        return "FCFS";
    }
};

class LCFSScheduler : public Scheduler
{
private:
    deque<Process *> runQ;

public:
    void add_process(Process *p)
    {
        runQ.push_front(p);
    }

    Process *get_next_process()
    {
        if (runQ.empty())
        {
            return nullptr;
        }
        Process *p = runQ.front();
        runQ.pop_front();
        return p;
    }

    string to_string()
    {
        return "LCFS";
    }
};

class SRTFcheduler : public Scheduler
{
private:
    deque<Process *> runQ;

public:
    void add_process(Process *p)
    {
        runQ.push_front(p);
        if (runQ.size() == 1)
        {
            return;
        }

        Process *proc = runQ[0];
        int j = 1;
        while (j < runQ.size() && proc->remaining_cpu_time < runQ[j]->remaining_cpu_time)
        {
            runQ[j - 1] = runQ[j];
            j = j + 1;
        }
        runQ[j - 1] = proc;
    }

    Process *get_next_process()
    {
        if (runQ.empty())
        {
            return nullptr;
        }
        Process *p = runQ.back();
        runQ.pop_back();
        return p;
    }

    string to_string()
    {
        return "SRTF";
    }
};

class RRScheduler : public Scheduler
{
public:
    RRScheduler(int num) : Scheduler(num) {}

    void add_process(Process *p)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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

    void add_process(Process *p)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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

    void add_process(Process *p)
    {
        /*code*/
    }

    Process *get_next_process()
    {
        return new Process("");
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

    Event(Process *p)
    {
        process = p;
    }
};

class DES_Layer
{
    // private:

public:
    // TODO: Make member private
    deque<Event *> eventQ;
    /**
     * Add the created processes to the Event Queue
     */
    void initialize(vector<Process *> processes)
    {
        for (Process *p : processes)
        {
            Event *e = new Event(p);
            e->timestamp = p->arrival_time;
            e->transition = TRANS_TO_READY;
            put_event(e);
        }
    }

    Event *get_event()
    {
        if (eventQ.empty())
        {
            return nullptr;
        }
        Event *e = eventQ.back();
        eventQ.pop_back();
        return e;
    }

    int get_next_event_time()
    {
        if (eventQ.empty())
        {
            return -1;
        }
        return eventQ.back()->timestamp;
    }

    void put_event(Event *e)
    {
        eventQ.push_front(e);
        if (eventQ.size() == 1)
        {
            return;
        }

        Event *evt = eventQ[0];
        int j = 1;
        while (j < eventQ.size() && evt->timestamp < eventQ[j]->timestamp)
        {
            eventQ[j - 1] = eventQ[j];
            j = j + 1;
        }
        eventQ[j - 1] = evt;
    }
};

/**
 * Global variables
 */

map<Proc_State, string> STATE_STRING = {
    {Proc_State::BLOCKED, "BLOCK"},
    {Proc_State::CREATED, "CREATED"},
    {Proc_State::PREEMPT, "PREEMPT"},
    {Proc_State::READY, "READY"},
    {Proc_State::RUNNING, "RUNNG"}}; // convert enums to strings

/** TODO - DElete this **/
map<Transitions, string> Transitions_Strings = {
    {Transitions::TRANS_TO_BLOCK, "TRANS_TO_BLOCK"},
    {Transitions::TRANS_TO_DONE, "TRANS_TO_DONE"},
    {Transitions::TRANS_TO_PREEMPT, "TRANS_TO_PREEMPT"},
    {Transitions::TRANS_TO_READY, "TRANS_TO_READY"},
    {Transitions::TRANS_TO_RUN, "TRANS_TO_RUN"}}; // convert enums to strings
int RANDVALS[100000];                             // initialize a list of random numbers
int OFS = 0;                                      // line offset for the random file
vector<Process *> PROCESSES;                      // initialize a list of processes
int Process::process_count = 0;                   // set process count static data to 0
int CURRENT_TIME = 0;                             // current CPU time
bool CALL_SCHEDULER = false;
Scheduler *SCHEDULER = nullptr;
Process *CURRENT_RUNNING_PROCESS = nullptr;
DES_Layer *DISPATCHER = nullptr;
bool VERBOSE = false;
bool SHOW_SCHED_DETAILS = false;
bool SHOW_EVENT_TRACE = false;
bool SHOW_PREEMPTION_TRACE = false;
bool SHOW_SINGLE_STEP = false;

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
    int random = 1 + (RANDVALS[OFS] % burst);
    OFS++;
    return random;
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
        return new SRTFcheduler();
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
    getline(rand_file, line);
    int count = stoi(line);
    for (int i = 0; i < count; i++)
    {
        getline(rand_file, line);
        RANDVALS[i] = stoi(line);
    }
}

/**
 * Parse the process information from the input file
 * @param - filename - input file
 */
void load_processes(char *filename)
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
        Process *p = new Process(line);
        /** Initialize the static and dynamic priorities **/
        p->static_priority = SCHEDULER ? myrandom(SCHEDULER->get_maxprio()) : myrandom(4);
        p->dynamic_priority = p->static_priority - 1;
        PROCESSES.push_back(p);
    }
}

/**
 * Start simulation
 */
void run_simulation()
{
    Event *evt;
    while ((evt = DISPATCHER->get_event()))
    {
        Process *proc = evt->process; // this is the process the event works on
        CURRENT_TIME = evt->timestamp;
        Transitions transition = evt->transition;
        int timeInPrevState = CURRENT_TIME - proc->state_start_time; // for accounting
        delete evt;
        evt = nullptr; // remove cur event obj and don’t touch anymore

        switch (transition)
        {
        case TRANS_TO_READY:
        {
            /** must come from BLOCKED or CREATED **/
            if (VERBOSE)
                printf("%d %d %d: %s -> %s\n",
                       CURRENT_TIME, proc->get_pid(), timeInPrevState,
                       STATE_STRING[proc->state].c_str(), STATE_STRING[READY].c_str());
            if (proc->state == BLOCKED)
            {
                /** perform accounting for BLOCKED to READY **/
                proc->io_time += timeInPrevState;
            }

            /** add process to run queue, no event created **/
            proc->state_start_time = CURRENT_TIME;
            proc->state = READY;

            SCHEDULER->add_process(proc);
            CALL_SCHEDULER = true;
            break;
        }
        case TRANS_TO_PREEMPT:
        {
            /**
             * Unknown Case
             */
            if (VERBOSE)
                printf("%d %d %d: %s -> %s\n",
                       CURRENT_TIME, proc->get_pid(), timeInPrevState,
                       STATE_STRING[proc->state].c_str(), STATE_STRING[PREEMPT].c_str());
            if (proc == CURRENT_RUNNING_PROCESS)
            {
                CURRENT_RUNNING_PROCESS = nullptr;
            }
            // add to runqueue (no event is generated)
            proc->state_start_time = CURRENT_TIME;
            proc->state = PREEMPT;

            SCHEDULER->add_process(proc);
            CALL_SCHEDULER = true;
            break;
        }
        case TRANS_TO_RUN:
        {
            /** perform accounting READY to RUNNING **/
            proc->cpu_wait_time += timeInPrevState;

            /** calculations for new state **/
            if (proc->curr_cpu_burst == 0)
            {
                int cb = myrandom(proc->cpu_burst);
                if (proc->remaining_cpu_time < cb)
                    cb = proc->remaining_cpu_time;
                proc->curr_cpu_burst = cb;
            }
            CURRENT_RUNNING_PROCESS = proc;

            /** create event for either preemption or blocking */
            if (SCHEDULER->get_quant() < proc->curr_cpu_burst)
            {
                /** create event for preemption **/
                Event *preempt_event = new Event(proc);
                preempt_event->timestamp = CURRENT_TIME + SCHEDULER->get_quant();
                preempt_event->transition = TRANS_TO_PREEMPT;
                DISPATCHER->put_event(preempt_event);
            }
            else if (proc->curr_cpu_burst == proc->remaining_cpu_time)
            {
                /** create event for done **/
                Event *done_event = new Event(proc);
                done_event->timestamp = CURRENT_TIME + proc->curr_cpu_burst;
                done_event->transition = TRANS_TO_DONE;
                DISPATCHER->put_event(done_event);
            }
            else
            {
                /** create event for blocking **/
                Event *block_event = new Event(proc);
                block_event->timestamp = CURRENT_TIME + proc->curr_cpu_burst;
                block_event->transition = TRANS_TO_BLOCK;
                DISPATCHER->put_event(block_event);
            }

            if (VERBOSE)
                printf("%d %d %d: %s -> %s cb=%d rem=%d prio=%d\n",
                       CURRENT_TIME, proc->get_pid(), timeInPrevState,
                       STATE_STRING[proc->state].c_str(), STATE_STRING[RUNNING].c_str(),
                       proc->curr_cpu_burst, proc->remaining_cpu_time, proc->dynamic_priority);

            proc->state_start_time = CURRENT_TIME;
            proc->state = RUNNING;

            CALL_SCHEDULER = true;
            break;
        }
        case TRANS_TO_BLOCK:
        {
            /** perform accounting RUNNING to BLOCK **/
            proc->remaining_cpu_time -= timeInPrevState;
            proc->curr_cpu_burst = 0;
            CURRENT_RUNNING_PROCESS = nullptr;

            /** calculations for new state **/
            int ib = myrandom(proc->io_burst);

            /** create an event for when process becomes READY again **/
            Event *ready_event = new Event(proc);
            ready_event->timestamp = CURRENT_TIME + ib;
            ready_event->transition = TRANS_TO_READY;
            DISPATCHER->put_event(ready_event);

            if (VERBOSE)
                printf("%d %d %d: %s -> %s  ib=%d rem=%d\n",
                       CURRENT_TIME, proc->get_pid(), timeInPrevState,
                       STATE_STRING[proc->state].c_str(), STATE_STRING[BLOCKED].c_str(),
                       ib, proc->remaining_cpu_time);

            proc->state_start_time = CURRENT_TIME;
            proc->state = BLOCKED;

            CALL_SCHEDULER = true;
            break;
        }
        case TRANS_TO_DONE:
        {
            /** perform accounting RUNNING to DONE **/
            proc->finishing_time = CURRENT_TIME;
            CURRENT_RUNNING_PROCESS = nullptr;

            if (VERBOSE)
                printf("%d %d %d: Done\n", CURRENT_TIME, proc->get_pid(), timeInPrevState);
            CALL_SCHEDULER = true;
            break;
        }
        }

        if (CALL_SCHEDULER)
        {
            if (DISPATCHER->get_next_event_time() == CURRENT_TIME)
                continue;           // process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            if (CURRENT_RUNNING_PROCESS == nullptr)
            {
                CURRENT_RUNNING_PROCESS = SCHEDULER->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr)
                    continue;

                /** create event to make this process runnable for same time **/
                Event *run_event = new Event(CURRENT_RUNNING_PROCESS);
                run_event->timestamp = CURRENT_TIME;
                run_event->transition = TRANS_TO_RUN;
                DISPATCHER->put_event(run_event);
            }
        }

        /**TODO - remove console logs **/
        // cout << "####################################################################################\n";
        // cout << "Current PID - " << CURRENT_RUNNING_PROCESS->get_pid() << endl;
        // cout << "Current Event Queue\n";
        // for (Event *e : DISPATCHER->eventQ)
        // {
        //     cout << "Event: " << e->process->get_pid() << " : " << e->timestamp << " : " << Transitions_Strings[e->transition] << endl;
        // }
        // cout << "####################################################################################\n";
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

    if (!SCHEDULER)
    {
        SCHEDULER = new FCFSScheduler();
    }
}

/**
 * Print the scheduling output in the format expected for grading
 */
void print_output()
{
    printf("%s\n", SCHEDULER->to_string().c_str());

    int time_cpubusy = 0;
    int time_iobusy = 0;
    int num_processes = 0;
    int total_turnaround = 0;
    int total_cpu_wait = 0;
    int finishtime = CURRENT_TIME;

    for (Process *p : PROCESSES)
    {
        int turnaround_time = p->finishing_time - p->arrival_time;
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
               p->get_pid(), p->arrival_time, p->total_cpu_time, p->cpu_burst, p->io_burst,
               p->static_priority, p->finishing_time, turnaround_time, p->io_time, p->cpu_wait_time);

        total_turnaround += turnaround_time;
        total_cpu_wait += p->cpu_wait_time;
        time_cpubusy += turnaround_time - p->io_time - p->cpu_wait_time;
        time_iobusy += p->io_time;
        num_processes += 1;
    }

    double cpu_util = 100.0 * (time_cpubusy / (double)finishtime);
    double io_util = 100.0 * (time_iobusy / (double)finishtime);
    double avg_turnaround_time = (total_turnaround / (double)num_processes);
    double avg_cpu_wait_time = (total_cpu_wait / (double)num_processes);
    double throughput = 100.0 * (num_processes / (double)finishtime);

    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
           finishtime, cpu_util, io_util, avg_turnaround_time, avg_cpu_wait_time, throughput);
}

int main(int argc, char **argv)
{
    read_arguments(argc, argv);
    parse_randoms(argv[optind + 1]);
    load_processes(argv[optind]);

    DISPATCHER = new DES_Layer();
    DISPATCHER->initialize(PROCESSES);
    run_simulation();

    print_output();
}