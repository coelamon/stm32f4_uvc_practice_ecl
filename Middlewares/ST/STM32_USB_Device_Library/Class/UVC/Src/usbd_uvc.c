
#include "usbd_uvc.h"

uint8_t  USBD_UVC_Init (USBD_HandleTypeDef *pdev,
                            uint8_t cfgidx);

uint8_t  USBD_UVC_DeInit (USBD_HandleTypeDef *pdev,
                              uint8_t cfgidx);

uint8_t  USBD_UVC_Setup (USBD_HandleTypeDef *pdev,
                             USBD_SetupReqTypedef *req);

uint8_t  USBD_UVC_EP0_RxReady (USBD_HandleTypeDef *pdev);

uint8_t  USBD_UVC_DataIn (USBD_HandleTypeDef *pdev,
                              uint8_t epnum);


uint8_t  USBD_UVC_DataOut (USBD_HandleTypeDef *pdev,
                               uint8_t epnum);

uint8_t  USBD_UVC_SOF (USBD_HandleTypeDef *pdev);

uint8_t  *USBD_UVC_GetHSCfgDesc (uint16_t *length);

uint8_t  *USBD_UVC_GetFSCfgDesc (uint16_t *length);

uint8_t  *USBD_UVC_GetOtherSpeedCfgDesc (uint16_t *length);

uint8_t  *USBD_UVC_GetDeviceQualifierDescriptor (uint16_t *length);

uint8_t  *USBD_UVC_GetUsrStrDescriptor(USBD_HandleTypeDef *pdev ,uint8_t index,  uint16_t *length);

USBD_UVC_LogRecordTypeDef *log_first = NULL;
USBD_UVC_LogRecordTypeDef *log_last  = NULL;
USBD_UVC_LogRecordTypeDef log[100];
uint32_t log_count = 0;
uint32_t log_sof_count = 0;
uint32_t log_ep0_rxready_count = 0;
uint32_t log_datain_count = 0;
uint32_t log_dataout_count = 0;

__ALIGN_BEGIN uint8_t USBD_UVC_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

USBD_ClassTypeDef  USBD_UVC =
{
  USBD_UVC_Init,
  USBD_UVC_DeInit,
  USBD_UVC_Setup,
  NULL, /*EP0_TxSent*/
  USBD_UVC_EP0_RxReady, /*EP0_RxReady*/
  USBD_UVC_DataIn,
  USBD_UVC_DataOut,
  USBD_UVC_SOF, /*SOF */
  NULL,
  NULL,
  NULL,
  USBD_UVC_GetFSCfgDesc,
  NULL,
  NULL,
  USBD_UVC_GetUsrStrDescriptor
};

