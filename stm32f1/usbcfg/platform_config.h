/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : platform_config.h
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Evaluation board specific configuration file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1_sys.h" 

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */
#if !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL)
 //#define USE_STM3210B_EVAL
 #define USE_STM3210E_EVAL //ALIENTE MINISTM32 使用的版本
#endif


/* Define the STM32F10x hardware depending on the used evaluation board */
#ifdef USE_STM3210B_EVAL
  #define USB_DISCONNECT                      GPIOD  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_9
  
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD
  #define RCC_APB2Periph_GPIO_KEY             RCC_APB2Periph_GPIOB

  #define GPIO_Pin_KEY                        GPIO_Pin_9   /* PB.09 */
  #define GPIO_Pin_UP                         GPIO_Pin_8   /* PD.08 */
  #define GPIO_Pin_DOWN                       GPIO_Pin_14  /* PD.14 */
  #define GPIO_Pin_LEFT                       GPIO_Pin_1   /* PE.01 */
  #define GPIO_Pin_RIGHT                      GPIO_Pin_0   /* PE.00 */ 

  #define RCC_APB2Periph_GPIO_JOY_SET1        RCC_APB2Periph_GPIOD
  #define RCC_APB2Periph_GPIO_JOY_SET2        RCC_APB2Periph_GPIOE

  #define GPIO_RIGHT                          GPIOE
  #define GPIO_LEFT                           GPIOE
  #define GPIO_DOWN                           GPIOD
  #define GPIO_UP                             GPIOD
  #define GPIO_KEY                            GPIOB

  #define GPIO_KEY_PORTSOURCE                 GPIO_PortSourceGPIOB
  #define GPIO_KEY_PINSOURCE                  GPIO_PinSource9
  #define GPIO_KEY_EXTI_Line                  EXTI_Line9

  #define EXTI_KEY_IRQChannel                 EXTI9_5_IRQChannel

  #define RCC_APB2Periph_ALLGPIO             (RCC_APB2Periph_GPIOA \
                                              | RCC_APB2Periph_GPIOB \
                                              | RCC_APB2Periph_GPIOC \
                                              | RCC_APB2Periph_GPIOD \
                                              | RCC_APB2Periph_GPIOE )

#else /* USE_STM3210E_EVAL */
  #define USB_DISCONNECT                      GPIOB  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_14
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOB
  #define RCC_APB2Periph_GPIO_KEY             RCC_APB2Periph_GPIOG
  
  #define GPIO_Pin_KEY                        GPIO_Pin_8   /* PA.0 */
  #define GPIO_Pin_UP                         GPIO_Pin_0   /* PA.0 */
  #define GPIO_Pin_DOWN                       GPIO_Pin_3   /* PD.03 */
  #define GPIO_Pin_LEFT                       GPIO_Pin_15  /* PA.15 */
  #define GPIO_Pin_RIGHT                      GPIO_Pin_13  /* PA.13 */    
  
  #define RCC_APB2Periph_GPIO_JOY_SET1        RCC_APB2Periph_GPIOA
  #define RCC_APB2Periph_GPIO_JOY_SET2        RCC_APB2Periph_GPIOA

  #define GPIO_RIGHT                          GPIOA
  #define GPIO_LEFT                           GPIOA
  #define GPIO_DOWN                           GPIOA
  #define GPIO_UP                             GPIOA
  #define GPIO_KEY                            GPIOC

                                                
  #define RCC_APB2Periph_ALLGPIO             (RCC_APB2Periph_GPIOA \
                                              | RCC_APB2Periph_GPIOB \
                                              | RCC_APB2Periph_GPIOC \
                                              | RCC_APB2Periph_GPIOD \
                                              | RCC_APB2Periph_GPIOE \
                                              | RCC_APB2Periph_GPIOF \
                                              | RCC_APB2Periph_GPIOG )

  #define GPIO_KEY_PORTSOURCE                 GPIO_PortSourceGPIOG
  #define GPIO_KEY_PINSOURCE                  GPIO_PinSource8
  #define GPIO_KEY_EXTI_Line                  EXTI_Line8

  #define EXTI_KEY_IRQChannel                 EXTI9_5_IRQChannel

#endif /* USE_STM3210B_EVAL */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

