/**
  ******************************************************************************
  * @note This file is highly optimized by JPN, from its original version that
  *       shipped with the STM32-Cube distribution.
  *
  * @file    usbd_req.c
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    13-June-2014 
  * @brief   This file provides the standard USB requests following chapter 9.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_ctlreq.h"
#include "usbd_ioreq.h"


/** @addtogroup STM32_USBD_STATE_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_REQ 
  * @brief USB standard requests module
  * @{
  */ 

/** @defgroup USBD_REQ_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_REQ_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_REQ_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_REQ_Private_Variables
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_REQ_Private_FunctionPrototypes
  * @{
  */ 
static uint8_t USBD_GetDescriptor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_SetAddress(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_SetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_SetFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_ClrFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t USBD_GetLen(uint8_t *buf);

typedef uint8_t (*USBD_fn)(USBD_HandleTypeDef*, USBD_SetupReqTypedef*);

/**
  * @}
  */ 


/** @defgroup USBD_REQ_Private_Functions
  * @{
  */ 


/**
* @note Optimized function by JPN, although the most optimized solution 
*       probably integrates each of the subordinate functions, given that they
*       have a ton of commonalities internally with each other.
*
* @brief  USBD_StdDevReq
*         Handle standard usb device requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
//USBD_StatusTypeDef USBD_StdDevReq(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef  *req) {
//    USBD_StatusTypeDef ret = USBD_OK;  
//  
//    switch (req->bRequest) {
//        case USB_REQ_GET_DESCRIPTOR: 
//            USBD_GetDescriptor(pdev, req) ;
//            break;
//    
//        case USB_REQ_SET_ADDRESS:                      
//            USBD_SetAddress(pdev, req);
//            break;
//    
//        case USB_REQ_SET_CONFIGURATION:                    
//            USBD_SetConfig (pdev , req);
//            break;
//    
//        case USB_REQ_GET_CONFIGURATION:                 
//            USBD_GetConfig (pdev , req);
//            break;
//    
//        case USB_REQ_GET_STATUS:                                  
//            USBD_GetStatus (pdev , req);
//            break;
//    
//        case USB_REQ_SET_FEATURE:   
//            USBD_SetFeature (pdev , req);    
//            break;
//    
//        case USB_REQ_CLEAR_FEATURE:                                   
//            USBD_ClrFeature (pdev , req);
//            break;
//    
//        default:  
//            USBD_CtlError(pdev , req);
//            break;
//    }
//  
//    return ret;
//}

USBD_StatusTypeDef USBD_StdDevReq(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req) {
    USBD_StatusTypeDef ret = USBD_OK;
    uint8_t errcode = 1;
  
    static const USBD_fn reqfn[10] = {  (USBD_fn)&USBD_GetStatus,
                                        (USBD_fn)&USBD_ClrFeature,
                                        (USBD_fn)&USBD_CtlError,
                                        (USBD_fn)&USBD_SetFeature,
                                        (USBD_fn)&USBD_CtlError,
                                        (USBD_fn)&USBD_SetAddress,
                                        (USBD_fn)&USBD_GetDescriptor,
                                        (USBD_fn)&USBD_CtlError,
                                        (USBD_fn)&USBD_SetConfig,
                                        (USBD_fn)&USBD_GetConfig
                                    };
                                    
    if (req->bRequest < 10) {
        errcode = reqfn[req->bRequest](pdev, req);
    }
    if (errcode != 0) {
        USBD_CtlError(pdev, req);
    }

    return ret;
}





/**
* @note This function optimized by JPN
* @brief  USBD_StdItfReq
*         Handle standard usb interface requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
USBD_StatusTypeDef  USBD_StdItfReq (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    //USBD_StatusTypeDef ret = USBD_OK; 
  
    if ((pdev->dev_state == USBD_STATE_CONFIGURED) \
    && (LOBYTE(req->wIndex) <= USBD_MAX_NUM_INTERFACES)) {
        pdev->pClass->Setup (pdev, req); 
      
        //if ((req->wLength == 0) && (ret == USBD_OK)) {
        if ((req->wLength == 0) {
            USBD_CtlSendStatus(pdev);
        }
    }
    else {
        USBD_CtlError(pdev, req);
    }

    return USBD_OK;
}



/**
* @note This function cleaned-up by JPN but it still needs a ton of optimization
* @brief  USBD_StdEPReq
*         Handle standard usb endpoint requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
USBD_StatusTypeDef USBD_StdEPReq (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef  *req) {
    uint8_t                 ep_addr;
    uint8_t                 ep_num;
    USBD_StatusTypeDef      ret = USBD_OK; 
    USBD_EndpointTypeDef   *pep;
    
    ep_addr = LOBYTE(req->wIndex); 
    ep_num  = ep_addr & 0x7F;
  
    // Impl Notes
    // - USBD_STATE_ADDRESSED Always results in the same process happening, streamline this
    // - Calls to USBD_LL_StallEP() all have the same prep, break this into a goto
    // - ...
  
    switch (req->bRequest) {
        case USB_REQ_SET_FEATURE :
            switch (pdev->dev_state) {
                case USBD_STATE_ADDRESSED:          
                    if (ep_num != 0) { //((ep_addr != 0x00) && (ep_addr != 0x80)) {
                        USBD_LL_StallEP(pdev, ep_addr);
                    }
                    break;	
      
                case USBD_STATE_CONFIGURED:   
                    if (req->wValue == USB_FEATURE_EP_HALT) {
                        if (ep_num != 0) { // ((ep_addr != 0x00) && (ep_addr != 0x80)) { 
                            USBD_LL_StallEP(pdev, ep_addr);
                        }
                    }
                    pdev->pClass->Setup(pdev, req);   
                    USBD_CtlSendStatus(pdev);
                    break;
      
                default:                         
                    USBD_CtlError(pdev, req);
                    break;    
            }
            break;
    
        case USB_REQ_CLEAR_FEATURE :
            switch (pdev->dev_state) {
                case USBD_STATE_ADDRESSED:          
                    if (ep_num != 0) { // ((ep_addr != 0x00) && (ep_addr != 0x80)) {
                        USBD_LL_StallEP(pdev, ep_addr);
                    }
                    break;	
      
                case USBD_STATE_CONFIGURED:   
                    if (req->wValue == USB_FEATURE_EP_HALT) {
                        if (ep_num != 0) {        
                            USBD_LL_ClearStallEP(pdev, ep_addr);
                            pdev->pClass->Setup(pdev, req);
                        }
                        USBD_CtlSendStatus(pdev);
                    }
                    break;
      
                default:                         
                    USBD_CtlError(pdev , req);
                    break;    
            }
            break;
    
        case USB_REQ_GET_STATUS:                  
            switch (pdev->dev_state) {
                case USBD_STATE_ADDRESSED:
                    if (ep_num != 0) {
                        USBD_LL_StallEP(pdev , ep_addr);
                    }
                    break;	
      
                case USBD_STATE_CONFIGURED:
                    pep = (ep_addr & 0x80) ? pdev->ep_in: pdev->ep_out;
                    pep = &pep[ep_num];
                    
                    pep->status = USBD_LL_IsStallEP(pdev, ep_addr);
                    USBD_CtlSendData(pdev, (uint8_t*)&pep->status, 2);
                    break;
      
                    default:                         
                    USBD_CtlError(pdev, req);
                    break;
            }
            break;
    
        default:
            break;
    }
    return ret;
}



/**
* @note JPN: This function has been cleaned-up but it is not optimized.
* @todo Optimize this function, it is horribly ugly
*
* @brief  USBD_GetDescriptor
*         Handle Get Descriptor requests
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static uint8_t USBD_GetDescriptor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    uint16_t len;
    uint8_t *pbuf;
  
    // Impl notes (JPN)
    // - The interior switch can be converted into a function table.
    // - The exterior switch could be streamlined if HIGH_SPEED is checked
    //      beforehand.
  
    switch (req->wValue >> 8) {
        case USB_DESC_TYPE_DEVICE:
            pbuf = pdev->pDesc->GetDeviceDescriptor(pdev->dev_speed, &len);
            break;
    
        case USB_DESC_TYPE_CONFIGURATION:
#       if defined(USB_HS_ENABLED)
            if (pdev->dev_speed == USBD_SPEED_HIGH) {
                pbuf   = (uint8_t*)pdev->pClass->GetHSConfigDescriptor(&len);
                pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
            }
            else 
#       endif
            {
                pbuf   = (uint8_t*)pdev->pClass->GetFSConfigDescriptor(&len);
                pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
            }
            break;
    
        case USB_DESC_TYPE_STRING: {
//            switch ((uint8_t)(req->wValue)) {
//                case USBD_IDX_LANGID_STR:
//                    pbuf = pdev->pDesc->GetLangIDStrDescriptor(pdev->dev_speed, &len);        
//                    break;
//      
//                case USBD_IDX_MFC_STR:
//                    pbuf = pdev->pDesc->GetManufacturerStrDescriptor(pdev->dev_speed, &len);
//                    break;
//      
//                case USBD_IDX_PRODUCT_STR:
//                    pbuf = pdev->pDesc->GetProductStrDescriptor(pdev->dev_speed, &len);
//                    break;
//      
//                case USBD_IDX_SERIAL_STR:
//                    pbuf = pdev->pDesc->GetSerialStrDescriptor(pdev->dev_speed, &len);
//                    break;
//      
//                case USBD_IDX_CONFIG_STR:
//                    pbuf = pdev->pDesc->GetConfigurationStrDescriptor(pdev->dev_speed, &len);
//                    break;
//      
//                case USBD_IDX_INTERFACE_STR:
//                    pbuf = pdev->pDesc->GetInterfaceStrDescriptor(pdev->dev_speed, &len);
//                    break;
//      
//                default:
//#               if (USBD_SUPPORT_USER_STRING == 1)
//                    pbuf = pdev->pClass->GetUsrStrDescriptor(pdev, (req->wValue) , &len);
//                    break;
//#               else      
//                    //USBD_CtlError(pdev , req);
//                    return 1;
//#               endif   
//            }
        
            ///@note this is the optimized version of above.  The function
            ///      table already exists, so you can imagine that I'm bemused
            ///      as to why a switch statement was implemented.  Amateurs.
            uint8_t desc_i = (uint8_t)(req->wValue);
            if (desc_i <= USBD_IDX_INTERFACE_STR) {
                desc_fn descriptor_lut = (desc_fn)pdev->pDesc;
                descriptor_lut[desc_i](pdev->dev_speed, &len);
            }
            else {
#           if (USBD_SUPPORT_USER_STRING == 1)
                pbuf = pdev->pClass->GetUsrStrDescriptor(pdev, (req->wValue), &len);
#           else      
                //USBD_CtlError(pdev , req);
                return 1;
#           endif
            }
        } break;
        
#       if defined(USB_HS_ENABLED)
        case USB_DESC_TYPE_DEVICE_QUALIFIER: 
            if (pdev->dev_speed == USBD_SPEED_HIGH) {
                pbuf = (uint8_t*)pdev->pClass->GetDeviceQualifierDescriptor(&len);
                break;
            }
            else {
                //USBD_CtlError(pdev, req);
                return 1;
            } 

        case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
            if (pdev->dev_speed == USBD_SPEED_HIGH) {
                pbuf    = (uint8_t *)pdev->pClass->GetOtherSpeedConfigDescriptor(&len);
                pbuf[1] = USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION;
                break; 
            }
            else {
                //USBD_CtlError(pdev, req);
                return 1;
            }
#       endif

        default: 
            //USBD_CtlError(pdev, req);
            return 1;
    }
  
    if ((len != 0) && (req->wLength != 0)) {
        len = MIN(len, req->wLength);
        USBD_CtlSendData (pdev, pbuf, len);
    }
}


/**
* @note Slightly optimized by JPN
* @brief  USBD_SetAddress
*         Set device address
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static uint8_t USBD_SetAddress(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    uint8_t dev_addr; 
  
    if ((req->wIndex == 0) && (req->wLength == 0)) {
        dev_addr = (uint8_t)(req->wValue) & 0x7F;     
    
        if (pdev->dev_state != USBD_STATE_CONFIGURED) {
            pdev->dev_address = dev_addr;
            USBD_LL_SetUSBAddress(pdev, dev_addr);               
            USBD_CtlSendStatus(pdev);                         
            
            // yields USBD_STATE_DEFAULT or USBD_STATE_ADDRESSED
            pdev->dev_state = USBD_STATE_DEFAULT + (dev_addr != 0);
            return 0;
        }
    }
    
    //USBD_CtlError(pdev , req);
    return 1;
}


/**
* @note Slightly optimized by JPN
* @brief  USBD_SetConfig
*         Handle Set device configuration request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static uint8_t USBD_SetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    static uint8_t  cfgidx;
  
    cfgidx = (uint8_t)(req->wValue);                 
  
    if (cfgidx > USBD_MAX_NUM_CONFIGURATION) {            
        //USBD_CtlError(pdev , req);
        return 1;                            
    } 
    else {
        switch (pdev->dev_state) {
            case USBD_STATE_ADDRESSED:
                if (cfgidx) {                                			   							   							   				
                    pdev->dev_config    = cfgidx;
                    pdev->dev_state     = USBD_STATE_CONFIGURED;
                    
                    if (USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL) {
                        //USBD_CtlError(pdev , req);  
                        return 1;
                    }
                }
                USBD_CtlSendStatus(pdev);
                break;
      
            case USBD_STATE_CONFIGURED:
                if (cfgidx == 0) {                           
                    pdev->dev_state     = USBD_STATE_ADDRESSED;
                    pdev->dev_config    = cfgidx;          
                    USBD_ClrClassConfig(pdev, cfgidx);
                } 
                else if (cfgidx != pdev->dev_config) {
                    USBD_ClrClassConfig(pdev, pdev->dev_config);    /* Clear old configuration */
        
                    /* set new configuration */
                    pdev->dev_config = cfgidx;
                    if (USBD_SetClassConfig(pdev, cfgidx) == USBD_FAIL) {
                        //USBD_CtlError(pdev, req);  
                        return 1;
                    }
                }
                USBD_CtlSendStatus(pdev);
                break;
      
            default:					
                //USBD_CtlError(pdev, req); break;                    
                return 1;
        }
    }
    return 0;
}

