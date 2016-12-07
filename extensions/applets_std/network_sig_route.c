/*  Copyright 2010-2012, JP Norair
  *
  * Licensed under the OpenTag License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
/**
  * @file       /otlibext/applets_std/network_sig_route.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Standard Network Routing Callback Routine
  *
  * Network layer uses a callback when a packet has been successfully received
  * and it is routed.  This routine will log the type of packet/frame that has
  * been received & routed, and its contents.
  */

#include <otstd.h>
#include <m2api.h>
#include <otlib/logger.h>


#ifdef EXTF_network_sig_route
void network_sig_route(void* route, void* session) {
	static const ot_u8 label_dialog[]	= { 9, 'M','2','_','D','i','a','l','o','g' };
	static const ot_u8 label_nack[]		= { 7, 'M','2','_','N','a','c','k' };
	static const ot_u8 label_stream[]	= { 9, 'M','2','_','S','t','r','e','a','m' };
	static const ot_u8 label_snack[]	= { 7, 'M','2','_','S','N','a','c','k' };

	static const ot_u8* labels[] 		= { label_dialog,
											label_nack,
											label_stream,
											label_snack };
	ot_u8 protocol;

	protocol = ((m2session*)session)->extra & 3;
	logger_msg(	MSG_raw,
					labels[protocol][0],
					q_length(&rxq),
					(ot_u8*)&(labels[protocol][1]),
					rxq.front	);
}
#endif
