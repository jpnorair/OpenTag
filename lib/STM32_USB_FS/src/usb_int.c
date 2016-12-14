/**
  ******************************************************************************
  * @file    usb_int.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    28-August-2012
  * @brief   Endpoint CTR (Low and High) interrupt's service routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
//OpenTag Conditional Compile
#include <board.h>
#if (MCU_CONFIG_USB)

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint16_t SaveRState;
__IO uint16_t SaveTState;

/* Extern variables ----------------------------------------------------------*/
extern void (*pEpInt_IN[7])(void);    /*  Handles IN  interrupts   */
extern void (*pEpInt_OUT[7])(void);   /*  Handles OUT interrupts   */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : CTR_LP.
* Description    : Low priority Endpoint Correct Transfer interrupt's service
*                  routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CTR_LP(void) {
    __IO uint16_t wEPVal = 0;
    /* stay in loop while pending interrupts */
    while (((wIstr = _GetISTR()) & ISTR_CTR) != 0) {
        /* extract highest priority endpoint number */
        EPindex = (uint8_t)(wIstr & ISTR_EP_ID);
        if (EPindex == 0) {
            /* Decode and service control endpoint interrupt */
            /* calling related service routine */
            /* (Setup0_Process, In0_Process, Out0_Process) */

            /* save RX & TX status */
            /* and set both to NAK */
      
            SaveRState = _GetENDPOINT(ENDP0);
            SaveTState = SaveRState & EPTX_STAT;
            SaveRState &=  EPRX_STAT;    

            _SetEPRxTxStatus(ENDP0,EP_RX_NAK,EP_TX_NAK);

            /* DIR bit = origin of the interrupt */

            if ((wIstr & ISTR_DIR) == 0) {
                /* DIR = 0 */

                /* DIR = 0      => IN  int */
                /* DIR = 0 implies that (EP_CTR_TX = 1) always  */

                _ClearEP_CTR_TX(ENDP0);
                In0_Process();

                /* before terminate set Tx & Rx status */

                _SetEPRxTxStatus(ENDP0,SaveRState,SaveTState);
                return;
            }
            else {
                /* DIR = 1 */

                /* DIR = 1 & CTR_RX       => SETUP or OUT int */
                /* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */

                wEPVal = _GetENDPOINT(ENDP0);
        
                if ((wEPVal &EP_SETUP) != 0) {
                    _ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
                    Setup0_Process();
                    /* before terminate set Tx & Rx status */

                    _SetEPRxTxStatus(ENDP0,SaveRState,SaveTState);
                    return;
                }

                else if ((wEPVal & EP_CTR_RX) != 0) {
                    _ClearEP_CTR_RX(ENDP0);
                    Out0_Process();
                    /* before terminate set Tx & Rx status */
     
                    _SetEPRxTxStatus(ENDP0,SaveRState,SaveTState);
                    return;
                }
            }
        }
        else {
            /* Decode and service non control endpoints interrupt  */

            /* process related endpoint register */
            wEPVal = _GetENDPOINT(EPindex);
            if ((wEPVal & EP_CTR_RX) != 0) {
                _ClearEP_CTR_RX(EPindex);       /* clear int flag */
                (*pEpInt_OUT[EPindex-1])();     /* call OUT service function */
            }

            if ((wEPVal & EP_CTR_TX) != 0) {
                _ClearEP_CTR_TX(EPindex);       /* clear int flag */
                (*pEpInt_IN[EPindex-1])();      /* call IN service function */
            }
        } 
    } 
}

/*******************************************************************************
* Function Name  : CTR_HP.
* Description    : High Priority Endpoint Correct Transfer interrupt's service 
*                  routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CTR_HP(void) {
    uint32_t wEPVal = 0;

    while (((wIstr = _GetISTR()) & ISTR_CTR) != 0) {
        _SetISTR((uint16_t)CLR_CTR); /* clear CTR flag */
        /* extract highest priority endpoint number */
        EPindex = (uint8_t)(wIstr & ISTR_EP_ID);
        
        /* process related endpoint register */
        wEPVal = _GetENDPOINT(EPindex);
        if ((wEPVal & EP_CTR_RX) != 0) {
            _ClearEP_CTR_RX(EPindex);   /* clear int flag */
            (*pEpInt_OUT[EPindex-1])(); /* call OUT service function */
        } 
        else if ((wEPVal & EP_CTR_TX) != 0) {
            _ClearEP_CTR_TX(EPindex);   /* clear int flag */
            (*pEpInt_IN[EPindex-1])();  /* call IN service function */
        }
    } 
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
#endif
