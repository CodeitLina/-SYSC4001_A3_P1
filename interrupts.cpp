#include<interrupts.hpp>

int main(int argc, char** argv) {

    if(argc != 3) {
        std::cout << "ERROR!\nExpected 2 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt> <your_vector_table.txt>" << std::endl;
    }

    std::ifstream input_vector_table;
    input_vector_table.open(argv[2]);
    if (!input_vector_table.is_open()) {
        std::cerr << "Error: Unable to open file: " << argv[2] << std::endl;
        return 1;
    }

    std::string vector;
    std::vector<std::string> vectors;
    while(std::getline(input_vector_table, vector)) {
        vectors.push_back(vector);
    }

    // Define LCG (more memory efficient than uniform distribution)
    SimpleLCG lcg(12345);  // Seed the LCG with a number (can be customized)

    std::string sim_output;
    std::string system_status;
    int current_time = 0;

    //allocate init
    std::vector<PCB> ready_queue;
    PCB running;

    auto [exec, mem] = run_simulation(  
                                        current_time, 
                                        ready_queue, 
                                        running, 
                                        argv[1], 
                                        vectors, 
                                        lcg
                        );

    // std::ofstream output_file("execution.txt");

    // if (output_file.is_open()) {
    //     output_file << sim_output;
    //     output_file.close();  // Close the file when done
    // } else {
    //     std::cerr << "Error opening file!" << std::endl;
    // }

    return 0;
}