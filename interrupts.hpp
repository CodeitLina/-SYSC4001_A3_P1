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

#define ADDR_BASE   0
#define VECTOR_SIZE 2

#define FORK_VECTOR 2
#define EXEC_VECTOR 3

int PID_count = 11;

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
    unsigned int    AT;
    unsigned int    PT;
    unsigned int    RT;
    int             partition_number;
    std::string     state;
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

std::string print_PCB(std::vector<PCB> _PCB) {
    const int tableWidth = 45;

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
              << std::setfill(' ') << std::setw(5) << "State"
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
                  << std::setw(5) << program.state
                  << std::setw(2) << "|" << std::endl;
    }
    
    // Print bottom border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();
}

std::string system_log(std::vector<PCB> ready_queue, int current_time) {

    std::stringstream buffer;

    buffer << "!" << std::setfill('-') << std::setw(60) << "!" << std::endl;
    buffer << "Save Time: " << current_time << " ms" << std::endl;

    buffer << print_PCB(ready_queue);

    buffer << "!" << std::setfill('-') << std::setw(60) << "!" << std::endl;

    return buffer.str();
}

std::string scheduler() {
    return "scheduler called";
}

PCB add_process(std::vector<std::string> tokens) {
    PCB process;
    process.PID = std::stoi(tokens[0]);
    process.size = std::stoi(tokens[1]);
    process.AT = std::stoi(tokens[2]);
    process.PT = std::stoi(tokens[3]);
    process.RT = std::stoi(tokens[3]);
    process.io_freq = std::stoi(tokens[4]);
    process.io_duration = std::stoi(tokens[5]);
    process.partition_number = -1;
    process.state = "New";

    return process;
}

std::tuple<std::string, std::string> run_simulation(  int _current_time, 
                                                        std::vector<PCB> _ready_queue,
                                                        PCB _running,
                                                        std::string input_file_string, 
                                                        std::vector<std::string> vectors, 
                                                        SimpleLCG lcg
                                                    ) {
    std::vector<PCB> ready_queue = _ready_queue;
    PCB running = _running;
    int current_time = _current_time;
    std::vector<PCB> wait_queue;

    std::ifstream input_file;
    input_file.open(input_file_string);
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << input_file_string << std::endl;
        exit;
    }

    std::string line;
    std::vector<PCB> input_processes;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        input_processes.push_back(new_process);
    }

    while(!ready_queue.empty() && !input_processes.empty()) {
        size_t size_input = input_processes.size();

        for(int i = 0; i < size_input; i++) {
            if(input_processes[i].AT == current_time) {
                ready_queue.push_back(input_processes[i]);
                input_processes.erase(input_processes.begin() + i);
                size_input = input_processes.size();
                i = 0;
            }
        }
        
    }

    std::cout << print_PCB(ready_queue) << std::endl;
    std::cout << print_PCB(input_processes) << std::endl;

    input_file.close();

    return std::make_tuple("Hello", "World");
}

#endif