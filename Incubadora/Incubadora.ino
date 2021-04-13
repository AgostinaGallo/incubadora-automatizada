// ------------------------ LIBRERIAS ---------------------------------------------------
#include <LiquidCrystal_I2C.h>
// include Wire library code (needed for I2C protocol devices)
#include <Wire.h>
#include "LedControl.h"
// ---------------------------------------------------------------------------------------


// -------------- DEFINICIONES DE PINES, MODULOS, ETC ------------------------------------
//                 (RS, E, D4, D5, D6, D7)
//LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
LiquidCrystal_I2C lcd1(0x38, 16, 2);
LiquidCrystal_I2C lcd2(0x3F, 16, 2);

LedControl ledControl = LedControl(11, 13, 10, 1);

#define LM35 A0
#define Relay 3
#define Triac 5

float temperaturaDigital;
float valorSensor;
int temperaturaEnGrados;

byte i, segundos, minutos, horas, diaIncubacion, dia, mes, anio;
char Time[]     = "  :  :   DAY:  ";
char Calendar[] = "DATE:  /  /20  ";
// ---------------------------------------------------------------------------------------



void establecerDiaFechaInicial(){
  segundos =    00; //0-59
  minutos =    00; //0-59
  horas   =    00; //0-23
  diaIncubacion  = 00; //0-..
  dia   =    17; //1-31
  mes  =    03; //1-12
  anio   =    21; //0-99

  Wire.beginTransmission(0x68);
  Wire.write(0); //stop Oscillator
  
  minutos = ((minutos / 10) << 4) + (minutos % 10);
  horas = ((horas / 10) << 4) + (horas % 10);
  diaIncubacion = ((diaIncubacion / 10) << 4) + (diaIncubacion % 10);
  dia = ((dia / 10) << 4) + (dia % 10);
  mes = ((mes / 10) << 4) + (mes % 10);
  anio = ((anio / 10) << 4) + (anio % 10);
  Wire.write(segundos);
  Wire.write(minutos);
  Wire.write(horas);
  Wire.write(diaIncubacion);
  Wire.write(dia);
  Wire.write(mes);
  Wire.write(anio);
  Wire.write(0); //start
  Wire.endTransmission();
}



/*-------------------------------------------------------------------------
 *   SETUP del ambiente
------------------------------------------------------------------------- */
void setup() {
  pinMode(LM35, INPUT);
  pinMode(Relay, INPUT);
  pinMode(Triac, OUTPUT);

  ledControl.shutdown(0, false); // Inicia el MAX7219
  ledControl.setIntensity(0, 4); // Establece brillo en 4
  ledControl.clearDisplay(0); // limpia el display
  
  pinMode(8, INPUT_PULLUP);                      // button1 is connected to pin 8
  pinMode(9, INPUT_PULLUP);                      // button2 is connected to pin 9
  
  lcd1.init(); //lcd 1 startup
  lcd1.backlight();
  lcd2.init(); //lcd 2 startup
  lcd2.backlight();

  Wire.begin();                                  // Join i2c bus 
  establecerDiaFechaInicial();
}
// -------------------------------------------------------------------------------------  



void button_Selector(){
      i = 0;
      horas   = edit(0, 0, horas);
      minutos = edit(3, 0, minutos);
      diaIncubacion    = edit(13, 0, diaIncubacion);
      dia   = edit(5, 1, dia);
      mes  = edit(8, 1, mes);
      anio   = edit(13, 1, anio);
      // Convert decimal to BCD
      minutos = ((minutos / 10) << 4) + (minutos % 10);
      horas = ((horas / 10) << 4) + (horas % 10);
      diaIncubacion = ((diaIncubacion / 10) << 4) + (diaIncubacion % 10);
      dia = ((dia / 10) << 4) + (dia % 10);
      mes = ((mes / 10) << 4) + (mes % 10);
      anio = ((anio / 10) << 4) + (anio % 10);
      // End conversion
      // Write data to DS1307 RTC
      Wire.beginTransmission(0x68);               // Start I2C protocol with DS1307 address
      Wire.write(0);                              // Send register address
      Wire.write(0);                              // Reset sesonds and start oscillator
      Wire.write(minutos);                         // Write minutos
      Wire.write(horas);                           // Write horas
      Wire.write(diaIncubacion);                            // Write diaIncubacion (not used)
      Wire.write(dia);                           // Write dia
      Wire.write(mes);                          // Write mes
      Wire.write(anio);                           // Write anio
      Wire.endTransmission();                     // Stop transmission and release the I2C bus
      delay(200);                                 // Wait 200ms
}

