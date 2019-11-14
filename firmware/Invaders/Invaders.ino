#include <FastLED.h>

#define DATA_PIN 16   
#define CLK_PIN 15  
#define COLOR_ORDER BGR
#define CHIPSET APA102
#define NUM_LEDS 136
#define BRIGHTNESS 10
#define FIRE_RATE 250
#define BEAM_SPEED 50
#define AGGRESSOR_X_SPEED 300
#define AGGRESSOR_Y_SPEED 2000
#define AGGRESSOR_FIRE_RATE 500
#define INPUT_DEBOUNCE 30
#define ANIMATION_RATE 10

CRGB leds[NUM_LEDS];

byte IDX_Invader(byte index){
  return(index+(8*(index/8)));
}

byte IDX_Beam(byte index){
  return(8+(index+(8*(index/8))));
}

byte IDX_Blaster(byte x){
  return(128 + x);  
}

const boolean alphaSplash[305] = {
0,1,1,0,0,1,1,0,0,1,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,1,1,0,
1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,
1,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,0,1,0,0,0,1,0,0,1,0,1,0,1,0,1,
1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,1,0,
1,1,1,0,1,1,1,0,1,1,1,0,1,0,0,0,1,0,1,0,1,1,1,0,0,0,1,0,1,0,1,1,0,0,1,1,0,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1
}


