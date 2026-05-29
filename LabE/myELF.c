#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>

#define MAX_FILES 2

const char *get_section_type_name(uint32_t type) {
    switch (type) {
        case SHT_NULL: return "NULL";
        case SHT_PROGBITS: return "PROGBITS";
        case SHT_SYMTAB: return "SYMTAB";
        case SHT_STRTAB: return "STRTAB";
        case SHT_RELA: return "RELA";
        case SHT_HASH: return "HASH";
        case SHT_DYNAMIC: return "DYNAMIC";
        case SHT_NOTE: return "NOTE";
        case SHT_NOBITS: return "NOBITS";
        case SHT_REL: return "REL";
        case SHT_SHLIB: return "SHLIB";
        case SHT_DYNSYM: return "DYNSYM";
        case SHT_INIT_ARRAY: return "INIT_ARRAY";
        case SHT_FINI_ARRAY: return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY: return "PREINIT_ARRAY";
        case SHT_GROUP: return "GROUP";
        case SHT_SYMTAB_SHNDX: return "SYMTAB_SHNDX";
        case SHT_NUM: return "NUM";
        case SHT_GNU_HASH: return "GNU_HASH";
        case SHT_GNU_verdef: return "VERDEF";
        case SHT_GNU_verneed: return "VERNEED";
        case SHT_GNU_versym: return "VERSYM";
        default: return "UNKNOWN";
    }
}

struct ELFFile {
    char filename[256];
    int fd;
    void *map_start;
    size_t file_size;
};

struct ELFFile files[MAX_FILES] = {{.fd = -1}, {.fd = -1}};
int debug_mode = 0;

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    printf("Debug mode is now %s\n", debug_mode ? "ON" : "OFF");
}

void unmap_and_close_file(struct ELFFile *file) {
    if (file->map_start) {
        munmap(file->map_start, file->file_size);
    }
    if (file->fd != -1) {
        close(file->fd);
    }
    file->fd = -1;
    file->map_start = NULL;
    file->file_size = 0;
    file->filename[0] = '\0';
}

