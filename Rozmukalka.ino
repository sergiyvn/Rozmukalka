// Пин 9 '+' пищалки
// GND   '-' пищалки
// Пин 7 первый провод размыкания
// Пин 2 второй провод размыкания (Вход прерывания)
//
// *** Выходы для ЖК экрана ***
// GND экрана -> GND на плате
// VCC экрана -> +5В на плате
// SDA экрана -> A5 на плате (Analog IN)
// SCL экрана -> A4  на плате (Analog IN)
//
// *** Настройка кода и время ***
// CODE - меняем на свой код
// TIME - время таймера в милисекундах!
// RANGE_TIME - промежуток времени до нуля в котором надо разомкнуть цепь
//
// !!! //Serial.print() после тестов всюду закоментировать!!! Грузит сильно ардуино
// Alex start
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce.h>

String deviceVersion = "1.1.0"; // Версия прибора :)

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

int buttonsDelay = 300; // ms
boolean gameStarted = false; //Press reset button to start game mode

int relePin = 2;// Пин реле
int disconnectBtnPin = 4;
int upBtnPin = 5;
int downBtnPin = 6;
int okBtnPin = 7;
int restartBtnPin = 8;
int beepPin = 9; //выход пищалки
int vibroPin = 10;//Пин датчика вибрации

Bounce bouncerUp = Bounce(upBtnPin, 40); //создаем экземпляр класса Bounce для 5 вывода
Bounce bouncerDown = Bounce(downBtnPin, 40); //создаем экземпляр класса Bounce для 6 вывода
Bounce bouncerOk = Bounce(okBtnPin, 40); //создаем экземпляр класса Bounce для 7 вывода
Bounce bouncerRestart = Bounce(restartBtnPin, 40); //создаем экземпляр класса Bounce для 8 вывода
Bounce bouncerDisconnect = Bounce(disconnectBtnPin, 40); //создаем экземпляр класса Bounce для 4 вывода
Bounce bouncerVibro = Bounce(vibroPin, 40); //создаем экземпляр класса Bounce для 10 вывода

int GAME_MODE = 1;// Режим игру по умолчанию
int GAME_COUNTS = 5;// количество режимов игры

volatile unsigned long int time1 = 0; // время размыкания

/** Опции игры */
boolean option_vibro = false; // Датчик вибрации включен?
boolean option_vibro_is_setup = false;

int option_vibro_delay_time = 60; // время в секундах, Датчик вибрации включен?
boolean option_vibro_delay_time_is_setup = false;

boolean option_detonator = false; // Реле для петарды включено?
boolean option_detonator_is_setup = false;

int CODE1 = -1;
int CODE2 = -1;
int CODE3 = -1;
int CODE4 = -1;
int _CODE = 0;//текущий код
boolean is_code_setted = false; //

boolean can_disconnect = false;
boolean is_disconnect_setup = false;

boolean canRestart = false; // возможен ли перезапуск сценария нажатием кнопки на приборе
boolean canRestart_is_setup = false;

char* MODE_NAME[] = {
  "  - PO3MUKAJIKA ",
  "  - HETY        ",
  "  - HETY        ",
  "  - HETY        ",
  "  - HETY        "
};
boolean is_mode_selected = false;//Проверяет или выбран режим игры

boolean is_mode_setup = false; // Проверяет установлены ли параметры выбраного режим игры

long TIME = 5000; //5sec Timer time
boolean is_time_setup = false;

int RANGE_TIME = 100;//мс
boolean is_range_time_setup = false;

int RANGE_TIME_BEGIN = 0;//мс
int RANGE_TIME_END = 100;//мс


//private vars
float divider = 1000;// 1000 для мс, 1000000 для микросекунд
float starTime;
boolean stopped = false;
boolean timeEnd = false;
boolean correctTime = false;


