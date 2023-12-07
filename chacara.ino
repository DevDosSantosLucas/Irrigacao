// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

//BIBLIOTECAS
#include <Arduino.h>
#include <Wire.h>
#include "RTClib.h"
#include <MicroLCD.h>

//CONFIGURACAO DO DISPLAY MICRO LCD 
LCD_SSD1306 lcd; /* para módulo contralado pelo CI SSD1306 OLED */
RTC_DS1307 rtc; /* para modulo de relogio */ 

//VARIAVEIS

int releBomba = 11 ; // Trababalha em nivel alto (HIGH)
int releCaixaDaAgua = 3  ;//trabalha em nivel baixo (LOW)

int releIrrigacao1 = 4  ;
int releIrrigacao2 = 5  ; // DEMORANDO PARA ACIONAR 2 SEG
int releIrrigacao3 = 6  ;
int releIrrigacao4 = 7  ;
int releIrrigacao5 = 8  ; // TRILHO 
int releIrrigacao6 = 9  ;
int releIrrigacao7 = 10; // TRILHO
int pinoSensorNivelBaixo = 12; //gnd+digital 1 = baixo / 0 = alto
int pinoSensorNivelAlto  = 13; //gnd+digital 1 = baixo/ 0 = alto

int estadoSensorNivelBaixo;
int estadoSensorNivelAlto;

int releAtivado = 5; 

int minuto = 2;//120 ;//colocar tempo de irrigacao contando que ficara 30min  ativo cada
int estadoReleIrrigacao = 0;
int estadoReleCaixaDaAgua = 0;

int primeiraHora = 0;//06hrs  // QUAIS SÃO OS HORARIOS
int segundaHora  = 18 ;//18hrs // DE IRRIGAÇÃO.


int umidadeSolo = 0;
int pinoSensor = A0;
int limiarSeco = 60; //VERIFICAR QUANTOS PORCENTO PARA DO SENSOR DE ÁGUA NO SOLO

char daysOfTheWeek[7][12] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};

void setup () {
  while (!Serial); // for Leonardo/Micro/Zero
  pinMode(releBomba, OUTPUT);
  pinMode(releCaixaDaAgua, OUTPUT); 
  pinMode(releIrrigacao1, OUTPUT);
  pinMode(releIrrigacao2, OUTPUT);
  pinMode(releIrrigacao3, OUTPUT);
  pinMode(releIrrigacao4, OUTPUT);
  pinMode(releIrrigacao5, OUTPUT);
  pinMode(releIrrigacao6, OUTPUT);
  pinMode(releIrrigacao7, OUTPUT);
  pinMode(pinoSensorNivelBaixo, INPUT_PULLUP); 
  pinMode(pinoSensorNivelAlto, INPUT_PULLUP); 

  lcd.begin();

  if (!rtc.begin()) {
    lcd.println("NÃO ENCONTROU RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    lcd.println("RTC NÃO ESTÁ RODANDO!");
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2023, 10, 26, 0, 30, 0));
    //rtc.adjust(DateTime(yyyy, mm, dd, hh, mm, ss));

  }
}

void loop () {
  digitalWrite(releBomba, LOW);         
  digitalWrite(releCaixaDaAgua, HIGH);  
  digitalWrite(releIrrigacao1, HIGH);   
  digitalWrite(releIrrigacao2, HIGH);   
  digitalWrite(releIrrigacao3, HIGH);
  digitalWrite(releIrrigacao4, HIGH);   
  digitalWrite(releIrrigacao5, HIGH); 
  digitalWrite(releIrrigacao6, HIGH);   
  digitalWrite(releIrrigacao7, HIGH);   

  lcd.clear();
  lcd.setFontSize(FONT_SIZE_MEDIUM);
  
  DateTime now = rtc.now();
  
  lcd.print(now.day(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.year(), DEC);
  lcd.print("-");
  lcd.println(daysOfTheWeek[now.dayOfTheWeek()]);

  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);
  lcd.println();
    
  delay(5000);
    
  lcd.clear();
  lcd.println("VERIFICANDO");
  lcd.println("CAIXA D'");
  lcd.println("AGUA");
  delay(2000);  
  
  verCaixa ();
  
  digitalWrite(releBomba, LOW); 
  digitalWrite(releCaixaDaAgua, HIGH); 
  digitalWrite(releIrrigacao1, HIGH);   
  digitalWrite(releIrrigacao2, HIGH);   
  digitalWrite(releIrrigacao3, HIGH);
  digitalWrite(releIrrigacao4, HIGH);   
  digitalWrite(releIrrigacao5, HIGH); 
  digitalWrite(releIrrigacao6, HIGH);   
  digitalWrite(releIrrigacao7, HIGH);   

   
  //if(now.hour() == primeiraHora || now.hour() == segundaHora ){ // 6 ou 18 hrs 
  if(now.hour() == primeiraHora | now.hour() == segundaHora ){ // 6 ou 18 hrs
    fazerIrrigacao();
    digitalWrite(releBomba, LOW);
 //   digitalWrite(releIrrigacao, HIGH);
    lcd.clear();
    lcd.println("BOMBA DESL");
    delay(2000);
  }else{
    if((now.hour() > segundaHora) | (now.hour() < primeiraHora)){
      lcd.clear();
      lcd.println("IRRIGACAO");
      lcd.println("AGENDADA");
      lcd.print("AS ");
      lcd.print(primeiraHora);
      lcd.print(" HORAS");
      delay(5000);      
    }else{
      lcd.clear();
      lcd.println("IRRIGACAO");
      lcd.println("AGENDADA");
      lcd.print("AS ");
      lcd.print(segundaHora);
      lcd.print(" HORAS");
      delay(5000);      
    }
  }
}