/**
* @note This function optimized by JPN
* @brief  USBD_GetConfig
*         Handle Get device configuration request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static uint8_t USBD_GetConfig(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    if ((req->wLength == 1) && (pdev->dev_state & USBD_STATE_ADDRESSED))  {
        uint8_t* config;
        config  = (pdev->dev_state & 1) ?   (uint8_t*)&pdev->dev_config : \
                                            (uint8_t*)&pdev->dev_default_config;
        USBD_CtlSendData(pdev, config, 1);
        return 0;
    }
    else {
        //USBD_CtlError(pdev, req);
        return 1;
    }
}


/**
* @note This function optimized by JPN
* @brief  USBD_GetStatus
*         Handle Get Status request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
//static void USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
//    switch (pdev->dev_state) {
//        case USBD_STATE_ADDRESSED:
//        case USBD_STATE_CONFIGURED:
//#           if ( USBD_SELF_POWERED == 1)
//            pdev->dev_config_status = USB_CONFIG_SELF_POWERED;                                  
//#           else
//            pdev->dev_config_status = 0;                                   
//#           endif       
//            if (pdev->dev_remote_wakeup) {
//                pdev->dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;                                
//            }
//            USBD_CtlSendData(pdev, (uint8_t*)&pdev->dev_config_status, 2);
//            break;
//    
//        default :
//            USBD_CtlError(pdev , req);                        
//            break;
//    }
//}

static uint8_t USBD_GetStatus(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    if (pdev->dev_state & USBD_STATE_ADDRESSED) {
#       if ( USBD_SELF_POWERED == 1)
            pdev->dev_config_status = USB_CONFIG_SELF_POWERED;                                  
#       else
            pdev->dev_config_status = 0;                                   
#       endif       
        if (pdev->dev_remote_wakeup) {
            pdev->dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;                                
        }
        USBD_CtlSendData(pdev, (uint8_t*)&pdev->dev_config_status, 2);
        return 0;
    }
    else {
        //USBD_CtlError(pdev, req); 
        return 1;
    }
}




/**
* @brief  USBD_SetFeature
*         Handle Set device feature request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static uint8_t USBD_SetFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    if (req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
        pdev->dev_remote_wakeup = 1;  
        pdev->pClass->Setup (pdev, req);   
        USBD_CtlSendStatus(pdev);
    }
}


/**
* @note THis function optimized by JPN
* @brief  USBD_ClrFeature
*         Handle clear device feature request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static uint8_t USBD_ClrFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    if (pdev->dev_state & USBD_STATE_ADDRESSED) {
        if (req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
            pdev->dev_remote_wakeup = 0; 
            pdev->pClass->Setup (pdev, req);   
            USBD_CtlSendStatus(pdev);
        }
        return 0;
    }
    else {
        //USBD_CtlError(pdev, req);
        return 1;
    }
}

/**
* @brief  USBD_ParseSetupRequest 
*         Copy buffer into setup structure
* @param  pdev: device instance
* @param  req: usb request
* @retval None
*/

