#include <OneWire.h>
#include <Wire.h>
#include <MsTimer2.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
//класс обработки сигналов кнопок__________________________________________________________________________________________________________
class Datchik{
   public:
     bool flagPress=0; //признак нажатия
     bool flagClick=0; //признак принятого решения о стабильном состоянии
     void scanState();//метод проверки состояния сигнала
     Datchik(byte pin, int timeButton); //конструктор, установка номера вывода
                                                 // и времени подтверждения
   private:
     int _count=1; //счетчик подтверждения стабильного состояния
     int _timeButton=0; //время подтверждения сотояния кнопки/датчика
     byte _pin=0; //номер вывода кнопки/датчика
};
//реализация методов класса_______________________________________________________________________________________________________________

//метод проверки нажатия кнопки
void Datchik::scanState(){
      flagPress=digitalRead(_pin);
  if (flagPress==1) {_count=0; flagClick=0;} //если состояние сигнала не меняется, обнул. счетчики нажатия и удержания 
  else _count++;// иначе +1 к счетчику состояния сигнала
  if (_count>=_timeButton){  //состояние сигнала стало устойчивым
       _count=0; //обнуление состояния счетчика
       flagClick=1; //принято решение о стаб.состоянии срабатывания
                      } 
    }
//конструктор - установка номера пина и времени подтверждения состояния
Datchik::Datchik(byte pin, int timeButton){
  _pin=pin;
  _timeButton=timeButton;
  pinMode(_pin,INPUT); //пин определяем как вход
}
//Создание объектов - датчиков(кнопок)___________________________________________________________________________________________________
Datchik datchik1(15,70); //кнопка 1 подключена к 14 пину.
Datchik datchik2(14,70); //кнопка 2 подключена к 15 пину.
OneWire Termom1(16); //датчик температуры, подключен к 16 выводу.
OneWire Termom2(17); //датчик температуры, подключен к 17 выводу.
 
LiquidCrystal_I2C lcd(0x27,16,2);


//Глобальные обьявления___________________________________________________________________________________________________________________
int timerCount=0; //Программный таймер для измерения температуры
byte gradus[8]={B01111,B01001,B01001,B01111,B00000,B00000,B00000,B00000,};//знак градуса
byte tsel[8]={B11111,B11111,B11111,B11111,B11111,B11111,B11111,B11111,};//знаак регулятора
byte nul[8]={B00000,B00000,B00000,B00000,B00000,B00000,B00000,B00000,};//знак пусто
byte T=0; //уставка
bool flagRegul=0;//признак изменения Т
int RegulCount=0;//таймер записи Т в память
bool flagSensReady;//признак готовности данных с датчиков температуры
byte Data[9];//буфер данных с 1 датчика температуры
byte Data2[9];//буфер данных со 2 датчика температуры
float temperature1=0;
byte t1=0; //готовое значение 1
float temperature2=0;
byte t2=0; //готовое значение 2

void setup() {
  MsTimer2::set(2, interrupt); //период прерывания по таймеру 2мс
  MsTimer2::start(); //разрешаем прерывания
  pinMode(5,OUTPUT);//выход на мотор
  pinMode(6,OUTPUT);//выход на мотор
  pinMode(9,OUTPUT);//выход на мотор
  pinMode(10,OUTPUT);//выход на мотор
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  digitalWrite(9,LOW);
  digitalWrite(10,LOW);
  
  
  //читаем Т из памяти 
T=EEPROM.read(0); 

  //надписи на дисплее
  lcd.init();
  lcd.backlight();
  lcd.createChar(1,gradus);// создаем знак градуса
  lcd.createChar(2,tsel);// создаем знак регулятора
  lcd.createChar(3,nul);// создаем знак регулятора
  
  lcd.setCursor(0,0);
  lcd.print("T1=");
  lcd.setCursor(5,0);
  lcd.print("\1C");

  lcd.setCursor(0,1);
  lcd.print("T2=");
  lcd.setCursor(5,1);
  lcd.print("\1C");

  lcd.setCursor(10,0);
  lcd.print("T=");
  lcd.setCursor(14,0);
  lcd.print("\1C");

  lcd.setCursor(8,0);
  lcd.print("\3");

  lcd.setCursor(8,1);
  lcd.print("\3");
}


