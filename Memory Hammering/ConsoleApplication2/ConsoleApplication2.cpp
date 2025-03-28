#include <Windows.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <intrin.h> //mmcluflush
using namespace std;

bool hammersuccess = false;
long long proba = 0;

const size_t mem_size = 64 * 1024; //64kB
int hammer_count = 1500000;

enum HammerMethod {
    CLFLUSH_OPT,
    CLFLUSH_READ,
    CLFLUSH_WRITE,
    MOVNTI,
    CLFLUSH_READ_DOUBLE_SIDED,
    CLFLUSH_READ_ASSEMBLY_ADDRESSING
};

//Initialize memory with alternating row patterns
uint64_t pattern1 = 0xFFFFFFFFFFFFFFFF; 
uint64_t pattern2 = 0x0000000000000000; 


// Function to generate and display matrix
void generate_and_display_matrix(uint64_t* addr1, uint64_t* addr2, uint64_t* memory, size_t mem_size) {
    const size_t row_size = 1024; // Row size in uint64_t (1024 * 8 bytes)
    const size_t num_rows = 5;    // Number of rows to generate

    cout << "Generating and displaying memory matrix:" << endl;

    for (int i = 0; i < num_rows; ++i) {
        uint64_t* row_start = (i == 0) ? (addr1 - row_size) : (i == 1 ? addr1 : (i == 2 ? addr2 : (i == 3 ? (addr2 + row_size) : (addr2 + 2*row_size))));
        cout << endl;
        cout << "Row " << hex << i + 1 << ":" << endl;
 
        for (size_t j = 1; j < row_size-1; ++j) {
            cout << row_start[j] << " ";
            if (j % 12 == 0) { cout << endl; }
        }
        cout << endl;
    }
}

void hammer_rows(uint64_t* addr1, uint64_t* addr2, uint64_t* addr3, int iterations, HammerMethod method) {
    for (int i = 0; i < iterations; i++) {
        switch (method) {
       
        case CLFLUSH_OPT:
            __asm {
                mov eax, addr1
                clflushopt[eax]
                
                mov eax, addr2
                clflushopt[eax]

                mfence
            }
            break;

        case CLFLUSH_READ:
            __asm {
                mov eax, [addr1]
                clflush[eax]

                mov eax, [addr2]
                clflush[eax]

                mov eax, [addr3]
                clflush[eax]

                mfence
            }
            break;

        case CLFLUSH_READ_ASSEMBLY_ADDRESSING:
            __asm {
                xor esi, esi
                mov esi, 0xFFFF
                mov edx, 0x0


                codeuno:
                mov eax, [addr1]
                sub eax, 0x2000
                add eax, edx
                mov addr2, eax
                add edx, 0x1  
                mov ecx, 0xFFFF

                dec esi
                cmp esi, 0
                JE end02
                jmp codedos

                codedos:
                mov eax, [addr1]
                clflush[eax]

                mov ebx, [addr2]
                clflush[ebx]

                dec ecx
                cmp ecx, 0
                JE codeuno
                jmp codedos

                end02:
                mfence

            }
            break;

        case CLFLUSH_READ_DOUBLE_SIDED:
            __asm {
                mov ecx, 0xFFFF
                code:
                    mov eax, [addr1]
                    mov ebx, [addr2]
                    clflush[eax]
                    clflush[ebx]
                    mfence
                    dec ecx
                    cmp ecx, 0
                    JE end0
                    jmp code
                 end0:
                    mfence
            }
            break;

        case CLFLUSH_WRITE:
            __asm {
                mov eax, addr1
                mov dword ptr[eax], 0x0

                mov eax, addr2
                mov dword ptr[eax], 0x0
                mfence
            }
            break;

        case MOVNTI:
            __asm {
                movnti[addr1], eax
                mov eax, [addr1]

                movnti[addr2], eax
                mov eax, [addr2]

                movnti[addr3], eax
                mov eax, [addr3]

                mfence
            }
            break;
        }
    }
}


