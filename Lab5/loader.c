#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// for task 2c
int startup(int argc, char **argv, void (*start)());

char* phdr_type(Elf32_Phdr *phdr){
    switch(phdr->p_type) {
        case PT_NULL: return "NULL";
        case PT_LOAD: return "LOAD";
        case PT_DYNAMIC: return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE: return "NOTE";
        case PT_SHLIB: return "SHLIB";
        case PT_PHDR: return "PHDR";
        case PT_TLS: return "TLS";
        case PT_NUM: return "NUM";
        case PT_LOOS: return "LOOS";
        case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
        case PT_GNU_STACK: return "GNU_STACK";
        case PT_GNU_RELRO: return "GNU_RELRO";
        case PT_LOSUNW: return "LOSUNW";
        case PT_SUNWSTACK: return "SUNWSTACK";
        case PT_HIPROC: return "HIPROC";
        default: return "";
    }
}

void task0(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %x\n", index, phdr->p_vaddr);
}

void task1a(Elf32_Phdr *phdr, int index) {
    char x = ' ', w = ' ', r = ' ';
    if (phdr->p_flags & PF_X) {
        x = 'E';
    }
    if (phdr->p_flags & PF_R) {
        r = 'R';
    }
    if (phdr->p_flags & PF_W) {
        w = 'W';
    }

    printf("%s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x\n",
           phdr_type(phdr), phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, r, w, x, phdr->p_align);
}

void task1b(Elf32_Phdr *phdr, int index) {
    printf("Protection flags: ");
    if (phdr->p_flags & PF_R) {
        printf("PROT_READ ");
    }
    if (phdr->p_flags & PF_W) {
        printf("PROT_WRITE ");
    }
    if (phdr->p_flags & PF_X) {
        printf("PROT_EXEC");
    }
    printf("\nMapping flags: MAP_FIXED, MAP_PRIVATE\n");
}

void load_phdr(Elf32_Phdr *phdr, int fd) {
    if (phdr->p_type == PT_LOAD) {
        int mapping = MAP_FIXED | MAP_PRIVATE;
        int prots = PROT_NONE;
        if (phdr->p_flags & PF_R) {
            prots |= PROT_READ;
        }
        if (phdr->p_flags & PF_W) {
            prots |= PROT_WRITE;
        }
        if (phdr->p_flags & PF_X) {
            prots |= PROT_EXEC;
        }
        void *vaddr = (void *)(phdr->p_vaddr & 0xfffff000);
        off_t offset = phdr->p_offset & 0xfffff000;
        size_t padding = phdr->p_vaddr & 0xfff;
        void *map = mmap(vaddr, phdr->p_memsz + padding, prots, mapping, fd, offset);
        if (map == MAP_FAILED) {
            perror("map");
        }
    }
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++) {
        if (arg == -1)
            func(phdr + i, i);
        else
            func(phdr + i, arg);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Invalid usage: %s fileName\n", argv[0]);
        exit(0);
    }
    int fd = open(argv[1], O_RDONLY);
    struct stat sb;
    if (fd == -1) {
        perror("open");
        return 1;
    }
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        return 1;
    }
    void *map_start = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    foreach_phdr(map_start, task0, -1);
    printf("Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align\n");
    foreach_phdr(map_start, task1a, -1);
    foreach_phdr(map_start, task1b, -1);
    foreach_phdr(map_start, load_phdr, fd);
    Elf32_Ehdr *elf_head = (Elf32_Ehdr *)map_start;
    startup(argc - 1, argv + 1, (void *)(elf_head->e_entry));
    return 0;
}
