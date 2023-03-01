#include <asf.h>
#include <stdio.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define LED1_PIO           PIOA                 // periferico que controla o LED        
#define LED1_PIO_ID  ID_PIOA  // ID do perif�rico PIOC (controla LED)   
#define LED1_PIO_IDX       0                    // ID do LED no PIO
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)   // Mascara para CONTROLARMOS o LED

#define BUT1_PIO   PIOD
#define BUT1_PIO_ID  ID_PIOD
#define BUT1_PIO_IDX       28  
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX)

#define BUT2_PIO   PIOC
#define BUT2_PIO_ID  ID_PIOC
#define BUT2_PIO_IDX       31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX)

#define BUT3_PIO   PIOA
#define BUT3_PIO_ID  ID_PIOA
#define BUT3_PIO_IDX       19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX)

volatile char but_flag;
int delay = 400;
double freq;
char show_display[128];

void io_init(void);
void pisca_led(int n);

void but_callback(void)
{	
	but_flag = 1;
}

void but_callback2(void)
{
	but_flag = 2;
}

void but_callback3(void)
{
	but_flag = 3;
}

void pisca_led(int n){
  for (int i=0;i<n;i++){
	gfx_mono_draw_filled_rect(5, 5, (i+1)*4, 10, GFX_PIXEL_SET);
	if(but_flag == 2){
		i = n;
		but_flag = 0;
	}
	if(but_flag == 3){
		delay -= 100;
		if(delay < 100){
			delay = 100;
		}
		freq = (double) 1000 / (2 * delay);
		sprintf(show_display, "Freq: %.2fHz", freq);
		gfx_mono_draw_string(show_display, 5,16, &sysfont);
		but_flag = 0;
	}
    pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
    delay_ms(delay);
    pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
    delay_ms(delay);
  }
  gfx_mono_draw_filled_rect(5, 5, 120, 10, GFX_PIXEL_CLR);
}

void io_init(void)
{

  // Configura led
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_IDX_MASK, PIO_DEFAULT);

	pmc_enable_periph_clk(BUT1_PIO_ID);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	pmc_enable_periph_clk(BUT2_PIO_ID);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);

  // Configura interrup��o no pino referente ao botao e associa
  // fun��o de callback caso uma interrup��o for gerada
  // a fun��o de callback � a: but_callback()
  pio_handler_set(BUT1_PIO,
                  BUT1_PIO_ID,
                  BUT1_PIO_IDX_MASK,
				  PIO_IT_FALL_EDGE,
                  but_callback);
				  
  pio_handler_set(BUT2_PIO,
                  BUT2_PIO_ID,
                  BUT2_PIO_IDX_MASK,
				  PIO_IT_FALL_EDGE,
                  but_callback2);

  pio_handler_set(BUT3_PIO,
                  BUT3_PIO_ID,
                  BUT3_PIO_IDX_MASK,
				  PIO_IT_FALL_EDGE,
                  but_callback3);


  // Ativa interrup��o e limpa primeira IRQ gerada na ativacao
  pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
  pio_get_interrupt_status(BUT1_PIO);
  
  pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
  pio_get_interrupt_status(BUT2_PIO);
  
  pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
  pio_get_interrupt_status(BUT3_PIO);
  
  // Configura NVIC para receber interrupcoes do PIO do botao
  // com prioridade 4 (quanto mais pr�ximo de 0 maior)
  NVIC_EnableIRQ(BUT1_PIO_ID);
  NVIC_SetPriority(BUT1_PIO_ID, 6); // Prioridade 6
  
  NVIC_EnableIRQ(BUT2_PIO_ID);
  NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4
  
    NVIC_EnableIRQ(BUT3_PIO_ID);
  NVIC_SetPriority(BUT3_PIO_ID, 5); // Prioridade 5
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	io_init();
  // Init OLED
	gfx_mono_ssd1306_init();
	
	freq = (double) 1000 / (2 * delay);
	sprintf(show_display, "Freq: %.2fHz", freq);
	gfx_mono_draw_string(show_display, 5,16, &sysfont);
	pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
	gfx_mono_draw_rect(4, 4, 122, 12, GFX_PIXEL_SET);
  /* Insert application code here, after the board has been initialized. */
	while(1) {
			if (but_flag == 1) {
				but_flag = 0;
				delay_ms(1000);
				if (pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
					// PINO == 1 --> Borda de subida
					delay -= 100;
					if(delay < 100){
						delay = 100;
					}
				} else {
					// PINO == 0 --> Borda de descida
					delay += 100;
					if(delay > 1000){
						delay = 1000;
					}
				}
				freq = (double) 1000 / (2 * delay);
				sprintf(show_display, "Freq: %.2fHz", freq);
				gfx_mono_draw_string(show_display, 5,16, &sysfont);
				pisca_led(30);
		   }
			if(but_flag == 3){
				but_flag = 0;
				delay -= 100;
				if(delay < 100){
					delay = 100;
				}
				freq = (double) 1000 / (2 * delay);
				sprintf(show_display, "Freq: %.2fHz", freq);
				gfx_mono_draw_string(show_display, 5,16, &sysfont);
			}
		   
		   // Entra em sleep mode
		   pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); // (1)
	}
}
