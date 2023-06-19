#include <Wire.h>
#include <INA219_WE.h>

INA219_WE ina219; // this is the instantiation of the library for the current sensor

float open_loop, closed_loop; // Duty Cycles
float va,vb,vref,iL,dutyref,current_mA,VPV; // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2,sensorValue3;  // ADC sample values declaration
float ev=0,cv=0,ei=0,oc=0; //internal signals
float Ts=0.001; //1 kHz control frequency. It's better to design the control period as integral multiple of switching period.
float kpv=0.05024,kiv=15.78,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float kpi=0.02512,kii=39.4,kdi=0; // float kpi=0.02512,kii=39.4,kdi=0; // current pid.
float u0i,u1i,delta_ui,e0i,e1i,e2i; // Internal values for the current controller
float uv_max=4, uv_min=0; //anti-windup limitation
float ui_max=50, ui_min=0; //anti-windup limitation
float current_limit;
float dv, di, dp;
float vb2, iL2;
boolean Boost_mode = 0;
boolean CL_mode = 0;
unsigned int loopTrigger;
unsigned int com_count=0;   // a variables to count the interrupts. Used for program debugging.


void setup() {

  //Basic pin setups
  
  noInterrupts(); //disable all interrupts
  pinMode(13, OUTPUT);  //Pin13 is used to time the loops of the controller
  pinMode(3, INPUT_PULLUP); //Pin3 is the input from the Buck/Boost switch
  pinMode(2, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC

  // TimerA0 initialization for control-loop interrupt.
  
  TCA0.SINGLE.PER = 999; //
  TCA0.SINGLE.CMP1 = 999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm; 

  // TimerB0 initialization for PWM output
  
  pinMode(6, OUTPUT);
  TCB0.CTRLA=TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz
  analogWrite(6,120); 

  Serial.begin(115200);   //serial communication enable. Used for program debugging.
  interrupts();  //enable interrupts.
  Wire.begin(); // We need this for the i2c comms for the current sensor
  ina219.init(); // this initiates the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  
}

 void loop() {
  if(loopTrigger) { // This loop is triggered, it wont run unless there is an interrupt
    digitalWrite(13, HIGH);   
    // set pin 13. Pin13 shows the time consumed by each control cycle. It's used for debugging.
    // Sample all of the measurements and check which control mode we are in
    sampling();
    CL_mode = digitalRead(3); // input from the OL_CL switch
    Boost_mode = digitalRead(2); // input from the Buck_Boost switch

if (Boost_mode) {
  if (CL_mode) {
    current_limit = 2;
    dv = vb - vb2;
    di = iL - iL2;
    dp = vb * iL - vb2 * iL2;
    closed_loop = MAXppt(dv, di, dp);
    closed_loop = saturation(closed_loop, 0.99, 0.05);
    pwm_modulate(closed_loop);
    vb2 = vb;
    iL2 = iL;
  } else {
   current_limit = 2;
   oc = iL - current_limit;
   if(oc > 0){
    open_loop = open_loop + 0.001;
   }
   else {
    open_loop = open_loop - 0.001;
   }
   open_loop = saturation(open_loop, 0.99, dutyref);
   pwm_modulate(open_loop);
   } 
}
  else {
  if (CL_mode) {
    pwm_modulate(1); // Disable the Buck as we are not using this mode
  } else {
    pwm_modulate(1); // Disable the Buck as we are not using this mode
  }
}

    com_count++;              //used for debugging.
    if (com_count >= 1000) {  //send out data every second.
      Serial.print("Va: ");
      Serial.print(va);
      Serial.print("\t");

      Serial.print("Vb: ");
      Serial.print(vb);
      Serial.print("\t");

      Serial.print("Inductor Current: ");
      Serial.print(iL);
      Serial.print("\t\t");

      Serial.print("Boost Mode: ");
      Serial.print(Boost_mode);
      Serial.print("\t\t");

      Serial.print("CL Mode: ");
      Serial.print(CL_mode);
      Serial.print("\n");
  
      com_count = 0;
    }

    digitalWrite(13, LOW);   // reset pin13.
    loopTrigger = 0;
  }
}



ISR(TCA0_CMP1_vect){
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_CMP1_bm; //clear interrupt flag
  loopTrigger = 1;
}
// This subroutine processes all of the analogue samples, creating the required values for the main loop

void sampling(){

  // Make the initial sampling operations for the circuit measurements
  
sensorValue0 = analogRead(A0); // sample Vb
sensorValue2 = analogRead(A2); // sample Vref
sensorValue3 = analogRead(A3); // sample Va
current_mA = ina219.getCurrent_mA(); // sample the inductor current (via the sensor chip)
sensorValue1 = analogRead(A1); // sample VPV

// Process the values so they are a bit more usable/readable
// The analogRead process gives a value between 0 and 1023
// representing a voltage between 0 and the analogue reference which is 4.096V

vb = sensorValue0 * (12400 / 2400) * (4.096 / 1023.0); // Convert the Vb sensor reading to volts
vref = sensorValue2 * (4.096 / 1023.0); // Convert the Vref sensor reading to volts
va = sensorValue3 * (12400 / 2400) * (4.096 / 1023.0); // Convert the Va sensor reading to volts
VPV = sensorValue1 * (12400 / 2400) * (4.096 / 1023.0); // Convert the VPV sensor reading to volts

  if (Boost_mode == 1){
    iL = -current_mA/1000.0;
    dutyref = saturation(sensorValue2 * (1.0 / 1023.0),0.99,0.33);
  }else{
    iL = current_mA/1000.0;
    dutyref = sensorValue2 * (1.0 / 1023.0);
  }
  
}


float saturation( float sat_input, float uplim, float lowlim){ // Saturatio function
  if (sat_input > uplim) sat_input=uplim;
  else if (sat_input < lowlim ) sat_input=lowlim;
  else;
  return sat_input;
}



void pwm_modulate(float pwm_input){ // PWM function
  analogWrite(6,(int)(255-pwm_input*255)); 
}

float MAXppt (float dv, float di, float dp){
  float MpptDuty;

  if (dp == 0){
    MpptDuty = MpptDuty;
  }
  else if (dp * dv > 0){
    MpptDuty = MpptDuty + 0.01;
  }
  else{
    MpptDuty = MpptDuty - 0.01;
  }
  return MpptDuty;
}


/*end of the program.*/
