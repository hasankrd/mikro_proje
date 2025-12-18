#include <16F887.h>
#device ADC=10

// 4MHz kristal
#fuses XT, NOWDT, NOPUT, NOPROTECT, NOLVP
#use delay(crystal=4000000)

// (Ýstersen Proteus Virtual Terminal için aç)
//#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, stream=TERM)

// Motor pinleri
#define MOTOR_IN1    PIN_C3
#define MOTOR_IN2    PIN_C4
#define MOTOR2_IN3   PIN_C5
#define MOTOR2_IN4   PIN_C6   // DÝKKAT: RC6 TX ile çakýþýr; terminal kullanacaksan bu pini deðiþtir!

unsigned long pwm_degeri = 0;

// ---------- NEC IR deðiþkenleri ----------
volatile int1 nec_ok = 0;
volatile unsigned int8 nec_state = 0, command = 0, inv_command = 0, i = 0;
volatile unsigned int16 address = 0;
volatile unsigned int32 nec_code = 0;

// Kumandandan okuyup buraya yazacaksýn:
#define KEY_up   0x62   // ÖRNEK! (senin kumandanda farklý olabilir)
#define KEY_down 0xA8   // ÖRNEK!
#define KEY_right 0xC2
#define KEY_left 0X22
#define KEY_ok 0X02

#INT_EXT
void ext_isr(void){
  unsigned int16 time;

  if(nec_state != 0){
    time = get_timer1();
    set_timer1(0);
  }

  switch(nec_state){

    case 0: // 9ms LOW baþý
      // 4MHz: Fosc/4=1MHz => prescaler 1:1 => 1 tick = 1us
      setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);
      set_timer1(0);

      nec_state = 1;
      i = 0;
      nec_code = 0;

      ext_int_edge(L_TO_H);
      return;

    case 1: // 9ms LOW sonu
      if((time > 9500) || (time < 8500)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
      } else {
        nec_state = 2;
      }
      ext_int_edge(H_TO_L);
      return;

    case 2: // 4.5ms HIGH sonu
      if((time > 5000) || (time < 4000)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
        return;
      }
      nec_state = 3;
      ext_int_edge(L_TO_H);
      return;

    case 3: // 560us LOW mark sonu
      if((time > 700) || (time < 400)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
      } else {
        nec_state = 4;
      }
      ext_int_edge(H_TO_L);
      return;

    case 4: // HIGH space sonu (0/1)
      if((time > 1800) || (time < 400)){
        nec_state = 0;
        setup_timer_1(T1_DISABLED);
        return;
      }

      if(time > 1000) bit_set(nec_code, (31 - i));
      else            bit_clear(nec_code, (31 - i));

      i++;

      if(i > 31){
        // decode bitti
        address     = (unsigned int16)(nec_code >> 16);
        command     = (unsigned int8)(nec_code >> 8);
        inv_command = (unsigned int8)(nec_code);

        // doðrulama
        if((command ^ inv_command) == 0xFF){
          nec_ok = 1;
        }

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

// ---------- Motor fonksiyonlarý ----------
void motors_stop(void){
  output_low(MOTOR_IN1);  output_low(MOTOR_IN2);
  output_low(MOTOR2_IN3); output_low(MOTOR2_IN4);
}

void motors_forward(void){
  // Sol motor ileri
  output_high(MOTOR_IN1); output_low(MOTOR_IN2);
  // Sað motor ileri
  output_high(MOTOR2_IN3); output_low(MOTOR2_IN4);
}

void motors_reverse(void){
  output_low(MOTOR_IN1); output_high(MOTOR_IN2);
  output_low(MOTOR2_IN3); output_high(MOTOR2_IN4);
}

void motors_turn_right(void){
  // Sol ileri, sað geri (pivot sað)
  output_high(MOTOR_IN1);  output_low(MOTOR_IN2);
  output_low(MOTOR2_IN3);  output_high(MOTOR2_IN4);
}

void motors_turn_left(void){
  // Sol geri, sað ileri (pivot sol)
  output_low(MOTOR_IN1);   output_high(MOTOR_IN2);
  output_high(MOTOR2_IN3); output_low(MOTOR2_IN4);
}

void main(){
  setup_adc_ports(NO_ANALOGS);
  setup_adc(ADC_OFF);

  // Port C çýkýþ (RB0 IR için giriþ zaten)
  set_tris_c(0x00);

  // Motorlarý durdur
  output_low(MOTOR_IN1); output_low(MOTOR_IN2);
  output_low(MOTOR2_IN3); output_low(MOTOR2_IN4);

  // PWM
  setup_timer_2(T2_DIV_BY_4, 255, 1);
  setup_ccp1(CCP_PWM);
  setup_ccp2(CCP_PWM);

  pwm_degeri = 500;
  set_pwm1_duty(pwm_degeri);
  set_pwm2_duty(pwm_degeri);

  // IR interrupt'lar
  enable_interrupts(GLOBAL);
  enable_interrupts(INT_EXT_H2L);
  clear_interrupt(INT_TIMER1);
  enable_interrupts(INT_TIMER1);

  while(TRUE){
    if(nec_ok){
      nec_ok = 0;

      // Ýstersen CMD'yi terminale bas (RC6 çakýþmasýna dikkat!)
      //fprintf(TERM,"CMD=0x%02X (%u)\r\n", command, command);

      if(command == KEY_up){
        motors_forward();
      }
      else if(command == KEY_down){
        motors_reverse();
      }
      else if(command == KEY_right){
        motors_turn_right();
      }
      else if(command == KEY_left){
         motors_turn_left();
      }
      else if(command == KEY_ok){
        motors_stop();
      }
      // bir sonraki okuma için reset
      nec_state = 0;
      setup_timer_1(T1_DISABLED);
      enable_interrupts(INT_EXT_H2L);
    }
  }
}