const byte lvl1[72] = {
    0, 0, 10, 10, 10, 10, 0, 0,
    0, 0, 11, 11, 11, 11, 0, 0,
    0, 0, 12, 12, 12, 12, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* AGGRESSOR FIELD 
 * -------------------------
 * 0 - Vacant Space
 * 1-9 - Exploding Invader 
 * 10 - Live Invader COLOR 1
 * 11 - Live Invader COLOR 2
 * 12 - Live Invader COLOR 3
 * 13 - Live Invader COLOR 4
 * 
 */
byte aggressorField[72];

 /* BEAM FIELD
  * ------------------------
  * 0 - Vacant Space
  * 10-19 - Going down
  * 20-29 - Going up
  * 
  * Beams ADD to next space so any
  * value > 29 on the beam field 
  * marks a beam-beam collision
  */
byte beamField[72];

 /* BLASTER COLUMN
  * ------------------------
  * 0 - Vacant Space
  * 1-9 - Exploding Blaster 
  */
byte blasterCol[8];

long explodeAnimation[8] = {
  0x200000,
  0x570000,
  0xab0000,
  0xff0000,
  0xff2b2b,
  0xff8282,
  0xffbdbd,
  0xffffff
};

long blasterColor[4] = {
  0xffffff,
  0xff8800,
  0xff8800,
  0xff8800
};

long aggressorColor[4] = {
  0x00ff00,
  0x00ffff,
  0x0000ff,
  0xff00ff
};

boolean fire_enable = true;
double fire_tick = 0;
double beam_tick = 0;
double aggressor_x_tick = 0;
double aggressor_y_tick = 0;
double input_tick = 0;
double animation_tick = 0;
double aggressor_fire_tick = 0;
boolean aggressor_direction = 0;

byte hue = 0;

void setup(){

  FastLED.addLeds<CHIPSET,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  randomSeed(analogRead(A0));

  level_1();

}


void loop(){

  hue++;

  if(fire_tick > FIRE_RATE){
    fire_enable = true;
    fire_tick = 0; 
  }else{
     fire_tick++;
  }

  if(aggressor_fire_tick > AGGRESSOR_FIRE_RATE){
    aggressorFire();
    aggressor_fire_tick = 0; 
  }else{
     aggressor_fire_tick++;
  }  

  if(beam_tick > BEAM_SPEED){
    updateBeams();
    beam_tick = 0; 
  }else{
     beam_tick++;
  }
  
  if(aggressor_x_tick > AGGRESSOR_X_SPEED){
    scootInvaders();  
    aggressor_x_tick = 0; 
  }else{
    aggressor_x_tick++;
  }

  if(aggressor_y_tick > AGGRESSOR_Y_SPEED){
    dropInvaders();  
    aggressor_y_tick = 0; 
  }else{
    aggressor_y_tick++;
  }  

  if(input_tick > INPUT_DEBOUNCE){
    checkInput();
    input_tick = 0; 
  }else{
     input_tick++;
  }  

 if(animation_tick > ANIMATION_RATE){
    updateAnimations();
    animation_tick = 0; 
  }else{
     animation_tick++;
  }    
  
  checkCollision();
  updateLEDs();

  checkGamestate();

  FastLED.show();

}

void level_1(){

  for(byte i = 0; i < 72; i++){
    beamField[i] = 0;
  }

  for(byte i = 0; i < 8; i++){
    blasterCol[i] = 0;
  }

  blasterCol[3] = 10;

  memcpy(aggressorField, lvl1, 72);

}

void checkCollision(){

  for(byte i = 0; i < 72; i++){

    // Check for beam-beam collision
    // both beams eliminated
    if(beamField[i] > 29){
      beamField[i] = 0;
    }

    // Check for beam-aggressor collision
    // beam eliminated, aggressor set to exploding
    if(aggressorField[i] > 9 && beamField[i] > 19){
      beamField[i] = 0;
      aggressorField[i] = 9;
    }

    // Check for beam-blaster collision
    // beam eliminated, blaster set to exploding
    if(i > 63 && beamField[i] > 0 && blasterCol[i%8] > 9){
      beamField[i] = 0;
      blasterCol[i%8] = 9;      
    }

    // Check for aggressor-blaster collision
    // aggressor and blaster set to exploding
    if(i > 63 && aggressorField[i] > 9 && blasterCol[i%8] > 9){
       aggressorField[i] = 9;
       blasterCol[i%8] = 9; 
    }    
    
  }

}

void checkInput(){

  // LEFT 14
  // RIGHT 10
  // FIRE 11
  // FIRE 9

  if(digitalRead(14)==0){
    byte blasterPos;
    for(byte pos = 0; pos < 8; pos++){
      if(blasterCol[pos]!=0){
        blasterPos = pos;
      }
    }
    if(blasterPos>0){
      blasterCol[blasterPos-1]=blasterCol[blasterPos];
      blasterCol[blasterPos] = 0;
    }
  }
  if(digitalRead(10)==0){
    byte blasterPos;
    for(byte pos = 0; pos < 8; pos++){
      if(blasterCol[pos]!=0){
        blasterPos = pos;
      }
    }
    if(blasterPos<7){
      blasterCol[blasterPos+1]=blasterCol[blasterPos];
      blasterCol[blasterPos] = 0;
    }    
  }
  if(digitalRead(11) == 0 || digitalRead(9) == 0){
    if(fire_enable){
        byte blasterPos;
        for(byte pos = 0; pos < 8; pos++){
          if(blasterCol[pos]!=0){
            blasterPos = pos;
          }
        }
      beamField[56+blasterPos] = 20; 
      fire_enable = false;
    }
  }
  
}

void updateBeams(){
  
  for(byte pos = 0; pos<72; pos++){
    if(beamField[pos] > 18 && beamField[pos] < 30){
      //going up
        if(pos>7){
          beamField[pos-8] = beamField[pos-8] + beamField[pos];
          beamField[pos] = 0;   
        }else{
          beamField[pos] = 0;      
        }
      }
  }


  for(byte pos = 71; pos>=0; pos--){
    
       if(beamField[pos] > 0 && beamField[pos] < 19){
        //going down
          if(pos<64){
            beamField[pos+8] = beamField[pos+8] + beamField[pos];
            beamField[pos] = 0;
          }else{
            beamField[pos] = 0;
          }
       }
  }

}

void updateAnimations(){

    // update frame numbers
  for(byte pos = 0; pos<64; pos++){
    if(aggressorField[pos]>0 && aggressorField[pos]<10){
      aggressorField[pos]--;
    }
    if(pos<8){
      if(blasterCol[pos]>0 && blasterCol[pos]<10){
        blasterCol[pos]--;
      }
    }
  }
  
}

void updateLEDs(){

  // update FastLED Buffer
  for(byte pos = 0; pos<64; pos++){
    if(aggressorField[pos]>0 && aggressorField[pos]<10){
      leds[IDX_Invader(pos)] = explodeAnimation[aggressorField[pos]];
    }else if(aggressorField[pos]>9){
      leds[IDX_Invader(pos)] =  aggressorColor[aggressorField[pos]-10];
    }else{
      leds[IDX_Invader(pos)] = 0x000000;
    }
    if(beamField[pos]>0 && beamField[pos]<30){
      leds[IDX_Beam(pos)].setHue(hue);
    }else{
      leds[IDX_Beam(pos)] = 0x000000;
    }
    if(pos<8){
      if(blasterCol[pos]>0 && blasterCol[pos]<10){
        leds[IDX_Blaster(pos)] = explodeAnimation[blasterCol[pos]];
      }else if(blasterCol[pos]>9){
        leds[IDX_Blaster(pos)] = blasterColor[blasterCol[pos]-10];
      }else{
        leds[IDX_Blaster(pos)] = 0x000000;      
      }
    }
  }
 
}

void scootInvaders(){

  if(aggressor_direction){

    boolean edge_detect = false;
    for(byte i = 0; i < 72; i++){
      if( i%8 == 0 && aggressorField[i] > 9 ){
        edge_detect = true;
      }
    }

    if(edge_detect){
      aggressor_direction = 0;
      for(byte i = 71; i > 0; i--){
        aggressorField[i] = aggressorField[i-1];
      }
      aggressorField[0] = 0;
    }else{
      for(byte i = 0; i < 71; i++){
        aggressorField[i] = aggressorField[i+1];
      }
    }
    
  }else{

    boolean edge_detect = false;
    for(byte i = 0; i < 72; i++){
      if( i%8 == 7 && aggressorField[i] > 9 ){
        edge_detect = true;
      }
    }

    if(edge_detect){
      aggressor_direction = 1;
      for(byte i = 0; i < 71; i++){
        aggressorField[i] = aggressorField[i+1];
      }      
    }else{
      for(byte i = 71; i > 0; i--){
        aggressorField[i] = aggressorField[i-1];
      }      
      aggressorField[0] = 0;
    }
    
  }
  
}

void dropInvaders(){

  for(byte i = 71; i > 7; i--){
    aggressorField[i] = aggressorField[i-8];
  }

  for(byte i = 0; i < 8; i++){
    aggressorField[i] = 0;
  }
  
}

void checkGamestate(){

  boolean aggressorsDefeated = true;
  for(byte i = 0; i < 64; i++){
    if(aggressorField[i] > 0){
      aggressorsDefeated = false;
    }
  }

  boolean playerDefeated = true;
  for(byte i = 0; i < 8; i++){
    if(blasterCol[i] > 0){
      playerDefeated = false;
    }
  }

  if(playerDefeated){

    dieAnimation();
    while(1);
    
  }else if(aggressorsDefeated){

    winAnimation();
    while(1);
    
  }
  
}

void aggressorFire(){

  for(byte col = 56; col < 64; col++){
    
    for(byte row = 0; row < 7; row++){

      if( aggressorField[col-(8*row)] > 9 ){

        if(random(0,100)>70){

          beamField[col-(8*(row-1))] = 10;
          
        }
        break;
        
      }
      
    }
    
  }
  
}

void dieAnimation(){

  for(byte frame = 8; frame >= 0; frame--){
    delay(45);
    fill_solid(leds, 136, explodeAnimation[frame]);
    FastLED.show();
  }
  fill_solid(leds, 136, 0x000000);
  FastLED.show(); 
}

void winAnimation(){

  byte rainbowHue = 0;

  while(1){
    rainbowHue++;
    fill_rainbow(leds, 136, rainbowHue, 5);
    FastLED.show();
  }
  
}
