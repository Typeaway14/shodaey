#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<linux/input.h>//structure definition of input_event is here
#include<unistd.h>//read()
#include<fcntl.h>//has open() and O_RDONLY macro
#include<errno.h>//for errno and EINTR. Look into these and what these mean

typedef struct activation_keys_status
{
    int act_key_code[3];
    int positions[84];
    int esc; //always set to 0, but if esc key is pressed, it is set to one and the programs resets
}ACT_KEY;

ACT_KEY* act_key;

int fd; // global variable file descriptor for kbd_path

void* key_detect(void *array_position);//function containing only loop part to make recursion possible
void* check_activate(void* unused_variable);
int init(int, int); // last one is the key that decides which alphabet will trigger which script
void keyboard_listener(int* key_details);
void alter_step(char *script_path, float max, float min, float step);
int script_runner();

int main()
{
    act_key = (ACT_KEY*)malloc(sizeof(ACT_KEY));
    if(!init(42,54))
    {
        printf("Initialization of required variables failed!\n");
        exit(0);
    }
    pthread_t thread_k1;
    pthread_t thread_k4;
    pthread_create(&thread_k1, NULL, key_detect, NULL);
    pthread_create(&thread_k4, NULL, check_activate, NULL);
    printf("dekho dekho, hai shaam badi deewani\n");
    pthread_join(thread_k1,NULL);
    pthread_join(thread_k4,NULL);
    close(fd);
    return 1;
}

int init(int k1, int k2)
{
    const char *kbd_path = "/dev/input/event2";
    fd = open(kbd_path,O_RDONLY);
    if(fd == -1)
    {
        printf("Failed to open required files. Terminating.\n");
        return 0;
    }

    act_key->act_key_code[0] = k1;
    act_key->act_key_code[1] = k2;
    for(int i = 0; i < 84; i ++)
        act_key->positions[0] = 0;
    act_key->esc = 0;

    return 1;
}

void* key_detect(void* unused_variable)
{
    struct input_event ev;
    ssize_t read_return;
    while(1)
    {
        read_return = read(fd, &ev, sizeof(struct input_event));
        if(read_return == (ssize_t)-1)
        {
            if(errno = EINTR)//Read into this
                continue;
            else
                break;
        }
        if (read_return != sizeof(struct input_event))
        {
            continue;
        }
        if((ev.type == EV_KEY) && (ev.value <=2 && ev.value >=0))
        {
            if(ev.value == 1 || ev.value ==2)
            {
                act_key->positions[ev.code] = 1;
            }
            else if(ev.value == 0)
            {
                act_key->positions[ev.code] = 0;
            }
        }
    }
}

void keyboard_listener(int* key_details) // returns value of any keyboard event that is pressed
{
    struct input_event ev;
    ssize_t read_return;
    while(1)
    {
        read_return = read(fd, &ev, sizeof(ev));
        if(read_return == (ssize_t)-1)
        {
            if(errno = EINTR)//Read into this
                continue;
            else
                break;
        }
        if (read_return != sizeof(ev))
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

void* check_activate(void* unused_variable)
{
    uint i = 1;
    while(1)
    {
        if((act_key->positions[act_key->act_key_code[0]] == 1 && act_key->positions[act_key->act_key_code[1]] == 1))
        {
            printf("Im here nice\n");
            script_runner();
        }
        i++;
    }
}

int script_runner()
{
    printf("Entered Script Runner\n");
    char buffer[50];
    float bright_value = 1;
    int key_details[2];
    keyboard_listener(key_details);
    while(1)
    {
        switch(key_details[0])
        {
            case 48:
                printf("Brightness control activated\n");
                alter_step("./scripts/brightc.sh %0.2f",1.0,0.2,0.05);
                return 48;
            break;
            //add on more codes here

            case 1:
                return -1;
            break;
            default:
                return 0;
            
        }
    }

}

void alter_step(char *script_path, float max, float min, float step)
{
    printf("Entered alter_step\n");
    int key_details[2];
    float current_value = max;
    char buffer[50];
    do
    {
        keyboard_listener(key_details);
        if(key_details[0] == 24 && key_details[1] > 0 && current_value > min) // 24 is code for o
        {
            current_value -= step;
        }
        else if(key_details[0] == 25 && key_details[1] > 0 && current_value != max) // 25 is code for p
        {
            current_value += step;
        }
        else if(key_details[0] == 1 && key_details[1] == 0) // 1 is code for esc
        {
            printf("Exiting alter_step\n");
            return;
        }
        else
        {
            continue;
        }
        sprintf(buffer, script_path, current_value);
        system(buffer); // runs the actual command in terminal
    }
    while(1);
}