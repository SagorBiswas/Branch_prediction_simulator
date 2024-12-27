#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdint.h>
#include <bitset>
#include <unordered_map>
#include <iomanip>


bool debug_mode = false;
int max_debug_line = 10;

bool output_to_file = false;
bool simulation = true;
bool graph = false;



enum Outcome {
    NotTaken = 0,
    Taken = 1
};

Outcome get_Outcome(char inp) {
    if (inp == 't') return Taken;
    
    return NotTaken;
}

std::string print_Outcome(Outcome outcome) {
    if (outcome == Taken) return "Taken";
    else return "Not Taken";
}

void printBinary(uint32_t value) {
    std::bitset<32> binary(value);  // Convert value to a 32-bit binary representation
    std::string binaryStr = binary.to_string();

    // Insert spaces every 4 bits
    for (int i = 0; i < 32; i += 4) {
        std::cout << binaryStr.substr(i, 4) << " ";
    }
    std::cout << std::endl;
}

uint32_t hexAddrToUint32(std::string hexString) {
    
    // Buffer to store the padded string if needed
    char paddedHex[9] = {0}; // 8 characters + 1 for null terminator

    // Determine the length of the input
    // int len = hexString.length();
    int zeros = 8 - hexString.length();
    zeros =  (zeros>=0) ? zeros : 0 ;

    int i = 0;
    while(i<zeros) paddedHex[i++] = '0';

    int j = 0;
    while(i <= 8) paddedHex[i++] = hexString[j++];    // number of zeros starts count from 1, but index from 0. So having n zeros means you need to start writing from nth index


    uint32_t address = std::stoul(paddedHex, nullptr, 16);
    return address;

}

std::string uint32ToHexChar_wzero(uint32_t value) {
    // Allocate space for 8 hex characters + null terminator
    char hexString[9];
    std::snprintf(hexString, sizeof(hexString), "%08x", value); // Format as zero-padded 8-digit hex
    return std::string(hexString);
}

std::string uint32ToHexChar(uint32_t value) {
    std::stringstream ss;
    ss << std::hex << std::nouppercase << value; // Convert to lowercase hex without padding
    return ss.str();
}




class Smith {
private:
    int n_bits;  // Represents a specific bit number

    int counter, max_counter, min_counter, threshold ;

    int n_access, n_mispredictions;

public:
    // Constructor to initialize variables
    Smith(int bitNum)                                                                   // , bool initialPrediction)
        : n_bits(bitNum), n_access(0), n_mispredictions(0) {             // , prediction(initialPrediction)
            max_counter = (1 << bitNum) - 1 ;   // 2^n - 1
            min_counter = 0 ;
            counter = 1 << (bitNum-1) ;     // 2^(n-1)
            threshold = 1 << (bitNum-1) ;   // (2^n)/2
        }

    Outcome smith_predict() {
        if(counter >= threshold) return Taken;   // Taken

        return NotTaken;           // Not Taken
    }

    // Function to update the state based on the actual outcome
    void smith_update(Outcome Actual) {

        n_access++;
        if (Actual != smith_predict()) n_mispredictions++;

        // Update the counter 
        if(Actual == Taken) counter = (counter+1 > max_counter) ? max_counter : counter+1;      
        else counter = (counter-1 < min_counter) ? min_counter : counter-1;

    }

    // Function to print the current state of the Predictor
    void smith_print_state() {
        double miss_pred_rate = (double)n_mispredictions/n_access ;

        if(graph) std::cout << (miss_pred_rate * 100) << std::endl;

        std::cout << "number of predictions:\t\t" << n_access << "\n";
        std::cout << "number of mispredictions:\t" << n_mispredictions << "\n";
        std::cout << "misprediction rate:\t\t" << (miss_pred_rate * 100) << "%\n";
        std::cout << "FINAL COUNTER CONTENT:\t\t" << counter << "\n" ;

    }
};


class Bimodal {
private:
    int n_bits, predictor_m, predictor_n;  // Represents a specific bit number

    int max_counter, min_counter, threshold ;

    int n_access, n_mispredictions, table_size;

    int* prediction_table;

