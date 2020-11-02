#include <pic.h>

#define LEFT_SENSOR_CHANNEL  2
#define RIGHT_SENSOR_CHANNEL 1

#define RIGHT_MOTOR_FORWARD  RB4
#define RIGHT_MOTOR_REVERSE  RB5
#define LEFT_MOTOR_FORWARD   RB7
#define LEFT_MOTOR_REVERSE   RB6

#define SENSOR_THRESHOLD     50
#define MAX_WIDTH            15
#define ENTER_EXIT_DELAY     750000

__CONFIG( FOSC_INTRCIO & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_OFF & IESO_OFF & FCMEN_OFF );

enum Sensor {RIGHT_SENSOR, LEFT_SENSOR};
enum Direction {RIGHT, LEFT};

void init_hardware(void);
int get_sensor(enum Sensor side);
char black(int reading);
char white(int reading);
void reset_barcode_width(void);
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
void drive_right(void);
void drive_left(void);
void count_marker(enum Direction direction);
void start(void);
void leave(enum Direction direction);
void enter(enum Direction direction);
void adjust_position(void);
void scan_barcode(void);
void go_to_destination(unsigned char destination);
void dock(enum Direction direction);
void go_home(unsigned char destination);


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

    stop();

    while((RA5 == 0))
    {
        left_sensor = 0;
        right_sensor = 0;
        marker_count = 0;
        barcode = 0;
        width = 0;
        destination = 0;
        reset_barcode_width();

        PORTC = 0;

        left_sensor = get_sensor(LEFT_SENSOR);
        right_sensor = get_sensor(RIGHT_SENSOR);

        while (RA5 == 0);
        _delay(2000000);

        // ================ START =============== //

        start();

        enter(RIGHT);
        adjust_position();

        scan_barcode();

        go_to_destination(destination);
        go_home(destination);

        // ================= END ================ //
    }
}



void go_home(unsigned char destination)
{
    switch (destination)
    {
        case 0:
            turn_right();
            while (black(get_sensor(RIGHT_SENSOR)));
            while (white(get_sensor(RIGHT_SENSOR)));

            enter(RIGHT);

            forward();
            _delay(ENTER_EXIT_DELAY);

            while (white(get_sensor(RIGHT_SENSOR)))
            {
                turn_right();
            }

            break;

        case 1:
            turn_left();
            while (black(get_sensor(LEFT_SENSOR)));
            while (white(get_sensor(LEFT_SENSOR)));

            enter(LEFT);

            forward();
            _delay(ENTER_EXIT_DELAY);

            while (white(get_sensor(LEFT_SENSOR)))
            {
                turn_left();
            }

            break;

        case 2:
            turn_right();
            while (black(get_sensor(RIGHT_SENSOR)));
            while (white(get_sensor(RIGHT_SENSOR)));

            marker_count = 0;
            while (marker_count < 1)
            {
                drive_right();
                count_marker(RIGHT);
            }

            enter(RIGHT);

            forward();
            _delay(ENTER_EXIT_DELAY);

            while (white(get_sensor(RIGHT_SENSOR)))
            {
                turn_right();
            }

            break;

        case 3:
            turn_left();
            while (black(get_sensor(LEFT_SENSOR)));
            while (white(get_sensor(LEFT_SENSOR)));

            marker_count = 0;
            while (marker_count < 1)
            {
                drive_left();
                count_marker(LEFT);
            }

            enter(LEFT);

            forward();
            _delay(ENTER_EXIT_DELAY);

            while (white(get_sensor(LEFT_SENSOR)))
            {
                turn_left();
            }

            break;

        default:
            break;
    }

    stop();
}

void dock(enum Direction direction)
{
    if (direction == RIGHT)
    {
        forward();

        _delay(ENTER_EXIT_DELAY);

        while (white(get_sensor(RIGHT_SENSOR)))
        {
            turn_right();
        }

        while (white(get_sensor(LEFT_SENSOR)))
        {
            swing_right();
        }
    }
    else if (direction == LEFT)
    {
        forward();

        _delay(ENTER_EXIT_DELAY);

        while (white(get_sensor(LEFT_SENSOR)))
        {
            turn_left();
        }

        while (white(get_sensor(RIGHT_SENSOR)))
        {
            swing_left();
        }
    }
}

void go_to_destination(unsigned char destination)
{
    markers_to_destination = 0;
    marker_count = 0;

    switch (destination)
    {
        case 0:
            reverse_left();
            while (white(get_sensor(RIGHT_SENSOR)));
            while (black(get_sensor(RIGHT_SENSOR)));
            while (white(get_sensor(RIGHT_SENSOR)));

            stop();

            markers_to_destination = 2;
            while (marker_count < markers_to_destination)
            {
                drive_right();
                count_marker(RIGHT);
            }

            dock(LEFT);
            break;

        case 1:
            reverse_right();
            while (white(get_sensor(LEFT_SENSOR)));
            while (black(get_sensor(LEFT_SENSOR)));
            while (white(get_sensor(LEFT_SENSOR)));

            markers_to_destination = 2;
            while (marker_count < markers_to_destination)
            {
                drive_left();
                count_marker(LEFT);
            }

            dock(RIGHT);
            break;

        case 2:
            reverse_left();
            while (white(get_sensor(RIGHT_SENSOR)));
            while (black(get_sensor(RIGHT_SENSOR)));
            while (white(get_sensor(RIGHT_SENSOR)));

            stop();

            markers_to_destination = 1;
            while (marker_count < markers_to_destination)
            {
                drive_right();
                count_marker(RIGHT);
            }

            dock(LEFT);
            break;

        case 3:
            reverse_right();
            while (white(get_sensor(LEFT_SENSOR)));
            while (black(get_sensor(LEFT_SENSOR)));
            while (white(get_sensor(LEFT_SENSOR)));

            markers_to_destination = 1;
            while (marker_count < markers_to_destination)
            {
                drive_left();
                count_marker(LEFT);
            }

            dock(RIGHT);
            break;

        default:
            break;
    }

    stop();
    _delay(2000000);
}