void setup() {
  lcd.init();                     // инициализация LCD
  lcd.backlight();                // включаем подсветку
  lcd.clear();                    // очистка дисплея

  pinMode(relePin, OUTPUT); //Выход на реле
  pinMode(beepPin, OUTPUT); //Выход на пищалку
  pinMode(vibroPin, INPUT); //Вход вибро пина
  digitalWrite(vibroPin, 1);  //включаем на вибро пине подтягивающий резистор

  pinMode(upBtnPin, INPUT);   //переключаем 4 вывод в режим входа
  digitalWrite(upBtnPin, 1);  //включаем на нем подтягивающий резистор
  pinMode(downBtnPin, INPUT);   //переключаем 5 вывод в режим входа
  digitalWrite(downBtnPin, 1);  //включаем на нем подтягивающий резистор
  pinMode(okBtnPin, INPUT);   //переключаем 6 вывод в режим входа
  digitalWrite(okBtnPin, 1);  //включаем на нем подтягивающий резистор
  pinMode(restartBtnPin, INPUT);   //переключаем 6 вывод в режим входа
  digitalWrite(restartBtnPin, 1);  //включаем на нем подтягивающий резистор

  //Serial.begin(9600);
  //Serial.print(" * * START * * ");
  //Serial.print("\n");
  lcd.setCursor(0, 0);
  lcd.print(" A-Team device");
  lcd.setCursor(0, 1);
  lcd.print(" version ");
  lcd.print(deviceVersion);
  
  delay(5000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setup mode:");
  lcd.setCursor(13, 0);
  lcd.print(GAME_MODE);
  lcd.setCursor(0, 1);
  lcd.print(MODE_NAME[0]);
}


void loop() {

  //Проверям сначало установку режима
  if (!is_mode_selected)
  {
    checkUpButton();
    checkDownButton();
    checkOkButton();
  }
  // Устанавливаем параметры выбраного режима игры
  else if (!is_mode_setup)
  {
    setupMode();
  }
  // включение датчика вибрации
  else if (!option_vibro_is_setup)
  {
    checkUpButton();
    checkDownButton();
    checkOkButton();
  }
  //установка время на выход из строя прибора
  else if (!option_vibro_delay_time_is_setup)
  {
    checkUpButton();
    checkDownButton();
    checkOkButton();
  }
  //Установка детонатора
  else if (!option_detonator_is_setup)
  {
    checkUpButton();
    checkDownButton();
    checkOkButton();
  }
  //Устанавливаем код
  else if (!is_code_setted)
  {
    checkUpButton();
    checkDownButton();
    checkOkButton();
  }
  else if (!is_time_setup)
  {
    checkUpButton();
    checkDownButton();
    checkOkButton();
  }
  else if (!gameStarted)
  {
    checkStartButton();
  }
  // Запускаем сценарий игры
  else if (is_mode_selected && is_code_setted && is_mode_setup && gameStarted)
  {
    switch (GAME_MODE)
    {
        // **********/ GAME MODE 1 BEGIN /*********************************************
      case 1:
        runMode_1();
        break;
        // **********/ GAME MODE 2 BEGIN /*********************************************
      case 2:
        runMode_2();
        break;
        // **********/ GAME MODE 3 BEGIN /*********************************************
      case 3:
        runMode_3();
        break;
        // **********/ GAME MODE 4 BEGIN /*********************************************
      case 4:
        runMode_4();
        break;
        // **********/ GAME MODE 5 BEGIN /*********************************************
      case 5:
        runMode_5();
        break;
    }

    if (option_vibro)
      if (bouncerVibro.update())
        //если произошло событие
        if (bouncerVibro.read() == 0) {  //если кнопка нажата
          //Serial.print("RESET Button");
          //Serial.print("\n");

          //          bouncerVibro.rebounce(option_vibro_delay_time*divider);      //повторить событие через 1000мс

          //Выводим сообщение о выходе из строя прибора
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Device damaged");
          lcd.setCursor(0, 1);
          lcd.print("Reload time:");
          lcd.print(option_vibro_delay_time);
          lcd.print("s");

          //Запуск детонации бомбы /включаем питание на реле на 2сек и выключаем
          if (option_detonator)
          {
            digitalWrite(relePin, HIGH);
            delay(2000);
            digitalWrite(relePin, LOW);
            option_detonator = false;
          }

          if (option_vibro_delay_time - 2 > 0)
          {
            delay((option_vibro_delay_time - 2 ) * divider); //Задержка сценария на заданое время
          }
          if (canRestart)
            resetMode();

          bouncerVibro = Bounce(vibroPin, 40);
        }

    if (canRestart)
      if (bouncerRestart.update())
        //если произошло событие
        if (bouncerRestart.read() == 0) {  //если кнопка нажата
          //Serial.print("RESET Button");
          //Serial.print("\n");
          bouncerRestart.rebounce(500);      //повторить событие через 500мс
          resetMode();
        }

    // Проверка на размыкание контакта
    if (!stopped && can_disconnect && is_disconnect_setup)
      if (bouncerDisconnect.update())
        if (bouncerDisconnect.read() == 1) {  //если контакт разомкнут
          sensor_1();
        }
  }

}

//Установка параметров для определенного режима игры
void setupMode()
{
  //Установка режимов
  switch (GAME_MODE)
  {
    case 1:
      option_vibro_delay_time = 60; //Вывод из строя прибора на 5мин по умолчанию
      TIME = 60000;
      RANGE_TIME = 100;
      RANGE_TIME_BEGIN = 0;//мс //пока не использую
      RANGE_TIME_END = 100;//мс //пока не использую
      can_disconnect = true;
      is_disconnect_setup = true;
      canRestart = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Vibration on?:");
      lcd.setCursor(10, 1);
      lcd.print(" No");
      //      starTime = millis();
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
  }

  is_mode_setup = true;
}


//Проверка для кнопки "Вверх" или нажата
void checkUpButton()
{
  if (bouncerUp.update())
  { //если произошло событие
    if (bouncerUp.read() == 0) {  //если кнопка нажата
      upDownBehaviour(true);
      //Serial.print("UP Button");
      //Serial.print("\n");
      bouncerUp.rebounce(buttonsDelay);      //повторить событие через 500мс
    }
  }
}


//Проверка для кнопки "Вниз" или нажата
void checkDownButton()
{
  if (bouncerDown.update())
  { //если произошло событие
    if (bouncerDown.read() == 0) {  //если кнопка нажата
      upDownBehaviour(false);
      //Serial.print("DOWN Button");
      //Serial.print("\n");
      bouncerDown.rebounce(buttonsDelay);      //повторить событие через 500мс
    }
  }
}

void upDownBehaviour(boolean increase)
{
  if (!is_mode_selected)
  {
    if (increase) {
      GAME_MODE ++;
      if (GAME_MODE > GAME_COUNTS)
        GAME_MODE = 1;
    } else {
      GAME_MODE --;
      if (GAME_MODE < 1)
        GAME_MODE = GAME_COUNTS;
    }

    lcd.setCursor(13, 0);
    lcd.print(GAME_MODE);
    lcd.setCursor(0, 1);
    lcd.print(MODE_NAME[GAME_MODE - 1]);
  }
  else if (!option_vibro_is_setup)
  {
    option_vibro = !option_vibro;
    lcd.setCursor(10, 1);
    if (option_vibro)
      lcd.print("Yes");
    else
      lcd.print(" No");
  }
  else if (option_vibro && !option_vibro_delay_time_is_setup)
  {
    if (increase)
      option_vibro_delay_time += 5;
    else
      option_vibro_delay_time -= 5;

    if (option_vibro_delay_time < 0)
      option_vibro_delay_time = 0;

    //    lcd.clear();
    //    lcd.setCursor(0, 0);
    //    lcd.print("Deactivation");
    //    lcd.setCursor(4, 1);
    //    lcd.print("time:");
    lcd.setCursor(9, 1);
    lcd.print("   ");
    lcd.setCursor(9, 1);
    lcd.print(option_vibro_delay_time);
    lcd.setCursor(12, 1);
    lcd.print("sec");
  }
  else if (!option_detonator_is_setup)
  {
    option_detonator = !option_detonator;
    lcd.setCursor(10, 1);
    if (option_detonator)
      lcd.print("Yes");
    else
      lcd.print(" No");
  }
  else if (!is_code_setted)
  {
    increaseCode(increase);
    if (CODE1 == -1)
    {
      lcd.setCursor(10, 0);
    }
    else if (CODE2 == -1)
    {
      lcd.setCursor(11, 0);
    }
    else if (CODE3 == -1)
    {
      lcd.setCursor(12, 0);
    }
    else if (CODE4 == -1)
    {
      lcd.setCursor(13, 0);
    }
    lcd.print(_CODE);
  }
  else if (!is_time_setup)
  {
    if (increase)
      TIME += 5000;
    else
      TIME -= 5000;
      
    lcd.setCursor(9, 1);
    lcd.print("   ");
    lcd.setCursor(9, 1);
    float _t = TIME / 1000;
    lcd.print(int(_t));
  }

}


//Проверка для кнопки "ОК" или нажата
void checkOkButton()
{
  if (bouncerOk.update())
  { //если произошло событие
    if (bouncerOk.read() == 0) {  //если кнопка нажата
      //Serial.print("OK BUtton");
      //Serial.print("\n");
      if (!is_mode_selected)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Vibration on?:");
        lcd.setCursor(10, 1);
        lcd.print(" No");

        is_mode_selected = true;
      }
      else if (!option_vibro_is_setup)
      {
        if (option_vibro)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Deactivation");
          lcd.setCursor(4, 1);
          lcd.print("time:");
          lcd.setCursor(9, 1);
          lcd.print(option_vibro_delay_time);
          lcd.setCursor(12, 1);
          lcd.print("sec");
        } else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Detonator on?:");
          lcd.setCursor(10, 1);
          lcd.print(" No");
          option_vibro_delay_time_is_setup = true;
        }

        option_vibro_is_setup = true;
      }
      else if (option_vibro && !option_vibro_delay_time_is_setup)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Detonator on?:");
        lcd.setCursor(10, 1);
        lcd.print(" No");

        if (option_vibro_delay_time <= 0)
          option_vibro = false;

        option_vibro_delay_time_is_setup = true;
      }
      else if (!option_detonator_is_setup)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set code: ");
        lcd.setCursor(10, 0);
        lcd.print(_CODE);

        option_detonator_is_setup = true;
      }
      else if (!is_code_setted)
      {
        if (CODE1 == -1)
        {
          CODE1 = _CODE;
          _CODE = 0;
          lcd.setCursor(11, 0);
          lcd.print(_CODE);
        }
        else if (CODE2 == -1)
        {
          CODE2 = _CODE;
          _CODE = 0;
          lcd.setCursor(12, 0);
          lcd.print(_CODE);
        }
        else if (CODE3 == -1)
        {
          CODE3 = _CODE;
          _CODE = 0;
          lcd.setCursor(13, 0);
          lcd.print(_CODE);
        }
        else if (CODE4 == -1) {
          CODE4 = _CODE;
          _CODE = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Setup timer");
          lcd.setCursor(4, 1);
          lcd.print("time:");
          lcd.setCursor(9, 1);
          float _t = TIME / 1000;
          lcd.print(int(_t));
          lcd.setCursor(12, 1);
          lcd.print("sec");

          is_code_setted = true;
        }
      }
      else if (!is_time_setup)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Press BUTTON");
        lcd.setCursor(0, 1);
        lcd.print("    to START");

        is_time_setup = true;
      }

      bouncerOk.rebounce(500);      //повторить событие через 500мс
    }
  }
}