    uint32_t index_extractor_mask;

public:
    // Constructor to initialize variables
    Bimodal(int m, int n=0, int n_Bits=3)                                                          // , bool initialPrediction)
        : n_bits(n_Bits), n_access(0), n_mispredictions(0), predictor_m(m), predictor_n(n) {                     // , prediction(initialPrediction)

            max_counter = (1 << n_bits) - 1 ;   // 2^n - 1
            min_counter = 0 ;
            threshold = 1 << (n_bits-1) ;       // (2^n)/2

            table_size = 1 << (m) ;
            int init_counter = 1 << (n_bits-1) ;     // 2^(n-1)

            prediction_table = (int*) malloc (table_size * sizeof(int)) ;
            for(int i=0; i<table_size; i++) prediction_table[i] = init_counter ;

            index_extractor_mask = ~((~0) << (m));    
            // std::cout << "mask is = " ; printBinary(index_extractor_mask);
        }

    Outcome bimodal_predict(int index) {
        if(prediction_table[index] >= threshold) return Taken;   // Taken

        return NotTaken;           // Not Taken
    }

    int bimodal_get_index(uint32_t address) {
        
        return ((address >> 2) & index_extractor_mask) ; 
    }

    // Function to update the state based on the actual outcome
    void bimodal_update(uint32_t address, Outcome Actual) {
        n_access++;

        int index = bimodal_get_index(address);
        // if(index == -1) return;

        if (Actual != bimodal_predict(index)) n_mispredictions++;

        // Update the counter 
        if(Actual == Taken) prediction_table[index] = (prediction_table[index]+1 > max_counter) ? max_counter : prediction_table[index]+1;      
        else prediction_table[index] = (prediction_table[index]-1 < min_counter) ? min_counter : prediction_table[index]-1;

    }

    // Function to print the current state of the Predictor
    void bimodal_print_state(bool only_content=false) {
        if (n_access == 0) n_access = 1;
        double miss_pred_rate = (double)n_mispredictions/n_access ;

        if(graph) std::cout << (miss_pred_rate * 100) << std::endl;

        if(debug_mode) std::cout << "max count = " << max_counter << "min count = " << min_counter << "threshold = " << threshold << "\n"; 
        if(!only_content) {
            std::cout << "number of predictions:\t\t" << n_access << "\n";
            std::cout << "number of mispredictions:\t" << n_mispredictions << "\n";
            std::cout << "misprediction rate:\t\t" << (miss_pred_rate * 100) << "%\n";
        }
        
        std::cout << "FINAL BIMODAL CONTENTS\n";
        for(int i=0; i<table_size; i++) 
            std::cout << i << "\t" << prediction_table[i] << "\n";

    }
};


class GShare {
private:
    int n_bits, predictor_m, predictor_n ;  // Represents a specific bit number

    int max_counter, min_counter, threshold ;

    int n_access, n_mispredictions, table_size ;

    int* prediction_table;

    uint32_t index_extractor_mask;
    uint32_t gshare_reg, n_mask;

public:
    // Constructor to initialize variables
    GShare(int m, int n=0, int n_Bits=3)                                                          // , bool initialPrediction)
        : n_bits(n_Bits), n_access(0), n_mispredictions(0), predictor_m(m), predictor_n(n) {                     // , prediction(initialPrediction)

            max_counter = (1 << n_bits) - 1 ;   // 2^n - 1
            min_counter = 0 ;
            threshold = 1 << (n_bits-1) ;       // (2^n)/2

            table_size = 1 << (m) ;     // 2^m
            int init_counter = 1 << (n_bits-1) ;     // 2^(n-1)

            prediction_table = (int*) malloc (table_size * sizeof(int)) ;
            for(int i=0; i<table_size; i++) prediction_table[i] = init_counter ;

            
            gshare_reg = 0 ;    // initialized with all zeros

            index_extractor_mask = ~((~0) << (m));    
            n_mask = ~((~0) << (n));
        }

    ~GShare() {
        free(prediction_table);
    }
        

