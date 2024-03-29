#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <map>

using namespace std;

enum Transitions {
    TRANS_TO_BLOCK,
    TRANS_TO_DONE,
    TRANS_TO_PREEMPT,
    TRANS_TO_READY,
    TRANS_TO_RUN,
};

enum Proc_State {
    BLOCKED,
    CREATED,
    PREEMPT,
    READY,
    RUNNING
};

class Process {
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

    explicit Process(const string &args) {
        arrival_time = total_cpu_time = cpu_burst = io_burst = 0;
        pid = Process::process_count++;
        char *buffer = new char[args.length() + 1];
        strcpy(buffer, args.c_str());

        sscanf(buffer, "%d %d %d %d", &arrival_time, &total_cpu_time, &cpu_burst, &io_burst);
        delete[] buffer;

        /** default initialization **/
        state = CREATED;
        state_start_time = arrival_time;
        remaining_cpu_time = total_cpu_time;
        curr_cpu_burst = 0;
        io_time = 0;
        cpu_wait_time = 0;
        finishing_time = arrival_time;
        static_priority = 1;
        dynamic_priority = 0;
    }

    [[nodiscard]] int get_pid() const {
        return pid;
    }
};

class Event {
public:
    Process *process;
    int timestamp;
    Transitions transition;

    explicit Event(Process *p) {
        process = p;
    }
};

class DES_Layer {
private:
    deque<Event *> eventQ;
public:

    /**
     * Add the created processes to the Event Queue
     */
    void initialize(const vector<Process *> &processes) {
        for (Process *p: processes) {
            auto *e = new Event(p);
            e->timestamp = p->arrival_time;
            e->transition = TRANS_TO_READY;
            put_event(e);
        }
    }

    Event *get_event() {
        if (eventQ.empty()) {
            return nullptr;
        }
        Event *e = eventQ.back();
        eventQ.pop_back();
        return e;
    }

    int get_next_event_time() {
        if (eventQ.empty()) {
            return -1;
        }
        return eventQ.back()->timestamp;
    }

    void put_event(Event *e) {
        eventQ.push_front(e);
        if (eventQ.size() == 1) {
            return;
        }

        Event *evt = eventQ[0];
        int j = 1;
        while (j < eventQ.size() && evt->timestamp < eventQ[j]->timestamp) {
            eventQ[j - 1] = eventQ[j];
            j = j + 1;
        }
        eventQ[j - 1] = evt;
    }

    bool has_pending_events(Process *process, int time) {
        int i = (int) eventQ.size() - 1;
        while (i >= 0) {
            Process *p = eventQ[i]->process;
            if (p->get_pid() == process->get_pid() && eventQ[i]->timestamp == time) {
                return true;
            }
            i--;
        }
        return false;
    }

    void remove_events(Process *process, int now) {

        auto itr = eventQ.begin();
        while (itr != eventQ.end()) {
            Event *e = *itr;
            if (e->process->get_pid() == process->get_pid() && e->timestamp != now) {
                itr = eventQ.erase(itr);
                delete e;
            } else {
                // move to next element
                ++itr;
            }
        }
    }
};

class Scheduler {
protected:
    int quantum;
    int max_priority;

public:
    explicit Scheduler(int q = 10000, int maxprio = 4) {
        quantum = q;
        max_priority = maxprio;
    }

    virtual void add_process(Process *p) = 0;

    virtual Process *get_next_process() = 0;

    virtual string to_string() = 0;

    virtual bool test_preempt(Process *activated_proc, Process *curr_proc, DES_Layer *dispatcher, int curr_time) = 0;

    [[nodiscard]] int get_maxprio() const {
        return max_priority;
    }

    [[nodiscard]] int get_quant() const {
        return quantum;
    }

    virtual ~Scheduler() = default;
};

class FCFSScheduler : public Scheduler {
private:
    deque<Process *> runQ;

public:
    void add_process(Process *p) override {
        runQ.push_front(p);
    }

    Process *get_next_process() override {
        if (runQ.empty()) {
            return nullptr;
        }
        Process *p = runQ.back();
        runQ.pop_back();
        return p;
    }

    bool test_preempt(Process *activated_proc, Process *curr_proc, DES_Layer *dispatcher, int curr_time) override {
        return false;
    }

    string to_string() override {
        return "FCFS";
    }
};

