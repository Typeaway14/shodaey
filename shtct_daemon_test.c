/**
 * 
 * IMPLEMENT IN FUTURE
 * Pass array of integers to keyboard_listener()
 */

#include<stdio.h>
#include<stdlib.h>
#include<linux/input.h>//structure definition of input_event is here
#include<unistd.h>//read()
#include<fcntl.h>//has open() and O_RDONLY macro
#include<errno.h>//for errno and EINTR. Look into these and what these mean

int key_detect(int* key_code, ssize_t* n,int* fd,struct input_event* ev, int recurse);//function containing only loop part to make recursion possible
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
    int key_detect_return;
    int recurse = 0;

    fd = open(kbd_path,O_RDONLY);
    if(fd == -1)
    {
        printf("Failed to open required files. Terminating.\n");
        exit(0);
    }
    if(activation_key_codes[1] != -1)
    {
        recurse = 1;
    }
    
    key_detect_return = key_detect(activation_key_codes,&n,&fd,&ev,recurse);
    close(fd);
    return key_detect_return;
}

int key_detect(int* activation_key_codes, ssize_t* n,int* fd,struct input_event* ev, int recurse)
{
    while(1)
    {
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
                return key_detect(activation_key_codes+1,n,fd,ev,recurse);
            }
            else if((*ev).code == activation_key_codes[0] && recurse ==0 && ((*ev).value == 1 || (*ev).value == 2))
            {    
                printf("\n\n I am done and will now be returning 1 \n");    
                return 1;
            }
        }
    }
}