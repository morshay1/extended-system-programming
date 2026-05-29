#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEXA_MODE 1
#define DECI_MODE 0

typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
} state;

void toggle_debug_mode(state* s);
void set_file_name(state* s);
void set_unit_size(state* s);
void load_into_memory(state* s);
void toggle_display_mode(state* s);
void file_display(state* s);
void memory_display(state* s);
void save_into_file(state* s);
void memory_modify(state* s);
void quit(state* s);

typedef struct {
    char *name;
    void (*fun)(state*);
} fun_desc;

int display_mode = 0; // DECIMAL = 0, HEXA = 1

static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

fun_desc menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"Load Into Memory", load_into_memory},
    {"Toggle Display Mode", toggle_display_mode},
    {"File Display", file_display},
    {"Memory Display", memory_display},
    {"Save Into File", save_into_file},
    {"Memory Modify", memory_modify},
    {"Quit", quit},
    {NULL, NULL}
};

void toggle_debug_mode(state* s) {
    if (s->debug_mode) {
        printf("Debug flag now off\n");
        s->debug_mode = HEXA_MODE;
    } else {
        printf("Debug flag now on\n");
        s->debug_mode = DECI_MODE;
    }
}

void set_file_name(state* s) {
    printf("Enter file name: ");
    fgets(s->file_name, sizeof(s->file_name), stdin);
    size_t len = strlen(s->file_name);
    if (len > 0 && s->file_name[len - 1] == '\n') {
        s->file_name[len - 1] = '\0';
    }
    if (s->debug_mode) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void set_unit_size(state* s) {
    printf("Enter unit size (1, 2, or 4): ");
    char input[10];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid input.\n");
        return;
    }
    int size = atoi(input);
    if (size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if (s->debug_mode) {
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("Invalid unit size.\n");
    }
}

void load_into_memory(state* s) {
    if (s->file_name[0] == '\0') {
        printf("Error: No file name set.\n");
        return;
    }

    printf("Enter file location (in hexadecimal) and length (in decimal): ");
    char input[100];
    unsigned int location;
    int units;

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid input.\n");
        return;
    }

    if (sscanf(input, "%x %d", &location, &units) != 2) {
        printf("Invalid input format. Please enter offset in hexadecimal and length in decimal.\n");
        return;
    }

    FILE* file = fopen(s->file_name, "rb");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", s->file_name);
        return;
    }

    if (fseek(file, location, SEEK_SET) != 0) {
        printf("Error: Failed to seek to offset 0x%x in the file.\n", location);
        fclose(file);
        return;
    }

    size_t buffer_size = units * s->unit_size;
    if (buffer_size > sizeof(s->mem_buf)) {
        printf("Error: Buffer overflow. The requested size exceeds memory buffer capacity.\n");
        fclose(file);
        return;
    }

    size_t bytes_read = fread(s->mem_buf, s->unit_size, units, file);
    if (bytes_read == 0) {
        printf("Error: Could not read from the file.\n");
        fclose(file);
        return;
    }

    fclose(file);
    s->mem_count = bytes_read * s->unit_size; // Update the memory count with the actual number of bytes read.

    printf("Loaded %zu bytes into memory.\n", s->mem_count);
}

void toggle_display_mode(state* s) {
    if (display_mode == DECI_MODE) {
        printf("Display flag now off, hexadecimal representation\n");
        display_mode = HEXA_MODE;
    } else {
        printf("Display flag now on, decimal representation\n");
        display_mode = DECI_MODE;
    }
}

void file_display(state* s) {
    if (s->file_name[0] == '\0') {
        printf("Error: No file name set.\n");
        return;
    }

    printf("Enter file offset (in hexadecimal) and length (in decimal): ");
    char input[100];
    unsigned int offset;
    int units;

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid input.\n");
        return;
    }

    if (sscanf(input, "%x %d", &offset, &units) != 2) {
        printf("Invalid input format. Please enter offset in hexadecimal and length in decimal.\n");
        return;
    }

    FILE* file = fopen(s->file_name, "rb");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", s->file_name);
        return;
    }

    if (fseek(file, offset, SEEK_SET) != 0) {
        printf("Error: Failed to seek to offset 0x%x in the file.\n", offset);
        fclose(file);
        return;
    }

    unsigned char buffer[10000];
    size_t bytes_read = fread(buffer, s->unit_size, units, file);
    if (bytes_read == 0) {
        printf("Error: Could not read from the file.\n");
        fclose(file);
        return;
    }

    fclose(file);

    printf("%s\n", display_mode ? "Hexadecimal" : "Decimal");
    printf("=======\n");

    for (size_t i = 0; i < units * s->unit_size; i += s->unit_size) {
        unsigned int value = 0;

        // Extract value based on unit size
        for (int j = 0; j < s->unit_size; j++) {
            value |= buffer[i + j] << (8 * j);
        }

        // Print the value in the appropriate format
        if (display_mode) {
            printf(hex_formats[s->unit_size - 1], value);
        } else {
            printf(dec_formats[s->unit_size - 1], value);
        }
    }
    
    printf("\n");
}

