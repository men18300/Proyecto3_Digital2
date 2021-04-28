//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************


//Universidad del Valle de Guatemala
//Depto de Ingenieria Mecatronica y Electronica
//Prof. Pablo Mazariegos y Kurt Keller
//Proyecto 3: Super Smash Bros
//Santiago Fernandez 18171
//Diego Mencos 18300

#include <SPI.h>
#include <SD.h>
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);


//Creadas para el juego especificamente
void LCD_FondoSD(char name[]);
void ataque_jug(int x, int y, int atacador);
void porcentaje_vida(int jug, int vida);
void animacionGolpe(int jugador, int golpeo);



//Variables que se encuentran en la memoria FLASH
extern uint8_t LinkSalto[];
extern uint8_t LinkEspada[];
extern uint8_t LinkParado[];
extern uint8_t LinkGolpeadoEscenario1[];

extern uint8_t DonkeyCorriendoEscenario1[];
extern uint8_t DonkeyAtaque1Escenario1[];

extern uint8_t DonkeyAtaque1Escenario1Corregido[];
extern uint8_t DonkeyParadoEscenario1[];
extern uint8_t DonkeyGolpeadoEscenario1[];

extern uint8_t PersonajeMiniatura[];
//extern uint8_t PersonajeGrande[];

struct control {
  const int horizontal;
  const int vertical ;
  const int ataque1 ;
};


//struct jugador {
//  int x
//  int lado
//  int  huboataque
//  int dano
//};

int x = 0; //Sirve para indicar la posicion del jugador 1 en la pantalla
int y = 0; //Sirve para indicar la posicion del jugador 1 en la pantalla

int salto = 0;
int reinicio = 0;
int ladojugador = 0; //Indica para que lado esta viendo el personaje
int ladojugador2 = 0;
int huboataque = 0;
int huboataque2 = 0;
int dano1 = 0;
int dano2 = 0;

File myFile;


