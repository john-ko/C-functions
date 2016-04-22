#include <stdio.h>
#include <stdlib.h>

struct date {
    unsigned char month;
    unsigned char date;
    unsigned int year;
    char string[16];
};

struct time {
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    char string[16];
};

unsigned char pressed(unsigned char r, unsigned char c)
{
    DDRC = 0x0F;
    PORTC = 0xff;
    SET_BIT(DDRC, r);
    CLR_BIT(PORTC, r);
    wait_avr(1);
    
    if(GET_BIT(PINC, c+4))
    {
        return 0;
    }
    return 1;
}

unsigned char get_key()
{
    unsigned char r,c;
    for(r=0;r<4;r++)
    {
        for(c=0;c<4;c++)
        {
            if(pressed(r,c))
            {
                return (r*4)+c+1;
            }
        }
    }
    return 0;
}

unsigned char parse_key(unsigned char in)
{
    if (in <= 3 && in > 0)
        return in;
    if (in > 4 && in < 8)
        return in-1;
    if (in > 8 && in < 12)
        return in-2;
    if (in == 14)
        return 0;
    if (in == 15)
        return 100;
    return 200;
}

unsigned char get_actual_key() {
    unsigned char key;
    for (;;) {
        key = parse_key(get_get());

        if (key >= 0 && key <= 9)
        {
            return '0' + key;
        }
    }
}

void increment_date(struct date *date) {
    
    date->date++;

    switch(date->month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            // months that have 31 days
            if (date->date >= 31) {
                date->date = 1;
                date->month++;
            }

            if (date->month > 12) {
                date->month = 1;
                date->year++;
            }

        break;
        default:
            if(date->month == 2) {
                //leap year?
                if ((date->year % 4 == 0 && date->date == 30) || (date->year % 4 != 0 && date->date == 29)) {
                    date->date = 1;
                    date->month++;
                }
            } else {
                if (date->date >= 30) {
                    date->date = 1;
                    date->month++;
                }
            }
    }

}

/**
 * increment_time
 * incremnts time
 * returns true if date needs to be incremented
 */
unsigned char increment_time(struct time *time) {
    time->second++;

    if (time->second >= 60) {
        time->second = 0;
        time->minute++;
    }

    if (time->minute >= 60) {
        time->minute = 0;
        time->hour++;
    }

    if (time->hour >= 24) {
        time->hour = 0;
        return 1;
    }

    return 0;
}

void print_time(struct time *time) {
    printf("%d:%d:%d\n", (int)time->hour, (int)time->minute, (int)time->second);
}

void print_date(struct date *date) {
    printf("%d/%d/%d\n", (int)date->month, (int)date->date, (int)date->year);
}

struct time *time_factory() {
    struct time *time = malloc(sizeof(time));
    time->hour = 0;
    time->minute = 0;
    time->second = 0;

    return time;
}

struct date *date_factory() {
    struct date *date = malloc(sizeof(date));
    date->month = 0;
    date->date = 0;
    date->year = 0;

    return date;
}

void set_month(struct date *date) {

    for(;;) {
        month_1 = get_actual_key();
        if (month_1 == 0 || month_1 == 1) {
            // update_screen @ pos 1
            wait_avr(400);
            break;
        }
        wait_avr(100);
    }

    for(;;) {
        month_2 = get_actual_key();
        if (month_1 == 1 && month_2 > 2) {
            // Error
            wait_avr(100);
            continue;
        }
        // update_screen @ pos 2
        wait_avr(400);
        break;
    }

    date->month = month_1 * 10 + month_2;
}

void set_date(struct date *date) {
    unsigned char date_1;
    unsigned char date_2;

    for (;;) {
        date_1 = get_actual_key();
        if (date_1 >= 0 || date_1 <= 3) {
            //update screen
            wait_avr(400);
            break;
        }
        wait_avr(100);
    }

    for (;;) {
        date_2 = get_actual_key();
        if (date_1 == 3 && date_2 != 1) {
            //failed
            wait_avr(100);
            continue;
        }
        // update screen
        wait_avr(400);
        break;
    }

    date->date = date_1 * 10 + date_2;
}

void set_year(struct date *date) {
    unsigned char year[4];

    for (unsigned char i = 0; i < 4; i++) {
        for (;;) {
            year[i] = get_actual_key();
            // update_screen
            wait_avr(400);
            break;
        }
    }

    date->year = year[0] * 1000 + year[1] * 100 + year[2] * 10 + year[3];
}



unsigned char get_time_input(struct time *time) {
    unsigned char input_1;
    unsigned char input_2
    for(;;) {
        input_1 = get_actual_key();
        if (input_1 <= 5 {
            // update_screen_time

            wait_avr(400);
            break;
        }
    }

    input_2 = get_actual_key();
    // update_screen_time

    return input * 10 + input_2;
}

void set_hour(struct time *time) {
    unsigned char hour_1;
    unsigned char hour_2;

    for(;;) {
        hour_1 = get_actual_key();
        if (hour_1 >= 0 && hour_1 <= 2) {
            // update_screen
            wait_avr(400);
            break;
        }
    }

    for (;;) {
        hour_2 = get_actual_key();
        // error
        if (hour_1 == 2 && hour_2 > 3)) {
            continue;
        }

        wait_avr(400);
        // update screen
    }

    time->hour = hour_1 * 10 + hour_2;
}

void set_minute(struct time *time) {
    unsigned char input_1;
    unsigned char input_2
    for(;;) {
        input_1 = get_actual_key();
        if (input_1 <= 5 {
            time->minute 
            // update_screen_time

            wait_avr(400);
            break;
        }
    }

    input_2 = get_actual_key();
    // update_screen_time

    return input * 10 + input_2;
}

void set_second(struct time *time) {
    unsigned char input_1;
    unsigned char input_2
    for(;;) {
        input_1 = get_actual_key();
        if (input_1 <= 5 {
            // update_screen_time

            wait_avr(400);
            break;
        }
    }

    input_2 = get_actual_key();
    // update_screen_time

    return input * 10 + input_2;
}

void set_date_not_used(struct date *date) {
    set_month(date);
    set_date(date);
    set_year(date);
}

int main() {
    struct time *time = malloc(sizeof(time));
    time->hour = 23;
    time->minute = 59;
    time->second = 58;

    

    struct date *date = malloc(sizeof(date));
    date->month = 12;
    date->date = 31;
    date->year = 1999;

    print_date(date);
    print_time(time);
    
    if (increment_time(time)) {
        increment_date(date);
    }
    print_date(date);
    print_time(time);

    if (increment_time(time)) {
        increment_date(date);
    }
    print_date(date);
    print_time(time);


    int key = -1;

    char test = '0' + key;
    printf("[%c]\n", test);
    printf("asdf");
    



    return 0;
}