bool detect_bit_flip(uint64_t* start, size_t length, uint64_t original_value) {
    for (size_t i = 0; i < length - 1; ++i) {
        if (start[i] != original_value) {
            return true;
        }
    }
    return false;

}

int main() {
   
    while (hammersuccess == false) {
        uint64_t* memory = reinterpret_cast<uint64_t*>(VirtualAlloc(NULL, mem_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

        if (memory == nullptr) {
            cerr << "Failed to allocate memory" << endl;
            return -1;
        }

        for (size_t i = 0; i < mem_size / sizeof(uint64_t); i += 1024) {
            
            uint64_t* row_start = &memory[i];
            uint64_t pattern = (i / 1024 % 2 == 0) ? pattern1 : pattern2;
           
            for (size_t j = 0; j < 1024; ++j) {
                row_start[j] = pattern;
            }

        }

        vector<uint64_t*> addresses;
        for (size_t i = 0; i < mem_size / sizeof(uint64_t); i += 1024) {
            addresses.push_back(&memory[i]);
        }

        srand(static_cast<unsigned>(time(0)));

        //Selection of rows
        size_t idx1 = 1;
        size_t idx2 = 2;
        size_t idx3 = rand() % (addresses.size() - 1);
       
        //size_t idx3 = rand() % (addresses.size() - 2);
        uint64_t* addr1 = &memory[1024];
        uint64_t* addr2 = &memory[2048];
        uint64_t* addr3 = addresses[idx3];

            hammer_rows(addr1, addr2, addr3, hammer_count, CLFLUSH_OPT);
            hammer_rows(addr1, addr2, addr3, hammer_count, CLFLUSH_READ);
            //hammer_rows(addr1, addr2, addr3, hammer_count, CLFLUSH_WRITE);
            hammer_rows(addr1, addr2, addr3, hammer_count, MOVNTI);
            hammer_rows(addr1, addr2, addr3, 10, CLFLUSH_READ_DOUBLE_SIDED);
            hammer_rows(addr1, addr2, addr3, 1, CLFLUSH_READ_ASSEMBLY_ADDRESSING);
        
        // Displaying hammering results
        cout << "Hammering completed." << endl;
        //generate_and_display_matrix(addr1, addr2, memory, mem_size);
        //getchar();

        // Checking for bit flips 
        uint64_t original_value1 = (idx1 % 2 == 0) ? pattern1 : pattern2;
        uint64_t original_value2 = (idx2 % 2 == 0) ? pattern1 : pattern2;

       
        if (detect_bit_flip(addr1-1024, 1024, original_value2)) {
            cout << "Bit flip detected near address: " << addr1 << " 0" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else if (detect_bit_flip(addr1, 1024, original_value1)) {
            cout << "Bit flip detected near address: " << addr1 << " 1024" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else if (detect_bit_flip(addr2, 1024, original_value2)) {
            cout << "Bit flip detected near address: " << addr2 << " 2048" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else if (detect_bit_flip(addr2 + 1024, 1024, original_value1)) {
            cout << "Bit flip detected near address: " << addr2 << " 3072" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else if (detect_bit_flip(addr2 + 2048, 1024, original_value2)) {
            cout << "Bit flip detected near address: " << addr2 << " 4096" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else if (detect_bit_flip(addr2 + 3072, 1024, original_value1)) {
            cout << "Bit flip detected near address: "  << addr2 << " 5120" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else if (detect_bit_flip(addr2 + 4096, 1024, original_value2)) {
            cout << "Bit flip detected near address: " << addr2 << " 6244" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else if (detect_bit_flip(addr2 + 5120, 1024, original_value1)) {
            cout << "Bit flip detected near address: " << addr2 << " 7368" << endl;
            hammersuccess = true;
            generate_and_display_matrix(addr1, addr2, memory, mem_size);
        }

        else {
            cout << "Proba " << proba << " nie udana" << endl;
            proba++;
        }

        VirtualFree(memory, 0, MEM_RELEASE);
    }
    return 0;
}