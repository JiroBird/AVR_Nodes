#define F_CPU 8000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

void W0()
{
    PORTD = 0b00100000;
    _delay_us(0.30-0.2);
    PORTD = 0b00000000;
    _delay_us(0.80-0.2);
}

void W1()
{
    PORTD = 0b00100000;
    _delay_us(0.80-0.2);
    PORTD = 0b00000000;
    _delay_us(0.30-0.2);
}

uint8_t bright = 0;

int main ()
{
   DDRD  = 0b00100000;
   PORTD = 0b00000000;

   _delay_ms(1);

   for (uint8_t ledID = 0; ledID < 100; ++ledID)
      {   
          for (uint8_t id = 0; id < 24; ++id)
          {   
              W0();
          }
      }

      _delay_ms(1000);


   bright = 1;

   while (1)
   {
//      for (int s = 0; s < 1000; s++)
//     {
      for (int ledID = 0; ledID < 20; ++ledID)
      {
          for (int id = 0; id < 3; ++id)
          {  
              if (bright & 128) {W1();}else{W0();}
              if (bright & 64) {W1();}else{W0();}
              if (bright & 32) {W1();}else{W0();}
              if (bright & 16) {W1();}else{W0();}
              if (bright & 8) {W1();}else{W0();}
              if (bright & 4) {W1();}else{W0();}
              if (bright & 2) {W1();}else{W0();}
              if (bright & 1) {W1();}else{W0();}
          }
      }
      _delay_ms(1000);
//    }
//   bright++;
//      if (bright > 4){ bright = 0;}

 for (uint8_t ledID = 0; ledID < 100; ++ledID)
      {
          for (uint8_t id = 0; id < 24; ++id)
          {
              W0();
          }
      }
_delay_ms(100);
/* for (uint8_t ledID = 0; ledID < 100; ++ledID)
      {   
          for (uint8_t id = 0; id < 24; ++id)
          {   
              W0();
          }
      }
_delay_us(650);*/

   }
}
