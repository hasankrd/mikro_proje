#define LCD_RS_PIN      PIN_B1
#define LCD_RW_PIN      PIN_B2
#define LCD_ENABLE_PIN  PIN_B3
#define LCD_DATA4       PIN_B4
#define LCD_DATA5       PIN_B5
#define LCD_DATA6       PIN_B6
#define LCD_DATA7       PIN_B7

#include <16F887.h>
#fuses HS,NOMCLR,NOBROWNOUT,NOLVP,NOWDT,NOPROTECT
#use delay(clock=4000000)

#include <lcd.c>

void main(){
   setup_adc_ports(NO_ANALOGS);          // analoglarý kapat (RB0 dahil her þey dijital garanti)

   lcd_init();
   lcd_putc('\f');

   while(TRUE){
      for(int i = 1; i<10; i++){
         printf(lcd_putc,"Test yazisi %i",i);
         delay_ms(1000);
         lcd_putc('\f');
      }
   }
}
