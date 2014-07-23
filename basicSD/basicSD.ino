#include <SPI.h>         
#include <SD.h>


boolean SD_OK = false;
void setup() 
{
 // Open serial communication
  Serial.begin(9600);
  Serial.println("Starting...");
  
  //Initialize SD card
  pinMode(10, OUTPUT);
  
  if (SD.begin(4)) 
  {
    SD_OK = true;
    Serial.println("SD OK");
  }
  
  delay(1000);
  
  //If the "/data" directory does not exist, create it
  if (SD_OK)
  {
    if (!SD.exists("/data"))
    {
      SD.mkdir("/data");
      Serial.println("made the dir");  
    }
    
    else
      Serial.println("directory data exists");
  }
}


void loop()
{
  char * fileName = {"/data/newfile.txt"};
  
  File newFile = SD.open(fileName,FILE_WRITE);
  
  if (newFile)
  {
     newFile.println("YAY!"); 
     newFile.close();
  }
  
  else
    Serial.println("no go");
  
  
  newFile = SD.open(fileName);
  
  if (newFile)
  {
     Serial.println("...it's there!"); 
  } 
  
}
  
