#include <16F887.h>
#device ADC=10
#use delay(crystal=4000000)
#fuses XT, NOWDT, NOPUT, NOPROTECT, NOLVP

// Motor yön pinleri
#define MOTOR_IN1    PIN_C3
#define MOTOR_IN2    PIN_C4
#define MOTOR2_IN3   PIN_C5
#define MOTOR2_IN4   PIN_C6

unsigned long pwm_degeri = 0;

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

void main()
{
   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);

   set_tris_c(0x00);

   // PWM kurulumu
   setup_timer_2(T2_DIV_BY_4, 255, 1);
   setup_ccp1(CCP_PWM);
   setup_ccp2(CCP_PWM);

   motors_stop();

   pwm_degeri = 500; // hýz

   while(TRUE)
   {
      // 3 sn ileri
      motors_forward();
      delay_ms(3000);

      // 1.5 sn dur
      motors_stop();
      delay_ms(1500);

      // 3 sn geri
      motors_reverse();
      delay_ms(3000);

      // 1.5 sn dur
      motors_stop();
      delay_ms(1500);
   }
}

