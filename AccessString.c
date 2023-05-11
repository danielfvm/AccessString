// cc SigMaker.c -o sigmaker -ludis86

#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include <libudis86/extern.h>
#include <libudis86/types.h>

#define MAX_RESULTS 64

#define USAGE "Usage:\n" \
              "  %s <file> <string> [max_offset]\n\n" \
              "  file       Path to executable file\n" \
              "  string     String to be referenced\n" \
              "  max_offset Allow provided string to be offset by value, default 0\n"

char noZero(char* start, char* end) {
    while (start++ != end) {
        if (*start == '\0') {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    char* endptr = NULL;

	if (argc < 3) {
		printf(USAGE, argv[0]);
		return 0;
	}

    // Convert size argument to int
    const int max_offset = argc == 4 ? strtoll(argv[3], &endptr, 10) : 0;
    if (argc == 4 && endptr != argv[3]+strlen(argv[3])) {
        printf("%s: Invalid number for size: %s\n", argv[0], argv[3]);
        return 0;
    }

    // Open fd to the binary file
    const int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        printf("%s: Failed to open file: %s\n", argv[0], argv[1]);
        return 0;
    }

    struct stat s;
    if (fstat(fd, &s) == -1) {
        printf("%s: Failed to read stats\n", argv[0]);
        close(fd);
        return 0;
    }

    const int size = s.st_size;

    void* memory = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ((intptr_t)memory == -1) {
        printf("%s: Failed to run mmap\n", argv[0]);
        close(fd);
        return 0;
    }

    uint64_t str_addr[MAX_RESULTS];
    int str_addr_i = 0;

    for (void* addr = memory; addr < memory + size; addr ++) {
        if (memcmp(addr, argv[2], strlen(argv[2])) == 0) {
            printf("Found string at: %lx\n", addr-memory);
            str_addr[str_addr_i] = addr-memory;
            str_addr_i ++;
        }
    }

    if (str_addr_i == 0) {
        printf("%s: Couldn't find specified string in binary\n", argv[0]);
        munmap(memory, size);
        close(fd);
        return 0;
    }

    putchar('\n');

    // Initialize libudis86 decompiler
    ud_t ud;
    ud_init(&ud);
    ud_set_mode(&ud, 64); // 64bit program TODO: Make it optional
    ud_set_syntax(&ud, UD_SYN_INTEL); // Assambly syntax
    ud_set_input_buffer(&ud, memory, size);

    printf("Address   Code                               String     Text\n");
    while (ud_disassemble(&ud)) {
        const uint64_t offset = ud_insn_off(&ud);
        const size_t len = ud_insn_len(&ud);
        const char* code = ud_insn_asm(&ud);
        const ud_operand_t* op;
        int op_i = 0;

        while ((op = ud_insn_opr(&ud, op_i++)) != NULL) {
            if (op->base != UD_R_RIP)
                continue;

            for (int i = 0; i < str_addr_i; ++ i) {
                unsigned int ptr = offset + len + op->lval.sdword;
                if (ptr >= str_addr[i] - max_offset && ptr <= str_addr[i]) {
                    if (!noZero((char*)memory + ptr, (char*)memory+str_addr[i]))
                        continue;

                    char string[50]; 
                    memset(string, 0, 50);
                    strncpy(string, memory + ptr, 49);

                    printf("%-10lx%-35s#%-10lx", offset, code, str_addr[i]);
                    int j;
                    for (j = 0; j < 50; ++ j) {
                        char c = *((char*)memory + offset + len + op->lval.sdword + j);
                        if (c == '\n')
                            printf("\\n");
                        else if (c == '\t')
                            printf("\\t");
                        else if (c == '\r')
                            printf("\\r");
                        else if (c < ' ') 
                            break;
                        else
                            putchar(c);
                    }
                    putchar('\n');
                }
            }
        }

    }

    // Cleanup
    munmap(memory, size);
    close(fd);

    return 0;
}
