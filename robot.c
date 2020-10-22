#include <pic.h>

#define LEFT_SENSOR_CHANNEL  2
#define RIGHT_SENSOR_CHANNEL 1

#define RIGHT_MOTOR_FORWARD  RB4
#define RIGHT_MOTOR_REVERSE  RB5
#define LEFT_MOTOR_FORWARD   RB7
#define LEFT_MOTOR_REVERSE   RB6

#define SENSOR_THRESHOLD     50
#define STARTING_MARKERS     2

__CONFIG( FOSC_INTRCIO & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_OFF & IESO_OFF & FCMEN_OFF );

enum Sensor {RIGHT, LEFT};

void init_hardware(void);
int get_sensor(enum Sensor side);
void stop(void);
void forward(void);
void reverse(void);
void turn_left(void);
void turn_right(void);
void test(void);
void drive(void);
void count_marker(void);
void start(void);

unsigned int left_sensor = 0;
unsigned int right_sensor = 0;
signed char marker_count = 0;
signed char markers_to_destination = 0;

void main(void)
{
    init_hardware();

    TRISA = 0b00110110;

    ANSEL = 0b00000110;

    ADCON0 = 0b00000001;

    while((RA5 == 0))
    {
        left_sensor = 0;
        right_sensor = 0;
        marker_count = 0;

        left_sensor = get_sensor(LEFT);
        right_sensor = get_sensor(RIGHT);

        while (RA5 == 0);
        _delay(1000000);

        start();

        markers_to_destination = 4;

        while (marker_count < markers_to_destination)
        {
            drive();
            count_marker();
        }

        stop();
        PORTC = 0;
    }
}

void start(void)
{
    while (marker_count < STARTING_MARKERS)
    {
        drive();
        count_marker();
    }

    marker_count = 0;
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
