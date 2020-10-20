#include <pic.h>

#define LEFT_SENSOR_CHANNEL  2
#define RIGHT_SENSOR_CHANNEL 1

#define RIGHT_MOTOR_FORWARD  RB4
#define RIGHT_MOTOR_REVERSE  RB5
#define LEFT_MOTOR_FORWARD   RB7
#define LEFT_MOTOR_REVERSE   RB6

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

void main(void)
{
    init_hardware();
    stop();

    TRISA = 0b00110110;

    ADFM = 1;
    ANSEL = 0b00000110;

    ADCON0 = 0b00000001;

    while(!(RA4 == 1 && RA5 == 1))
    {
        drive();
    }
}

void drive(void)
{
    if (get_sensor(RIGHT) > 100)
    {
        turn_right();
    }
    else if (get_sensor(LEFT) > 100)
    {
        turn_left();
    }
    else if (get_sensor(RIGHT) < 100 && get_sensor(LEFT) < 100)
    {
        forward();
    }
    else
    {
        stop();
    }
}

void turn_right(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 1;
    LEFT_MOTOR_REVERSE = 0;
}

void turn_left(void)
{
    RIGHT_MOTOR_FORWARD = 1;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 0;
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
