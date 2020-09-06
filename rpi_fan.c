#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "interface/vmcs_host/vc_vchi_gencmd.h"
#include "interface/vmcs_host/vc_gencmd_defs.h"

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define PIN  24 /* P1-18 */
#define POUT 4  /* P1-07 */

VCHI_INSTANCE_T vchi_instance;
VCHI_CONNECTION_T *vchi_connection = NULL;
int pin;
int temperature;
bool pinOn = false;
int delay_ms;
volatile bool done = false;

static int GPIOExport(int pin)
{
#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int GPIODirection(int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";

#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}

	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

static int GPIOWrite(int pin, int value)
{
#define VALUE_MAX 30
	static const char s_values_str[] = "01";

	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

void sig_term_handler(int signum)
{
   done = true;
}

int initialize()
{
   int ret;
   struct sigaction *action;

   vcos_init();

   if ( vchi_initialise( &vchi_instance ) != 0)
   {
      fprintf( stderr, "VCHI initialization failed\n" );
      return -1;
   }

   //create a vchi connection
   if ( vchi_connect( NULL, 0, vchi_instance ) != 0)
   {
      fprintf( stderr, "VCHI connection failed\n" );
      return -1;
   }

   vc_vchi_gencmd_init(vchi_instance, &vchi_connection, 1 );

   if (GPIOExport(pin) != 0)
   {
      return -1;
   }

   if (GPIODirection(pin, OUT) != 0)
   {
      return -1;
   }

   action = (struct sigaction *)calloc(1, sizeof(struct sigaction));
   action->sa_handler = sig_term_handler;
   sigaction(SIGTERM, action, NULL);

   action = (struct sigaction *)calloc(1, sizeof(struct sigaction));
   action->sa_handler = sig_term_handler;
   sigaction(SIGINT, action, NULL);

   setpriority(PRIO_PROCESS, 0, 19);

   return 0;
}

void shutdown()
{
   vc_gencmd_stop();
   vchi_disconnect( vchi_instance );
   
   GPIOWrite(pin, LOW);
   GPIOUnexport(pin);
}

int get_temperature()
{
   char buffer[ GENCMDSERVICE_MSGFIFO_SIZE ];
   int ret = -1;

   if (( ret = vc_gencmd( buffer, GENCMDSERVICE_MSGFIFO_SIZE, "%s", "measure_temp" )) == 0 )
   {
      char *decimal_char;

      decimal_char = rindex(buffer, '\'');
      if (!decimal_char)
      {
         return -1;
      }

      decimal_char = '\0';
      return atoi(buffer + 5);
   }

   return -1;
}

void enable_fan()
{
   GPIOWrite(pin, HIGH);
   pinOn = true;
}

void disable_fan()
{
   GPIOWrite(pin, LOW);
   pinOn = false;
}

int main( int argc, char **argv )
{
   int ret;
   int current_temp;

   if (argc == 4)
   {
      pin = atoi(argv[1]);
      if (pin < 0 || pin > 25)
      {
         return -1;
      }

      temperature = atoi(argv[2]);
      if (temperature < 30 || temperature > 70)
      {
         return -1;
      }

      delay_ms = atoi(argv[3]);
      if (delay_ms < 1)
      {
         return -1;
      }
   } else
   {
      fprintf(stderr, "Usage: rpi-fan PIN TEMP DELAY\n\n");
      fprintf(stderr, "PIN: GPIO pin to use\n");
      fprintf(stderr, "TEMP: CPU target temperature in Celsius (30 to 70)\n");
      fprintf(stderr, "DELAY: Time between temperature checks in ms\n");
      return -1;
   }

   ret = initialize();
   if (ret)
   {
      return ret;
   }

   while (!done)
   {
      current_temp = get_temperature();
      if (current_temp >= temperature && !pinOn)
      {
         enable_fan();
      } else if (current_temp < temperature && pinOn)
      {
         disable_fan();
      }

      usleep(100);
   }

   shutdown();

   return 0;
}