void DS1307_display(){
  // Convert BCD to decimal
  segundos = (segundos >> 4) * 10 + (segundos & 0x0F);
  minutos = (minutos >> 4) * 10 + (minutos & 0x0F);
  horas   = (horas >> 4)   * 10 + (horas & 0x0F);
  diaIncubacion    = (diaIncubacion >> 4)    * 10 + (diaIncubacion & 0x0F);
  dia   = (dia >> 4)   * 10 + (dia & 0x0F);
  mes  = (mes >> 4)  * 10 + (mes & 0x0F);
  anio   = (anio >> 4)   * 10 + (anio & 0x0F);
  // End conversion
  Time[14]     = diaIncubacion   % 10 + 48;
  Time[13]     = diaIncubacion    / 10 + 48;
  Time[7]      = segundos % 10 + 48;
  Time[6]      = segundos / 10 + 48;
  Time[4]      = minutos % 10 + 48;
  Time[3]      = minutos / 10 + 48;
  Time[1]      = horas   % 10 + 48;
  Time[0]      = horas   / 10 + 48;
  Calendar[14] = anio   % 10 + 48;
  Calendar[13] = anio   / 10 + 48;
  Calendar[9]  = mes  % 10 + 48;
  Calendar[8]  = mes  / 10 + 48;
  Calendar[6]  = dia   % 10 + 48;
  Calendar[5]  = dia   / 10 + 48;
  lcd1.setCursor(0, 0);
  lcd1.print(Time);                               // Display time
  lcd1.setCursor(0, 1);
  lcd1.print(Calendar);                           // Display calendar
}
void blink_parameter(){
  byte j = 0;
  while(j < 10 && digitalRead(8) && digitalRead(9)){
    j++;
    delay(25);
  }
}
byte edit(byte x, byte y, byte parameter){
  char text[3];
  while(!digitalRead(8));                        // Wait until button (pin #8) released
  while(true){
    while(!digitalRead(9)){                      // If button (pin #9) is pressed
      parameter++;
      if(i == 0 && parameter > 23)               // If horass > 23 ==> hours = 0
        parameter = 0;
      if(i == 1 && parameter > 59)               // If minutoss > 59 ==> minutoss = 0
        parameter = 0;
      if(i == 2 && parameter > 31)               // If dia > 31 ==> dia = 1
        parameter = 1;
      if(i == 3 && parameter > 12)               // If mes > 12 ==> mes = 1
        parameter = 1;
      if(i == 4 && parameter > 99)               // If anio > 99 ==> anio = 0
        parameter = 0;
      sprintf(text,"%02u", parameter);
      lcd1.setCursor(x, y);
      lcd1.print(text);
      delay(200);                                // Wait 200ms
    }
    lcd1.setCursor(x, y);
    lcd1.print("  ");                             // Display two spaces
    blink_parameter();
    sprintf(text,"%02u", parameter);
    lcd1.setCursor(x, y);
    lcd1.print(text);
    blink_parameter();
    if(!digitalRead(8)){                         // If button (pin #8) is pressed
      i++;                                       // Increament 'i' for the next parameter
      return parameter;                          // Return parameter value and exit
    }
  }
}

int lecturaTemperatura(){
  int temperaturaRetornada;
  valorSensor = analogRead(LM35);
  temperaturaDigital = (valorSensor * 5.0) / 1023.0;
  temperaturaEnGrados = (temperaturaDigital * 100) / 5.0; // Conversion a GradosCentigrados
  temperaturaRetornada = temperaturaEnGrados;
  return temperaturaRetornada;  
  
}
void displayTemperatura(int temperaturaLeida){
  ledControl.setDigit(0,0, temperaturaLeida % 10, false);
  ledControl.setDigit(0,1, (temperaturaLeida / 10) % 10, false);
  ledControl.setDigit(0,2, (temperaturaLeida / 100) % 10, false);
  ledControl.setDigit(0,3, (temperaturaLeida / 1000) % 10, false);
}