/* USB Vide Class device Configuration Descriptor */
uint8_t USBD_UVC_CfgFSDesc[USB_UVC_CONFIG_DESC_TOTAL_SIZE]  __ALIGN_END =
{
  /* Configuration 1 */
  9,   /* bLength: Configuation Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,   /* bDescriptorType: Configuration */
  WBVAL(USB_UVC_CONFIG_DESC_TOTAL_SIZE), // wTotalLength
  2,   /* bNumInterfaces: 2 interfaces */
  0x01,   /* bConfigurationValue: Configuration's Id */
  UVC_USR_STR_CONFIG,   /* iConfiguration: */
  USB_CONFIG_BUS_POWERED,   /* bmAttributes: */
  USB_CONFIG_POWER_MA(100),   /* bMaxPower: 100 mA */

  /* Interface Association Descriptor */
  8, /* bLength */
  USB_DESC_TYPE_INTERFACE_ASSOCIATION, /* bDescriptorType */
  0x00, /* bFirstInterface */
  2, /* bInterfaceCount */
  CC_VIDEO,                      /* bFunctionClass     */
  SC_VIDEO_INTERFACE_COLLECTION, /* bFunctionSubClass  */
  PC_PROTOCOL_UNDEFINED,         /* bInterfaceProtocol */
  UVC_USR_STR_IAD, /* iFunction */

  /* VideoControl Interface Descriptor */

  /* Standard VC Interface Descriptor = interface 0 */
  9, /* bLength */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType */
  0x00, /* bInterfaceNumber */
  0x00, /* bAlternateSetting */
  0x00, /* bNumEndpoints */
  CC_VIDEO, /* bInterfaceClass */
  SC_VIDEOCONTROL, /* bInterfaceSubClass */
  PC_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
  UVC_USR_STR_VC_ITF, /* iInterface */

  /* Class-specific VC Interface Descriptor */
  12+1, /* bLength */
  CS_INTERFACE, /* bDescriptorType */
  VC_HEADER, /* bDescriptorSubType */
  WBVAL(0x0100), /* bcdVDC: UVC 1.0 */
  WBVAL(VC_DESC_TOTAL_SIZE), /* wTotalLength */
  DBVAL(6000000), /* dwClockFrequency: 6.000000 MHz - why 6MHz? - don't know */
  1, /* bInCollection */
  1, /* baInterfaceNr(1) */

  /* Camera Terminal Descriptor */
  15+2, /* bLength */
  CS_INTERFACE, /* bDescriptorType */
  VC_INPUT_TERMINAL, /* bDescriptorSubtype */
  0x01, /* bTerminalID */
  WBVAL(ITT_CAMERA), /* wTerminalType */
  0x00, /* bAssocTerminal */
  UVC_USR_STR_VC_IT, /* iTerminal */
  WBVAL(0x0000), /* wObjectiveFocalLengthMin */
  WBVAL(0x0000), /* wObjectiveFocalLengthMax */
  WBVAL(0x0000), /* wOcularFocalLength       */
  2,             /* bControlSize */
  0x00, 0x00,    /* bmControls */

  /* Output Terminal Descriptor */
  9, /* bLength */
  CS_INTERFACE, /* bDescriptorType */
  VC_OUTPUT_TERMINAL, /* bDescriptorSubtype */
  0x02, /* bTerminalID */
  WBVAL(TT_STREAMING), /* wTerminalType */
  0x00, /* bAssocTerminal */
  0x01, /* bSourceID: is connected to terminal 0x01 */
  UVC_USR_STR_VC_OT, /* iTerminal */

  /* VideoStreaming Interface Descriptor */

  /* Standard VS Interface Descriptor  = interface 1 */
  // alternate setting 0 = Zero Bandwidth
  9, /* bLength */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType */
  0x01, /* bInterafaceNumber */
  0x00, /* bAlternateSetting */
  0, /* bNumEndpoints */
  CC_VIDEO, /* bInterafaceClass */
  SC_VIDEOSTREAMING, /* bInterafaceSubClass */
  PC_PROTOCOL_UNDEFINED, /* bInterafaceProtocol */
  UVC_USR_STR_VS_ITF_AS0, /* iInterface */

  /* Class-specific VS Interface Input Header Descriptor */
  13+1*1, /* bLength */
  CS_INTERFACE,                              /* bDescriptorType */
  VS_INPUT_HEADER,                           /* bDescriptorSubtype */
  1,                                         /* bNumFormats */
  WBVAL(VS_DESC_TOTAL_SIZE), /* wTotalLength */
  USB_ENDPOINT_IN(1),                        /* bEndPointAddress */
  0x00,                                      /* bmInfo */
  0x02,                                      /* bTerminalLink: it's endpoint is connected to terminal 0x02 */
  0x00,                                      /* bStillCaptureMethod */
  0x01,                                      /* bTriggerSupport */
  0x00,                                      /* bTriggerUsage */
  1,                                         /* bControlSize */
  0x00,                                      /* bmaControls(1) */

  /* Uncompressed Video Format Descriptor */
  27, /* bLength */
  CS_INTERFACE,                         /* bDescriptorType */
  VS_FORMAT_UNCOMPRESSED,               /* bDescriptorSubType */
  0x01,                                 /* bFormatIndex */
  1,                                    /* bNumFrameDescriptors */
  0x59,0x55,0x59,0x32,                  /* Giud Format YUY2 {32595559-0000-0010-8000-00AA00389B71} */
  0x00,0x00,
  0x10,0x00,
  0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71,
  16,                                   /* bBitsPerPixel: 16 for yuy2 */
  0x01,                                 /* bDefaultFrameIndex: Default frame index is 1 */
  0x00,                                 /* bAspectRatioX */
  0x00,                                 /* bAspectRatioY */
  0x00,                                 /* bmInterlaceFlags */
  0x00,                                 /* bCopyProtect */

  /* Uncompressed Video Frame Descriptor */
  30, /* bLength */
  CS_INTERFACE, /* bDescriptorType */
  VS_FRAME_UNCOMPRESSED, /* bDscriptorSubtype */
  0x01, /* bFrameIndex */
  0x02, /* bmCapabilities: fixed frame-rate */
  WBVAL(160), /* wWidth */
  WBVAL(120), /* wHeight */
  DBVAL(160*120*16*5), /* dwMinBitRate */
  DBVAL(160*120*16*5), /* dwMaxBitRate */
  DBVAL(160*120*2), /* dwMaxVideoFrameBufSize */
  DBVAL(2000000), /* dwDefaultFrameInterval: for 5 FPS */
  1, /* bFrameIntervalType */
  DBVAL(2000000), /* dwFrameInterval(1) */

  /* Color Matching Descriptor */
  6, /* bLength */
  CS_INTERFACE, /* bDescriptorType */
  VS_COLORFORMAT, /* bDescriptorSubtype */
  1, /* bColorPrimaries: 1 - BT.709, sRGB */
  1, /* bTransferCharacteristics: 1 - BT.709 */
  4, /* bMatrixCoefficients: 4 - SMPTE 170M */

  /* Standard VS Interface Descriptor  = interface 1 */
  // alternate setting 1 = operational setting
  9, /* bLength */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType */
  0x01, /* bInterafaceNumber */
  0x01, /* bAlternateSetting */
  1, /* bNumEndpoints */
  CC_VIDEO, /* bInterafaceClass */
  SC_VIDEOSTREAMING, /* bInterafaceSubClass */
  PC_PROTOCOL_UNDEFINED, /* bInterafaceProtocol */
  UVC_USR_STR_VS_ITF_AS1, /* iInterface */

  /* Standard VS Isochronous Video Data Endpoint Descriptor */
  7, /* bLength */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType */
  USB_ENDPOINT_IN(1), /* bEndpointAddress */
  0x01, /* bmAttributes: 1 - Isochronous, 2 - Bulk */
  WBVAL(UVC_IN_EP1_PACKET_SIZE), /* wMaxPacketSize */
  1 /* bInterval */
};

