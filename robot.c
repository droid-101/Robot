#include <pic.h>

#define LEFT_SENSOR_CHANNEL  2
#define RIGHT_SENSOR_CHANNEL 1

#define RIGHT_MOTOR_FORWARD  RB4
#define RIGHT_MOTOR_REVERSE  RB5
#define LEFT_MOTOR_FORWARD   RB7
#define LEFT_MOTOR_REVERSE   RB6

#define LIGHT_THRESHOLD      40

__CONFIG( FOSC_INTRCIO & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_OFF & IESO_OFF & FCMEN_OFF );

enum Sensor {RIGHT, LEFT};
enum Direction {CLOCKWISE, COUNTER_CLOCKWISE};

void init_hardware(void);
int get_sensor(enum Sensor side);
void stop(void);
void forward(void);
void reverse(void);
void turn_left(void);
void turn_right(void);
void test(void);
void drive(enum Direction direction);
void read_marker(enum Direction direction);
int read_barcode(void);
void disable(void);

unsigned int left_sensor_value = 0;
unsigned int right_sensor_value = 0;
unsigned int location_counter = 0;

void main(void)
{
    init_hardware();

    TRISA = 0b00110110;

    ADCON0 = 0b00000001;

    ANSEL = 0b00000110;

    OSCCONbits.IRCF = 111;

    disable();

    while(!(RA4 == 1 && RA5 == 1))
    {
        while (location_counter < 3)
        {
            left_sensor_value = get_sensor(LEFT);
            right_sensor_value = get_sensor(RIGHT);
            drive(COUNTER_CLOCKWISE);

			PORTC = location_counter;
        }

        while (right_sensor_value < LIGHT_THRESHOLD && left_sensor_value < LIGHT_THRESHOLD)
		{
			left_sensor_value = get_sensor(LEFT);
            right_sensor_value = get_sensor(RIGHT);
			drive(COUNTER_CLOCKWISE);
		}
    }
}

int read_barcode(void)
{
	int counter = 0;

	forward();

	if (right_sensor_value > LIGHT_THRESHOLD && left_sensor_value > LIGHT_THRESHOLD)
	{
		counter++;
		_delay(2500000);
	}

	return counter;
}

void disable(void)
{
    stop();
    while (RA5 != 1);
}

void read_marker(enum Direction direction)
{
    if (direction == CLOCKWISE)
    {
        location_counter++;
        stop();
        _delay(1000000);
        turn_right();
        _delay(250000);
    }
    else if (direction == COUNTER_CLOCKWISE)
    {
        location_counter++;
        stop();
        _delay(1000000);
        turn_left();
        _delay(250000);
    }
}

void drive(enum Direction direction)
{
    if (right_sensor_value > LIGHT_THRESHOLD && direction == CLOCKWISE)
    {
        // RC0 = 1;
        // RC1 = 0;
        turn_right();

        if (left_sensor_value > LIGHT_THRESHOLD && location_counter < 3)
        {
            read_marker(direction);
        }
    }
    else if (left_sensor_value > LIGHT_THRESHOLD && direction == COUNTER_CLOCKWISE)
    {
        // RC0 = 0;
        // RC1 = 1;
        turn_left();

        if (right_sensor_value > LIGHT_THRESHOLD && location_counter < 3)
        {
            read_marker(direction);
        }
    }
    else if (right_sensor_value < LIGHT_THRESHOLD && left_sensor_value < LIGHT_THRESHOLD)
    {
        // RC0 = 0;
        // RC1 = 0;
        forward();
    }
    // else if (right_sensor_value > LIGHT_THRESHOLD && left_sensor_value > LIGHT_THRESHOLD)
    // {
    //     RC0 = 1;
    //     RC1 = 1;
    // }
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