void verCaixa(){
  int estadoSensorNivelBaixo = digitalRead(pinoSensorNivelBaixo);
  int estadoSensorNivelAlto  = digitalRead(pinoSensorNivelAlto);
  
  if( estadoSensorNivelBaixo){
    lcd.clear();
    lcd.println("NIVEL CAIXA: ");
    lcd.println("VAZIA"); //1 LIGAR RELE DA SOLENOIDE CAIXA
    
    desativaRelesDeIrrigacao();
    
    lcd.println("IrriG desl"); 
    lcd.println("CAIXA LIGADA!"); 
    digitalWrite(releCaixaDaAgua, LOW);  
    digitalWrite(releBomba, HIGH);
    delay(2000);

    int minutosEnchendoCaixa = 0 ;
    int contandoSegundosEnchendoCaixa=1 ;
    
    while(estadoSensorNivelAlto){
      lcd.clear();
      lcd.println("NIVEL CAIXA: ");
      lcd.print("ENCHENDO ");
      lcd.print(minutosEnchendoCaixa);

      delay(1000);
      estadoSensorNivelAlto=digitalRead(pinoSensorNivelAlto);
      
      //inserindo segurança na caixa caso o sensor dê problemas
      contandoSegundosEnchendoCaixa + 1 ;
      if(contandoSegundosEnchendoCaixa = 60){
         contandoSegundosEnchendoCaixa = 0
         minutosEnchendoCaixa + 1  ;
      while(minutosEnchendoCaixa >= 20){
        digitalWrite(releBomba, LOW);
        lcd.clear();
        lcd.println("SENSORES");
        lcd.println("COM DEFEITO");
        delay(5000);
        }   
      }
      //----------------------------------------------------- 
    
    }
    lcd.clear();
    lcd.println("NIVEL CAIXA: ");
    lcd.println("CHEIA");
    digitalWrite(releBomba, LOW); 
    digitalWrite(releCaixaDaAgua, HIGH); 

    minutosEnchendoCaixa = 0 ;
    contandoSegundosEnchendoCaixa=1 ;
    
    delay(1000);
  }else{
    lcd.clear();
    lcd.println("NIVEL CAIXA: ");

    if( estadoSensorNivelAlto){
      lcd.println("ACABANDO"); //0  
    }else{
      lcd.println("CHEIA"); //0   
      }
      delay(1000);
  }

  if( estadoSensorNivelAlto){
    lcd.clear();
    lcd.println("NIVEL CAIXA: ");
  
    if( estadoSensorNivelBaixo){
      lcd.println("QUASE CHEIA"); 
    }else{
      lcd.println("ACABANDO");  
    }
    delay(1000);      
  }else{
    lcd.clear();
    lcd.println("NIVEL CAIXA: ");
    lcd.println("CHEIA"); //0  desligar Rele da solenoide CAIXA
    delay(1000);
  }
}