//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  Serial2.begin(9600); //Serial que utilizaremos para la comunicacion con la otro tiva

  while (!Serial);
  while (!Serial2);

  //Memoria SD
  SPI.setModule(0);  //iniciamos comunicacion SPI en el modulo 0
  Serial.print("Initializing SD card...");
  pinMode(12, OUTPUT);  //Colocamos el CS del modulo SPI-0 como Output
  //Se verifica que se haya iniciado correctamente la SD
  if (!SD.begin(12)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  //************

  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);

  LCD_FondoSD("INIT.txt");

  pinMode(PUSH1, INPUT_PULLUP);
  pinMode(PUSH2, INPUT_PULLUP);


  //Primer jugador
  pinMode(PE_0, INPUT);
  pinMode(PE_3, INPUT);
  pinMode(PF_1, INPUT);





  //Para el segundo jugador

  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  pinMode(PA_7, INPUT);

  reinicio = 1;
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {

  String textoInicio = "PRESS START";
  LCD_Print(textoInicio, 75, 160, 2, 0xffff, 0x421b);
 Serial2.write(1);

  //struct control control1 = {digitalRead(PUSH1), digitalRead(PUSH2), digitalRead(PA_7), digitalRead(PF_1)};
  struct control control1 = {analogRead(PE_3), analogRead(PE_0), digitalRead(PF_1)};
  struct control control2 = {analogRead(A9), analogRead(A8),  digitalRead(PA_7)};


  if ((control1.ataque1 == LOW || control2.ataque1 == LOW) && reinicio == 1) {

    
    reinicio = 0;
    LCD_FondoSD("ESC1.txt");


    for (int z = 0; z < 100; z++) {
      delay(5);
      int anim = (z / 11) % 8;
      LCD_Sprite(100, 68, 40, 40, PersonajeMiniatura, 8, anim, 0, 0);
      //   LCD_Sprite(46, 68, 102, 102, PersonajeGrande, 8, 0, 0, 0);
    }
    LCD_Sprite(100, 68, 40, 40, PersonajeMiniatura, 8, 0, 0, 0);
    //  LCD_Sprite(46, 68, 102, 102, PersonajeGrande, 8, 0, 0, 0);
    String textoInicio = "VS";
    LCD_Print(textoInicio, 151, 80, 1.2, 0xffff, 0x0002);
    delay(1000);

    for (int z = 0; z < 100; z++) {
      delay(5);
      int anim = (z / 11) % 8;
      LCD_Sprite(175, 68, 40, 40, PersonajeMiniatura, 8, anim, 0, 0);
      //   LCD_Sprite(46, 68, 102, 102, PersonajeGrande, 8, 0, 0, 0);
    }
    LCD_Sprite(175, 68, 40, 40, PersonajeMiniatura, 8, 1, 0, 0);
    //  LCD_Sprite(46, 68, 102, 102, PersonajeGrande, 8, 0, 0, 0);

    delay(500);


    LCD_FondoSD("ESC1.txt");

    LCD_Sprite(50, 180, 40, 40, PersonajeMiniatura, 8, 0, 0, 0);
    LCD_Sprite(195, 180, 40, 40, PersonajeMiniatura, 8, 1, 0, 0);


    ///Limites de personajes
    x = 18;
    y = 247;


    ladojugador2 = 1;

    //Reseteamos barra de salud
    porcentaje_vida(0, 1);

    
    

    while (reinicio != 1) {

//Comando para la musica
      Serial2.write(3);

      Serial.println (x);
      Serial.println(y);


      struct control control1 = {analogRead(PE_3), analogRead(PE_0), digitalRead(PF_1)};
      struct control control2 = {analogRead(A9), analogRead(A8), digitalRead(PA_7)};

      //Para el primer jugador, el de la izquierda
      if (control1.horizontal <= 1000 && x != y - 25) {

        if (x < 283) {
          x+=2;
          int anim = (x / 11) % 8;
          LCD_Sprite(x, 103, 35, 48, LinkCorriendo, 6, anim, 0, 0);
          V_line( x - 1, 103, 48, 0x0002);
          V_line( x - 2, 103, 48, 0x0002);
          ladojugador = 2;

        }
        else {
          int anim = (x / 11) % 8;
          LCD_Sprite(x, 103, 35, 48, LinkCorriendo, 6, anim, 0, 0);
          V_line( x - 1, 103, 48, 0x0002);
          V_line( x - 2, 103, 48, 0x0002);
          ladojugador = 2;

        }


      }
      //Para el primer jugador yendo a la derecha
      else if (control1.horizontal >= 3000) {


        if (x > 18) {
          int anim = (x / 11) % 8;
          x-=2;
          LCD_Sprite(x, 103, 35, 48, LinkCorriendo, 6, anim, 1, 0);
          V_line( x + 35, 103, 48, 0x0002);
          V_line( x + 36, 103, 48, 0x0002);
          ladojugador = 1;
        }

        else {
          int anim = (x / 11) % 8;
          LCD_Sprite(x, 103, 35, 48, LinkCorriendo, 6, anim, 1, 0);
          V_line( x + 35, 103, 48, 0x0002);
          V_line( x + 36, 103, 48, 0x0002);
          // ladojugador = 1;
          ladojugador = 1;
        }


      }

      //Cuando el primer jugador quiere saltar
      else if (control1.vertical <= 1000) {
        //Para arriba
        salto = 0;
        for (int y1 = 0; y1 < 50; y1++) {
          delay(5);
          int anim = (y1 / 11) % 8;
          salto++;
          LCD_Sprite(x, 77 - salto, 31, 74, LinkSalto, 3, anim, 0, 0);
          H_line( x, 77 - salto + 74, 31, 0x0002);
        }
        int alturafinal = 0;
        alturafinal = 77 - salto;
        salto = 0;

        for (int y1 = 0; y1 < 32; y1++) {
          delay(5);
          int anim = (y1 / 11) % 8;
          salto++;

          LCD_Sprite(x, alturafinal + salto, 31, 74, LinkSalto , 3, anim, 0, 0);
          H_line( x, alturafinal + salto - 1, 31, 0x0002);
        }
        salto = 0;
        for (int i = 0; i < 50; i++) {
          H_line( x, 103 - i, 31, 0x0002);
        }
      }


      ///No sirve el boton de este push, por lo que lo haremos con la accion para abajo
     // else if (control1.vertical >= 3000) {
else if (control1.ataque1 == LOW) {

        ataque_jug( x,  y, 1 );
        huboataque = 1;

      }


      //Cuando esta parado el primer jugador
      else {
        if (ladojugador == 1) {
          int anim = (x / 11) % 8;
          //  LCD_SpriteSD(x, 120, 22, 48, "LINKP.txt", 2, anim, 0, 0);
          LCD_Sprite(x, 103, 22, 48, LinkParado, 2, 0, 1, 0);

          if (huboataque == 1) {
            huboataque = 0;
            for (int i = 22; i < 60; i++) {
              V_line( x + i, 103, 48, 0x0002);
              V_line( x - i, 103, 48, 0x0002);
              H_line( x + 23, 152, 65, 0x0002);
            }
          }

          else {
            for (int i = 22; i < 36; i++) {
              V_line( x + i, 103, 48, 0x0002);
              H_line( x + 23, 151, 65, 0x0002);
            }
          }

        }
        else { //Para Jugador 1 viendo hacia la derecha
          int anim = (x / 11) % 8;
          LCD_Sprite(x, 103, 22, 48, LinkParado, 2, 0, 0, 0);

          if (huboataque == 1) {
            huboataque = 0;
            for (int i = 22; i < 140; i++) {
              V_line( x + i, 103, 48, 0x0002);
              V_line( x - i + 22, 103, 48, 0x0002);
              H_line( x + 23, 151, 90, 0x0002);
            }

          }

          else {
            for (int i = 22; i < 36; i++) {
              V_line( x + i, 103, 48, 0x0002);
              H_line( x + 23, 151, 65, 0x0002);
            }
          }
        }
      }


      //////////////////////////////////////////
      //SEGUNDO JUGADOR////

      //Segundo Jugador al lado izquierdo
      if (control2.horizontal >= 3000 && y != x + 25) {

        if (y > 4) {
          int anim = (y  / 11) % 8;
          y --;
          LCD_Sprite(y , 106, 71, 45, DonkeyCorriendoEscenario1, 5, anim, 1, 0);
          V_line( y  + 71, 103, 45, 0x0002);
          H_line( y , 106, 31, 0x0002);

          ladojugador2 = 1;
        }
        else {
          int anim = (y / 11) % 8;
          LCD_Sprite(y, 106, 71, 45, DonkeyCorriendoEscenario1, 5, anim, 1, 0);
          V_line( y  + 71, 103, 45, 0x0002);
          H_line( y , 106, 31, 0x0002);

          ladojugador2 = 1;

        }



      }

      //  Segundo Jugador al lado derecho
      else if (control2.horizontal <= 1000 && y <= 247) {
        if (y < 247) {
          y++;
          int anim = (y / 11) % 8;
          LCD_Sprite(y, 106, 71, 45, DonkeyCorriendoEscenario1, 5, anim, 0, 0);
          V_line( y - 1, 103, 45, 0x0002);
          H_line( y, 106, 31, 0x0002);
          //    V_line( x - 1, 103, 48, 0x0002);
          ladojugador2 = 2;
        }
        else {

          int anim = (y / 11) % 8;
          LCD_Sprite(y, 106, 71, 45, DonkeyCorriendoEscenario1, 5, anim, 0, 0);
          V_line( y - 1, 103, 45, 0x0002);
          H_line( y, 106, 31, 0x0002);
          //    V_line( x - 1, 103, 48, 0x0002);
          ladojugador2 = 2;

        }



      }


      else if (control2.ataque1 == LOW) {

        ataque_jug( x,  y, 2);
        huboataque2 = 1;

      }


      //////////////////////////////////////
      //CUANDO NO SE APACHA NADA CON DONKEY
      ////////////////////////////////////////

      else {


        //     if (ladujugar) //Para cuandoe estan muy cerca chekear que y-x sea mayor a 35


        if (ladojugador2 == 1) {

          LCD_Sprite(y, 105, 43, 45, DonkeyParadoEscenario1, 3, 0, 1, 0);
          for (int i = 43; i < 70; i++) {
            V_line( y  + i, 103, 48, 0x0002);
            //H_line( x + 23, 151, 65, 0x0002);
          }

        }
        if (ladojugador2 == 2) {
          LCD_Sprite(y, 105, 43, 45, DonkeyParadoEscenario1, 3, 0, 0, 0);
          for (int i = 43; i < 70; i++) {
            V_line( y + i, 103, 48, 0x0002);
            //H_line( x + 23, 151, 65, 0x0002);
          }

        }
      }
    }
  }
}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y + i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }


  }
  digitalWrite(LCD_CS, HIGH);
}