void examine_elf_file() {
    if (files[0].fd != -1 && files[1].fd != -1) {
        printf("Only 2 ELF files can be mapped at a time.\n");
        return;
    }

    char filename[256];
    printf("Enter ELF file name: ");
    scanf("%255s", filename);

    struct ELFFile *file = files[0].fd == -1 ? &files[0] : &files[1];
    file->fd = open(filename, O_RDONLY);
    if (file->fd < 0) {
        perror("Error opening file");
        return;
    }

    struct stat st;
    if (fstat(file->fd, &st) != 0) {
        perror("Error getting file stats");
        close(file->fd);
        return;
    }

    file->file_size = st.st_size;
    file->map_start = mmap(NULL, file->file_size, PROT_READ, MAP_PRIVATE, file->fd, 0);
    if (file->map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(file->fd);
        return;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *)file->map_start;

    // Validate ELF file
    if (strncmp((char *)header->e_ident, "\x7f""ELF", 4) != 0) {
        printf("Not an ELF file.\n");
        unmap_and_close_file(file);
        return;
    }

    // Print ELF header details
    printf("Magic:   %.3s\n", header->e_ident + 1);
    printf("Data:    %s\n", header->e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian" : "2's complement, big endian");
    printf("Entry point address: 0x%x\n", header->e_entry);
    printf("Start of section headers: %d (bytes into file)\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of section headers: %d (bytes)\n", header->e_shentsize);
    printf("Start of program headers: %d (bytes into file)\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of program headers: %d (bytes)\n", header->e_phentsize);

    if (debug_mode) {
        printf("Debug: Successfully examined ELF file %s\n", filename);
    }
}

void print_section_names() {
    for (int i = 0; i < MAX_FILES; i++) {
        struct ELFFile *file = &files[i];
        if (file->fd == -1) {
            continue;
        }

        Elf32_Ehdr *header = (Elf32_Ehdr *)file->map_start;
        Elf32_Shdr *section_headers = (Elf32_Shdr *)(file->map_start + header->e_shoff);
        Elf32_Shdr *shstrtab_header = &section_headers[header->e_shstrndx];
        const char *shstrtab = (const char *)(file->map_start + shstrtab_header->sh_offset);

        printf("File: %s\n", file->filename);

        for (int j = 0; j < header->e_shnum; j++) {
            Elf32_Shdr *section = &section_headers[j];
            const char *section_name = shstrtab + section->sh_name;

            printf("[%2d] %-20s 0x%08x 0x%06x 0x%06x %-10s\n",
                   j, section_name, section->sh_addr, section->sh_offset,
                   section->sh_size, get_section_type_name(section->sh_type));

            if (debug_mode) {
                printf("Debug: Section index: %d, Name offset: %d\n", j, section->sh_name);
            }
        }
        printf("\n");
    }
}

void print_symbols() {
    for (int i = 0; i < MAX_FILES; i++) {
        struct ELFFile *file = &files[i];
        if (file->fd == -1) {
            continue;
        }

        Elf32_Ehdr *header = (Elf32_Ehdr *)file->map_start;
        Elf32_Shdr *section_headers = (Elf32_Shdr *)(file->map_start + header->e_shoff);
        const char *shstrtab = (const char *)(file->map_start + section_headers[header->e_shstrndx].sh_offset);

        printf("File: %s\n", file->filename);

        int symbols_found = 0;

        for (int j = 0; j < header->e_shnum; j++) {
            Elf32_Shdr *section = &section_headers[j];

            // Find symbol table sections
            if (section->sh_type == SHT_SYMTAB || section->sh_type == SHT_DYNSYM) {
                symbols_found = 1;

                // Symbol table details
                const char *symtab_name = shstrtab + section->sh_name;
                Elf32_Sym *symbols = (Elf32_Sym *)(file->map_start + section->sh_offset);
                int symbol_count = section->sh_size / section->sh_entsize;

                // Associated string table for symbol names
                Elf32_Shdr *strtab_header = &section_headers[section->sh_link];
                const char *strtab = (const char *)(file->map_start + strtab_header->sh_offset);

                if (debug_mode) {
                    printf("Debug: Symbol table '%s' contains %d symbols.\n", symtab_name, symbol_count);
                }

                for (int k = 0; k < symbol_count; k++) {
                    Elf32_Sym *symbol = &symbols[k];
                    const char *symbol_name = strtab + symbol->st_name;

                    // Get section name
                    const char *section_name = (symbol->st_shndx < header->e_shnum)
                                                   ? shstrtab + section_headers[symbol->st_shndx].sh_name
                                                   : "ABSENT";

                    // Print symbol details
                    printf("[%2d] 0x%08x %4d %-20s %s\n",
                           k, symbol->st_value, symbol->st_shndx, section_name, symbol_name);
                }
                printf("\n");
            }
        }

        if (!symbols_found) {
            printf("No symbols found in file: %s\n", file->filename);
        }
    }
}


void check_merge() {
    if (files[0].fd == -1 || files[1].fd == -1) {
        printf("Error: Exactly two ELF files must be loaded for this operation.\n");
        return;
    }

    Elf32_Ehdr *headers[2] = {(Elf32_Ehdr *)files[0].map_start, (Elf32_Ehdr *)files[1].map_start};
    Elf32_Shdr *section_headers[2] = {
        (Elf32_Shdr *)(files[0].map_start + headers[0]->e_shoff),
        (Elf32_Shdr *)(files[1].map_start + headers[1]->e_shoff)
    };

    Elf32_Sym *symtabs[2] = {NULL, NULL};
    const char *strtabs[2] = {NULL, NULL};
    int symbol_counts[2] = {0, 0};

    // Locate symbol tables and calculate the number of symbols
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < headers[i]->e_shnum; j++) {
            Elf32_Shdr *section = &section_headers[i][j];
            if (section->sh_type == SHT_SYMTAB) {
                if (symtabs[i] != NULL) {
                    printf("Feature not supported: Multiple symbol tables in file %d.\n", i + 1);
                    return;
                }
                symtabs[i] = (Elf32_Sym *)(files[i].map_start + section->sh_offset);
                strtabs[i] = (const char *)(files[i].map_start + section_headers[i][section->sh_link].sh_offset);
                if (section->sh_entsize == 0) {
                    printf("Error: Invalid symbol table in file %d (entry size is 0).\n", i + 1);
                    return;
                }
                symbol_counts[i] = section->sh_size / section->sh_entsize;
            }
        }
        if (symtabs[i] == NULL) {
            printf("Error: No symbol table found in file %d.\n", i + 1);
            return;
        }
    }

    // Compare symbols in both files
    for (int i = 0; i < 2; i++) {
        int other = 1 - i;

        for (int j = 1; j < symbol_counts[i]; j++) { // Skip the first (dummy) symbol
            Elf32_Sym *symbol = &symtabs[i][j];
            const char *symbol_name = strtabs[i] + symbol->st_name;

            if (strlen(symbol_name) == 0) { // Skip symbols with empty names
                continue;
            }

            if (symbol->st_shndx == SHN_UNDEF) { // Undefined symbol
                int found = 0, undef_in_other = 0;
                for (int k = 1; k < symbol_counts[other]; k++) {
                    Elf32_Sym *other_symbol = &symtabs[other][k];
                    const char *other_symbol_name = strtabs[other] + other_symbol->st_name;

                    if (strcmp(symbol_name, other_symbol_name) == 0) {
                        found = 1;
                        if (other_symbol->st_shndx == SHN_UNDEF) {
                            undef_in_other = 1;
                        }
                        break;
                    }
                }
                if (!found || undef_in_other) {
                    printf("Symbol %s undefined.\n", symbol_name);
                }
            } else { // Defined symbol
                for (int k = 1; k < symbol_counts[other]; k++) {
                    Elf32_Sym *other_symbol = &symtabs[other][k];
                    const char *other_symbol_name = strtabs[other] + other_symbol->st_name;

                    if (strcmp(symbol_name, other_symbol_name) == 0 &&
                        other_symbol->st_shndx != SHN_UNDEF) {
                        printf("Symbol %s multiply defined.\n", symbol_name);
                    }
                }
            }
        }
    }
}


