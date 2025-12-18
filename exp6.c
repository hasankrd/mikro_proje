#include <16F887.h>
#device ADC=10
#use delay(crystal=4000000) // Kristal hýzýný buraya gir (4MHz varsayýldý)

#fuses XT, NOWDT, NOPUT, NOPROTECT, NOLVP

// Motor Yön Pinleri (PWM deðil, Düz Dijital Çýkýþ)
#define MOTOR_IN1   PIN_C3
#define MOTOR_IN2   PIN_C4
#define MOTOR2_IN3   PIN_C5
#define MOTOR2_IN4   PIN_C6
// Not: Hýz pini (RC1) tanýmlanmadý çünkü onu CCP2 modülü otomatik yönetecek.

// Deðiþkenler
unsigned long pwm_degeri = 0; 

void main()
{
   // --- Giriþ/Çýkýþ Ayarlarý ---
   setup_adc_ports(NO_ANALOGS); 
   setup_adc(ADC_OFF);
   
   set_tris_c(0x00); // Port C Çýkýþ
   
   // Pinleri temizle
   output_low(MOTOR_IN1);
   output_low(MOTOR_IN2);

   output_low(MOTOR2_IN3);
   output_low(MOTOR2_IN4);
   // --- PWM Kurulumu (Sadece CCP2 - RC1 Pini için) ---
   // Timer2 Ayarý: Bölücü 4 yaparak frekansý yaklaþýk 1kHz'e çektik (Motor sesi azalýr)
   setup_timer_2(T2_DIV_BY_4, 255, 1);
   
   setup_ccp1(CCP_PWM);
   setup_ccp2(CCP_PWM); // RC1'de PWM AÇIK (Enable pini buraya baðlý)
   
   set_pwm1_duty(0);
   set_pwm2_duty(0); // Baþlangýç hýzý 0

   while(TRUE)
   {
      
      // --- MOTOR KONTROLÜ ---
      
      // 1. HIZI AYARLA (RC1 - Enable Pini)
      // Duty cycle'ý hesapla ve RC1 pinine (CCP2) gönder
      pwm_degeri = 500;
      set_pwm1_duty(pwm_degeri);
      set_pwm2_duty(pwm_degeri); 
     
      // IN1=1, IN2=0
      output_high(MOTOR_IN1); // RC2 High
      output_low(MOTOR_IN2);  // RC3 Low

      output_high(MOTOR2_IN3); // RC2 High
      output_low(MOTOR2_IN4);  // RC3 Low

      delay_ms(1000);
      
      output_low(MOTOR_IN1); // RC2 High
      output_high(MOTOR_IN2);  // RC3 Low

      output_low(MOTOR2_IN3); // RC2 High
      output_high(MOTOR2_IN4);  // RC3 Low
   }
}