void LCD_FondoSD(char name[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239);
  myFile = SD.open(name);
  char a;
  char C[2];
  int n;
  if (myFile) { //Si se logro abrir de manera correcta
    // read from the file until there's nothing else in it:
    while (myFile.available()) { //Se va leyendo cada caracter del archivo hasta que ya se hayan leido todos
      n = 0;
      while (1) {
        a = myFile.read();
        if ((a == ',') | (myFile.available() == 0)) {
          break;
        }
        else {
          C[n] = a;
          n++;
        }
      }
      uint8_t num = (uint8_t)strtol(C, NULL, 16);
      LCD_DATA(num);
    }
    // close the file:
    myFile.close(); //Se cierra el archivo
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening Hola.txt");
  }
  digitalWrite(LCD_CS, HIGH);
}

void ataque_jug(int x, int y, int atacador) {

  if (atacador == 1) {
    if (x == y - 25) {
      dano2++;
      animacionGolpe(1, 1);
      porcentaje_vida(2, dano2);
    }
    else {
      animacionGolpe(1, 0);
    }
  }

  else {//Se supone atacador es el segundo
    if (y == x + 25) {
      dano1++;
      animacionGolpe(2, 1);
      porcentaje_vida(1, dano1);
    }

    else {
      animacionGolpe(2, 0);
    }
  }
}

