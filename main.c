#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#define ADC1_NUM_CHANNELS   1
#define ADC1_BUF_DEPTH      1

int znach = 1;
int cnt = 0;

static const SerialConfig sdcfg = {
    .speed  = 9600,
    .cr1    = 0,
    .cr2    = 0,
    .cr3    = 0
};

static adcsample_t adc_buffer[ADC1_NUM_CHANNELS * ADC1_BUF_DEPTH];

/*
 * GPT4 configuration. This timer is used as trigger for the ADC.
 */
static const GPTConfig gpt4cfg1 = {
  .frequency =  100000,
  .callback  =  NULL,
  .cr2       =  TIM_CR2_MMS_1,  /* MMS = 010 = TRGO on Update Event.        */
  .dier      =  0U
  /* .dier field is direct setup of register, we don`t need to set anything here until now */
};


/* ADC streaming callback */
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  (void)adcp;
  (void)buffer;
  (void)n;

  adcsample_t result = adc_buffer[0];
  //result = 65529;
  znach = (int16_t) result;
  cnt++;


}

/* ADC errors callback, should never happen */
static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;


}

static const ADCConversionGroup adcgrpcfg1 = {
  .circular     = true,                                           // working mode = looped
  .num_channels = ADC1_NUM_CHANNELS,
  .end_cb       = adccallback,
  .error_cb     = adcerrorcallback,
  .cr1          = 0,
  .cr2          = ADC_CR2_EXTEN_RISING | ADC_CR2_EXTSEL_SRC(0b1100),  // Commutated from GPT
  .smpr2        = ADC_SMPR2_SMP_AN4(ADC_SAMPLE_144),              // for AN10  - 144 samples
  .sqr1         = ADC_SQR1_NUM_CH(ADC1_NUM_CHANNELS),
  .sqr2         = 0,
  .sqr3         = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN4)
  /* If we can macro ADC_SQR2_SQ... we need to write to .sqr2 */
};

int main(void)
{

    chSysInit();
    halInit();

    gptStart(&GPTD4, &gpt4cfg1);
    // ADC driver
    adcStart(&ADCD1, NULL);
    palSetLineMode( LINE_ADC1_IN4, PAL_MODE_INPUT_ANALOG );  // PA4

    adcStartConversion(&ADCD1, &adcgrpcfg1, adc_buffer, ADC1_BUF_DEPTH);
    gptStartContinuous(&GPTD4, gpt4cfg1.frequency/1000);          // how often we need ADC value
    /* Just set the limit (interval) of timer counter, you can use this function
       not only for ADC triggering, but start infinite counting of timer for callback processing */

    sdStart( &SD2, &sdcfg );

        palSetPadMode( GPIOD, 5, PAL_MODE_ALTERNATE(8) );    // TX
        palSetPadMode( GPIOD, 6, PAL_MODE_ALTERNATE(8) );    // RX

    while (true)
    {


    chprintf(((BaseSequentialStream *)&SD2), "Znach: %d\n\r cnt: %d \r", znach, cnt );
    chThdSleepMilliseconds(100);
    }
}
