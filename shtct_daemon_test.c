/**
 * 
 * IMPLEMENT IN FUTURE
 * Pass array of integers to keyboard_listener()
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<linux/input.h>//structure definition of input_event is here
#include<unistd.h>//read()
#include<fcntl.h>//has open() and O_RDONLY macro
#include<errno.h>//for errno and EINTR. Look into these and what these mean
#include<time.h>//for struct tm and other tiem related functions. GET BETTER UNDERSTANDING

typedef struct keycodes_structure
{
    char key_name[12];
    int key_code;
}KSTR;

void date_and_time(char*);
int kbd_read(KSTR*);
int key_detect(int* key_code, ssize_t* n,int* fd,struct input_event* ev, KSTR* kbd_array, int recurse);//function containing only loop part to make recursion possible
int keyboard_listener(int* activation_key_code);

int main()
{
    FILE *fp;
    int activation_key_codes[4] = {42,54,48,-1}; // save last element as -1 to denote end
        if(keyboard_listener(activation_key_codes))
        {
            printf("Both shifts pressed\n");

            char buffer[50];
            float bright_value = 1;
            sprintf(buffer,"./scripts/brightc.sh %0.2f", bright_value);
            fp = popen(buffer,"r");
            if(fp == NULL)
            {
                printf("popen failed to open brightc.sh sed \n");
            }
            pclose(fp);
        }
    return 1;
}

int keyboard_listener(int* activation_key_codes)
{
    const char *kbd_path = "/dev/input/event11";
    struct input_event ev;
    int fd;
    ssize_t n;
    KSTR kbd_array[85];
    int kbd_array_status;
    int key_detect_return;
    int recurse = 0;

    kbd_array_status = kbd_read(kbd_array);
    fd = open(kbd_path,O_RDONLY);
    if(fd == -1 || !kbd_array_status)
    {
        printf("Failed to open required files. Terminating.\n");
        exit(0);
    }
    if(activation_key_codes[1] != -1)
    {
        recurse = 1;
    }
    
    key_detect_return = key_detect(activation_key_codes,&n,&fd,&ev,kbd_array,recurse);
    close(fd);
    return key_detect_return;
}

int key_detect(int* activation_key_codes, ssize_t* n,int* fd,struct input_event* ev, KSTR* kbd_array, int recurse)
{
    char timenow[30];
    while(1)
    {
        fflush(stdin);
        *n = read(*fd, ev, sizeof(struct input_event));
        if(*n == (ssize_t)-1)
        {
            if(errno = EINTR)//Read into this
                continue;
            else
                break;
        }
        if (*n != sizeof(struct input_event))
        {
            continue;
        }
        if(((*ev).type == EV_KEY) && ((*ev).value <=2 && (*ev).value >=0))
        {
            if((*ev).code == activation_key_codes[0] && recurse == 1 && ((*ev).value == 1 || (*ev).value == 2))
            {
                if((activation_key_codes+1)[1] == -1)
                {
                    recurse = 0;
                }
                printf("\n\nI am now going to call the same function \n");    
                return key_detect(activation_key_codes+1,n,fd,ev,kbd_array,recurse);
            }
            else if((*ev).code == activation_key_codes[0] && recurse ==0 && ((*ev).value == 1 || (*ev).value == 2))
            {    
                printf("\n\n I am done and will now be returning 1 \n");    
                return 1;
            }
        }
    }
}

int kbd_read(KSTR* kbd_array)
{
    FILE* key_codes_fp = fopen("c_keycodes.csv","r");
    if(!key_codes_fp)
    {
        fclose(key_codes_fp);
        return 0;
    }
    char buffer[700];
    int row=0;
    while(fgets(buffer,700,key_codes_fp))
    {
        strcpy(kbd_array[row].key_name,strtok(buffer,","));
        kbd_array[row].key_code = atoi(strtok(NULL,","));
        row++;
    }
    fclose(key_codes_fp);
    return 1;
}