/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Program: robot.c
Authour: Shane Pinto
Date: Monday, November 2, 2020
Description: Runs a routine to make
a robot complete a playing field.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <pic.h>

// HARDWARE INPUT AND OUTPUT PIN DESIGNATIONS //
#define LEFT_SENSOR_CHANNEL  2
#define RIGHT_SENSOR_CHANNEL 1

#define RIGHT_MOTOR_FORWARD  RB4
#define RIGHT_MOTOR_REVERSE  RB5
#define LEFT_MOTOR_FORWARD   RB7
#define LEFT_MOTOR_REVERSE   RB6

// VALUES DEPENDENT ON BATTERY CHARGE AND SPEED //
#define SENSOR_THRESHOLD     50
#define MAX_WIDTH            15
#define ENTER_EXIT_DELAY     750000

__CONFIG( FOSC_INTRCIO & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF & CPD_OFF & BOREN_OFF & IESO_OFF & FCMEN_OFF );

enum Sensor {RIGHT_SENSOR, LEFT_SENSOR}; // Arguments that will determine which sensor is read.
enum Direction {RIGHT, LEFT};            // Arguments that will determine which direction the robot is travelling around the field.

// INITIALIZATION FUNCTIONS //
void init_hardware(void);
void reset_barcode_width(void);

// SENSOR COMPUTING FUNCTIONS //
int get_sensor(enum Sensor side);
char black(int reading);
char white(int reading);

// MOVEMENT FUNCTIONS //
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

// ROUTINE FUNCTIONS //
void count_marker(enum Direction direction);
void start(void);
void leave(enum Direction direction);
void enter(enum Direction direction);
void adjust_position(void);
void scan_barcode(void);
void go_to_destination(unsigned char destination);
void dock(enum Direction direction);
void go_home(unsigned char destination);

// VARIABLE DECLARTIONS //
unsigned int left_sensor = 0;                    // Stores the value of the last reading from the left light sensor.
unsigned int right_sensor = 0;                   // Stores the value of the last reading from the right light sensor.
signed char marker_count = 0;                    // Keeps track of how many markers or sections have been passed.
signed char markers_to_destination = 0;          // Determines how many markers the robot must pass to reach its destination.
signed char barcode = 0;                         // Keeps track of how many barcode lines have been read.
signed char width = 0;                           // Stores the width of a line before it is sent to the array for storage.
unsigned char destination = 0;                   // Stores the destination in which the robot must travel to.
signed char barcode_width [5] = {0, 0, 0, 0, 0}; // Stores the widths of the lines read.


// ========================= MAIN ========================= //

void main(void)
{
    OSCCONbits.IRCF = 0b111; // Set clock speed to 8MHz.
    OSCCONbits.SCS = 1;      // Use internal oscillator for system clock.

    init_hardware();

    TRISA = 0b00110110; // Set pins AN1 and AN2 on the A register to inputs for the sensors.

    ANSEL = 0b00000110;  // Set pins AN1 and AN2 to analogue inputs.
    ADCON0 = 0b00000001; // Turn on the ADC.

    stop();

    while((RA5 == 0))
    {
        // ======= PROGRAM INITIALIZATION =======//
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



// ========================= METHODS ========================= //


/* ================================
Function: go_home
Paramaters: unsigned char destination
return type: none
Description: Drives robot back to
start from current location.
================================ */
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

/* ================================
Function: dock
Paramaters: enum Direction direction
return type: none
Description: Instructions for the
robot to turn into the destination.
================================ */
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

/* ================================
Function: go_to_destination
Paramaters: unsigned char destination
return type: none
Description: Robot will drive to
the destination read and will dock
into the section.
================================ */
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

/* ================================
Function: scan_barcode
Paramaters: none
return type: none
Description: Moves robot over
barcode, calculates the correct
destination, then exits the section.
================================ */
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
        while (black(get_sensor(RIGHT)));
        reverse();
        while (white(get_sensor(RIGHT)));
    }
    _delay(250000);

    stop();
}