    int gshare_get_index(uint32_t address) {
        // if(!predictor_n)     // n == 0 : Bimodal
        //     return (address >> 2) & index_extractor_mask ;

        address = address >> 2;     // first two bits are 00

        return (((address & n_mask) ^ (gshare_reg & n_mask)) | ((address >> predictor_n) << predictor_n)) & index_extractor_mask ; 
        // XOR last n bits of address  and gshare register  |  (last n bits of address is zeros) AND (lowest m bits are 1, others 0)
    }
    

    Outcome gshare_predict(int index) {
        if(prediction_table[index] >= threshold) return Taken;   // Taken

        return NotTaken;           // Not Taken
    }

    void update_gshare_register(Outcome Actual) {
        // update gshare register
        gshare_reg = (gshare_reg >> 1) ;
        if (Actual == Taken) 
            gshare_reg |= (1 << predictor_n-1) ;    // setting MSB = 1 : MSB is 0 by default after right shift
    }

    // Function to update the state based on the actual outcome
    void gshare_update(uint32_t address, Outcome Actual) {
        n_access++;

        int index = gshare_get_index(address);
        if(debug_mode) std::cout << "found index = " << index << " for actual outcome : " << print_Outcome(Actual) << "\n";

        if (Actual != gshare_predict(index)) n_mispredictions++;


        // Update the counter 
        if(Actual == Taken) 
            prediction_table[index] = (prediction_table[index]+1 > max_counter) ? max_counter : prediction_table[index]+1;      
        else 
            prediction_table[index] = (prediction_table[index]-1 < min_counter) ? min_counter : prediction_table[index]-1;


        if(debug_mode) {
            std::cout << "gshare before update is =\t"; 
            printBinary(gshare_reg);
        }
        // update gshare register
        update_gshare_register(Actual);
        // gshare_reg = (gshare_reg >> 1) ;
        // if (Actual == Taken) 
        //     gshare_reg |= (1 << predictor_n-1) ;    // setting MSB = 1 : MSB is 0 by default after right shift
        
        if(debug_mode) {
            std::cout << "gshare after update is =\t"; 
            printBinary(gshare_reg);
            std::cout << "********************************\n";
        }

    }

    // Function to print the current state of the Predictor
    void gshare_print_state(bool only_content=false) {
        if (n_access == 0) n_access = 1;
        double miss_pred_rate = (double)n_mispredictions/n_access ;

        if(graph) std::cout << (miss_pred_rate * 100) << std::endl;

        if(debug_mode) std::cout << "max count = " << max_counter << "min count = " << min_counter << "threshold = " << threshold << "\n";
        
        if(!only_content){
            std::cout << "number of predictions:\t\t" << n_access << "\n";
            std::cout << "number of mispredictions:\t" << n_mispredictions << "\n";
            std::cout << "misprediction rate:\t\t" << (miss_pred_rate * 100) << "%\n";
        }
        // if(predictor_n) 
            std::cout << "FINAL GSHARE CONTENTS\n";
        // else 
        //     std::cout << "FINAL BIMODAL CONTENTS\n";
        for(int i=0; i<table_size; i++) 
            std::cout << i << "\t" << prediction_table[i] << "\n";

    }
};


class Hybrid {
private:
    int n_bits, chooser_k ;          // Represents a specific bit number

    int max_counter, min_counter, threshold ;
    int n_access, n_mispredictions, table_size ;

    int* chooser_table;

    GShare * gshare_predictor;
    Bimodal * bimodal_predictor;

    uint32_t index_extractor_mask ;


public:
    // Constructor to initialize variables
    Hybrid(int k, int m1, int n, int m2, int n_Bits=2) 
        : n_bits(n_Bits), chooser_k(k), n_access(0), n_mispredictions(0) {
        
            max_counter = (1 << n_bits) - 1 ;   // 2^n - 1
            min_counter = 0 ;
            threshold = 1 << (n_bits-1) ;       // (2^n)/2
            
            index_extractor_mask = ~((~0) << (k)); 

            table_size = 1 << (k) ;     // 2^k
            chooser_table = (int*) malloc (table_size*sizeof(int));
            for(int i=0; i<table_size; i++) chooser_table[i] = 1 ;  // fixed for chooser table

            gshare_predictor = new GShare(m1, n);
            bimodal_predictor = new Bimodal(m2);

            if(debug_mode) {
                std::cout << "Hybrid predictor initialized with " << max_counter << " max counter, " 
                        << min_counter << " min_counter, and " << threshold << " threshold\n";
                std::cout << "table size is = " << k << "\n";

                gshare_predictor->gshare_print_state(false);
                bimodal_predictor->bimodal_print_state(false);
            }

    }