//data array for Video Probe and Commit
VideoControl    videoCommitControl =
{
  {0x00,0x00},                      // bmHint
  {0x01},                           // bFormatIndex
  {0x01},                           // bFrameIndex
  {DBVAL(2000000),},                // dwFrameInterval
  {0x00,0x00,},                     // wKeyFrameRate
  {0x00,0x00,},                     // wPFrameRate
  {0x00,0x00,},                     // wCompQuality
  {0x00,0x00,},                     // wCompWindowSize
  {0x00,0x00},                      // wDelay
  {DBVAL(160*120*2)},               // dwMaxVideoFrameSize
  {0x00, 0x00, 0x00, 0x00},         // dwMaxPayloadTransferSize
  {0x00, 0x00, 0x00, 0x00},         // dwClockFrequency
  {0x00},                           // bmFramingInfo
  {0x00},                           // bPreferedVersion
  {0x00},                           // bMinVersion
  {0x00},                           // bMaxVersion
};

VideoControl    videoProbeControl =
{
  {0x00,0x00},                      // bmHint
  {0x01},                           // bFormatIndex
  {0x01},                           // bFrameIndex
  {DBVAL(2000000),},                // dwFrameInterval
  {0x00,0x00,},                     // wKeyFrameRate
  {0x00,0x00,},                     // wPFrameRate
  {0x00,0x00,},                     // wCompQuality
  {0x00,0x00,},                     // wCompWindowSize
  {0x00,0x00},                      // wDelay
  {DBVAL(160*120*2)},               // dwMaxVideoFrameSize
  {0x00, 0x00, 0x00, 0x00},         // dwMaxPayloadTransferSize
  {0x00, 0x00, 0x00, 0x00},         // dwClockFrequency
  {0x00},                           // bmFramingInfo
  {0x00},                           // bPreferedVersion
  {0x00},                           // bMinVersion
  {0x00},                           // bMaxVersion
};

USBD_UVC_LogRecordTypeDef* USBD_UVC_LogMessage(uint32_t messageid)
{
	if (log_count >= 100)
	{
		return NULL;
	}
	if (log_first == NULL)
	{
		log_first = &log[0];
		log_last = &log[0];
		log_count = 1;
	}
	else
	{
		log_last = &log[log_count];
		log_count++;
	}
	log_last->messageid = messageid;
	log_last->req = NULL;
	return log_last;
}