/* ================================
Function: adjust_position
Paramaters: none
return type: none
Description: Lines robot up to
read barcode.
================================ */
void adjust_position(void)
{
    forward();
    _delay(ENTER_EXIT_DELAY);

    while (white(get_sensor(RIGHT_SENSOR)))
    {
        turn_right();
    }

    while (white(get_sensor(RIGHT_SENSOR)) && white(get_sensor(LEFT_SENSOR)))
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

/* ================================
Function: enter
Paramaters: enum Direction direction
return type: none
Description: Moves robot into
a section either from the right or
left depending on the argument given.
================================ */
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

/* ================================
Function: leave
Paramaters: enum Direction direction
return type: none
Description: Moves robot out of
starting section either right or
left depending on the argument given.
================================ */
void leave(enum Direction direction)
{
    forward();

    right_sensor = get_sensor(RIGHT_SENSOR);
    left_sensor = get_sensor(LEFT_SENSOR);
    while (white(right_sensor) && white(left_sensor))
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

/* ================================
Function: start
Paramaters: none
return type: none
Description: Drives robot to the
entrance of the barcode.
================================ */
void start(void)
{
    forward();
    while (white(get_sensor(RIGHT_SENSOR)));
    while (black(get_sensor(RIGHT_SENSOR)));

    leave(RIGHT);

    markers_to_destination = 2;

    while (marker_count < markers_to_destination)
    {
        drive_right();
        count_marker(RIGHT);
    }
}

/* ================================
Function: counter_marker
Paramaters: enum Direction direction
return type: none
Description: Counts the number of
parking spots on the outside of the
playing field.
================================ */
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
}

/* ================================
Function: drive_left
Paramaters: none
return type: none
Description: Makes the robot follow
a line moving counterclockwise.
================================ */
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

/* ================================
Function: drive_right
Paramaters: none
return type: none
Description: Makes the robot follow
a line moving clockwise.
================================ */
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

/* ================================
Function: reverse_right
Paramaters: none
return type: none
Description: Moves right motor
backwards.
================================ */
void reverse_right(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 1;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 0;
}

/* ================================
Function: reverse_left
Paramaters: none
return type: none
Description: Moves left motor
backwards.
================================ */
void reverse_left(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 1;
}

/* ================================
Function: swing_right
Paramaters: none
return type: none
Description: Moves left motor
forward.
================================ */
void swing_right(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 1;
    LEFT_MOTOR_REVERSE = 0;
}

/* ================================
Function: swing_left
Paramaters: none
return type: none
Description: Moves right motor
forward.
================================ */
void swing_left(void)
{
    RIGHT_MOTOR_FORWARD = 1;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 0;
}

/* ================================
Function: turn_right
Paramaters: none
return type: none
Description: Moves left motor
forward and right motor backwards.
================================ */
void turn_right(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 1;
    LEFT_MOTOR_FORWARD = 1;
    LEFT_MOTOR_REVERSE = 0;
}

/* ================================
Function: turn_left
Paramaters: none
return type: none
Description: Moves right motor
forward and left motor backwards.
================================ */
void turn_left(void)
{
    RIGHT_MOTOR_FORWARD = 1;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 1;
}

/* ================================
Function: stop
Paramaters: none
return type: none
Description: Stops both motors.
================================ */
void stop(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 0;
}

/* ================================
Function: forward
Paramaters: none
return type: none
Description: Moves motors forward.
================================ */
void forward(void)
{
    RIGHT_MOTOR_FORWARD = 1;
    RIGHT_MOTOR_REVERSE = 0;
    LEFT_MOTOR_FORWARD = 1;
    LEFT_MOTOR_REVERSE = 0;
}

/* ================================
Function: reverse
Paramaters: none
return type: none
Description: Reverses both motors.
================================ */
void reverse(void)
{
    RIGHT_MOTOR_FORWARD = 0;
    RIGHT_MOTOR_REVERSE = 1;
    LEFT_MOTOR_FORWARD = 0;
    LEFT_MOTOR_REVERSE = 1;
}

/* ================================
Function: test
Paramaters: none
return type: none
Description: Test function for
testing new code before implementing.
================================ */
void test(void)
{
    // PUT TEST CODE HERE
}

/* ================================
Function: reset_barcode_width
Paramaters: none
return type: none
Description: Initialization function
that clears the array that stores
the width of the barcode lines.
================================ */
void reset_barcode_width(void)
{
    for (int i = 0; i < 5; i++)
    {
        barcode_width[i] = 0;
    }
}

/* ================================
Function: black
Paramaters: int reading
return type: char
Description: Returns 1 if sensor
reads black.
================================ */
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


/* ================================
Function: white
Paramaters: int reading
return type: char
Description: Returns 1 if sensor
reads white.
================================ */
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

/* ================================
Function: get_sensor
Paramaters: enum Sensor side
return type: int
Description: Reads sensor of the
side given and returns its value.
================================ */
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


/* ================================
Function: init_hardware
Paramaters: none
return type: none
Description: Initializes PIC
hardware for the PIC Testboard.
================================ */
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
