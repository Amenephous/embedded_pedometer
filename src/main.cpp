#include <mbed.h>                               //using mbed board
#include <math.h>                               //Using math header
#include <vector>                               //Using vector header
#include "drivers/LCD_DISCO_F429ZI.h"           //Using lcd header file
#define BACKGROUND 1                            //Setting background
#define FOREGROUND 0                            //Setting background
#define _USE_MATH_DEFINES                       //To use more math functions
#define PI 3.14159265358979323846               //Setting pi


//Initialize LCD
LCD_DISCO_F429ZI lcd;
//sets the background layer 
//to be visible, transparent, and
//resets its colors to all black
void setup_background_layer(){
  lcd.SelectLayer(BACKGROUND);
  lcd.Clear(LCD_COLOR_BLACK);
  lcd.SetBackColor(LCD_COLOR_BLACK);
  lcd.SetTextColor(LCD_COLOR_GREEN);
  lcd.SetLayerVisible(BACKGROUND,ENABLE);
  lcd.SetTransparency(BACKGROUND,0x7Fu);
}

//resets the foreground layer to
//all black
void setup_foreground_layer(){
    lcd.SelectLayer(FOREGROUND);
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
}

volatile int flag = 0;
// mosi, miso, sclk
SPI spi(PF_9, PF_8, PF_7);
DigitalOut cs(PC_1);

int8_t Time_Counter = 20;
int8_t size = Time_Counter / 0.05;
int16_t Data[400];
double totalDist = 0;


void setFlag();
void setMode();
int16_t Z_data(int x);

//Initializing functions for counting Velocity and Distance
double Distance_Counter();
double Velocity_Counter();



int main() {
  cs=1;
  setMode();
  spi.format(8,3);
  spi.frequency(100000);
	Ticker t;
  t.attach(&setFlag,0.05);
  uint16_t iteration = 0;                       //Initializing a counter
  uint16_t k = 0;                               //Initializing a counter
  //uint16_t l = 0;

  LCD_DISCO_F429ZI lcd;
  //set lcd display
  lcd.SetTextColor(LCD_COLOR_WHITE);
  lcd.SetBackColor(LCD_COLOR_BLACK);
  lcd.Clear(LCD_COLOR_BLACK);
  double vel=0;                                 //Counter to store velocity data whenever sampled

  while(1) {
    if(flag){
      int16_t dataZ = Z_data(0xAC);             //Extracting angular velocity around z-axis
      Data[iteration] = dataZ;                  //Storing data in the list after extracting angular velocity around z-axis
      iteration = iteration + 1;                //Iteration counter
      k += 1;                                   //A counter for loop management

      setup_background_layer();
      setup_foreground_layer();
      
      char displaybuffer[3][60];
      if (iteration == 400){  
        double distance = Distance_Counter();
        double velocity = Velocity_Counter();

        //char displaybuffer[2][60];
        snprintf(displaybuffer[0],20,"Velocity: %4.5f\n", velocity);
        snprintf(displaybuffer[1],20,"Distance: %4.5f\n", distance);
        //snprintf(displaybuffer[2],60,"\n");
        lcd.DisplayStringAt(0, LINE(0), (uint8_t *)displaybuffer[0], LEFT_MODE);
        lcd.DisplayStringAt(0, LINE(2), (uint8_t *)displaybuffer[1], LEFT_MODE);
        //lcd.DisplayStringAt(0, LINE(4), (uint8_t *)displaybuffer[2], LEFT_MODE);

        //printf("Current Velocity is: %d centimeters per second\n", (int)velocity);
        //printf("The final distance is: %d centimeters \n", (int)distance);
        break;
      }
      else
      {
        if (k == 10)
        {
          double velocity = Velocity_Counter();                                   //Store Velocity data from extracting velocity of the person
          vel = velocity;                                                         //Storing data outside the loop to be displayed when velocity is not being sampled

          //Printing on LCD
          snprintf(displaybuffer[0],20,"Velocity: %4.5f\n", velocity);       
          lcd.DisplayStringAt(0, LINE(0), (uint8_t *)displaybuffer[0], LEFT_MODE);
          k = 0;                                                                  //resetting k counter to leave the loop for 10 iterations
        }
      }
      snprintf(displaybuffer[0],20,"Velocity: %4.5f\n", vel);                     //Printing velocity for each loop on LCD
      lcd.DisplayStringAt(0, LINE(0), (uint8_t *)displaybuffer[0], LEFT_MODE);    
       
    flag = 0;                                                                       //Breaking while loop
    }
    
  }
}

void setFlag() {          
  flag = 1;                                                                     //For while loop
}

void setMode() {                                                                //SPI is set
  cs=0;
  spi.write(0x20);
  spi.write(0xCF);
  cs=1;
}

//Function where Z_data is recorded/extracted
int16_t Z_data(int x){
  cs = 0;
  spi.write(x);
  uint8_t z_low =spi.write(0x00);
  cs = 1;
  cs = 0;
  spi.write(x+1);
  int8_t z_high =spi.write(0x00);
  cs = 1;
  int16_t z= z_high*256 + z_low;
  return z;
}



double Distance_Counter(){
  double Omega;
  double distance = 0;
  for (int i = 0; i < 400; i++)
  {
    Data[i] = abs(Data[i]);
    if (Data[i] > 5000){
      // Transform to angle velocity. 
      Omega = (0.875 * (Data[i] + 63));
      distance = distance + ((double) 0.05) * (Omega / 360) * 2 * PI * 1.015;
    }
  }
  return distance;
}

double Velocity_Counter(){
  double Omega;
  double curr_velocity = 0;
  double distance = 0;
  int j = 1;
  int leg = 85;
  for (int i = 0; i < 400; i++)
  {
    Data[i] = abs(Data[i]);
    if (Data[i] > 5000){
       
      Omega = (0.00875 * (Data[i] + 63));
      curr_velocity = leg * Omega / 60;      // make it to cm per second
      //distance = distance + ((double) 0.05) * (Omega / 360) * 2 * PI * 1.015;
      //curr_dist[j] = ((double) 0.05) * (Omega / 360) * 2 * PI * 1.015;

    }
  }
  return curr_velocity;

}