    int get_chooser_index(uint32_t address) {
        return ((address >> 2) & index_extractor_mask) ; 
    }


    void hybrid_update(uint32_t address, Outcome Actual) {
        n_access++;

        // make the prediction
        Outcome pred;
        int chooser_index = get_chooser_index(address);
        if(debug_mode) std::cout << "Chooser index is = " << chooser_index << "\n";

        // get gshare prediction
        int g_index = gshare_predictor->gshare_get_index(address);
        Outcome g_prediction = gshare_predictor->gshare_predict(g_index);
        if(debug_mode) std::cout << "GShare prediction = " << print_Outcome(g_prediction) << "\n";

        // get bimodal prediction
        int b_index = bimodal_predictor->bimodal_get_index(address);
        Outcome b_prediction = bimodal_predictor->bimodal_predict(b_index);
        if(debug_mode) std::cout << "Bimodal prediction = " << print_Outcome(b_prediction) << "\n";

        if(chooser_table[chooser_index] >= threshold) {     // use gshare
            pred = g_prediction ;
            // update the predictor :: gshare register updated within
            gshare_predictor->gshare_update(address, Actual);

        } else {
            pred = b_prediction ;
            // update the predictor and gshare register
            bimodal_predictor->bimodal_update(address, Actual);
            gshare_predictor->update_gshare_register(Actual);
        }


        // calculate and update according to prediction
        if (Actual != pred) n_mispredictions++;

        if(debug_mode) std::cout << "prediction calculation done\n" ;

        // Update the chooser counter 
        if(g_prediction != b_prediction) {
            if(g_prediction == Actual) 
                chooser_table[chooser_index] = (chooser_table[chooser_index]+1 > max_counter) ? max_counter : chooser_table[chooser_index]+1;   
            else 
                chooser_table[chooser_index] = (chooser_table[chooser_index]-1 < min_counter) ? min_counter : chooser_table[chooser_index]-1;

        } // else no update

    }


    // Function to print the current state of the Predictor
    void hybrid_print_state(bool only_content=false) {
        double miss_pred_rate = (double)n_mispredictions/n_access ;

        if(!only_content) {
            std::cout << "number of predictions:\t\t" << n_access << "\n";
            std::cout << "number of mispredictions:\t" << n_mispredictions << "\n";
            std::cout << "misprediction rate:\t\t" << (miss_pred_rate * 100) << "%\n";
        }
        
        std::cout << "FINAL CHOOSER CONTENTS\n";
        for(int i=0; i<table_size; i++) 
            std::cout << i << "\t" << chooser_table[i] << "\n";

        gshare_predictor->gshare_print_state(true);
        bimodal_predictor->bimodal_print_state(true);

    } 
};