void USBD_UVC_LogSetupMessage(USBD_SetupReqTypedef *req)
{
	USBD_UVC_LogRecordTypeDef *log_record = USBD_UVC_LogMessage(UVC_LOG_SETUP);
	if (log_record == NULL)
	{
		return;
	}
	log_record->req = USBD_malloc(sizeof(USBD_SetupReqTypedef));
	log_record->req->bmRequest = req->bmRequest;
	log_record->req->bRequest = req->bRequest;
	log_record->req->wValue = req->wValue;
	log_record->req->wIndex = req->wIndex;
	log_record->req->wLength = req->wLength;
}

uint8_t  USBD_UVC_Init (USBD_HandleTypeDef *pdev,
                        uint8_t cfgidx)
{
  int16_t ret = 0;

  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
  		USB_ENDPOINT_IN(1),
        USBD_EP_TYPE_ISOC,
		UVC_IN_EP1_PACKET_SIZE);

  uint32_t debug_otg_fs_diepctl1 = *((uint32_t*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + (1 * USB_OTG_EP_REG_SIZE)));

  pdev->pClassData = USBD_malloc(sizeof (USBD_UVC_HandleTypeDef));

  if(pdev->pClassData == NULL)
  {
	  return USBD_FAIL;
  }

  USBD_UVC_HandleTypeDef *huvc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  huvc->interface = 0;
  huvc->play_status = 0;
  huvc->frame_bytes_count = 0;
  huvc->frame_toggle_byte = 0;
  huvc->frame_count = 0;

  return USBD_OK;
}

uint8_t  USBD_UVC_DeInit (USBD_HandleTypeDef *pdev,
                          uint8_t cfgidx)
{
  /* Close EP IN */
  USBD_LL_CloseEP(pdev, USB_ENDPOINT_IN(1));

  /* Free MSC Class Resources */
  if(pdev->pClassData != NULL)
  {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

void debug_usb_registers()
{
	uint32_t debug_otg_fs_gintmsk = *((uint32_t*)(0x50000018)); // 0x800C3818

	uint32_t debug_otg_fs_dieptxf1 = *((uint32_t*)(0x50000104)); // 0xE80050

	uint32_t debug_otg_fs_dcfg = *((uint32_t*)(0x50000800)); // 0x200033
	uint32_t debug_otg_fs_dctl = *((uint32_t*)(0x50000804)); // 0
	uint32_t debug_otg_fs_dsts = *((uint32_t*)(0x50000808)); // 0x76306

	uint32_t debug_otg_fs_diepmsk = *((uint32_t*)(0x50000810)); // 0x0B
	uint32_t debug_otg_fs_daintmsk = *((uint32_t*)(0x5000081C)); // 0x10003
	uint32_t debug_otg_fs_diepempmsk = *((uint32_t*)(0x50000834)); // 0

	uint32_t debug_otg_fs_diepctl1 = *((uint32_t*)(0x50000920)); // 0x448302
	uint32_t debug_otg_fs_diepint1 = *((uint32_t*)(0x50000928)); // 0x80
	uint32_t debug_otg_fs_dieptsiz1 = *((uint32_t*)(0x50000930)); // 0
	uint32_t debug_otg_fs_dtxfsts1 = *((uint32_t*)(0x50000938)); // 0xE8 - 232
}

uint8_t  USBD_UVC_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_UVC_HandleTypeDef *huvc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;

  USBD_UVC_LogSetupMessage(req);

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {

  /* Class request */
  case USB_REQ_TYPE_CLASS :
    switch (req->bRequest)
    {
    case UVC_GET_CUR:
    case UVC_GET_DEF:
    case UVC_GET_MIN:
    case UVC_GET_MAX:
      UVC_Req_GetCurrent(pdev, req);
      break;

    case UVC_SET_CUR:
      UVC_Req_SetCurrent(pdev, req);
      break;

    default:
       USBD_CtlError(pdev, req);
       return USBD_FAIL;
    }
    break;
  /* Interface & Endpoint request */
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {

    case USB_REQ_GET_DESCRIPTOR:
    	break;

    case USB_REQ_GET_INTERFACE:
      USBD_CtlSendData (pdev,
                        (uint8_t *)&huvc->interface,
                        1);
      break;

    case USB_REQ_SET_INTERFACE:
      huvc->interface = (uint8_t)(req->wValue);
      if (huvc->interface == 1) {
    	uint32_t debug_otg_fs_diepctl1 = *((uint32_t*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + (1 * USB_OTG_EP_REG_SIZE)));
    	huvc->play_status = 1;
    	debug_usb_registers();
      } else {
        //USBD_LL_FlushEP(pdev, USB_ENDPOINT_IN(1));
        huvc->play_status = 0;
      }
      break;

    }
    break;

  default:
    break;
  }
  return USBD_OK;
}

uint8_t  USBD_UVC_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
  log_ep0_rxready_count++;
  return USBD_OK;
}

