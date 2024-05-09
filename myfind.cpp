#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

// global variable for the program name
char* program_name;

void error_message(){
    fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2] …[filenameN]",program_name);
    exit(EXIT_FAILURE);
}

char* complete_path_to_file(char* cwd, const char* prefix_path, char* filename){
    char* result = (char*)malloc(1024);
    if(result == NULL){
        exit(EXIT_FAILURE);
    }
    strcat(result, cwd);
    strcat(result,"/");
    strcat(result,prefix_path);
    strcat(result,filename);
    return result;
}

void to_lower(char* string) {
    for(size_t i = 0; i < strlen(string); i++){
        string[i] = tolower(string[i]);
    }
}

void getCWD(char* cwd){ // out parameter
    // string of the current working directory
    if(getcwd(cwd,255) == NULL){
        perror("Failed to get current working directory");
        exit(EXIT_FAILURE);
    }
}

void check_directory(char* path, char** filenames, int filesnames_size, bool option_i=false, bool option_R=false, const char* prefix_path=""){
    
    DIR* dir_path;
    if((dir_path = opendir(path)) == NULL){
        fprintf(stderr,"Failed to open directory: %s\n", path);
        perror("Failed to open directory (perror)\n");
        exit(EXIT_FAILURE);
    }

    dirent* dir_entry;
    
    dirent* directory_list[255];
    int directory_list_index=0;

    char cwd[255];
    getCWD(cwd);

    while((dir_entry = readdir(dir_path)) != NULL){

        if( strcmp(dir_entry->d_name,".")!=0 && strcmp(dir_entry->d_name,"..")!=0 && dir_entry->d_type == DT_DIR){
            directory_list[directory_list_index] = dir_entry;
            printf("directory added: %s\n",dir_entry->d_name);
            ++directory_list_index;
        }

        //debug dir_entries - printf("dir_entry = %s\n",dir_entry->d_name);
        int i =0;
        
        while(i<filesnames_size){
            if(option_i){
                to_lower(dir_entry->d_name);
                to_lower(filenames[i]);
            }
            if(strcmp(dir_entry->d_name,filenames[i])==0)
            {
                printf("<%s>:<%s>\n",filenames[i], complete_path_to_file(cwd, prefix_path, filenames[i]));
            }
            ++i;
        }
    }

    if(option_R){ // recursively search for the files in subdirectories
        int j =0;
        while(j< directory_list_index){
            char new_prefix_path[1024]; // Adjust size as needed
            snprintf(new_prefix_path, sizeof(new_prefix_path), "%s%s/", prefix_path, directory_list[j]->d_name);

            check_directory(complete_path_to_file(cwd,prefix_path,directory_list[j]->d_name),filenames,filesnames_size,option_i,option_R,new_prefix_path);
            ++j;
        }
    }

}


int main(int argc, char** argv){

    // setting global program_name
    program_name = argv[0];


    char option;
    int option_R=0;
    int option_i=0;
    bool error = false;


    while ( (option = getopt(argc,argv,"Ri")) != -1){
       switch(option){
        case 'R': 
            if(option_R) error = true;
            ++option_R;
            // do something with the option
            break;
        case 'i': 
            if(option_i) error = true;
            ++option_i;
            // do something  with the option
            break;
        case '?': 
            error = true;
            break;
        default: assert(0); break; // default should never be called 
       }
    }

    if(error || (argc < optind+2)) error_message();

    // absolute/relative path that should be used to search for the files
    char* dir_path =argv[optind];

    // collect the files to search for in a list
    ++optind;
    int filesnames_size = argc-optind;
    // debug filesnames_size - printf("filename_size: %d\n",filesnames_size);
    char* filenames[filesnames_size];
    int filenames_index=0;
    while(optind < argc){
        filenames[filenames_index] = argv[optind];
        // debug filenames - printf("this is one of the files to search for: %s\n", argv[optind]);
        optind++;
        filenames_index++;
    }

    check_directory(dir_path, filenames, filesnames_size, option_i, option_R);
    

    printf("exit with success\n");

    return 0;
}