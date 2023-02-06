#include<stdio.h>
#include<stdlib.h>
#include<linux/input.h>//structure definition of input_event is here
#include<unistd.h>//read()
#include<fcntl.h>//has open() and O_RDONLY macro
#include<errno.h>//for errno and EINTR. Look into these and what these mean

int fd; // global variable file descriptor for kbd_path

int key_detect(int* key_code, ssize_t* read_return,struct input_event* ev, int recurse);//function containing only loop part to make recursion possible
int key_tracker(int* activation_key_code);
void keyboard_listener(int* key_details);
int init();

int main()
{
    if(!init())
    {
        printf("Initialization of required variables failed!\n");
        exit(0);
    }
    FILE *fp;
    int activation_key_codes[4] = {42,54,48,-1}; // save last element as -1 to denote end
    int key_details[2];
    if(key_tracker(activation_key_codes))
    {

        char buffer[50];
        float bright_value = 1;
        do
        {
            keyboard_listener(key_details);
            if(key_details[0] == 24 && key_details[1] > 0 && bright_value > 0.2) // 24 is code for o
            {
                bright_value -= 0.05;
            }
            else if(key_details[0] == 25 && key_details[1] > 0 && bright_value != 1) // 25 is code for p
            {
                bright_value += 0.05;
            }
            else if(key_details[0] == 16) // 16 is code for q
            {
                break;
            }
            else
            {
                continue;
            }
            sprintf(buffer,"./scripts/brightc.sh %0.2f", bright_value);
            system(buffer);
        }
        while(1);
    }
    close(fd);
    return 1;
}

int init()
{
    const char *kbd_path = "/dev/input/event2";
    fd = open(kbd_path,O_RDONLY);
    if(fd == -1)
    {
        printf("Failed to open required files. Terminating.\n");
        return 0;
    }
    return 1;
}

int key_tracker(int* activation_key_codes)
{
    struct input_event ev;
    ssize_t read_return;
    int key_detect_return;
    int recurse = 0;
    
    if(activation_key_codes[1] != -1)
    {
        recurse = 1;
    }
    
    key_detect_return = key_detect(activation_key_codes,&read_return,&ev,recurse);
    return key_detect_return;
}

int key_detect(int* activation_key_codes, ssize_t* read_return,struct input_event* ev, int recurse)
{
    while(1)
    {
        *read_return = read(fd, ev, sizeof(struct input_event));
        if(*read_return == (ssize_t)-1)
        {
            if(errno = EINTR)//Read into this
                continue;
            else
                break;
        }
        if (*read_return != sizeof(struct input_event))
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
                return key_detect(activation_key_codes+1,read_return,ev,recurse);
            }
            else if((*ev).code == activation_key_codes[0] && recurse ==0 && ((*ev).value == 1 || (*ev).value == 2))
            {    
                printf("\n\n I am done and will now be returning 1 \n");    
                return 1;
            }
        }
    }
}

void keyboard_listener(int* key_details) // returns value of any keyboard event that is pressed
{
    struct input_event ev;
    ssize_t n;
    while(1)
    {
        n = read(fd, &ev, sizeof(ev));
        if(n == (ssize_t)-1)
        {
            if(errno = EINTR)//Read into this
                continue;
            else
                break;
        }
        if (n != sizeof(ev))
        {
            errno = EIO;//Read into this
            continue;
        }
        if((ev.type == EV_KEY)) // 0->release ; 1->keypress ; 2->hold
        {
            key_details[0] = ev.code;
            key_details[1] = ev.value;
            return;
        }
    }
}