void porcentaje_vida(int jug, int vida) {

  if (jug == 0) { //Solo para resetear las barras
    FillRect(96, 197, 65, 10, 0x00); //Negro fondo
    FillRect(240, 197, 65, 10, 0x00);//Negro fondo
    FillRect(96, 197, 65, 10, 0x06E2);//Verde
    FillRect(240, 197, 65, 10, 0x06E2);//Verde

  }

  else if (jug == 1) {
    FillRect(96, 197, 65, 10, 0x00); //Negro fondo
    if (vida == 1) {
      FillRect(96, 197, 55, 10, 0x06E2);//Verde
    }
    else if (vida == 2) {
      FillRect(96, 197, 45, 10, 0x06E2);
    }
    else if (vida == 3) {
      FillRect(96, 197, 35, 10, 0xF720); //Amarillo
    }
    else if (vida == 4) {
      FillRect(96, 197, 25, 10, 0xF720);
    }
    else if (vida == 5) {
      FillRect(96, 197, 15, 10, 0xB800); //Rojo
    }
    else if (vida == 6) {
      FillRect(96, 197, 1, 10, 0xB800);

      dano1 = 0;
      dano2 = 0;
      reinicio = 1;

    }
    else {}

  }

  else if (jug == 2) {
    FillRect(240, 197, 65, 10, 0x00);//Negro fondo
    if (vida == 1) {
      FillRect(240, 197, 55, 10, 0x06E2);//Verde
    }
    else if (vida == 2) {
      FillRect(240, 197, 45, 10, 0x06E2);
    }
    else if (vida == 3) {
      FillRect(240, 197, 35, 10, 0xF720); //Amarillo
    }
    else if (vida == 4) {
      FillRect(240, 197, 25, 10, 0xF720);
    }
    else if (vida == 5) {
      FillRect(240, 197, 15, 10, 0xB800); //Rojo
    }
    else if (vida == 6) {
      FillRect(240, 197, 1, 10, 0xB800);

      dano1 = 0;
      dano2 = 0;
      reinicio = 1;

    }
    else {}

  }

}

