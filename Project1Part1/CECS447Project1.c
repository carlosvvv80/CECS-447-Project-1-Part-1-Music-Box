// Carlos Verduzco
// CECS 447
// February 16 2022
// CECS447ProjectPart1
// Embedded system able to play 3 different songs
// SW1 used to turn piano on/off, resets back to the first song
// SW2 used to stop playing song and play the next song

// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
#include "tm4c123gh6pm.h"
#include "CECS447Project1.h"
// Index for notes used in music scores
#define C5 0+7
#define D5 1+7
#define E5 2+7
#define F5 3+7
#define G5 4+7
#define A5 5+7
#define B5 6+7
#define C6 0+2*7
#define D6 1+2*7
#define E6 2+2*7
#define F6 3+2*7
#define G6 4+2*7
#define A6 5+2*7
#define PAUSE 255				// assume there are less than 255 tones used in any song
#define MAX_NOTES 50  // assume maximum number of notes in any song is 50. You can change this value if you add a long song.
#define SPEAKER (*((volatile unsigned long *)0x40004020)) // define SPEAKER connects to PA3: 
#define Speaker_Toggle     			0x00000008  // Used to toggle the speaker output
#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode

// 2. Declarations Section
unsigned int play_flag = 0;
unsigned int change_song = 0;
unsigned int i;
unsigned int current_song_index = 0;

const unsigned long Tone_Tab[] =
// initial values for three major notes for 16MHz system clock
// Note name: C, D, E, F, G, A, B
// Offset:0, 1, 2, 3, 4, 5, 6
{30534,27211,24242,22923,20408,18182,16194, // C4 major notes
 15289,13621,12135,11454,10204,9091,8099, // C5 major notes
 7645,6810,6067,5727,5102,4545,4050};// C6 major notes

 NTyp Score_Tab[][MAX_NOTES] =
// score table for Twinkle Twinkle Little Stars
{{C6,4,C6,4,G6,4,G6,4,A6,4,A6,4,G6,8,F6,4,F6,4,E6,4,E6,4,D6,4,D6,4,C6,8, 
 G6,4,G6,4,F6,4,F6,4,E6,4,E6,4,D6,8,G6,4,G6,4,F6,4,F6,4,E6,4,E6,4,D6,8, 
 C6,4,C6,4,G6,4,G6,4,A6,4,A6,4,G6,8,F6,4,F6,4,E6,4,E6,4,D6,4,D6,4,C6,8,0,0},

// score table for Happy Birthday
   {G5,2,G5,2,A5,4,G5,4,C6,4,B5,4,
  PAUSE,4,  G5,2,G5,2,A5,4,G5,4,D6,4,C6,4,
  PAUSE,4,  G5,2,G5,2,G6,4,E6,4,C6,4,B5,4,A5,8, 
	PAUSE,4,  F6,2,F6,2, E6,4,C6,4,D6,4,C6,8,0,0},

// score table for Mary Had A Little Lamb
{E6, 4, D6, 4, C6, 4, D6, 4, E6, 4, E6, 4, E6, 8, 
 D6, 4, D6, 4, D6, 8, E6, 4, G6, 4, G6, 8,
 E6, 4, D6, 4, C6, 4, D6, 4, E6, 4, E6, 4, E6, 8, 
 D6, 4, D6, 4, E6, 4, D6, 4, C6, 8, 0, 0 }};

static unsigned long c_note;
// 3. Function Prototypes
extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);
extern void DisableInterrupts(void);
void Delay(void); 

// 4. Subroutines Section
// MAIN: Mandatory for a C Program to be executable

int main(void){
	
  DisableInterrupts();
  Music_Init();
	SysTick_Init();
  Switch_Init();
  EnableInterrupts();  // SysTick uses interrupts
  
  while(1){
    if (is_music_on()) {
		  play_a_song(Score_Tab[current_song_index]);
    }
    else {
      WaitForInterrupt();
    }
  }
}