int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Insufficient arguments provided.\nUsage:\n"
                  << "  sim smith <B> <tracefile>\n"
                  << "  sim bimodal <M2> <tracefile>\n"
                  << "  sim gshare <M1> <N> <tracefile>\n"
                  << "  sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
        return 1;
    }

    std::string predictorType = argv[1];

    // ********* set customized settings *********

    // Set precision for a specific print
    std::cout << std::fixed << std::setprecision(2);

    std::ofstream outFile;
    std::streambuf* originalCoutBuffer;
    if(output_to_file){
        outFile.open("output.txt");

        // Redirect std::cout to outFile
        originalCoutBuffer = std::cout.rdbuf(); // Save the original buffer
        std::cout.rdbuf(outFile.rdbuf()); // Redirect std::cout to outFile
    }



    if (predictorType == "smith") {
        if (argc != 4) {
            std::cerr << "Invalid arguments for Smith Predictor.\nUsage: sim smith <B> <tracefile>\n";
            return 1;
        }
        int B = std::stoi(argv[2]);     // number of counter bits
        std::string trace_file = argv[3];

        Smith predictor(B);

        // start reading trace file for input and main process
        std::ifstream file(trace_file);
        if (!file.is_open()) {
            std::cout << "Failed to open trace file: " << trace_file << std::endl;
            return 1;
        }

        int i = 0;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char op;            // Operation: 'r' for read, 'w' for write_back
            std::string address_hex;

            if (!(iss >> address_hex >> op)) {
                continue;       // Skip invalid lines
            }

            predictor.smith_update(get_Outcome(op));

            i++;
            if(debug_mode && i>max_debug_line) break;
        }

        file.close();
        
        predictor.smith_print_state();


    } else if (predictorType == "bimodal") {
        if (argc != 4) {
            std::cerr << "Invalid arguments for Bimodal Predictor.\nUsage: sim bimodal <M2> <tracefile>\n";
            return 1;
        }
        int M = std::stoi(argv[2]);
        std::string trace_file = argv[3];
        
        Bimodal predictor(M);

        std::ifstream file(trace_file);
        if (!file.is_open()) {
            std::cout << "Failed to open trace file: " << trace_file << std::endl;
            return 1;
        }

        int i = 0;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char op;            // Operation: 'r' for read, 'w' for write_back
            std::string address_hex;

            if (!(iss >> address_hex >> op)) {
                continue;       // Skip invalid lines
            }

            predictor.bimodal_update(hexAddrToUint32(address_hex), get_Outcome(op));

            i++;
            if(debug_mode && i>max_debug_line) break;
        }

        file.close();
        
        predictor.bimodal_print_state();



    } else if (predictorType == "gshare") {
        if (argc != 5) {
            std::cerr << "Invalid arguments for Gshare Predictor.\nUsage: sim gshare <M1> <N> <tracefile>\n";
            return 1;
        }
        int M1 = std::stoi(argv[2]);
        int N = std::stoi(argv[3]);
        std::string trace_file = argv[4];
        // simulateGshare(M1, N, tracefile);

        GShare predictor(M1, N);

        std::ifstream file(trace_file);
        if (!file.is_open()) {
            std::cout << "Failed to open trace file: " << trace_file << std::endl;
            return 1;
        }

        int i = 0;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char op;            // Operation: 'r' for read, 'w' for write_back
            std::string address_hex;

            if (!(iss >> address_hex >> op)) {
                continue;       // Skip invalid lines
            }

            predictor.gshare_update(hexAddrToUint32(address_hex), get_Outcome(op));

            i++;
            if(debug_mode && i>max_debug_line) break;
        }

        file.close();
        
        // printf("printing gshare\n");
        predictor.gshare_print_state();


    } else if (predictorType == "hybrid") {
        if (argc != 7) {
            std::cerr << "Invalid arguments for Hybrid Predictor.\nUsage: sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
            return 1;
        }
        int K = std::stoi(argv[2]);
        int M1 = std::stoi(argv[3]);
        int N = std::stoi(argv[4]);
        int M2 = std::stoi(argv[5]);
        std::string trace_file = argv[6];
        // simulateHybrid(K, M1, N, M2, tracefile);

        Hybrid predictor(K, M1, N, M2);

        std::ifstream file(trace_file);
        if (!file.is_open()) {
            std::cout << "Failed to open trace file: " << trace_file << std::endl;
            return 1;
        }

        int i = 0;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char op;            // Operation: 'r' for read, 'w' for write_back
            std::string address_hex;

            if (!(iss >> address_hex >> op)) {
                continue;       // Skip invalid lines
            }

            predictor.hybrid_update(hexAddrToUint32(address_hex), get_Outcome(op));

            i++;
            if(debug_mode && i>max_debug_line) break;
        }

        file.close();
        
        // printf("printing hybrid\n");
        predictor.hybrid_print_state();



    } else {
        std::cerr << "Unknown predictor type: " << predictorType << "\n";
        return 1;
    }


    // ********* reset all the customized settings *********
    {
    if(output_to_file){
        // Restore the original std::cout buffer (console output)
        std::cout.rdbuf(originalCoutBuffer);        
        // Close the file
        outFile.close();
    }
        
    // Reset formatting
    std::cout.unsetf(std::ios::fixed); // Turn off fixed-point
    std::cout.precision(6);            // Reset precision to default
    }

    return 0;
}