void animacionGolpe(int jugador, int golpeo) {
  if (jugador == 1) {
    if (golpeo == 1) {
      if (ladojugador == 1) {
        for (int z = 0; z < 5; z++) {
          V_line( y - z, 103, 48, 0x0002);
        };

        for (int z = 0; z < 60; z++) {
          delay(5);
          int anim = (z / 11) % 8;
          LCD_Sprite(x - 40, 103, 103, 48, LinkEspada, 4, anim, 1, 0);
          LCD_Sprite(y, 103, 52, 45, DonkeyGolpeadoEscenario1, 6, anim, 1, 0);

        }

      }

      else {
        for (int z = 0; z < 5; z++) {
          V_line( y + 40 - z, 103, 48, 0x0002);
        };
        for (int z = 0; z < 60; z++) {
          delay(5);
          int anim = (z / 11) % 8;
          LCD_Sprite(x - 40, 103, 103, 48, LinkEspada, 4, anim, 0, 0);
          LCD_Sprite(y + 40, 103, 52, 45, DonkeyGolpeadoEscenario1, 6, anim, 1, 0);
        }

      }
    }
    else {
      if (ladojugador == 1) {
        for (int y = 0; y < 60; y++) {
          delay(5);
          int anim = (y / 11) % 8;
          LCD_Sprite(x-40, 103, 103, 48, LinkEspada, 4, anim, 1, 0);
        }

      }

      else {
        for (int y = 0; y < 60; y++) {
          delay(5);
          int anim = (y / 11) % 8;
          LCD_Sprite(x, 103, 103, 48, LinkEspada, 4, anim, 0, 0);
        }
      }


    }

  }

  else { //Para el segundo jugador

    if (golpeo == 1) {

      if (ladojugador2 == 1) {

        for (int z = 0; z < 88; z++) {
          delay(1);
          int anim = (z / 11) % 8;
          LCD_Sprite(y, 103, 46, 45, DonkeyAtaque1Escenario1, 8, anim, 1, 0);
          // LCD_Sprite(y, 103, 53, 52, DonkeyAtaque1Escenario1Corregido, 8, anim, 0, 0);
          // LCD_Sprite(x, 103, 53, 48, LinkGolpeadoEscenario1, 5, anim, 0, 0);
        }

        String textoInicio = "PUM!";
        LCD_Print(textoInicio, y, 80, 2, 0xffff, 0x0002);
        // delay(1000);

        for (int z = 0; z < 50; z++) {
          delay(1);
          int anim = (z / 11) % 8;
          LCD_Sprite(x - 20, 103, 53, 48, LinkGolpeadoEscenario1, 5, anim, 0, 0);

        }

      }

      else { //ladojugador2 es igual a 2
        for (int z = 0; z < 88 ; z++) {
          delay(1);
          int anim = (z / 11) % 8;
          LCD_Sprite(y, 103, 46, 45, DonkeyAtaque1Escenario1, 8, anim, 0, 0);
          // LCD_Sprite(y, 103, 53, 52, DonkeyAtaque1Escenario1Corregido, 8, anim, 0, 0);
          LCD_Sprite(x, 103, 53, 48, LinkGolpeadoEscenario1, 5, anim, 1, 0);
        }



      }
    }
    else { //No hay golpe
      if (ladojugador2 == 1) {

        for (int z = 0; z < 88; z++) {
          delay(1);
          int anim = (z / 11) % 8;
          LCD_Sprite(y, 103, 46, 45, DonkeyAtaque1Escenario1, 8, anim, 1, 0);
          // LCD_Sprite(y, 103, 53, 52, DonkeyAtaque1Escenario1Corregido, 8, anim, 0, 0);
        }

        String textoInicio = "PUM!";
        LCD_Print(textoInicio, y, 80, 2, 0xffff, 0x0002);

      }

      else { //ladojugador2 es igual a 2
        for (int z = 0; z < 88 ; z++) {
          delay(1);
          int anim = (z / 11) % 8;
          LCD_Sprite(y, 103, 46, 45, DonkeyAtaque1Escenario1, 8, anim, 0, 0);
         // LCD_Sprite(y, 103, 53, 52, DonkeyAtaque1Escenario1Corregido, 8, anim, 0, 0);
        }
      }

    }

    delay(80);
    FillRect(y, 80, 60, 50, 0x0002);

  }



}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//INFORMACION DE PERSONAJES
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//Link: (altura-ancho-#sprite)
//Corriendo;
//Parado: (48-22-2)
//Salto:
//Ataque1:
//Golpeado (48-53-5)

/////////////////////////////////
//Donkey: (altura-ancho-#sprite)
//Corriendo;
//Parado:
//Salto:
//Ataque1:(45-46-8)
//Golpeado(45-52-6)

/////////////////////////////////////
//Otros
//PersonaGrande: 102-102-8

/////////////////////////////////////
//Limite de personajes en patalla
////////////////////////////////////
//Escenario 1
//Donkey--> y=4-247
//Link---->18-283
