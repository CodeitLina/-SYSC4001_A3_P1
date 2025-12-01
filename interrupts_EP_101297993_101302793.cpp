/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include "interrupts_101297993_101302793.hpp"

void FCFS(std::vector<PCB> &ready_queue) {
    std::sort( 
                ready_queue.begin(),
                ready_queue.end(),
                []( const PCB &first, const PCB &second ){
                    return (first.arrival_time > second.arrival_time); 
                } 
            );
}
//going off above snippet, our EP:
void EP(std::vector<PCB> &ready_queue) {
    std::sort(
        ready_queue.begin(),
        ready_queue.end(),
        [](const PCB &a, const PCB &b) {
            return a.PID > b.PID; // bigger PID first, so smallest PID ends up at back()
        }
    );
}

std::tuple<std::string /* add std::string for bonus mark */ > run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).

    unsigned int current_time = 0;
    PCB running;

    //Initialize an empty running process
    idle_CPU(running);

    std::string execution_status;

    //make the output table (the header row)
    execution_status = print_exec_header();

    //Loop while till there are no ready or waiting processes.
    //This is the main reason I have job_list, you don't have to use it.
    while(!all_process_terminated(job_list) || job_list.empty()) {

        //Inside this loop, there are three things you must do:
        // 1) Populate the ready queue with processes as they arrive
        // 2) Manage the wait queue
        // 3) Schedule processes from the ready queue

        //Population of ready queue is given to you as an example.
        //Go through the list of proceeses
        for(auto &process : list_processes) {
            if(process.arrival_time == current_time) {//check if the AT = current time
                //if so, assign memory and put the process into the ready queue
                assign_memory(process);

                process.state = READY;  //Set the process state to READY
                ready_queue.push_back(process); //Add the process to the ready queue
                job_list.push_back(process); //Add it to the list of processes

                execution_status += print_exec_status(current_time, process.PID, NEW, READY);
            }
        }

        ///////////////////////MANAGE WAIT QUEUE/////////////////////////
        //This mainly involves keeping track of how long a process must remain in the ready queue
        std::vector<PCB> still_waiting;
        std::vector<PCB> finished_io;

        for (auto &p : wait_queue) {
            if (p.remaining_io_time > 0) {
                p.remaining_io_time--;
            }
            if (p.remaining_io_time == 0) {
                // I/O will be considered finished at the **end** of this ms.
                finished_io.push_back(p);
            } else {
                still_waiting.push_back(p);
            }
        }
        wait_queue.swap(still_waiting);

        /////////////////////////////////////////////////////////////////

        //////////////////////////SCHEDULER//////////////////////////////
        EP(ready_queue); //example of FCFS is shown here
        ////////////////////////////////////////////////////////////////
        if (running.PID == -1 && !ready_queue.empty()) {
            // Sort processes by priority with smallest PID = highest priority at back()
            EP(ready_queue);

            // Pick from READY into RUNNING using helper
            run_process(running, job_list, ready_queue, current_time);

            //READY to RUNNING
            execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
        }

        // 1ms of CPU for RUNNING
        bool running_event_io = false;
        bool running_event_terminate = false;

        if (running.PID != -1) {
            // consume 1 ms of CPU
            if (running.remaining_time > 0) {
                running.remaining_time--;
            }

            // I/O countdown only if process has I/O and still has CPU left
            if (running.io_freq > 0 && running.remaining_time > 0) {
                if (running.time_to_next_io > 0) {
                    running.time_to_next_io--;
                }
                if (running.time_to_next_io == 0) {
                    running_event_io = true;
                }
            }

            // Check for termination
            if (running.remaining_time == 0) {
                running_event_terminate = true;
            }
        }

        // end of this ms
        current_time++;

        //either I/O or TERMINATE
        if (running.PID != -1) {
            if (running_event_terminate) {
                // RUNNING to TERMINATED
                states old_state = running.state;
                running.state = TERMINATED;

                execution_status += print_exec_status(current_time, running.PID, old_state, TERMINATED);

                free_memory(running);
                sync_queue(job_list, running);
                idle_CPU(running);
            } else if (running_event_io) {
                // RUNNING to WAITING
                states old_state = running.state;
                running.state = WAITING;

                execution_status += print_exec_status(current_time, running.PID, old_state, WAITING);

                running.remaining_io_time = running.io_duration;
                running.time_to_next_io = running.io_freq;  // reset for next CPU burst

                wait_queue.push_back(running);
                sync_queue(job_list, running);
                idle_CPU(running);
            } else {
                // Still RUNNING, sync
                sync_queue(job_list, running);
            }
        }

        // I/O completed: WAITING to READY 
        for (auto &p : finished_io) {
            states old_state = WAITING;
            p.state = READY;

            p.time_to_next_io = p.io_freq; // reset for next CPU burst

            execution_status += print_exec_status(current_time, p.PID, old_state, READY); //log the transition

            ready_queue.push_back(p); //add to ready queue
            sync_queue(job_list, p); //sync the job list
        }
    }

    
    //Close the output table
    execution_status += print_exec_footer();
    execution_status += computing_metrics(job_list, current_time); //print metrics
    execution_status += print_memory_log(current_time); //bonus



    return std::make_tuple(execution_status);
}


int main(int argc, char** argv) {

    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt>" << std::endl;
        return -1;
    }

    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

    //Parse the entire input file and populate a vector of PCBs.
    //To do so, the add_process() helper function is used (see include file).
    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();

    //With the list of processes, run the simulation
    auto [exec] = run_simulation(list_process);

    write_output(exec, "execution.txt");

    return 0;
}