void scan_barcode(void)
{
    while (barcode_width[1] < MAX_WIDTH && barcode_width[2] < MAX_WIDTH && barcode_width[3] < MAX_WIDTH && barcode_width[4] < MAX_WIDTH)
    {
        width = 0;
        forward();

        if (black(get_sensor(RIGHT_SENSOR)))
        {
            while (black(get_sensor(RIGHT_SENSOR)))
            {
                width++;
                barcode_width[barcode] = width;
                _delay(10000);
            }

            barcode++;
            PORTC = barcode;
        }
    }

    stop();

    for (int i = 4; i > 0; i--)
    {
        if (barcode_width[i] > MAX_WIDTH)
        {
            destination = --i;
        }
    }

    PORTC = destination;

    _delay(2000000);

	for (int i = 0; i < destination + 2; i++)
    {
        while (get_sensor(RIGHT) > SENSOR_THRESHOLD);
        reverse();
        while (get_sensor(RIGHT) < SENSOR_THRESHOLD);
    }
    _delay(250000);

    stop();
}

void adjust_position(void)
{
    forward();
    _delay(ENTER_EXIT_DELAY);

    while (white(get_sensor(RIGHT_SENSOR)))
    {
        turn_right();
    }

    while (get_sensor(RIGHT_SENSOR) < SENSOR_THRESHOLD && get_sensor(LEFT_SENSOR) < SENSOR_THRESHOLD)
    {
        forward();
    }

    while (white(get_sensor(LEFT_SENSOR)))
    {
        swing_right();
    }

    while (white(get_sensor(RIGHT_SENSOR)))
    {
        swing_left();
    }

	stop();
    _delay(1000000);

	while (black(get_sensor(LEFT_SENSOR)))
    {
        reverse_right();
    }

    stop();
	_delay(1000000);

    while (white(get_sensor(LEFT_SENSOR)))
    {
        reverse_left();
    }

	stop();
    _delay(1000000);
}

void enter(enum Direction direction)
{
    if (direction == RIGHT)
    {
        while (1)
        {
            drive_right();

            left_sensor = get_sensor(LEFT_SENSOR);
            right_sensor = get_sensor(RIGHT_SENSOR);

            if (left_sensor > SENSOR_THRESHOLD && right_sensor > SENSOR_THRESHOLD)
            {
                return;
            }
        }

    }
    else if (direction == LEFT)
    {
        while (1)
        {
            drive_left();

            left_sensor = get_sensor(LEFT_SENSOR);
            right_sensor = get_sensor(RIGHT_SENSOR);

            if (left_sensor > SENSOR_THRESHOLD && right_sensor > SENSOR_THRESHOLD)
            {
                return;
            }
        }
    }
}

void leave(enum Direction direction)
{
    forward();
    while (!(black(right_sensor)) && black(get_sensor(left_sensor)))
    {
        right_sensor = get_sensor(RIGHT_SENSOR);
        left_sensor = get_sensor(LEFT_SENSOR);
    }

    _delay(ENTER_EXIT_DELAY);

    if (direction == RIGHT)
    {
        while (white(get_sensor(RIGHT_SENSOR)))
        {
            turn_right();
        }
    }
    else if (direction == LEFT)
    {
        while (white(get_sensor(LEFT_SENSOR)))
        {
            turn_left();
        }
    }
}

void start(void)
{
    forward();
    while (white(get_sensor(LEFT_SENSOR)));
    while (black(get_sensor(LEFT_SENSOR)));

    leave(RIGHT);

    markers_to_destination = 2;

    while (marker_count < markers_to_destination)
    {
        drive_right();
        count_marker(RIGHT);
    }
}

void count_marker(enum Direction direction)
{
    if (direction == RIGHT)
    {
        if (black(get_sensor(LEFT_SENSOR)))
        {
            marker_count++;
            while(black(get_sensor(LEFT_SENSOR)))
            {
                drive_right();
            }
        }
    }
    else if (direction == LEFT)
    {
        if (black(get_sensor(RIGHT_SENSOR)))
        {
            marker_count++;
            while(black(get_sensor(RIGHT_SENSOR)))
            {
                drive_left();
            }
        }
    }

    PORTC = marker_count;
}

void drive_left(void)
{
    left_sensor = get_sensor(LEFT_SENSOR);
    right_sensor = get_sensor(RIGHT_SENSOR);

    if (left_sensor > SENSOR_THRESHOLD)
    {
        turn_left();
    }
    else if (right_sensor < SENSOR_THRESHOLD && left_sensor < SENSOR_THRESHOLD)
    {
        forward();
    }
}

void drive_right(void)
{
    left_sensor = get_sensor(LEFT_SENSOR);
    right_sensor = get_sensor(RIGHT_SENSOR);

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

void reset_barcode_width(void)
{
    for (int i = 0; i < 5; i++)
    {
        barcode_width[i] = 0;
    }
}

char black(int reading)
{
    if (reading > SENSOR_THRESHOLD)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char white(int reading)
{
    if (reading < SENSOR_THRESHOLD)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int get_sensor(enum Sensor side)
{
    if (side == RIGHT_SENSOR)
    {
        ADCON0bits.CHS = RIGHT_SENSOR_CHANNEL;
    }
    else if (side == LEFT_SENSOR)
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