//Ждем нажатия кнопки до запуска игры
void checkStartButton()
{
  if (bouncerRestart.update())
    //если произошло событие
    if (bouncerRestart.read() == 0) {  //если кнопка нажата
      //Serial.print("RESET Button");
      //Serial.print("\n");
      bouncerRestart.rebounce(1000);      //повторить событие через 500мс
      gameStarted = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      printTime(TIME, 0, 0);
      lcd.setCursor(0, 1);
      lcd.print("Turn off on 0.1s");
      delay(1000);
      starTime = millis();
    }
}

//Перезапуск игры с уже установлеными настройками
void resetMode()
{
  switch (GAME_MODE)
  {
    case 1:
      lcd.clear();                    // очистка дисплея
      printTime(TIME, 0, 0);
      lcd.setCursor(0, 1);
      lcd.print("Turn off on 0.1s");
      delay(1000);
      correctTime = false;
      stopped = false;
      timeEnd = false;
      time1 = 0;
      starTime = millis();
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
  }
}


//  Запуск  режима 1
void runMode_1()
{
  if (!stopped)
  {
    float currentTime = millis();
    float tm = starTime + TIME - currentTime;
    if (time1 == 0 && tm <= 0)
    {
      stopped = true;
      timeEnd = true;
      detachInterrupt(0);
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Time end");
      lcd.setCursor(3, 1);
      lcd.print("Try again");
      //Serial.print("Time end, Try again");
      //Serial.print("\n");
    } else if (time1 == 0)
    {
      printTime(tm, 0, 0);
      //Serial.print(tm / divider);
      //Serial.print("\n");
    } else if (time1 != 0 )
    {
      stopped = true;
      detachInterrupt(0);
      lcd.clear();
      tm = starTime + TIME - time1;
      if ( tm > 0 && tm <= RANGE_TIME )
      {
        correctTime = true;
        lcd.setCursor(1, 0);
        lcd.print("Congratulation");
        lcd.setCursor(0, 1);
        lcd.print("Your code:  ");
        lcd.print(CODE1);
        lcd.print(CODE2);
        lcd.print(CODE3);
        lcd.print(CODE4);
        //Serial.print("Congratulation! Your code: ");
        //Serial.print(CODE1);
        //Serial.print(CODE2);
        //Serial.print(CODE3);
        //Serial.print(CODE4);
        //Serial.print("\n");
      } else {
        printTime(tm, 0, 0);
        lcd.setCursor(0, 1);
        lcd.print("Wrong, Try again");
        //Serial.print("Your time: ");
        //Serial.print(tm / divider);
        //Serial.print(" Wrong, Try again");
        //Serial.print("\n");
      }
    }
  } else {
    if (!correctTime && !timeEnd ) {
      beep(50);
    }
  }

}

