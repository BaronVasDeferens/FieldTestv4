/*
GardenMonitor -- FieldTest3
by Skot West, Michael Nishida

Onwakup, this sketch queries a Java time server (TimerServer.java) and receives a String formatted:
(hr:min:sec:month:day:yr)  where year is the last two digits from 2000. (Yes, I remember Y2K...so what?)
This data is parsed and the time is set with the setTime command (from Time.h).

USAGE: compile and run TimeServer. TimeServer will announce its IP. THIS IP MUST BE CODED INTO
THE "server" BYTE ARRAY FOUND BELOW. This does not use NTP.

Next, measurements are taken on a regaul basis (based on ambient light levels) and transmitted. If wifi
is unavailable at time of transmission, the data is stored in SD and re-transmission is tried later. 
Upon successful data transmission, the data backlog (datalog.txt) is cleared.

*/

#include <SPI.h>         
#include <WiFi.h>
#include <SD.h>
#include <Time.h>


//*** SYSTEMS PARAMETERS ***
const byte ZONE = 1;              //which area is monitored

boolean SD_OK = false;
boolean WIFI_OK = false;
boolean EVERYTHINGS_COOL = false;

boolean time_initialized = false;
time_t time;

//*** SENSORS ***
//int photoPin = A5;
//int thermoPin = A4;
//int moisturePin = A3;

//*** SD CARD ***
//int SDCardPin = 10;
//const int chipSelect = 4;

//*** DATA ***
String dataString;

//*** NETWORK ***

int networkRetries = 0;
const int MAX_RETRIES = 10;

//Local Wifi
WiFiClient client;
char ssid[] = "dlink"; // DOESN'T PLAY WELL WITH THIS ACCESS POINT! 

//Time server
byte server[] = {192,168,0,117}; // put TimeServer IP here
int port = 123;

String data;

//*************
//*** SETUP ***
//*************

void setup() 
{
 // Open serial communication
  Serial.begin(9600);
  Serial.println("Starting...");
  
  //Initialize data pins
  pinMode(A5, INPUT);
  pinMode(A4, INPUT);
  pinMode(A3, INPUT);
  
  //Status LED
  pinMode(2,OUTPUT);

  //Initialize SD card
  pinMode(10, OUTPUT);
  if (SD.begin(4)) 
  {
    SD_OK = true;
  }

  //If the "/data" directory does not exist, create it
  if (SD_OK)
  {
    if (!SD.exists("data"))
    {
      SD.mkdir("data");  
    }
   else\
    Serial.println("directory data exists"); 
  }  

    //Take a breath...
  delay(1000);
  
  //Attempt to synchronize with time server
  int connectRetries = 0;
  while ((WIFI_OK == false) && (connectRetries <= MAX_RETRIES)) 
  { 
      time_initialized = syncTime();
  
      if (time_initialized == true)
      {
        WIFI_OK = true; 
      }
      
      else
        connectRetries++;
  }
}

//******************
// *** MAIN LOOP ***
//******************

void loop()
{ 

    data = collectData();
    writeDataToSD(data);
    delay(1000);
    //publishLog();

}//loop




//*** SYNC TIME
//Connects to a remote time server, acquires a string, and uses it to syncronize the arduino clock.
boolean syncTime()
{
 if (activateWiFi())
 { 
  
  String timeString = NULL; 
  
  client.connect(server,port);
  delay(1000);

    if (client.available())
         {  
           timeString = client.readString();
           
           //Check for String population
           if (timeString != NULL)
           {
               client.stop();  
 
               //setTime(hr,min,sec,day,month,yr)
               setTime(timeString.substring(0,2).toInt(), 
               timeString.substring(3,5).toInt(), 
               timeString.substring(6,8).toInt(), 
               timeString.substring(12,14).toInt(), 
               timeString.substring(9,11).toInt(), 
               (timeString.substring(15,17).toInt()+2000));

              Serial.println("Time synced OK");
              deactivateWiFi();
              return (true);
              
           }//timeString != null
          
       }//client.available()
       
 }
}

