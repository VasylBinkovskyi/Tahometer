#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Servo.h>
Servo Servostrelka;

volatile  float timer1,timer_RPM;
int n=2;//кількість лопатей енкодера
int RPM, FiltrRPM;//змінна обертів
int servoRPM;//змінна обертів для серви
int otsechka;//ззмінна в яку записуємо відсічку
int in_taho=3;//вхід для приривання з датчика обертів
int in_pin=7;// вход кнопки
int otsechka_pin=4;//пін на який виводиться HIGH при відсічці
int red_led = 8;
int ledFlag=0;
int outRPM=0;
int in_ign=9,ign_flag=0; //вход включение зажигания
int grin_led = 10; 
float timer_strelka = 0;
float redLedTimer = 0,filtrTimer=0;
bool on=1, off=0, knop, ign, a=0, q=0,oil;//допоміжні змінні і флаги
unsigned long knopkatimer;//таймер для скидання кнопки


//########################################################################################################
int GetFiltrRPM(int RPM,int FiltrRPM )
{
  
if(RPM>0&&RPM<10000&&RPM>FiltrRPM)
{
  FiltrRPM += 10;
}
else if(RPM>0 && RPM < 10000 &&RPM<FiltrRPM)
{
  FiltrRPM -= 10;
} 
 return FiltrRPM;
}

void BlinkLed()
{
 if(q==1&&ign_flag==2)
 {
  if(millis()- redLedTimer>300)
  {
    ledFlag=ledFlag==0?ledFlag=1:0;
    digitalWrite(red_led, ledFlag);
    redLedTimer=millis();
  }
   digitalWrite(grin_led, 0);
 }
else
{
  if(FiltrRPM>100&&FiltrRPM<4000)
  {
    digitalWrite(grin_led, 1);
    digitalWrite(red_led, 0);
  }
  else if (FiltrRPM>4000)
  {
    digitalWrite(red_led, 1);
    digitalWrite(grin_led, 0);
  }
  else
  {
    digitalWrite(red_led, 0);
    digitalWrite(grin_led, 0);
  }
}  
}

void taho(){
 timer_RPM= micros()-timer1;
 timer1=micros();
  }

//=================================================================================
void setup() {
//Serial.begin(9600);  
Servostrelka.attach(5); //підключаємо до 5 піна сервомотор
attachInterrupt(1, taho, RISING);
pinMode(in_taho,INPUT_PULLUP);//pin3 вхід тахометра
pinMode(in_pin,INPUT_PULLUP);//pin7 кнопка вибора режимів
pinMode(otsechka_pin,OUTPUT);//pin4 отсечка по оборотам
pinMode(grin_led, OUTPUT);
pinMode(red_led, OUTPUT);
pinMode(in_ign,INPUT_PULLUP);//pin9 вход включение зажигания
q=EEPROM.get(0,q);
otsechka=EEPROM.get(10,otsechka);
}
//=================================================================================


//#################################################################################
void loop() {

BlinkLed();
//тест стрілки......................................................................
ign = digitalRead(in_ign);
if (ign == 0&& ign_flag==0){
timer_strelka=millis();
Servostrelka.write(0);
ign_flag=1;
RPM=0;
}

if(ign == 0&& ign_flag==1&&millis()-timer_strelka>1000){
ign_flag=2;
Servostrelka.write(180);
}
if(ign==1){
ign_flag=0;
timer_RPM=0;
Servostrelka.write(180);
}

//-----------------------------------------------------------------------------------
//вираховування і виведення обертів...................................................
//Serial.print("OBOROTY=                                       ");
//Serial.println(RPM);
//Serial.print("timer_RPM =   ");
//Serial.println(timer_RPM);

  
if(ign_flag==2&&micros()-timer1<1000000.0)
{
 RPM= (60/(timer_RPM/1000000))/n;

   if(millis()-filtrTimer>5)
   {
    FiltrRPM= GetFiltrRPM(RPM,FiltrRPM);
    filtrTimer=millis();
   }

   if(FiltrRPM>500&&FiltrRPM<5900)
   {
    servoRPM = map(FiltrRPM, 500,5900, 180,0);
    Servostrelka.write(servoRPM);
   } 
}
else if(ign_flag==2)
{
    Servostrelka.write(180);
    RPM=0;
    FiltrRPM=0;
}

if(ign==1){
RPM=0;
FiltrRPM=0;
Servostrelka.write(180);
}
//-----------------------------------------------------------------------------------
//відсічка по обертам ...........................................................
if (q==1&&FiltrRPM >otsechka){digitalWrite(otsechka_pin,HIGH);}
else{digitalWrite(otsechka_pin,LOW);}
//------------------------------------------------------------------------------------
//кнопка.............................................................................
//виставляєм відсічку
knop = digitalRead(in_pin);
if(knop==0&&a==0){a=1;}
if(knop==1&&a==1){
otsechka=RPM;
 a=0;
 q=1;
  EEPROM.put(0,q);
   EEPROM.put(10,otsechka);
 }
//скидаєм відсічку
if(a==0){knopkatimer=millis();}
if(knop==0&&((millis()-knopkatimer)>2000)){
a=0;
q=0;
EEPROM.put(0,q);

}//-------------------------------------------------------------------------------------

}
