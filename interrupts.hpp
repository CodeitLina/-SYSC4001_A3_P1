#ifndef INTERRUPTS_HPP_
#define INTERRUPTS_HPP_

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<random>
#include<utility>
#include<sstream>
#include<iomanip>
#include<queue>

#include<stdio.h>

int PID_count = 11;

enum states {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED,
    NOT_ASSIGNED
};
std::ostream& operator<<(std::ostream& os, const enum states& s) {

	std::string state_names[] = {
                                "NEW",
                                "READY",
                                "RUNNING",
                                "WAITING",
                                "TERMINATED",
                                "NOT_ASSIGNED"
    };
    return (os << state_names[s]);
}

struct memory_partition{
    unsigned int    partition_number;
    unsigned int    size;
    int             occupied;
} memory_paritions[] = {
    {1, 40, -1},
    {2, 25, -1},
    {3, 15, -1},
    {4, 10, -1},
    {5, 8, -1},
    {6, 2, -1}
};

struct PCB{
    unsigned int    PID;
    unsigned int    size;
    unsigned int    arrival_time;
    unsigned int    start_time;
    unsigned int    processing_time;
    unsigned int    remaining_time;
    int             partition_number;
    enum states     state;
    unsigned int    io_freq;
    unsigned int    io_duration;
};

class SimpleLCG {
    public:
    SimpleLCG(uint32_t seed) : current(seed) {}

    // Generate a random number using LCG
    uint32_t generate() {
        current = (current * 1664525 + 1013904223) % (1u << 31);
        return current;
    }

    // Get a number in the desired range
    int get_random(int min, int max) {
        return min + (generate() % (max - min + 1));
    }

    private:
    uint32_t current;
};

//------------------------------------HELPER FUNCTIONS FOR THE SIMULATOR------------------------------
// Following function was taken from stackoverflow; helper function for splitting strings
std::vector<std::string> split_delim(std::string input, std::string delim) {
    std::vector<std::string> tokens;
    std::size_t pos = 0;
    std::string token;
    while ((pos = input.find(delim)) != std::string::npos) {
        token = input.substr(0, pos);
        tokens.push_back(token);
        input.erase(0, pos + delim.length());
    }
    tokens.push_back(input);

    return tokens;
}

std::string print_PCB(std::vector<PCB> _PCB) {
    const int tableWidth = 54;

    std::stringstream buffer;
    
    // Print top border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;
    
    // Print headers
    buffer << "|"
              << std::setfill(' ') << std::setw(4) << "PID"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(11) << "Partition"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(5) << "Size"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(13) << "Arrival Time"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(11) << "State"
              << std::setw(2) << "|" << std::endl;
    
    // Print separator
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;
    
    // Print each PCB entry
    for (const auto& program : _PCB) {
        buffer << "|"
                  << std::setfill(' ') << std::setw(4) << program.PID
                  << std::setw(2) << "|"
                  << std::setw(11) << program.partition_number
                  << std::setw(2) << "|"
                  << std::setw(5) << program.size
                  << std::setw(2) << "|"
                  << std::setw(13) << program.arrival_time
                  << std::setw(2) << "|"
                  << std::setw(11) << program.state
                  << std::setw(2) << "|" << std::endl;
    }
    
    // Print bottom border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();
}

std::string print_PCB(PCB _PCB) {
    std::vector<PCB> temp;
    temp.push_back(_PCB);
    return print_PCB(temp);
}

std::string get_memory_header() {

    const int tableWidth = 91;

    std::stringstream buffer;
    
    // Print top border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;
    
    // Print headers
    buffer  << "|"
            << std::setfill(' ') << std::setw(13) << "Time of Event"
            << std::setw(2) << "|"
            << std::setfill(' ') << std::setw(11) << "Memory Used"
            << std::setw(2) << "|"
            << std::setfill(' ') << std::setw(22) << "Partitions State"
            << std::setw(2) << "|"
            << std::setfill(' ') << std::setw(17) << "Total Free Memory"
            << std::setw(2) << "|"
            << std::setfill(' ') << std::setw(18) << "Usable Free Memory"
            << std::setw(2) << "|" << std::endl;
    
    // Print separator
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();

}

std::string log_memory_status(unsigned int current_time, std::vector<PCB> job_queue) {

    const int tableWidth = 91;

    std::stringstream buffer;

    unsigned int memory_used = 0;
    unsigned int total_free_memory = 0;
    unsigned int usable_free_memory = 0;
    std::string partitions_state;

    for(auto partition : memory_paritions) {
        if(partition.occupied == -1) {
            usable_free_memory += partition.size;
            total_free_memory += partition.size;
        } else {
            for(auto job : job_queue) {
                if(partition.occupied == job.PID) {
                    total_free_memory += (partition.size - job.size);
                    memory_used += job.size;
                }
            }
        }

        if(partition.partition_number < 6){
            partitions_state += std::to_string(partition.occupied) + ", ";
        } else {
            partitions_state += std::to_string(partition.occupied);
        }

    }

    buffer  << "|"
            << std::setfill(' ') << std::setw(13) << current_time
            << std::setw(2) << "|"
            << std::setw(11) << memory_used
            << std::setw(2) << "|"
            << std::setw(22) << partitions_state
            << std::setw(2) << "|"
            << std::setw(17) << total_free_memory
            << std::setw(2) << "|"
            << std::setw(18) << usable_free_memory
            << std::setw(2) << "|" << std::endl;

    return buffer.str();
}

