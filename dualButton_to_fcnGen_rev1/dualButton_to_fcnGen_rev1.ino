
#include "Nextion.h"

const int ledPin =  13;  // the number of the LED pin. Also Pulse Output pin.

int ledState = HIGH;       // ledState used to set the LED
long previousMicros = 0;   // will store last time LED was updated

NexButton btRunStop = NexButton(0, 10, "btRunStop");
NexText freq = NexText(0, 7, "freq");
NexText pulseDur = NexText(0, 8, "pulseDur");
NexText signalDur = NexText(0, 9, "signalDur");
NexText msg = NexText(0, 11, "msg");

char buffer[100] = {0};

boolean runningNow = false;
boolean pulseDurationError = false;
boolean signalComplete = false;

uint32_t frequency = 0;
long period = 0;
uint32_t pulseDuration = 0;
uint32_t signalDuration = 0;
unsigned long signalStart = 0;
unsigned long currentMicros = 0; 

NexTouch *nex_listen_list[] = 
{
    &btRunStop,
    &freq,
    &pulseDur,
    &signalDur,
    &msg,
    NULL
};

void btRunStopPopCallback(void *ptr)
{
    dbSerialPrintln("bt0PopCallback");
    uint16_t len;
    uint16_t number;

    NexButton *btn = (NexButton *)ptr;
    dbSerialPrintln("bt0PopCallback");
    dbSerialPrint("ptr=");    
    dbSerialPrintln((uint32_t)ptr);
        
    if (runningNow == false){          
     
        memset(buffer, 0, sizeof(buffer));
        freq.getText(buffer, sizeof(buffer));
        frequency = atoi(buffer);
        dbSerialPrint("Frequency =");
        dbSerialPrintln(frequency); 

        memset(buffer, 0, sizeof(buffer));
        pulseDur.getText(buffer, sizeof(buffer));
        pulseDuration = atoi(buffer);
        // Multiply by 1000 for microseconds.
        pulseDuration = pulseDuration*1000;
        dbSerialPrint("Pulse Duration =");
        dbSerialPrintln(pulseDuration); 

// ADD FCN to check if pulse duration is Greater than period.
        chkPeriodvsPulseDur();

        memset(buffer, 0, sizeof(buffer));
        signalDur.getText(buffer, sizeof(buffer));
        signalDuration = atoi(buffer);
        // Multiply by 1000000 for microseconds.
        signalDuration = signalDuration*1000000;
        dbSerialPrint("Signal Duration =");
        dbSerialPrintln(signalDuration); 
           
      }else if(runningNow == true){        
        runningNow = false;
        pulseDurationError = false;        
      }

    dbSerialPrint("Running Now?:  ");
    dbSerialPrintln(runningNow);

    if (runningNow == true){ 
      msg.setText("SYSTEM RUNNING");      
    } 
    
    dbSerialPrint("pulseDurationError?:  ");
    dbSerialPrintln(pulseDurationError);
    
    if (pulseDurationError == false && runningNow == false ){
      msg.setText("SYSTEM STOPPED"); 
      btRunStop.setText("RUN");
    } 
    
    dbSerialPrint("pulseDurationError?:  ");
    dbSerialPrintln(pulseDurationError);
    
    if(pulseDurationError == true && runningNow == false ){
      msg.setText("PULSE DURATION > PERIOD ERROR"); 
      btRunStop.setText("RUN");
    }
    
}

void chkPeriodvsPulseDur(){

  // Multiply by 1000000 for microseconds.
  period = (1000000/frequency); 

  dbSerialPrint("Period:  ");
  dbSerialPrintln(period);

  if(period <  pulseDuration){
    
    pulseDurationError = true;
    runningNow = false;
     
  } else {
    runningNow = true;
    pulseDurationError = false;
    signalStart = micros();
  }

}

void freqPopCallback(void *ptr)
{
    dbSerialPrintln("freqPopCallback");
    freq.setText("");
}

void pulseDurPopCallback(void *ptr)
{
    dbSerialPrintln("pulseDurPopCallback");
    pulseDur.setText("");
}

void signalDurPopCallback(void *ptr)
{
    dbSerialPrintln("signalDurPopCallback");
    signalDur.setText("");
}

void msgPopCallback(void *ptr)
{
    dbSerialPrintln("msgPopCallback");
    msg.setText("GUTEN TAG");
}

void setup(void)
{    
     // set the digital pin as output:
    pinMode(ledPin, OUTPUT);
      
    nexInit();
    
    btRunStop.attachPop(btRunStopPopCallback, &btRunStop);
    
    freq.attachPop(freqPopCallback);
    freq.setText("");
    
    pulseDur.attachPop(pulseDurPopCallback);
    pulseDur.setText("");

    signalDur.attachPop(signalDurPopCallback);
    signalDur.setText("");

    msg.attachPop(msgPopCallback);
    msg.setText("GUTEN TAG");
    
    dbSerialPrintln("setup done"); 
}

void signalCompleted(){

  if ( runningNow == true ){
      
      runningNow = false;
      msg.setText("SIGNAL COMPLETE"); 
      btRunStop.setText("RUN");       
    
    }
  
  }

void loop() {
//
    currentMicros = micros(); 

    while ( (currentMicros - signalStart) < signalDuration ){

      // Check for input while signal is running.
      nexLoop(nex_listen_list);
   
      while  ( ( (currentMicros - previousMicros) < period ) && ( runningNow == true ) ){
     
        while ( (currentMicros - previousMicros) < pulseDuration ){
        
          ledState = HIGH; 
          digitalWrite(ledPin, ledState);  
          currentMicros = micros();  
        
        }
        
        ledState = LOW; 
        digitalWrite(ledPin, ledState); 
        currentMicros = micros(); 
     
     }

   previousMicros = currentMicros;

  } 

  signalCompleted();
  
  nexLoop(nex_listen_list);
}