void memory_display(state* s) {
    if (s->mem_count == 0) {
        printf("Error: No memory is loaded.\n");
        return;
    }

    printf("Enter address (in decimal) and number of units to display: ");
    char input[100];
    int addr;
    int units;

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid input.\n");
        return;
    }

    if (sscanf(input, "%d %d", &addr, &units) != 2) {
        printf("Invalid input format. Please enter address and units in decimal.\n");
        return;
    }

    if (addr < 0 || addr >= s->mem_count) {
        printf("Error: address and range exceed memory bounds.\n");
        return;
    }

    printf("%s\n", display_mode ? "Hexadecimal" : "Decimal");
    printf("=======\n");

    for (int i = 0; i < s->mem_count / s->unit_size ; i++) {
        unsigned int value = 0;

        // Extract value from memory
        for (int j = 0; j < s->unit_size; j++) {
            value |= s->mem_buf[addr + i * s->unit_size + j] << (8 * j);
        }

        // Print value in the correct format
        if (display_mode) {
            printf(hex_formats[s->unit_size - 1], value);
        } else {
            printf(dec_formats[s->unit_size - 1], value);
        }
    }

    printf("\n");
}

void save_into_file(state* s) {
    if (s->file_name[0] == '\0') {
        printf("Error: No file name set.\n");
        return;
    }

    printf("Enter source address (in hexadecimal), target location (in hexadecimal), and number of units (in decimal): ");
    char input[100];
    unsigned int source_addr, target_loc;
    int units;

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid input.\n");
        return;
    }

    if (sscanf(input, "%x %x %d", &source_addr, &target_loc, &units) != 3) {
        printf("Invalid input format. Please enter source address, target location, and number of units.\n");
        return;
    }

    // Check for out-of-bounds access in memory
    if (source_addr + units * s->unit_size > s->mem_count) {
        printf("Error: Source address exceeds memory bounds.\n");
        return;
    }

    FILE* file = fopen(s->file_name, "rb+");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", s->file_name);
        return;
    }

    // Seek to the target location in the file
    if (fseek(file, target_loc, SEEK_SET) != 0) {
        printf("Error: Failed to seek to target location 0x%x in the file.\n", target_loc);
        fclose(file);
        return;
    }


    // Write data from mem_buf to the file
    size_t bytes_to_write = units * s->unit_size;
    size_t bytes_written = fwrite(s->mem_buf + source_addr, 1, bytes_to_write, file);

    if (bytes_written < bytes_to_write) {
        printf("Error: Failed to write all bytes to the file.\n");
        fclose(file);
        return;
    }

    fclose(file);

    printf("Wrote %zu bytes to file at offset 0x%x.\n", bytes_written, target_loc);

    // Debug information
    if (s->debug_mode) {
        fprintf(stderr, "Debug: Wrote %zu bytes from memory offset 0x%x to file offset 0x%x.\n",
                bytes_written, source_addr, target_loc);
    }
}

void memory_modify(state* s) {
    if (s->mem_count == 0) {
        printf("Error: No memory is loaded.\n");
        return;
    }

    printf("Enter memory location (in hexadecimal) and value (in hexadecimal): ");
    char input[100];
    unsigned int location, value;

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Invalid input.\n");
        return;
    }

    if (sscanf(input, "%x %x", &location, &value) != 2) {
        printf("Invalid input format. Please enter location and value in hexadecimal.\n");
        return;
    }

    // Ensure the location is within memory bounds
    if (location >= s->mem_count) {
        printf("Error: Location is out of bounds.\n");
        return;
    }

    // Modify the memory at the specified location with the new value
    size_t byte_offset = location * s->unit_size;

    // Ensure we're within the bounds of the memory buffer
    if (byte_offset + s->unit_size > sizeof(s->mem_buf)) {
        printf("Error: Out of memory bounds.\n");
        return;
    }

    // Set the value at the location
    for (int i = 0; i < s->unit_size; i++) {
        s->mem_buf[byte_offset + i] = (value >> (8 * i)) & 0xFF;
    }

    // Debug output if debug mode is on
    if (s->debug_mode) {
        fprintf(stderr, "Debug: Memory modified at location 0x%x with value 0x%x\n", location, value);
    }

    printf("Memory modified successfully.\n");
}

void quit(state* s) {
    if (s->debug_mode) {
        fprintf(stderr, "quitting\n");
    }
    free(s);
    exit(0);
}

void not_implemented_yet(state* s) {
    printf("Not implemented yet.\n");
}

int main() {
    state* s = (state*)calloc(1, sizeof(state));
    if (s == NULL) {
        perror("Failed to allocate state");
        return 1;
    }

    s->unit_size = 1; // Default unit size

    int choice;
    char input[100]; 

    while (1) {
        printf("Choose action:\n");
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("\n");

        if (s->debug_mode) {
            fprintf(stderr, "Debug: unit_size=%d, file_name='%s', mem_count=%zu\n", 
                    s->unit_size, s->file_name, s->mem_count);
        }

        printf("Enter your choice: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Invalid input.\n");
            continue;
        }

        choice = atoi(input);

        if (choice >= 0 && choice < (sizeof(menu) / sizeof(menu[0]) - 1)) {
            menu[choice].fun(s);
            printf("DONE.\n\n");
        } else {
            printf("Not within bounds\n");
        }
    }

    return 0;
}

