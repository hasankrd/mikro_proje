////////////////////////////////////////////////////////////////////////////////////////////////
// NEC Protocol IR remote control decoder with PIC16F887 (CCS C)
// External crystal @ 4MHz (HS)
// Output to LCD (16x2) using lcd.c
////////////////////////////////////////////////////////////////////////////////////////////////

// LCD module connections (CCS lcd.c)
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

short nec_ok = 0;
unsigned int8 nec_state = 0, command, inv_command, i;
unsigned int16 address;
unsigned int32 nec_code = 0;

#INT_EXT
void ext_isr(void){
  unsigned int16 time;

  if(nec_state != 0){
    time = get_timer1();
    set_timer1(0);
  }

  switch(nec_state){

    case 0: // beginning of 9ms LOW
      // 4MHz: Timer1 clock = Fosc/4 = 1MHz => 1 tick = 1us when prescaler 1:1
      setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);
      set_timer1(0);

      nec_state = 1;
      i = 0;
      nec_code = 0;

      ext_int_edge(L_TO_H);
      return;

    case 1: // end of 9ms LOW
      if((time > 9500) || (time < 8500)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
      } else {
        nec_state = 2;
      }
      ext_int_edge(H_TO_L);
      return;

    case 2: // end of 4.5ms HIGH
      if((time > 5000) || (time < 4000)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
        return;
      }
      nec_state = 3;
      ext_int_edge(L_TO_H);
      return;

    case 3: // end of 560us LOW mark
      if((time > 700) || (time < 400)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
      } else {
        nec_state = 4;
      }
      ext_int_edge(H_TO_L);
      return;

    case 4: // end of HIGH space (0:~560us, 1:~1690us)
      if((time > 1800) || (time < 400)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
        return;
      }

      if(time > 1000) bit_set(nec_code, (31 - i));
      else            bit_clear(nec_code, (31 - i));

      i++;

      if(i > 31){
        nec_ok = 1;
        disable_interrupts(INT_EXT);
      }

      nec_state = 3;
      ext_int_edge(L_TO_H);
      return;
  }
}

#INT_TIMER1
void timer1_isr(void){
  nec_state = 0;
  ext_int_edge(H_TO_L);
  setup_timer_1(T1_DISABLED);
  clear_interrupt(INT_TIMER1);
}

void main(){
  setup_adc_ports(NO_ANALOGS);          // analoglarý kapat (RB0 dahil her þey dijital garanti)

  lcd_init();
  lcd_putc('\f');
  lcd_gotoxy(1,1);
  printf(lcd_putc,"NEC IR @4MHz");
  delay_ms(800);

  enable_interrupts(GLOBAL);
  enable_interrupts(INT_EXT_H2L);       // first edge
  clear_interrupt(INT_TIMER1);
  enable_interrupts(INT_TIMER1);

  lcd_putc('\f');
  lcd_gotoxy(1,1);
  printf(lcd_putc,"Address:0x0000");
  lcd_gotoxy(1,2);
  printf(lcd_putc,"Cmd:0x00 In:0x00");

  while(TRUE){
    if(nec_ok){
      nec_ok = 0;
      nec_state = 0;
      setup_timer_1(T1_DISABLED);

      address     = (unsigned int16)(nec_code >> 16);
      command     = (unsigned int8)(nec_code >> 8);
      inv_command = (unsigned int8)(nec_code);

      lcd_gotoxy(11,1);
      printf(lcd_putc,"%4LX", address);

      lcd_gotoxy(5,2);
      printf(lcd_putc,"%2X", command);

      lcd_gotoxy(14,2);
      printf(lcd_putc,"%2X", inv_command);

      // Ýstersen NEC doðrulama (cmd ^ inv == 0xFF) deðilse ekranda uyarý da basabilirsin
      // if((command ^ inv_command) != 0xFF){ lcd_gotoxy(1,1); printf(lcd_putc,"VERIFY FAIL   "); }

      enable_interrupts(INT_EXT_H2L);
    }
  }
}

