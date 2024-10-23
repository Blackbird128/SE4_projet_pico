#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#ifdef DEBUG
#include "serial.h"
#endif

#include "spi.h"
#include "spi_display.h"

int main(void)
{
#ifdef DEBUG
init_printf();
#endif
spi_init();
spi_display_init();
spi_display_cmd1(0x76);
_delay_ms(3000);
spi_display_cmd2(0x7A,255);
_delay_ms(100);
int i;
for(i=0;i<9999;i++){
  spi_display_data('0'+i/1000,'0'+(i/100)%10,'0'+(i/10)%10,'0'+i%10);
  _delay_ms(10);
  }
_delay_ms(1000);
spi_display_cmd1(0x76);
return 0;
}