class LCFSScheduler : public Scheduler {
private:
    deque<Process *> runQ;

public:
    void add_process(Process *p) override {
        runQ.push_front(p);
    }

    Process *get_next_process() override {
        if (runQ.empty()) {
            return nullptr;
        }
        Process *p = runQ.front();
        runQ.pop_front();
        return p;
    }

    bool test_preempt(Process *activated_proc, Process *curr_proc, DES_Layer *dispatcher, int curr_time) override {
        return false;
    }

    string to_string() override {
        return "LCFS";
    }
};

class SRTFScheduler : public Scheduler {
private:
    deque<Process *> runQ;

public:
    void add_process(Process *p) override {
        runQ.push_front(p);
        if (runQ.size() == 1) {
            return;
        }

        Process *proc = runQ[0];
        int j = 1;
        while (j < runQ.size() && proc->remaining_cpu_time < runQ[j]->remaining_cpu_time) {
            runQ[j - 1] = runQ[j];
            j = j + 1;
        }
        runQ[j - 1] = proc;
    }

    Process *get_next_process() override {
        if (runQ.empty()) {
            return nullptr;
        }
        Process *p = runQ.back();
        runQ.pop_back();
        return p;
    }

    bool test_preempt(Process *activated_proc, Process *curr_proc, DES_Layer *dispatcher, int curr_time) override {
        return false;
    }

    string to_string() override {
        return "SRTF";
    }
};

class RRScheduler : public Scheduler {
private:
    deque<Process *> runQ;

public:
    explicit RRScheduler(int num) : Scheduler(num) {}

    void add_process(Process *p) override {
        p->dynamic_priority = p->static_priority - 1;
        runQ.push_front(p);
    }

    Process *get_next_process() override {
        if (runQ.empty()) {
            return nullptr;
        }
        Process *p = runQ.back();
        runQ.pop_back();
        return p;
    }

    bool test_preempt(Process *activated_proc, Process *curr_proc, DES_Layer *dispatcher, int curr_time) override {
        return false;
    }

    string to_string() override {
        return "RR " + std::to_string(quantum);
    }
};

class PriorityScheduler : public Scheduler {
private:
    vector<deque<Process *>> *activeRunQ = new vector<deque<Process *>>;
    vector<deque<Process *>> *expiredRunQ = new vector<deque<Process *>>;

    static bool is_empty(vector<deque<Process *>> *priorityQ) {
        for (const deque<Process *> &q: *priorityQ)
            if (!q.empty())
                return false;

        return true;
    }

public:
    PriorityScheduler(int num, int maxprio) : Scheduler(num, maxprio) {
        if (max_priority <= 0)
            max_priority = 4;

        activeRunQ->resize(max_priority);
        expiredRunQ->resize(max_priority);
    }

    void add_process(Process *p) override {

        int priority = p->dynamic_priority;

        if (priority < 0) {
            // add to expired queue
            priority = p->static_priority - 1;
            p->dynamic_priority = priority;
            deque<Process *> *expired_q = &((*expiredRunQ)[priority]);
            expired_q->push_front(p);

            return;
        }

        // add to active queue
        deque<Process *> *active_q = &(*activeRunQ)[priority];
        active_q->push_front(p);
    }

    Process *get_next_process() override {
        if (is_empty(activeRunQ) && is_empty(expiredRunQ)) {
            return nullptr;
        }

        if (is_empty(activeRunQ)) {
            activeRunQ->swap(*expiredRunQ);
        }

        deque<Process *> *highest_priority_q = nullptr;
        for (int i = max_priority - 1; i >= 0; i--) {
            if (!(*activeRunQ)[i].empty()) {
                highest_priority_q = &((*activeRunQ)[i]);
                break;
            }
        }

        if (highest_priority_q) {
            Process *p = highest_priority_q->back();
            highest_priority_q->pop_back();
            return p;
        }

        return nullptr;
    }

    bool test_preempt(Process *activated_proc, Process *curr_proc, DES_Layer *dispatcher, int curr_time) override {
        return false;
    }

    string to_string() override {
        return "PRIO " + std::to_string(quantum);
    }

    ~PriorityScheduler() override {
        delete activeRunQ;
        delete expiredRunQ;
    }
};

class PreemptivePriorityScheduler : public Scheduler {
private:
    vector<deque<Process *>> *activeRunQ = new vector<deque<Process *>>;
    vector<deque<Process *>> *expiredRunQ = new vector<deque<Process *>>;