void play_a_song(NTyp notetab[])
{
	unsigned char i=0, j;
	unsigned int current_song = current_song_index;
	while ((notetab[i].delay) && (play_flag != 0) && (current_song == current_song_index)) {
		if (notetab[i].tone_index==PAUSE) // index = 255 indicate a pause: stop systick
			SysTick_stop(); // silence tone, turn off SysTick timer
		else {
			SysTick_Set_Current_Note(Tone_Tab[notetab[i].tone_index]);
			SysTick_start();
		}
		
		// tempo control: 
		// play current note for duration 
		// specified in the music score table
		for (j=0;j<notetab[i].delay;j++) 
			Delay();
		
		SysTick_stop();
		i++;  // move to the next note
	}
	
	// pause after each play
	for (j=0;j<3;j++) 
		Delay();
}

unsigned char is_music_on(void)
{
  return 1;
}

// Make PA3 an output to the speaker, enable digital I/O, ensure alt. functions off
void Music_Init(void){ 
	SYSCTL_RCGC2_R |= 0x00000001; // (a) activate clock for port A
	while ((SYSCTL_RCGC2_R & 0x00000001) != 0x00000001){}
		
	GPIO_PORTA_AMSEL_R &= ~0x08;  			// disable analog functionality on PA3
	GPIO_PORTA_PCTL_R &= ~0x0000F000; 	// configure PA3 as GPIO
	GPIO_PORTA_DIR_R |= 0x08;       		// PA3 output  
	GPIO_PORTA_AFSEL_R &= ~0x08;  			// disable alt funct on PA3
  GPIO_PORTA_DEN_R |= 0x08;     			// enable digital I/O on PA3
}

// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	unsigned long volatile time;
  time = 727240*20/91;  // 0.1sec for 16MHz
//  time = 727240*100/91;  // 0.1sec for 80MHz
  while(time){
		time--;
  }
}


// Subroutine to initialize port F pins for the two onboard switches
// enable PF4 and PF0 for SW1 and SW2 respectively with falling edge interrupt enabled.
// Priority 5
// Inputs: None
// Outputs: None
void Switch_Init(void){ 
	SYSCTL_RCGC2_R |= 0x00000020; // (a) activate clock for port F
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;         // allow changes to PF4,0
  GPIO_PORTF_DIR_R &= ~0x11;    // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x11;  //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x11;     //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R &= ~0x000F000F; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x11;  //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;    //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF0FFFFF)|0x00800000; // (g) bits:23-21 for PORTF, set priority to 5
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC //
}

// ISR for PORTF
void GPIOPortF_Handler(void){
	for(i=0;i<=200000;i++){}//delay for button press to stabilize
	if(GPIO_PORTF_RIS_R&0x10){  // SW1 pressed
    GPIO_PORTF_ICR_R = 0x10;  // acknowledge interrupt
		if(play_flag==0) {
			play_flag = 1; //music starts
		}
		else if(play_flag==1) {
			play_flag = 0; //music stops
			current_song_index = 0;
		}
	}
	if(GPIO_PORTF_RIS_R&0x01){  // SW2 pressed
    GPIO_PORTF_ICR_R = 0x01;  // acknowledge interrupt
		if(play_flag==1) {
			current_song_index = (current_song_index + 1)%3;
		}
	}
}

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
}

void SysTick_start(void)
{
  NVIC_ST_RELOAD_R = c_note;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+
	                 NVIC_ST_CTRL_CLK_SRC+NVIC_ST_CTRL_INTEN;
}
void SysTick_stop(void)
{
	  NVIC_ST_CTRL_R = 0;
}

void SysTick_Set_Current_Note(unsigned long n_value)
{
	  c_note = n_value-1;// reload value
}

// Interrupt service routine, 
// frequency is determined by current tone being played
void SysTick_Handler(void){
	NVIC_ST_CTRL_R = 0;
	SPEAKER ^= Speaker_Toggle; // inverse bit 3
	NVIC_ST_RELOAD_R = c_note;// reload current initial value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+
	                 NVIC_ST_CTRL_CLK_SRC+NVIC_ST_CTRL_INTEN;
	}