void loop() {

//выводим Т            
       lcd.setCursor(12,0);
       lcd.print(T);  

//Вычисление температуры, вывод на дисплей
   if (flagSensReady==true) {
       flagSensReady=false;
      
      //Датчик 1
      if(OneWire::crc8(Data,8)==Data[8]) //проверка целостности данных 1
             {
       int16_t raw1=(Data[1]<<8) | Data[0];
       temperature1=(float)raw1/16;
       t1=round(temperature1);
       
       if (t1<2) t1=2;
       if ((t1>85)and (t1<200)) t1=85;
       if (t1>200)  {t1=0; lcd.setCursor(3,0); lcd.print(0); lcd.setCursor(4,0);lcd.print(0); goto label1;}
       if (t1<10)  {lcd.setCursor(3,0); lcd.print(0); lcd.setCursor(4,0);lcd.print(t1); goto label1;}
       
       lcd.setCursor(3,0);
       lcd.print(round(t1)); 
            
      }
       label1:    

       //Датчик 2
       if(OneWire::crc8(Data2,8)==Data2[8]) //проверка целостности данных 2
                      {
       int16_t raw2=(Data2[1]<<8) | Data2[0];
       temperature2=(float)raw2/16;
       t2=round(temperature2);
      
       if (t2<2) t2=2;
       if ((t2>85) and (t2<200)) t2=85;
       if (t2>200)  {t2=0; lcd.setCursor(3,1); lcd.print(0); lcd.setCursor(4,1);lcd.print(0); goto label2;}
       if (t2<10)  {lcd.setCursor(3,1); lcd.print(0); lcd.setCursor(4,1);lcd.print(t2);goto label2;}
       
       lcd.setCursor(3,1);
       lcd.print(round(t2));
      
       } 
   }
        label2:

if (t1<T) {
           digitalWrite(5,HIGH);
           digitalWrite(6,HIGH);
           lcd.setCursor(8,0);
           lcd.print("\2");
           }
if (t1>T) {
           digitalWrite(5,LOW);
           digitalWrite(6,LOW);
           lcd.setCursor(8,0);
           lcd.print("\3");           
           }

if (t2<T) {
           digitalWrite(9,HIGH);
           digitalWrite(10,HIGH);
           lcd.setCursor(8,1);
           lcd.print("\2"); 
           }
if (t2>T) {
           digitalWrite(9,LOW);
           digitalWrite(10,LOW);
           lcd.setCursor(8,1);
           lcd.print("\3"); 
           }
  
//Конпка 1
   if (datchik1.flagClick==1) {
    T++;
    flagRegul=true;//разрешаем отчет времени до записи Т
    RegulCount=0;// сбрасываем время до записи 
   if (T>35) T=35;
    datchik1.flagClick=0;
}

//Конпка 2
   if (datchik2.flagClick==1) {
    T--;
    flagRegul=true;//разрешаем отчет времени до записи Т
    RegulCount=0;// сбрасываем время до записи 
    if (T<10) T=10;
    datchik2.flagClick=0;
}

//Проверка времени и запись Т в память
  if (RegulCount=5) {
    flagRegul=false;
    EEPROM.update(0,T);
    }
  
}
//Обработчик прерывания___________________________________________________________________________________________________________________
void interrupt() {
  datchik1.scanState();// вызов метода стабильного ожидания стаб. состояния кнопки 1
  datchik2.scanState();//вызов метода стаб. состояния кнопки 2
  

  //Обработка программного счетчика-таймера + замер температуры
  timerCount++;
  if (timerCount>=500) {
      timerCount=0;
      flagSensReady=true;
 //считаем время до записи Т в память
      if (flagRegul=true) RegulCount++;
      }

//Датчик 1 включение
       if (timerCount==0) Termom1.reset(); //сброс шины
       if (timerCount==1) Termom1.write(0xCC,0); //пропуск ROM
       if (timerCount==2) Termom1.write(0x44,0); //начать измерение
 //Датчик 2 включение
       if (timerCount==3) Termom2.reset(); //сброс шины
       if (timerCount==4) Termom2.write(0xCC,0); //пропуск ROM
       if (timerCount==5) Termom2.write(0x44,0); //начать измерение


 //Чтение датчика1     
       if (timerCount==470) Termom1.reset(); //сброс шины
       if (timerCount==471) Termom1.write(0xCC,0); //пропуск ROM
       if (timerCount==472) Termom1.write(0xBE,0); //чтение памяти датчика
       if (timerCount==473) Data[0]=Termom1.read();
       if (timerCount==474) Data[1]=Termom1.read();  
       if (timerCount==475) Data[2]=Termom1.read();
       if (timerCount==476) Data[3]=Termom1.read();
       if (timerCount==477) Data[4]=Termom1.read();
       if (timerCount==478) Data[5]=Termom1.read();
       if (timerCount==479) Data[6]=Termom1.read();
       if (timerCount==480) Data[7]=Termom1.read();
       if (timerCount==481) Data[8]=Termom1.read();          
      
  //Чтение датчика2    
       if (timerCount==487) Termom2.reset(); //сброс шины
       if (timerCount==488) Termom2.write(0xCC,0); //пропуск ROM
       if (timerCount==489) Termom2.write(0xBE,0); //чтение памяти датчика
       if (timerCount==490) Data2[0]=Termom2.read();
       if (timerCount==491) Data2[1]=Termom2.read(); 
       if (timerCount==492) Data2[2]=Termom2.read();
       if (timerCount==493) Data2[3]=Termom2.read();
       if (timerCount==494) Data2[4]=Termom2.read();
       if (timerCount==495) Data2[5]=Termom2.read();
       if (timerCount==496) Data2[6]=Termom2.read();
       if (timerCount==497) Data2[7]=Termom2.read();
       if (timerCount==498) Data2[8]=Termom2.read();
  
}