void merge_files() {
    if (files[0].fd == -1 || files[1].fd == -1) {
        printf("Error: Two ELF files must be loaded for merging.\n");
        return;
    }

    // Open output file
    int outfd = open("out.ro", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (outfd == -1) {
        perror("Error creating output file");
        return;
    }

    // Get headers and section tables for both files
    Elf32_Ehdr *header1 = (Elf32_Ehdr *)files[0].map_start;
    Elf32_Ehdr *header2 = (Elf32_Ehdr *)files[1].map_start;
    Elf32_Shdr *sections1 = (Elf32_Shdr *)(files[0].map_start + header1->e_shoff);
    Elf32_Shdr *sections2 = (Elf32_Shdr *)(files[1].map_start + header2->e_shoff);

    // Write initial ELF header (copy from first file)
    Elf32_Ehdr new_header = *header1;
    if (write(outfd, &new_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error writing ELF header");
        close(outfd);
        return;
    }

    // Create new section header table (initially copy from first file)
    Elf32_Shdr *new_sections = malloc(header1->e_shnum * sizeof(Elf32_Shdr));
    memcpy(new_sections, sections1, header1->e_shnum * sizeof(Elf32_Shdr));

    // Get string tables
    const char *shstrtab1 = (char *)(files[0].map_start + sections1[header1->e_shstrndx].sh_offset);
    const char *shstrtab2 = (char *)(files[1].map_start + sections2[header2->e_shstrndx].sh_offset);

    // Current offset for writing sections (start after ELF header)
    off_t current_offset = sizeof(Elf32_Ehdr);

    // Process each section
    for (int i = 0; i < header1->e_shnum; i++) {
        const char *section_name = shstrtab1 + sections1[i].sh_name;
        
        // Update section offset in new section header
        new_sections[i].sh_offset = current_offset;

        // Check if this is a mergeable section
        if (strcmp(section_name, ".text") == 0 || 
            strcmp(section_name, ".data") == 0 || 
            strcmp(section_name, ".rodata") == 0) {
            
            // Write content from first file
            if (sections1[i].sh_size > 0) {
                if (write(outfd, files[0].map_start + sections1[i].sh_offset, 
                         sections1[i].sh_size) != sections1[i].sh_size) {
                    perror("Error writing section from first file");
                    free(new_sections);
                    close(outfd);
                    return;
                }
            }

            // Find and write corresponding section from second file
            for (int j = 0; j < header2->e_shnum; j++) {
                const char *section_name2 = shstrtab2 + sections2[j].sh_name;
                if (strcmp(section_name, section_name2) == 0) {
                    if (sections2[j].sh_size > 0) {
                        if (write(outfd, files[1].map_start + sections2[j].sh_offset,
                                sections2[j].sh_size) != sections2[j].sh_size) {
                            perror("Error writing section from second file");
                            free(new_sections);
                            close(outfd);
                            return;
                        }
                    }
                    // Update size in new section header
                    new_sections[i].sh_size = sections1[i].sh_size + sections2[j].sh_size;
                    break;
                }
            }
        } else {
            // Copy section as-is from first file
            if (sections1[i].sh_size > 0) {
                if (write(outfd, files[0].map_start + sections1[i].sh_offset,
                         sections1[i].sh_size) != sections1[i].sh_size) {
                    perror("Error writing non-mergeable section");
                    free(new_sections);
                    close(outfd);
                    return;
                }
            }
        }

        // Update current offset
        if (sections1[i].sh_size > 0) {
            current_offset = lseek(outfd, 0, SEEK_CUR);
        }
    }

    // Record section header table offset
    off_t sh_offset = current_offset;

    // Write section header table
    if (write(outfd, new_sections, header1->e_shnum * sizeof(Elf32_Shdr)) != 
        header1->e_shnum * sizeof(Elf32_Shdr)) {
        perror("Error writing section header table");
        free(new_sections);
        close(outfd);
        return;
    }

    // Update and rewrite ELF header with correct section header table offset
    new_header.e_shoff = sh_offset;
    lseek(outfd, 0, SEEK_SET);
    if (write(outfd, &new_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error updating ELF header");
        free(new_sections);
        close(outfd);
        return;
    }

    free(new_sections);
    close(outfd);
    printf("Successfully created merged file 'out.ro'\n");
}

// Part 3.3: Symbol Resolution
// Debug helper function
void print_symbol_info(const char* prefix, Elf32_Sym* sym, const char* name) {
    printf("%s: %s (value: 0x%x, size: %d, section: %d)\n", 
           prefix, name, sym->st_value, sym->st_size, sym->st_shndx);
}

void resolve_symbols() {
    if (files[0].fd == -1 || files[1].fd == -1) {
        printf("Error: Two ELF files must be loaded for symbol resolution.\n");
        return;
    }

    Elf32_Ehdr *header1 = (Elf32_Ehdr *)files[0].map_start;
    Elf32_Ehdr *header2 = (Elf32_Ehdr *)files[1].map_start;
    Elf32_Shdr *sections1 = (Elf32_Shdr *)(files[0].map_start + header1->e_shoff);
    Elf32_Shdr *sections2 = (Elf32_Shdr *)(files[1].map_start + header2->e_shoff);

    // Get string tables
    const char *shstrtab1 = (const char *)(files[0].map_start + sections1[header1->e_shstrndx].sh_offset);
    const char *shstrtab2 = (const char *)(files[1].map_start + sections2[header2->e_shstrndx].sh_offset);

    Elf32_Sym *symtab1 = NULL;
    Elf32_Sym *symtab2 = NULL;
    const char *strtab1 = NULL;
    const char *strtab2 = NULL;
    int symtab_size1 = 0;
    int symtab_size2 = 0;
    Elf32_Shdr *symtab_header1 = NULL;
    Elf32_Shdr *symtab_header2 = NULL;

    // Find symbol tables and string tables in both files
    for (int i = 0; i < header1->e_shnum; i++) {
        if (sections1[i].sh_type == SHT_SYMTAB) {
            symtab1 = (Elf32_Sym *)(files[0].map_start + sections1[i].sh_offset);
            strtab1 = (const char *)(files[0].map_start + sections1[sections1[i].sh_link].sh_offset);
            symtab_size1 = sections1[i].sh_size / sections1[i].sh_entsize;
            symtab_header1 = &sections1[i];
            break;
        }
    }

    for (int i = 0; i < header2->e_shnum; i++) {
        if (sections2[i].sh_type == SHT_SYMTAB) {
            symtab2 = (Elf32_Sym *)(files[1].map_start + sections2[i].sh_offset);
            strtab2 = (const char *)(files[1].map_start + sections2[sections2[i].sh_link].sh_offset);
            symtab_size2 = sections2[i].sh_size / sections2[i].sh_entsize;
            symtab_header2 = &sections2[i];
            break;
        }
    }

    if (!symtab1 || !symtab2 || !strtab1 || !strtab2) {
        printf("Error: Symbol tables not found in both files.\n");
        return;
    }

    if (debug_mode) {
        printf("Found symbol tables - File1: %d symbols, File2: %d symbols\n", 
               symtab_size1, symtab_size2);
    }

    // Create new symbol table with resolved symbols
    Elf32_Sym *new_symtab = malloc(symtab_header1->sh_size);
    memcpy(new_symtab, symtab1, symtab_header1->sh_size);

    // Calculate offsets for merged sections
    off_t text_offset = 0;
    off_t data_offset = 0;
    off_t rodata_offset = 0;

    // Get first file's section sizes
    for (int i = 0; i < header1->e_shnum; i++) {
        const char *section_name = shstrtab1 + sections1[i].sh_name;
        if (strcmp(section_name, ".text") == 0) text_offset = sections1[i].sh_size;
        else if (strcmp(section_name, ".data") == 0) data_offset = sections1[i].sh_size;
        else if (strcmp(section_name, ".rodata") == 0) rodata_offset = sections1[i].sh_size;
    }

    if (debug_mode) {
        printf("Section offsets - .text: %ld, .data: %ld, .rodata: %ld\n", 
               text_offset, data_offset, rodata_offset);
    }

    // Resolve undefined symbols from first file with definitions from second file
    for (int i = 1; i < symtab_size1; i++) {
        if (new_symtab[i].st_shndx == SHN_UNDEF) {
            const char *sym_name = strtab1 + new_symtab[i].st_name;
            
            if (debug_mode) {
                printf("Looking for undefined symbol: %s\n", sym_name);
            }
            
            // Search for definition in second file
            for (int j = 1; j < symtab_size2; j++) {
                const char *sym_name2 = strtab2 + symtab2[j].st_name;
                if (strcmp(sym_name, sym_name2) == 0 && symtab2[j].st_shndx != SHN_UNDEF) {
                    if (debug_mode) {
                        print_symbol_info("Found symbol", &symtab2[j], sym_name2);
                    }

                    // Copy symbol information and adjust value based on section
                    const char *sym_section_name = 
                        (symtab2[j].st_shndx < header2->e_shnum) ?
                        shstrtab2 + sections2[symtab2[j].st_shndx].sh_name : "";

                    off_t offset = 0;
                    if (strcmp(sym_section_name, ".text") == 0) 
                        offset = text_offset;
                    else if (strcmp(sym_section_name, ".data") == 0)
                        offset = data_offset;
                    else if (strcmp(sym_section_name, ".rodata") == 0)
                        offset = rodata_offset;

                    if (debug_mode) {
                        printf("Adjusting symbol value by offset %ld for section %s\n", 
                               offset, sym_section_name);
                    }

                    new_symtab[i].st_value = symtab2[j].st_value + offset;
                    new_symtab[i].st_size = symtab2[j].st_size;
                    new_symtab[i].st_info = symtab2[j].st_info;
                    new_symtab[i].st_other = symtab2[j].st_other;
                    new_symtab[i].st_shndx = symtab2[j].st_shndx;
                    break;
                }
            }
        }
    }

    // Open the merged file for updating
    int outfd = open("out.ro", O_RDWR);
    if (outfd == -1) {
        perror("Error opening merged file for symbol resolution");
        free(new_symtab);
        return;
    }

    // Find symbol table offset in merged file
    Elf32_Ehdr out_header;
    if (read(outfd, &out_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error reading merged file header");
        free(new_symtab);
        close(outfd);
        return;
    }

    Elf32_Shdr *out_sections = malloc(out_header.e_shnum * sizeof(Elf32_Shdr));
    lseek(outfd, out_header.e_shoff, SEEK_SET);
    if (read(outfd, out_sections, out_header.e_shnum * sizeof(Elf32_Shdr)) != 
        out_header.e_shnum * sizeof(Elf32_Shdr)) {
        perror("Error reading merged file section headers");
        free(new_symtab);
        free(out_sections);
        close(outfd);
        return;
    }

    // Find symbol table section and update it
    for (int i = 0; i < out_header.e_shnum; i++) {
        if (out_sections[i].sh_type == SHT_SYMTAB) {
            if (debug_mode) {
                printf("Writing symbol table at offset 0x%lx\n", 
                       (unsigned long)out_sections[i].sh_offset);
            }
            lseek(outfd, out_sections[i].sh_offset, SEEK_SET);
            if (write(outfd, new_symtab, out_sections[i].sh_size) != out_sections[i].sh_size) {
                perror("Error writing resolved symbol table");
                free(new_symtab);
                free(out_sections);
                close(outfd);
                return;
            }
            break;
        }
    }

    free(new_symtab);
    free(out_sections);
    close(outfd);
    printf("Successfully resolved symbols in merged file\n");
}

void quit_program() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].fd != -1) {
            unmap_and_close_file(&files[i]);
        }
    }
    printf("Exiting program.\n");
    exit(EXIT_SUCCESS);
}

int main() {
    struct fun_desc {
        char *name;
        void (*fun)();
    };

    struct fun_desc menu[] = {
        {"Toggle Debug Mode", toggle_debug_mode},
        {"Examine ELF File", examine_elf_file},
        {"Print Section Names", print_section_names},
        {"Print Symbols", print_symbols},
        {"Check Files for Merge", check_merge},
        {"Merge ELF Files", merge_files},
        {"Resolve Symbols", resolve_symbols},
        {"Quit", quit_program},
        {NULL, NULL}
    };


    int menu_size = sizeof(menu) / sizeof(menu[0]) - 1;

    while (1) {
        printf("Choose action:\n");
        for (int i = 0; i < menu_size; i++) {
            printf("%d) %s\n", i, menu[i].name);
        }

        printf("Enter your choice: ");
        int choice;
        if (scanf("%d", &choice) != 1) {
            break;
        }

        if (choice >= 0 && choice < menu_size) {
            menu[choice].fun();
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }

    quit_program();
    return 0;
}