//  Запуск  режима 2
void runMode_2()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Wrong mode 2");
}

//  Запуск  режима 3
void runMode_3()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Wrong mode 3");
}

//  Запуск  режима 4
void runMode_4()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Wrong mode 4");
}

//  Запуск  режима 5
void runMode_5()
{
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Wrong mode 5");
}


//функция размынкания контакта
void sensor_1()
{
  float _time1 = millis();
  if ( time1 == 0 && starTime != _time1)
  {
    time1 = _time1;
    //Serial.print("Start time: ");
    //Serial.print(starTime);
    //Serial.print("   Off time: ");
    //Serial.print(time1);
    //Serial.print("\n");
  }
}


//buttons UP/DOWN
void sensor_2()
{

}


void increaseCode(boolean increase)
{
  if (increase)
  {
    if (_CODE < 9)
      _CODE ++;
    else
      _CODE = 0;
  } else {
    if (_CODE > 0)
      _CODE --;
    else
      _CODE = 9;
  }
}


void printTime(float value, int x, int y)
{
  float s = value / divider;

  lcd.setCursor(x, y);
  lcd.print("Time : ");
  lcd.print(s);
}


void beep(unsigned char delayms) {
  //  Для пьезопещалки
  //  analogWrite(beepPin, 20);      // значение должно находится между 0 и 255
  //  delay(delayms);          // пауза delayms мс
  //  analogWrite(beepPin, 0);       // 0 - выключаем пьезо
  //  delay(delayms);          // пауза delayms мс

  //  Для динамиков
  tone(beepPin, 950, delayms);
  delay(2 * delayms);
}
