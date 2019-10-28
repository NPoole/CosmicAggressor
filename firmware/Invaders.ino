#include <FastLED.h>

#define DATA_PIN 16   
#define CLK_PIN 15  
#define COLOR_ORDER BGR
#define CHIPSET APA102
#define NUM_LEDS 136
#define BRIGHTNESS 10
#define GAME_TICK 50
#define FIRE_RATE 5
#define BEAM_SPEED 1
#define INVADER_SPEED 20

CRGB leds[NUM_LEDS];

int IDX_Invader(int index){
  return(index+(8*(index/8)));
}

int IDX_Beam(int index){
  return(8+(index+(8*(index/8))));
}

int IDX_Blaster(int x){
  return(128 + x);  
}


const int lvl1[72] = {
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

/* INVADER FIELD 
 * -------------------------
 * 0 - Vacant Space
 * 1-9 - Exploding Invader 
 * 10 - Live Invader COLOR 1
 * 11 - Live Invader COLOR 2
 * 12 - Live Invader COLOR 3
 * 13 - Live Invader COLOR 4
 * 
 */
int invaderField[72];

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
int beamField[72];

 /* BLASTER COLUMN
  * ------------------------
  * 0 - Vacant Space
  * 1-9 - Exploding Blaster 
  */
int blasterCol[8];

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
  0xff8800,
  0xff8800,
  0xff8800,
  0xff8800
};

long invaderColor[4] = {
  0x00ff00,
  0x00ffff,
  0x0000ff,
  0xff00ff
};

boolean fire_enable = true;
boolean invader_update = false;
int fire_tick = 0;
int beam_tick = 0;
int invader_tick = 0;

byte hue = 0;

void setup(){

  FastLED.addLeds<CHIPSET,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  level_1();

}


void loop(){

  delay(GAME_TICK);

  hue++;

  if(fire_tick > FIRE_RATE){
    fire_enable = true;
    fire_tick = 0; 
  }else{
     fire_tick++;
  }

  if(beam_tick > BEAM_SPEED){
    updateBeams();
    beam_tick = 0; 
  }else{
     beam_tick++;
  }
  
  if(invader_tick > INVADER_SPEED){
    invader_update = true;
    invader_tick = 0; 
  }else{
     invader_tick++;
  }
  
  checkCollision();
  checkInput();
  updateInvaderPos();  
  updateAnimations();

  FastLED.show();

}

void level_1(){

  for(int i = 0; i < 72; i++){
    beamField[i] = 0;
  }

  for(int i = 0; i < 8; i++){
    blasterCol[i] = 0;
  }

  blasterCol[3] = 10;

  memcpy(invaderField, lvl1, 72);

}

void checkCollision(){

  for(int i = 0; i < 72; i++){

    // Check for beam-beam collision
    // both beams eliminated
    if(beamField[i] > 29){
      beamField[i] = 0;
    }

    // Check for beam-invader collision
    // beam eliminated, invader set to exploding
    if(invaderField[i] > 9 && beamField[i] > 19){
      beamField[i] = 0;
      invaderField[i] = 9;
    }

    // Check for beam-blaster collision
    // beam eliminated, blaster set to exploding
    if(i > 63 && beamField[i] > 0 && blasterCol[i%8] > 9){
      beamField[i] = 0;
      blasterCol[i%8] = 9;      
    }

    // Check for invader-blaster collision
    // invader and blaster set to exploding
    if(i > 63 && invaderField[i] > 9 && blasterCol[i%8] > 9){
       invaderField[i] = 9;
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
    int blasterPos;
    for(int pos = 0; pos < 8; pos++){
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
    int blasterPos;
    for(int pos = 0; pos < 8; pos++){
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
        int blasterPos;
        for(int pos = 0; pos < 8; pos++){
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
  
  for(int pos = 0; pos<72; pos++){
    if(beamField[pos] > 0 && beamField[pos] < 30){
      
       if(beamField[pos] < 19){
        //going down
          if(pos<56){
            beamField[pos+8] = beamField[pos+8] + beamField[pos];
            beamField[pos] = 0;
          }else{
            beamField[pos] = 0;
          }
       }else{
        //going up
          if(pos>7){
            beamField[pos-8] = beamField[pos-8] + beamField[pos];
            beamField[pos] = 0;   
          }else{
            beamField[pos] = 0;      
          }
       }
       
    }
  }
  
}

void updateAnimations(){

  // update FastLED Buffer
  for(int pos = 0; pos<64; pos++){
    if(invaderField[pos]>0 && invaderField[pos]<10){
      leds[IDX_Invader(pos)] = explodeAnimation[invaderField[pos]];
    }else if(invaderField[pos]>9){
      leds[IDX_Invader(pos)] =  invaderColor[invaderField[pos]-10];
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

  // update frame numbers
  for(int pos = 0; pos<64; pos++){
    if(invaderField[pos]>0 && invaderField[pos]<10){
      invaderField[pos]--;
    }
    if(pos<8){
      if(blasterCol[pos]>0 && blasterCol[pos]<10){
        blasterCol[pos]--;
      }
    }
  }
  
}

void updateInvaderPos(){
  }