    static bool is_empty(vector<deque<Process *>> *priorityQ) {
        for (const deque<Process *> &q: *priorityQ)
            if (!q.empty())
                return false;

        return true;
    }

public:
    PreemptivePriorityScheduler(int num, int maxprio) : Scheduler(num, maxprio) {
        if (max_priority <= 0)
            max_priority = 4;

        activeRunQ->resize(max_priority);
        expiredRunQ->resize(max_priority);
    }

    void add_process(Process *p) override {

        int priority = p->dynamic_priority;

        if (priority < 0) {
            // add to expired queue
            priority = p->static_priority - 1;
            p->dynamic_priority = priority;
            deque<Process *> *expired_q = &((*expiredRunQ)[priority]);
            expired_q->push_front(p);

            return;
        }

        // add to active queue
        deque<Process *> *active_q = &(*activeRunQ)[priority];
        active_q->push_front(p);
    }

    Process *get_next_process() override {
        if (is_empty(activeRunQ) && is_empty(expiredRunQ)) {
            return nullptr;
        }

        if (is_empty(activeRunQ)) {
            activeRunQ->swap(*expiredRunQ);
        }

        deque<Process *> *highest_priority_q = nullptr;
        for (int i = max_priority - 1; i >= 0; i--) {
            if (!(*activeRunQ)[i].empty()) {
                highest_priority_q = &((*activeRunQ)[i]);
                break;
            }
        }

        if (highest_priority_q) {
            Process *p = highest_priority_q->back();
            highest_priority_q->pop_back();
            return p;
        }

        return nullptr;
    }

    bool test_preempt(Process *activated_proc, Process *curr_proc, DES_Layer *dispatcher, int curr_time) override {
        if (curr_proc == nullptr) {
            return false;
        }
        if (activated_proc->get_pid() == curr_proc->get_pid()) {
            return false;
        }
        bool is_higher_priority = activated_proc->dynamic_priority > curr_proc->dynamic_priority;
        bool has_no_pending_events = dispatcher != nullptr && !dispatcher->has_pending_events(curr_proc, curr_time);
        return is_higher_priority && has_no_pending_events;
    }

    string to_string() override {
        return "PREPRIO " + std::to_string(quantum);
    }

    ~PreemptivePriorityScheduler() override {
        delete activeRunQ;
        delete expiredRunQ;
    }
};

/**
 * Global variables
 */
map<Proc_State, string> STATE_STRING = {
        {Proc_State::BLOCKED, "BLOCK"},
        {Proc_State::CREATED, "CREATED"},
        {Proc_State::PREEMPT, "PREEMPT"},
        {Proc_State::READY,   "READY"},
        {Proc_State::RUNNING, "RUNNG"}};      // convert enums to strings
int RAND_COUNT = 0;                         // total number of random numbers in file
vector<int> RANDVALS;                       // initialize a list of random numbers
int OFS = 0;                                // line offset for the random file
vector<Process *> PROCESSES;                // initialize a list of processes
int Process::process_count = 0;             // set process count static data to 0
int CURRENT_TIME = 0;                       // current CPU time
int BLOCKED_PROCESS_COUNT = 0;              // total number of blocked process at a particular time
int TIME_IO_BUSY = 0;                       // time at least one process is performing IO
bool CALL_SCHEDULER = false;                // flag to call the next process in the scheduler
Scheduler *SCHEDULER = nullptr;             // Scheduler instance being used in simulation
Process *CURRENT_RUNNING_PROCESS = nullptr; // pointer to the current running process
DES_Layer *DISPATCHER = nullptr;            // DES Layer being used in the simulation
bool VERBOSE = false;                       // flag to display extra information for every event