//*** COLLECT DATA
//Read sensor data, format, and return a String
String collectData()
{
         
         String dataString = "{";

	 time = now();

         dataString += "\"T\":";
         dataString += time;
         dataString += ",";
       
	 //Attach Zone
	 dataString += "\"Z\":";
	 dataString += ZONE;
         dataString += ",";
	 
	 //read LIGHT
	 dataString += "\"L\":";
	 dataString += analogRead(A5);
         dataString += ",";
	 
	 //read TEMP (needs fix)
	 dataString += "\"F\":";
         float temperature = analogRead(A4) * .004882814;
	 temperature = (((temperature - .5) * 100) * 1.8) + 32;
	 dataString += (int)temperature;
         dataString += ",";
	 
	 //read MOISTURE
	 dataString += "\"M\":";
	 dataString += analogRead(A3);

         //dataString += "}";

	 dataString += ",";

	 dataString += "\"B\":";
         //dataString += readBattery();
         dataString += "100";

         dataString += "}"; 

        Serial.println("New data : " + dataString);
        return(dataString);

       
}



//*** WRITE DATA TO SD
//Creates a unique file to store data.
boolean writeDataToSD(String data)
{
  //File root = SD.open("/data/");
  
  //The name of the data file will be the time stamp
  if (SD.exists("/data/"))
    Serial.println("/data/ is there");
  
  String fileName = "data/";
  fileName += data.substring(5,15);
  fileName += ".txt";
  
  char fileNameArray [fileName.length()+1];
  fileName.toCharArray(fileNameArray,fileName.length()+1);
  
  Serial.print("Trying to make ");
  Serial.print(fileNameArray);
  Serial.println("...");
  
  //OPEN SD CARD
   File dataFile = SD.open(fileNameArray, FILE_WRITE);
   //File dataFile = SD.open("data/derp.txt", FILE_WRITE);
   if (dataFile)
   {
     dataFile.println(data);
     dataFile.close();
     
     Serial.println("...created " + fileName);
     
     return (true);
   } 
   
   else
     return (false);
}



//*** PUBLISH LOG DATA
//Reads previously unpublished data and attempts to publish them.
//After each entry is successfully published, remove the file and read the next.
//Continue until either internet connectivity is lost OR all files are published successfully
void publishLog()
{
  
  //Root data directory
  File dataDirectory;
  File dataFile;
  dataDirectory = SD.open("/data/");
  
  boolean moreToDo = true;
  boolean hasInternet = false;

  String outputString = "";
  

  //Are there any files here? If so, attempt to connect to WiFi and read the file in...
  while ((moreToDo))
  { 
    dataFile =  dataDirectory.openNextFile();
    
    if (dataFile)
    {
        
        if (dataFile.isDirectory() != true)
        {
          Serial.println("Reading from " + (String)dataFile.name());
          //Read in the contents of the file 
           while (dataFile.available())
           {
             outputString += (char)dataFile.read();
           }
        }
        
        else
        {
          Serial.println("No more files?");
          moreToDo = false;
        }
                 
       dataFile.close();
    }
    
    else
      moreToDo = false; //quit
     
     //Moving on...
     if (outputString.length() > 0)
       moreToDo = false;
     else
     {  
       Serial.println("Read " + (String)outputString.length() + " chars from file");
       Serial.println(outputString);
      String fileName = outputString.substring(5,15) + ".txt";
         char fileNameChar [fileName.length() +1];
         fileName.toCharArray(fileNameChar, fileName.length()+1);
         SD.remove(fileNameChar);
         
         Serial.println("Deleted " + fileName);
     }
   
  }
       
}  



//*** ACTIVATE WIFI
boolean activateWiFi()
{
    //Serial.println("Activating WiFi...");
  
    int status = WL_IDLE_STATUS;
  
    while (status != WL_CONNECTED) 
    {
      status = WiFi.begin(ssid);
    }
  
    if (status == WL_CONNECTED)
    {
      return (true); 
    }
    else
      return (false);
    
}  

//*** DEACTIVATE WIFI
void deactivateWiFi()
{
  //Serial.println("Turning off WiFi..."); 
  WiFi.disconnect();
}