std::string get_memory_footer() {
    const int tableWidth = 91;
    std::stringstream buffer;

    // Print bottom border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();
}
//--------------------------------------------FUNCTIONS FOR THE "OS"-------------------------------------

//Assign memory partition to program
bool assign_memory(PCB &program) {
    int size_to_fit = program.size;
    int available_size = 0;

    for(int i = 5; i >= 0; i--) {
        available_size = memory_paritions[i].size;

        if(size_to_fit <= available_size && memory_paritions[i].occupied == -1) {
            memory_paritions[i].occupied = program.PID;
            program.partition_number = memory_paritions[i].partition_number;
            return true;
        }
    }

    return false;
}

bool free_memory(PCB &program){
    for(int i = 5; i >= 0; i--) {
        if(program.PID == memory_paritions[i].occupied) {
            memory_paritions[i].occupied = -1;
            program.partition_number = -1;
            return true;
        }
    }
    return false;
}

PCB add_process(std::vector<std::string> tokens) {
    PCB process;
    process.PID = std::stoi(tokens[0]);
    process.size = std::stoi(tokens[1]);
    process.arrival_time = std::stoi(tokens[2]);
    process.processing_time = std::stoi(tokens[3]);
    process.remaining_time = std::stoi(tokens[3]);
    process.io_freq = std::stoi(tokens[4]);
    process.io_duration = std::stoi(tokens[5]);
    process.partition_number = -1;
    process.state = NOT_ASSIGNED;

    return process;
}

bool all_process_terminated(std::vector<PCB> processes) {

    for(auto process : processes) {
        if(process.state != TERMINATED) {
            return false;
        }
    }

    return true;
}

std::vector<PCB> FCFS(std::vector<PCB> &list_processes, std::vector<PCB> new_processes, std::vector<PCB> ready_queue) {
    for(const auto process : new_processes) {
        auto ready_process = process;
        ready_process.state = READY;

        for(auto &process : list_processes) {
            if(ready_process.PID == process.PID) {
                process.state = READY;
            }
        }

        ready_queue.push_back(ready_process);
    }

    return ready_queue;
}

std::tuple<std::string, std::string> run_simulation(std::vector<PCB> list_processes, SimpleLCG lcg) {
    bool mem_event = false;
    bool event = false;

    std::vector<PCB> ready_queue;
    std::vector<PCB> wait_queue;
    std::vector<PCB> job_queue;

    unsigned int current_time = 0;
    PCB running;

    //Initialize an empty running process
    running.processing_time = 0;
    running.start_time = 0;

    std::string memory_status;
    memory_status = get_memory_header();
    memory_status += log_memory_status(current_time, job_queue);

    while(!all_process_terminated(job_queue) || job_queue.empty()) {
        std::vector<PCB> new_processes;

        for(auto &process : list_processes) {
            if(process.arrival_time == current_time) {
                assign_memory(process);
                new_processes.push_back(process);
                job_queue.push_back(process);
                mem_event = true;
            }
        }

        //FOR NOW; only schedule is there is a process in new state
        if(!new_processes.empty()) {
            ready_queue = FCFS(job_queue, new_processes, ready_queue);
            event = true;
        }

        if(current_time >= (running.processing_time + running.start_time)) {
            for(auto &process : job_queue) {
                if(process.PID == running.PID) {
                    process.state = TERMINATED;
                }
            }

            if(ready_queue.empty()) {
                running.remaining_time = 0;
                running.state = TERMINATED;
                free_memory(running);
                mem_event = true;

                running.start_time = 0;
                running.processing_time = 0;
            } else {
                running.remaining_time = 0;
                running.state = TERMINATED;
                free_memory(running);
                mem_event = true;

                running = ready_queue.back();
                ready_queue.pop_back();
                running.start_time = current_time;
                running.state = RUNNING;

                for(auto &process : job_queue) {
                    if(process.PID == running.PID) {
                        process.state = RUNNING;
                    }
                }
            }

            event = true;
        }

        if(event){
            std::cout << "At t = " << current_time << ": " << std::endl;
            std::cout << print_PCB(job_queue) << std::endl;
        }
        
        if (mem_event) {
            memory_status += log_memory_status(current_time, job_queue);
        }

        current_time++;
        event = false;
        mem_event = false;
    }

    memory_status += get_memory_footer();
    std::cout << memory_status << std::endl;

    return std::make_tuple("Hello", memory_status);
}

#endif