void USBD_ParseSetupRequest(USBD_SetupReqTypedef *req, uint8_t *pdata) {
    req->bmRequest  = *(uint8_t*)(pdata);
    req->bRequest   = *(uint8_t*)(pdata + 1);
    req->wValue     = SWAPBYTE(pdata + 2);
    req->wIndex     = SWAPBYTE(pdata + 4);
    req->wLength    = SWAPBYTE(pdata + 6);
}

/**
* @brief  USBD_CtlError 
*         Handle USB low level Error
* @param  pdev: device instance
* @param  req: usb request
* @retval None
*/
uint8_t USBD_CtlError(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    USBD_LL_StallEP(pdev, 0x80);
    USBD_LL_StallEP(pdev, 0);
    return 0;
}


/**
  * @brief  USBD_GetString
  *         Convert Ascii string into unicode one
  * @param  desc : descriptor buffer
  * @param  unicode : Formatted string buffer (unicode)
  * @param  len : descriptor length
  * @retval None
  */
void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len) {
    uint8_t idx = 0;
    
    if (desc != NULL) {
        *len            =  USBD_GetLen(desc) * 2 + 2;    
        unicode[idx++]  = *len;
        unicode[idx++]  =  USB_DESC_TYPE_STRING;
    
        while (*desc != '\0') {
            unicode[idx++] = *desc++;
            unicode[idx++] =  0x00;
        }
    } 
}

/**
  * @brief  USBD_GetLen
  *         return the string length
   * @param  buf : pointer to the ascii string buffer
  * @retval string length
  */
static uint8_t USBD_GetLen(uint8_t *buf) {
    uint8_t len = 0;
    while (*buf != '\0') {
        len++;
        buf++;
    }
    return len;
}
/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