/**------------------------------------------------
**   Constante Ejecucion
----------------------------------------------- */
void loop() {
  // Proceso de incubacion menor al dia 23 = finalizacion
  while(diaIncubacion <= 22){
      
      if(!digitalRead(8)){             // Cuando el boton SELECTOR (pin #8) se presiona
        button_Selector();  
      }
      Wire.beginTransmission(0x68);                 // Start I2C protocol with DS1307 address
      Wire.write(0);                                // Send register address
      Wire.endTransmission(false);                  // I2C restart
      Wire.requestFrom(0x68, 7);                    // Request 7 bytes from DS1307 and release I2C bus at end of reading
      segundos = Wire.read();                         // Read segundos from register 0
      minutos = Wire.read();                         // Read minuts from register 1
      horas   = Wire.read();                         // Read horas from register 2
      diaIncubacion = Wire.read();                  // Read diaIncubacion from register 3 
      dia   = Wire.read();                         // Read dia from register 4
      mes  = Wire.read();                         // Read mes from register 5
      anio   = Wire.read();                         // Read anio from register 6 
  
              /*
			  *	
			  */
              // Dia 0, primeras 18 horas => Informa pre calentamiento
              if(diaIncubacion == 0 && horas <= 18){
                lcd2.setCursor(0,0);
                lcd2.print("Etapa: Pre");
                lcd2.setCursor(0,1);
                lcd2.print("Calentamiento");
              }
              // Dia 1 en adelante => INCUBACION COMENZADA  
              if (diaIncubacion > 0){
                  lcd2.clear();
                  lcd2.setCursor(0,0);
                  lcd2.print("Inicio");
                  lcd2.setCursor(0,1);
                  lcd2.print("Incubacion");
				  
                  if (diaIncubacion < 18){
                    // Entre el dia 1 a 18 => se activa el PIN TRIAC(Foco calentador)
                    analogWrite(Triac,255);
                    
					// Desde el dia 3 en adelante => informo que giran los huevos incubados
                    if (diaIncubacion >= 3){
                        lcd2.setCursor(0,0);
                        lcd2.print("Giro de");
                        lcd2.setCursor(0,1);
                        lcd2.print("Huevos");
                     }
                  }
                  // Dia 18 en adelante  
                  else{  
                        temperaturaEnGrados = lecturaTemperatura();
                        
						// Control de temperatura para encender Foco CALENTADOR
                        if (temperaturaEnGrados < 36){ 
                          analogWrite(Triac,255);
                          displayTemperatura(temperaturaEnGrados); 
                        }
						// Control de temperatura para encender Cooler ENFRIADOR
                        else if (temperaturaEnGrados > 36){
                          analogWrite(Triac,0);
                          digitalWrite(Relay,HIGH);
                          displayTemperatura(temperaturaEnGrados); 
                        }
                        
						// Informe de detencion de Giro de Huevos incubados
                        lcd2.setCursor(0,0);
                        lcd2.print("FIN Giro de"); 
                        lcd2.setCursor(0,1);
                        lcd2.print("Huevos.");
                  }  
              } 
			  /*
			  ** FIN de control de Incubacion.
			  */

	
	
	/*
	* Constante muestreo de temperatura y tiempo y fecha. Cada 50milisegundos
 	*/
      temperaturaEnGrados = lecturaTemperatura(); 
      displayTemperatura(temperaturaEnGrados); 
      DS1307_display();  
      delay(50); 
    }

    // Dia 22, FINALIZACION de Incubacion, e informe
    if(diaIncubacion > 22){
      lcd2.clear();
      lcd2.setCursor(0,0);
      lcd2.print("Incubacion");
      lcd2.setCursor(0,1);
      lcd2.print("Finalizada.");
      delay(3000);
      lcd1.clear();
      lcd2.clear();
    }
}
