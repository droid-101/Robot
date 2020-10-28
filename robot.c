#include <pic.h>

#define LEFT_SENSOR_CHANNEL  2
#define RIGHT_SENSOR_CHANNEL 1

#define RIGHT_MOTOR_FORWARD  RB4
#define RIGHT_MOTOR_REVERSE  RB5
#define LEFT_MOTOR_FORWARD   RB7
#define LEFT_MOTOR_REVERSE   RB6

#define SENSOR_THRESHOLD     50

__CONFIG( FOSC_INTRCIO & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_OFF & IESO_OFF & FCMEN_OFF );

enum Sensor {RIGHT, LEFT};

void init_hardware(void);
int get_sensor(enum Sensor side);
void stop(void);
void forward(void);
void reverse(void);
void turn_left(void);
void turn_right(void);
void swing_right(void);
void swing_left(void);
void reverse_right(void);
void reverse_left(void);
void test(void);
void drive(void);
void count_marker(void);
void leave(void);
void enter(void);
void adjust_position(void);
void scan_barcode(void);

unsigned int left_sensor = 0;
unsigned int right_sensor = 0;
signed char marker_count = 0;
signed char markers_to_destination = 0;
signed char barcode = 0;
signed char width = 0;
unsigned char destination = 0;

signed char barcode_width [5] = {0, 0, 0, 0, 0};

void main(void)
{
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 1;

    init_hardware();

    TRISA = 0b00110110;

    ANSEL = 0b00000110;

    ADCON0 = 0b00000001;
	PORTC = 0;

    while((RA5 == 0))
    {
        left_sensor = 0;
        right_sensor = 0;
        marker_count = 0;
        barcode = 0;

        left_sensor = get_sensor(LEFT);
        right_sensor = get_sensor(RIGHT);

        while (RA5 == 0);
        _delay(2000000);

        // ================ START =============== //

        // leave();

        // markers_to_destination = 2;

        // while (marker_count < markers_to_destination)
        // {
        //    drive();
        //    count_marker();
        // }

        // enter();
        // adjust_position();

        scan_barcode();

        while (RA5 == 0);
    }
}



void scan_barcode(void)
{
    while (barcode_width[1] < 22 && barcode_width[2] < 22 && barcode_width[3] < 22 && barcode_width[4] < 22)
    {
        width = 0;

        if (get_sensor(LEFT) > SENSOR_THRESHOLD)
        {
            forward();

            if (get_sensor(RIGHT) > SENSOR_THRESHOLD)
            {
                while (get_sensor(RIGHT) > SENSOR_THRESHOLD)
                {
                    width++;
                    barcode_width[barcode] = width;
                    _delay(10000);
                }

                barcode++;
                PORTC = barcode;
            }
        }
        else if (get_sensor(LEFT) < SENSOR_THRESHOLD)
        {
            while(get_sensor(LEFT) < SENSOR_THRESHOLD && get_sensor(RIGHT) < SENSOR_THRESHOLD)
            {
                swing_right();
            }
        }
    }

    stop();

    for (int i = 4; i > 0; i--)
    {
        if (barcode_width[i] > 22)
        {
            destination = --i;
        }
    }

    PORTC = destination;

    _delay(2000000);

    while (1)
    {
        while (get_sensor(RIGHT) < SENSOR_THRESHOLD)
        {
            swing_left();
        }

        stop();
        break;
    }

    while (1)
    {
        left_sensor = get_sensor(LEFT);
        right_sensor = get_sensor(RIGHT);

        drive();

        if (left_sensor > SENSOR_THRESHOLD && right_sensor > SENSOR_THRESHOLD)
        {
            stop();
            break;
        }
    }
}