void fazerIrrigacao(){

  int estadoSensorNivelAlto  = digitalRead(pinoSensorNivelAlto);
  for(int i=0; i < 5; i++) {
    lcd.clear();
    // Posiciona o cursor do LCD na coluna 0 linha 1
    // (Obs: linha 1 é a segunda linha, a contagem começa em 0
    lcd.setCursor(0, 1);
    // Exibe a mensagem no Display LCD:
    lcd.println("UMIDADE");
    lcd.print("NO SOLO: ");
    
    // Faz a leitura do sensor de umidade do solo
    umidadeSolo = analogRead(pinoSensor);
    // Converte a variação do sensor de 0 a 1023 para 0 a 100
    umidadeSolo = map(umidadeSolo, 1023, 0, 0, 100);
    // Exibe a mensagem no Display LCD:
    lcd.print(umidadeSolo);
    lcd.print(" %    ");
    // Espera um segundo
    delay(2000);
  }  
  if(umidadeSolo < limiarSeco) {
    estadoSensorNivelAlto = digitalRead(pinoSensorNivelAlto);
    while(estadoSensorNivelAlto){    
    lcd.clear();
    lcd.println("COMPLETANDO CX" ); 
    estadoSensorNivelAlto = digitalRead(pinoSensorNivelAlto);
    lcd.println(estadoSensorNivelAlto ); 
    digitalWrite(releCaixaDaAgua, LOW);  
    digitalWrite(releBomba, HIGH); 
    delay(2000);
  }
  lcd.clear();
  lcd.print("HORA DE REGAR");
  delay(5000);

  int tempo = 0; 
  while(minuto>0){
    verCaixa(); 
    digitalWrite(releCaixaDaAgua, HIGH);
    ativaRelesDeIrrigacao();  
    digitalWrite(releBomba, HIGH);  
    
    lcd.clear();
    lcd.println("IRRIGANDO!");
    lcd.println("POR"); 
    
    tempo++ ;
    
    if(tempo == 13 )  {                  
      minuto = minuto-1;
      delay(1000);
      if(releAtivado <= 11){
      //if(minuto==30 || minuto==60 || minuto==90){ 
        if(minuto==119|| minuto==118 || minuto==117 ) {
          releAtivado++;
        }
        desativaRelesDeIrrigacao();
        ativaRelesDeIrrigacao();
      }
      if(releAtivado > 11){
        releAtivado = 5;
      }
      tempo = 0 ; 
    }
    lcd.printLong(minuto);
    lcd.print(" minutos");
    lcd.print( releAtivado);
    delay(3000); //56MINUTOS     
  }
  minuto = 60;// 12= 60/5
  
  estadoSensorNivelAlto = digitalRead(pinoSensorNivelAlto);
    while(estadoSensorNivelAlto){    
      desativaRelesDeIrrigacao(); 
      lcd.clear();
      lcd.println("COMPLETANDO CX" ); 
      estadoSensorNivelAlto = digitalRead(pinoSensorNivelAlto);
      lcd.println(estadoSensorNivelAlto ); 
      digitalWrite(releCaixaDaAgua, LOW);  
      digitalWrite(releBomba, HIGH); 
      delay(2000);
    }
  }else{
    lcd.clear();
    // Posiciona o cursor do LCD na coluna 0 linha 1
    // (Obs: linha 1 é a segunda linha, a contagem começa em 0
    //lcd.setCursor(0, 1);
    // Exibe a mensagem no Display LCD:
    lcd.print("Solo Encharcado ");
    // Espera o tempo estipulado
    delay(3000);
  }
}

void desativaRelesDeIrrigacao(){
  digitalWrite(releIrrigacao1, HIGH); 
  digitalWrite(releIrrigacao2, HIGH); 
  digitalWrite(releIrrigacao3, HIGH);
  digitalWrite(releIrrigacao4, HIGH); 
  digitalWrite(releIrrigacao5, HIGH); 
  digitalWrite(releIrrigacao6, HIGH); 
  digitalWrite(releIrrigacao7, HIGH); 
}
void ativaRelesDeIrrigacao(){
  if (releAtivado == 5) {
    digitalWrite(releIrrigacao1, LOW);
  }
  else if (releAtivado == 6) {
    digitalWrite(releIrrigacao2, LOW);
  }
  else if (releAtivado == 7) {
    digitalWrite(releIrrigacao3, LOW);
  }
  else if (releAtivado == 8) {
    digitalWrite(releIrrigacao4, LOW);
  }
  else if (releAtivado == 9) {
    digitalWrite(releIrrigacao5, LOW);
  }
  else if (releAtivado == 10) {
    digitalWrite(releIrrigacao6, LOW);
  }
  else if (releAtivado == 11) {
    digitalWrite(releIrrigacao7, LOW);
  }
}


