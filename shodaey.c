#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<signal.h>
#include<linux/input.h>//structure definition of input_event is here
#include<unistd.h>//read()
#include<fcntl.h>//has open() and O_RDONLY macro
#include<errno.h>//for errno and EINTR. Look into these and what these mean

typedef struct activation_keys_status
{
    int act_key_code[3];
    int positions[84];
    int run_script;
}ACT_KEY;

ACT_KEY* act_key;
pthread_mutex_t act_key_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

int kbd_fd; // global variable file descriptor for kbd_path

void* key_detect();//function containing only loop part to make recursion possible
void* check_activate();
int init(int, int);
void keyboard_listener(int* key_details);
int alter_step(char *script_path, float max, float min, float step);
int script_runner();
void signal_handlers();
void sigint_handler();
void sigtstp_handler();

int main()
{
    signal_handlers();
    act_key = (ACT_KEY*)malloc(sizeof(ACT_KEY));
    if(!init(42,54))
    {
        printf("Initialization of required variables failed!\n");
        exit(0);
    }
    pthread_t thread_k1;
    pthread_t thread_k2;
    pthread_create(&thread_k1, NULL, key_detect, NULL);
    pthread_create(&thread_k2, NULL, check_activate, NULL);
    printf("dekho dekho, hai shaam badi deewani\n");
    pthread_join(thread_k1,NULL);
    pthread_join(thread_k2,NULL);
    free(act_key);
    close(kbd_fd);
    return 1;
}

int init(int k1, int k2)
{
    const char *kbd_path = "/dev/input/event2";
    kbd_fd = open(kbd_path,O_RDONLY);
    if(kbd_fd == -1)
    {
        printf("Failed to open required files. Terminating.\n");
        return 0;
    }

    act_key->act_key_code[0] = k1;
    act_key->act_key_code[1] = k2;
    for(int i = 0; i < 84; i ++)
        act_key->positions[i] = 0;
    act_key->run_script = 0;

    return 1;
}

void* key_detect()
{
    
    struct input_event kbd_ev;
    ssize_t read_return;
    while(1)
    {
        read_return = read(kbd_fd, &kbd_ev, sizeof(struct input_event));
        if(read_return == (ssize_t)-1)
        {
            if(errno = EINTR)
                continue;
            else
                break;
        }
        if (read_return != sizeof(struct input_event))
        {
            continue;
        }
        if((kbd_ev.type == EV_KEY) && (kbd_ev.value <=2 && kbd_ev.value >=0))
        {
            if(kbd_ev.value == 1 || kbd_ev.value ==2)
            {
                act_key->positions[kbd_ev.code] = 1;
            }
            else if(kbd_ev.value == 0)
            {
                act_key->positions[kbd_ev.code] = 0;
            }
        }
        if((act_key->positions[act_key->act_key_code[0]] == 1 && act_key->positions[act_key->act_key_code[1]] == 1))
        {
            pthread_mutex_lock(&act_key_lock);
            act_key->run_script = 1;
            pthread_mutex_unlock(&act_key_lock);
            pthread_cond_wait(&condition,&wait_lock);
            act_key->run_script = 0;
        }
    }   
}

void keyboard_listener(int* key_details) // returns value of any keyboard event that is pressed
{
    struct input_event kbd_ev;
    ssize_t read_return;
    while(1)
    {
        read_return = read(kbd_fd, &kbd_ev, sizeof(kbd_ev));
        if(read_return == (ssize_t)-1)
        {
            if(errno = EINTR)
                continue;
            else
                break;
        }
        if (read_return != sizeof(kbd_ev))
        {
            errno = EIO;
            continue;
        }
        if((kbd_ev.type == EV_KEY)) // 0->release ; 1->keypress ; 2->hold
        {
            key_details[0] = kbd_ev.code;
            key_details[1] = kbd_ev.value;
            return;
        }
    }
}

void* check_activate()
{
    int ch;
    while(1)
    {
        if(act_key->run_script == 1)
        {
            printf("Im here nice\n"); 
            ch = script_runner();
            if(ch)
            {
                pthread_mutex_lock(&act_key_lock);
                act_key->run_script = 0;
                act_key->positions[act_key->act_key_code[0]] = 0;
                act_key->positions[act_key->act_key_code[1]] = 0;
                pthread_cond_signal(&condition);
                pthread_mutex_unlock(&act_key_lock);
            }
        }
    }
}

int script_runner()
{
    printf("Entered Script Runner\n");
    system("./scripts/ledcont.sh 1");
    int key_details[2];
    while(1)
    {
        keyboard_listener(key_details);
        switch(key_details[0])
        {
            case 48: // 48 is code for B
                printf("Brightness control activated\n");
                if(alter_step("./scripts/brightc.sh %0.2f",1.5,0.0,0.05))
                {
                    return 48;
                }
                return 0;
            break;
            //add on more codes here

            case 1: // 1 is code for esc
                system("./scripts/ledcont.sh 0");
                return -1;
            break;
            default:
                system("./scripts/ledcont.sh 0");
                continue;
            
        }
    }

}

int alter_step(char *script_path, float max, float min, float step)
{
    printf("Entered alter_step\n");
    int key_details[2];
    float current_value = 1.0;
    char buffer[50];
    do
    {
        keyboard_listener(key_details);
        if(key_details[0] == 24 && key_details[1] > 0 && current_value > min) // 24 is code for o
        {
            current_value -= step;
        }
        else if(key_details[0] == 25 && key_details[1] > 0 && current_value < max) // 25 is code for p
        {
            current_value += step;
        }
        sprintf(buffer, script_path, current_value);
        system(buffer); // runs the actual command in terminal
        if(key_details[0] == 1 && key_details[1] > 0 ) // 1 is code for esc
        {
            printf("Exiting alter_step\n");
            system("./scripts/ledcont.sh 0");
            return 1;
        }  
    }
    while(1);
    return 0;
}

void signal_handlers()
{
    signal(SIGINT,sigint_handler);
    signal(SIGTSTP,sigtstp_handler);
}

void sigint_handler()
{
    signal(SIGINT,sigint_handler);
    printf("SIGINT detected, ignoring...\n");
    return;
}

void sigtstp_handler()
{
    printf("SIGTSTP detected, cleaning and exiting\n");
    free(act_key);
    close(kbd_fd);
    exit(0);   
}