void adjust_position(void)
{
    forward();
    _delay(1000000);

    while (get_sensor(RIGHT) < SENSOR_THRESHOLD)
    {
        turn_right();
    }

    stop();

    while (get_sensor(RIGHT) < SENSOR_THRESHOLD && get_sensor(LEFT) < SENSOR_THRESHOLD)
    {
        forward();
    }

    stop();

    while (get_sensor(LEFT) < SENSOR_THRESHOLD)
    {
        swing_right();
    }

    stop();

    while (get_sensor(RIGHT) < SENSOR_THRESHOLD)
    {
        swing_left();
    }

	stop();
    _delay(1000000);

	while (get_sensor(LEFT) > SENSOR_THRESHOLD)
    {
        reverse_right();
    }

    stop();
	_delay(1000000);

    while (get_sensor(LEFT) < SENSOR_THRESHOLD)
    {
        reverse_left();
    }

	stop();
    _delay(1000000);
}

void enter(void)
{
    while (1)
    {
        left_sensor = get_sensor(LEFT);
        right_sensor = get_sensor(RIGHT);

        drive();

        if (left_sensor > SENSOR_THRESHOLD && right_sensor > SENSOR_THRESHOLD)
        {
            return;
        }
    }

    stop();
}

void leave(void)
{
    while (get_sensor(RIGHT) < SENSOR_THRESHOLD && get_sensor(LEFT) < SENSOR_THRESHOLD)
    {
        forward();
    }

    _delay(500000);

    while (get_sensor(RIGHT) < SENSOR_THRESHOLD)
    {
        turn_right();
    }
}

void count_marker(void)
{
    if (left_sensor > SENSOR_THRESHOLD)
    {
        marker_count++;
        while(get_sensor(LEFT) > SENSOR_THRESHOLD)
        {
            drive();
        }
    }

    PORTC = marker_count;
}

void drive(void)
{
    left_sensor = get_sensor(LEFT);
    right_sensor = get_sensor(RIGHT);

    if (right_sensor > SENSOR_THRESHOLD)
    {
        turn_right();
    }
    else if (right_sensor < SENSOR_THRESHOLD && left_sensor < SENSOR_THRESHOLD)
    {
        forward();
    }
}

void reverse_right(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 1;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 0;
}


void reverse_left(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 1;
}

void swing_right(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 1;
    LEFT_MOTOR_REVERSE = 0;
}

void swing_left(void)
{
    RIGHT_MOTOR_FORWARD = 1;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 0;
}

void turn_right(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 1;
    LEFT_MOTOR_FORWARD = 1;
    LEFT_MOTOR_REVERSE = 0;
}

void turn_left(void)
{
    RIGHT_MOTOR_FORWARD = 1;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 1;
}

void stop(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 0;
}

void forward(void)
{
    RIGHT_MOTOR_FORWARD = 1;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 1;
    LEFT_MOTOR_REVERSE = 0;
}

void reverse(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 1;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 1;
}

void test(void)
{
    // PUT TEST CODE HERE
}

int get_sensor(enum Sensor side)
{
    if (side == RIGHT)
    {
        ADCON0bits.CHS = RIGHT_SENSOR_CHANNEL;
    }
    else if (side == LEFT)
    {
        ADCON0bits.CHS = LEFT_SENSOR_CHANNEL;
    }

    GO_DONE = 1;
    while(GO_DONE);
    return ADRESH;
}

void init_hardware(void)
{
	TRISA = 0b00110000;
	TRISB = 0b00000000;
	TRISC = 0b00000000;

	ANSEL = 0b00000000;
	ANSELH = 0b00000000;

	PORTC = 0b00000000;

    _delay(100000);

    char led = 0b00000001;

    for (int i = 0; i < 8; i++)
    {
        PORTC = PORTC | led;
        led = led << 1;
        _delay(100000);
    }

    _delay(150000);

    for (int i = 0; i < 8; i++)
    {
        PORTC = PORTC << 1;
        _delay(100000);
    }

	for (int i =0; i < 2; i++)
	{
		PORTC = 0b11111111;
		_delay(125000);
		PORTC = 0b00000000;
		_delay(125000);
	}

	_delay(750000);
}