/** Not implemented these features **/
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
bool hasChar(const char *str, char ch) {
    while (*str != '\0') {
        if (*str == ch) {
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
int get_random(int burst) {
    int offset = OFS % RAND_COUNT;
    int random = 1 + (RANDVALS[offset] % burst);
    OFS++;
    return random;
}

/**
 * Print error message for incorrect input arguments
 * @param - filename - executable file's name
 */
void print_usage(char *filename) {
    printf("Usage: %s [-v] [-t] [-e] [-p] [-i] [-s sched] inputfile randomfile\n"
           "-v enables verbose\n"
           "-t enables scheduler details\n"
           "-e enables event tracing\n"
           "-p enables E scheduler preemption tracing\n"
           "-i single steps event by event\n",
           filename);
}

/**
 * Get the appropriate scheduler based on the arguments
 * @param - args - string that needs to parsed to fetch the scheduler type
 *
 * @returns - the correct Scheduler based on the arguments
 */
Scheduler *getScheduler(char *args) {
    switch (args[0]) {
        case 'F':
            return new FCFSScheduler();
        case 'L':
            return new LCFSScheduler();
        case 'S':
            return new SRTFScheduler();
        case 'R': {
            int quantum;
            sscanf(args, "R%d", &quantum);
            if (quantum <= 0) {
                printf("Invalid scheduler param <%s>\n", args);
                exit(1);
            }
            return new RRScheduler(quantum);
        }
        case 'P': {
            int quantum = 0;
            int maxprio = 0;
            sscanf(args, "P%d:%d", &quantum, &maxprio);
            if (quantum <= 0 || (hasChar(args, ':') && maxprio <= 0)) {
                printf("Invalid scheduler param <%s>\n", args);
                exit(1);
            }
            return new PriorityScheduler(quantum, maxprio);
        }
        case 'E': {
            int quantum = 0;
            int maxprio = 0;
            sscanf(args, "E%d:%d", &quantum, &maxprio);
            if (quantum <= 0 || (hasChar(args, ':') && maxprio <= 0)) {
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
void parse_randoms(char *filename) {
    fstream rand_file;
    rand_file.open(filename, ios::in);

    if (!rand_file.is_open()) {
        printf("Not a valid inputfile <%s>\n", filename);
        exit(1);
    }

    string line;
    getline(rand_file, line);
    RAND_COUNT = stoi(line);

    for (int i = 0; i < RAND_COUNT; i++) {
        getline(rand_file, line);
        RANDVALS.push_back(stoi(line));
    }
}

/**
 * Parse the process information from the input file
 * @param - filename - input file
 */
void load_processes(char *filename) {
    fstream input_file;

    input_file.open(filename, ios::in);

    if (!input_file.is_open()) {
        printf("Not a valid inputfile <%s>\n", filename);
        exit(1);
    }

    string line;
    while (getline(input_file, line)) {
        auto *p = new Process(line);
        /** Initialize the static and dynamic priorities **/
        p->static_priority = SCHEDULER ? get_random(SCHEDULER->get_maxprio()) : get_random(4);
        p->dynamic_priority = p->static_priority - 1;
        PROCESSES.push_back(p);
    }
}

/**
 * Start simulation
 */
void run_simulation() {
    int io_busy_start_time = 0;
    Event *evt;
    while ((evt = DISPATCHER->get_event())) {
        Process *proc = evt->process; // this is the process the event works on
        CURRENT_TIME = evt->timestamp;
        Transitions transition = evt->transition;
        int timeInPrevState = CURRENT_TIME - proc->state_start_time; // for accounting
        delete evt; // delete cur event obj and don’t touch anymore

        switch (transition) {
            case TRANS_TO_READY: {
                /** must come from BLOCKED or CREATED **/
                if (proc->state != BLOCKED && proc->state != CREATED && proc->state != RUNNING) {
                    printf("TRANS_TO_READY - Incorrect incoming state - %s, expected BLOCKED/CREATED/RUNNING\n",
                           STATE_STRING[proc->state].c_str());
                    exit(1);
                }

                if (VERBOSE)
                    printf("%d %d %d: %s -> %s\n",
                           CURRENT_TIME, proc->get_pid(), timeInPrevState,
                           STATE_STRING[proc->state].c_str(), STATE_STRING[READY].c_str());
                if (proc->state == BLOCKED) {
                    /** perform accounting for BLOCKED to READY **/
                    proc->io_time += timeInPrevState;
                    BLOCKED_PROCESS_COUNT--;
                    if (BLOCKED_PROCESS_COUNT == 0) {
                        TIME_IO_BUSY += CURRENT_TIME - io_busy_start_time;
                        io_busy_start_time = 0;
                    }
                    // reset dynamic priority
                    proc->dynamic_priority = proc->static_priority - 1;
                }

                /** add process to run queue, no event created **/
                proc->state_start_time = CURRENT_TIME;
                proc->state = READY;

                /** new process ready, preempt the current running process **/
                if (SCHEDULER->test_preempt(proc, CURRENT_RUNNING_PROCESS, DISPATCHER, CURRENT_TIME)) {
                    // remove the later events
                    DISPATCHER->remove_events(CURRENT_RUNNING_PROCESS, CURRENT_TIME);

                    // preempt the current running process
                    Process *p = CURRENT_RUNNING_PROCESS;
                    auto *preprio_event = new Event(p);
                    preprio_event->timestamp = CURRENT_TIME;
                    preprio_event->transition = TRANS_TO_PREEMPT;
                    DISPATCHER->put_event(preprio_event);
                }

                SCHEDULER->add_process(proc);
                CALL_SCHEDULER = true;
                break;
            }
            case TRANS_TO_PREEMPT: // similar to TRANS_TO_READY
            {
                if (proc->state != RUNNING) {
                    printf("TRANS_TO_PREEMPT - Incorrect incoming state - %s, expected RUNNING\n",
                           STATE_STRING[proc->state].c_str());
                    exit(1);
                }
                /** perform accounting for RUNNING to PREEMPT **/
                proc->remaining_cpu_time -= timeInPrevState;
                proc->curr_cpu_burst -= timeInPrevState;

                /** must come from RUNNING (preemption) **/
                if (VERBOSE)
                    printf("%d %d %d: %s -> %s  cb=%d rem=%d prio=%d\n",
                           CURRENT_TIME, proc->get_pid(), timeInPrevState,
                           STATE_STRING[proc->state].c_str(), STATE_STRING[READY].c_str(),
                           proc->curr_cpu_burst, proc->remaining_cpu_time, proc->dynamic_priority);
                if (proc == CURRENT_RUNNING_PROCESS) {
                    CURRENT_RUNNING_PROCESS = nullptr;
                }

                /** add process to run queue, no event created **/
                proc->dynamic_priority--;
                proc->state_start_time = CURRENT_TIME;
                proc->state = READY;

                /** new process ready, preempt the current running process **/
                if (SCHEDULER->test_preempt(proc, CURRENT_RUNNING_PROCESS, DISPATCHER, CURRENT_TIME)) {
                    // remove the later events
                    DISPATCHER->remove_events(CURRENT_RUNNING_PROCESS, CURRENT_TIME);

                    // preempt the current running process
                    Process *p = CURRENT_RUNNING_PROCESS;
                    auto *preprio_event = new Event(p);
                    preprio_event->timestamp = CURRENT_TIME;
                    preprio_event->transition = TRANS_TO_PREEMPT;
                    DISPATCHER->put_event(preprio_event);
                }

                SCHEDULER->add_process(proc);
                CALL_SCHEDULER = true;
                break;
            }
            case TRANS_TO_RUN: {
                if (proc->state != READY) {
                    printf("TRANS_TO_RUN - Incorrect incoming state - %s, expected READY\n",
                           STATE_STRING[proc->state].c_str());
                    exit(1);
                }
                /** perform accounting READY to RUNNING **/
                proc->cpu_wait_time += timeInPrevState;

                /** calculations for new state **/
                if (proc->curr_cpu_burst == 0) {
                    int cb = get_random(proc->cpu_burst);
                    if (proc->remaining_cpu_time < cb)
                        cb = proc->remaining_cpu_time;
                    proc->curr_cpu_burst = cb;
                }
                CURRENT_RUNNING_PROCESS = proc;

                /** create event for either preemption or blocking */
                if (SCHEDULER->get_quant() < proc->curr_cpu_burst) {
                    /** create event for preemption **/
                    auto *preempt_event = new Event(proc);
                    preempt_event->timestamp = CURRENT_TIME + SCHEDULER->get_quant();
                    preempt_event->transition = TRANS_TO_PREEMPT;
                    DISPATCHER->put_event(preempt_event);
                } else if (proc->curr_cpu_burst == proc->remaining_cpu_time) {
                    /** create event for done **/
                    auto *done_event = new Event(proc);
                    done_event->timestamp = CURRENT_TIME + proc->curr_cpu_burst;
                    done_event->transition = TRANS_TO_DONE;
                    DISPATCHER->put_event(done_event);
                } else {
                    /** create event for blocking **/
                    auto *block_event = new Event(proc);
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
            case TRANS_TO_BLOCK: {
                if (proc->state != RUNNING) {
                    printf("TRANS_TO_BLOCK - Incorrect incoming state - %s, expected RUNNING\n",
                           STATE_STRING[proc->state].c_str());
                    exit(1);
                }

                /** perform accounting RUNNING to BLOCK **/
                proc->remaining_cpu_time -= timeInPrevState;
                proc->curr_cpu_burst = 0;
                CURRENT_RUNNING_PROCESS = nullptr;

                /** calculations for new state **/
                int ib = get_random(proc->io_burst);
                BLOCKED_PROCESS_COUNT++;
                if (BLOCKED_PROCESS_COUNT == 1) {
                    io_busy_start_time = CURRENT_TIME;
                }

                /** create an event for when process becomes READY again **/
                auto *ready_event = new Event(proc);
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
            case TRANS_TO_DONE: {
                if (proc->state != RUNNING) {
                    printf("TRANS_TO_DONE - Incorrect incoming state - %s, expected RUNNING\n",
                           STATE_STRING[proc->state].c_str());
                    exit(1);
                }

                /** perform accounting RUNNING to DONE **/
                proc->finishing_time = CURRENT_TIME;
                CURRENT_RUNNING_PROCESS = nullptr;

                if (VERBOSE)
                    printf("%d %d %d: Done\n", CURRENT_TIME, proc->get_pid(), timeInPrevState);
                CALL_SCHEDULER = true;
                break;
            }
        }

        if (CALL_SCHEDULER) {
            if (DISPATCHER->get_next_event_time() == CURRENT_TIME)
                continue;           // process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            if (CURRENT_RUNNING_PROCESS == nullptr) {
                CURRENT_RUNNING_PROCESS = SCHEDULER->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr)
                    continue;

                /** create event to make this process runnable for same time **/
                auto *run_event = new Event(CURRENT_RUNNING_PROCESS);
                run_event->timestamp = CURRENT_TIME;
                run_event->transition = TRANS_TO_RUN;
                DISPATCHER->put_event(run_event);
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
void read_arguments(int argc, char **argv) {
    int option;
    while ((option = getopt(argc, argv, "vtepis:")) != -1) {
        switch (option) {
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
            case 's': {
                SCHEDULER = getScheduler(optarg);
                break;
            }
            default:
                print_usage(argv[0]);
                exit(1);
        }
    }

    if (argc == optind) {
        printf("Not a valid inputfile <(null)>\n");
        exit(1);
    }

    if (argc == optind + 1) {
        printf("Not a valid random file <(null)>\n");
        exit(1);
    }

    if (!SCHEDULER) {
        SCHEDULER = new FCFSScheduler();
    }
}

/**
 * Print the scheduling output in the format expected for grading
 */
void print_output() {
    printf("%s\n", SCHEDULER->to_string().c_str());

    int time_cpu_busy = 0;
    int num_processes = 0;
    int total_turnaround = 0;
    int total_cpu_wait = 0;
    int finish_time = CURRENT_TIME;

    for (Process *p: PROCESSES) {
        int turnaround_time = p->finishing_time - p->arrival_time;
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
               p->get_pid(), p->arrival_time, p->total_cpu_time, p->cpu_burst, p->io_burst,
               p->static_priority, p->finishing_time, turnaround_time, p->io_time, p->cpu_wait_time);

        total_turnaround += turnaround_time;
        total_cpu_wait += p->cpu_wait_time;
        time_cpu_busy += turnaround_time - p->io_time - p->cpu_wait_time;
        num_processes += 1;
    }

    double cpu_util = 100.0 * (time_cpu_busy / (double) finish_time);
    double io_util = 100.0 * (TIME_IO_BUSY / (double) finish_time);
    double avg_turnaround_time = (total_turnaround / (double) num_processes);
    double avg_cpu_wait_time = (total_cpu_wait / (double) num_processes);
    double throughput = 100.0 * (num_processes / (double) finish_time);

    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
           finish_time, cpu_util, io_util, avg_turnaround_time, avg_cpu_wait_time, throughput);
}

/**
 * Deallocate memory used in the program
 */
void garbage_collection() {
    delete SCHEDULER;
    delete DISPATCHER;

    for (Process *p: PROCESSES)
        delete p;
}

int main(int argc, char **argv) {
    read_arguments(argc, argv);
    parse_randoms(argv[optind + 1]);
    load_processes(argv[optind]);

    DISPATCHER = new DES_Layer();
    DISPATCHER->initialize(PROCESSES);
    run_simulation();

    print_output();

    garbage_collection();
}