uint8_t  USBD_UVC_DataIn (USBD_HandleTypeDef *pdev,
                          uint8_t epnum)
{
  USBD_UVC_HandleTypeDef *huvc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;

  log_datain_count++;

  USBD_LL_FlushEP(pdev, USB_ENDPOINT_IN(1));

  if (huvc->frame_bytes_count >= 160*120*2)
  {
	  huvc->frame_bytes_count = 0;
	  huvc->frame_toggle_byte ^= 1;
	  huvc->frame_count++;
  }

  uint32_t packet_size = 0;
  uint8_t packet[UVC_IN_EP1_PACKET_SIZE];
  uint8_t header[2] = {2, huvc->frame_toggle_byte};

  packet[0] = header[0];
  packet[1] = header[1];

  packet_size = 2;

  while (packet_size < UVC_IN_EP1_PACKET_SIZE && huvc->frame_bytes_count < 160*120*2)
  {
	  if (huvc->frame_bytes_count % 2 == 0) {
		  packet[packet_size] = (((huvc->frame_count % 10) > 5) ? (10-(huvc->frame_count % 10)) : (huvc->frame_count % 10)) * 200 / 5;
	  } else {
		  packet[packet_size] = 128;
	  }
	  packet_size++;
	  huvc->frame_bytes_count++;
  }

  if (huvc->play_status == 2) {
	  USBD_LL_Transmit(pdev, USB_ENDPOINT_IN(1), (uint8_t*)&packet, (uint32_t)packet_size);
  }

  return USBD_OK;
}

uint8_t  USBD_UVC_DataOut (USBD_HandleTypeDef *pdev,
                           uint8_t epnum)
{
  log_dataout_count++;

  return USBD_OK;
}

uint8_t  *USBD_UVC_GetFSCfgDesc (uint16_t *length)
{
  *length = USB_UVC_CONFIG_DESC_TOTAL_SIZE;
  return USBD_UVC_CfgFSDesc;
}

uint8_t  USBD_UVC_SOF (USBD_HandleTypeDef *pdev)
{
  USBD_UVC_HandleTypeDef *huvc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;

  log_sof_count++;

  if (huvc->play_status == 1)
  {
	//USBD_LL_FlushEP (pdev, USB_ENDPOINT_IN(1));
	  USBD_LL_Transmit (pdev, USB_ENDPOINT_IN(1), (uint8_t*)0x0002, 2);//header
    huvc->play_status = 2;
  }
  return USBD_OK;
}

uint8_t  *USBD_UVC_GetUsrStrDescriptor(USBD_HandleTypeDef *pdev ,uint8_t index,  uint16_t *length)
{
  USBD_UVC_LogMessage(UVC_LOG_USR_STR);

  switch (index)
  {
  case UVC_USR_STR_IAD:
	  USBD_GetString (UVC_NAME, USBD_UVC_StrDesc, length);
	  return USBD_UVC_StrDesc;
  default:
	  break;
  }
  USBD_GetString ("USR_STR", USBD_UVC_StrDesc, length);
  return USBD_UVC_StrDesc;
}

void UVC_Req_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  /* Send the current mute state */
  USBD_UVC_LogMessage(UVC_LOG_GET_CUR);

  //USBD_LL_FlushEP (pdev, USB_ENDPOINT_OUT(0)); // ???

  if(req->wValue == 256)
  {
    //Probe Request
    USBD_CtlSendData (pdev, (uint8_t*)&videoProbeControl, req->wLength);
  }
  else if (req->wValue == 512)
  {
    //Commit Request
    USBD_CtlSendData (pdev, (uint8_t*)&videoCommitControl, req->wLength);
  }
}

void UVC_Req_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_UVC_LogMessage(UVC_LOG_SET_CUR);
  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    if(req->wValue == 256)
    {
      //Probe Request
      USBD_CtlPrepareRx (pdev, (uint8_t*)&videoProbeControl, req->wLength);
    }
    else if (req->wValue == 512)
    {
      //Commit Request
      USBD_CtlPrepareRx (pdev, (uint8_t*)&videoCommitControl, req->wLength);
    }

  }
}
