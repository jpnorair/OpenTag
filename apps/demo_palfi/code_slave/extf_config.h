/*  Copyright 2010-2011, JP Norair
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
  * @file       /apps/demo_palfi/code_slave/extf_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       16 April 2012
  * @brief      Extension Function Configuration File for PaLFi Demo
  *
  * Don't actually include this.  Include OTAPI.h or OT_config.h instead.
  *
  * This include file specifies all extension functions that should be compiled
  * into the build.  Extension functions are replacements/patches for functions
  * declared in OTlib, so if you define an Extension Function (EXTF), OpenTag
  * will build and link your function instead of the regular OTlib version.
  ******************************************************************************
  */

#ifndef __EXTF_CONFIG_H
#define __EXTF_CONFIG_H


/** @note Function extensions declared in this build are:
  * <LI> network_sig_route(): a callback type< /LI>
  * <LI> sys_sig_panic(): a callback type </LI>
  * <LI> sys_sig_rfainit(): a callback type </LI>
  * <LI> sys_sig_rfaterminate(): a callback type </LI>
  */




/// ALP Module EXTFs
//#define EXTF_alp_load_retval
//#define EXTF_alp_proc
//#define EXTF_alp_proc_filedata
//#define EXTF_alp_proc_sensor
//#define EXTF_alp_proc_dashforth
//#define EXTF_alp_proc_logger
//#define EXTF_alp_proc_api_session
//#define EXTF_alp_proc_api_system
//#define EXTF_alp_proc_api_query
//#define EXTF_alp_proc_sec_example





/// Auth Module EXTFs
//#define EXTF_auth_init
//#define EXTF_auth_isroot
//#define EXTF_auth_check
//#define EXTF_auth_new_nlsuser
//#define EXTF_auth_search_user
//#define EXTF_auth_get_dllskey





/// Buffer Module EXTFs
//#define EXTF_buffers_init






/// CRC16 Module EXTFs
//#define EXTF_crc_calc_block
//#define EXTF_crc_init_stream
//#define EXTF_crc_calc_stream
//#define EXTF_crc_update_stream
//#define EXTF_crc_check
//#define EXTF_crc_get






/// Encode Module EXTFs
//#define EXTF_em2_encode_newpacket
//#define EXTF_em2_decode_newpacket
//#define EXTF_em2_encode_newframe
//#define EXTF_em2_decode_newframe
//#define EXTF_em2_remaining_frames
//#define EXTF_em2_remaining_bytes
//#define EXTF_em2_complete





/// External Module EXTFs
//#define EXTF_ext_init
//#define EXTF_ext_get_m2appflags





/// M2 Network Module EXTFs
//#define EXTF_network_init
//#define EXTF_network_parse_bf
//#define EXTF_network_route_ff
#define EXTF_network_sig_route
//#define EXTF_m2np_header
//#define EXTF_m2np_footer
//#define EXTF_m2np_put_deviceid
//#define EXTF_m2np_idcmp
//#define EXTF_m2advp_open
//#define EXTF_m2advp_close
//#define EXTF_m2advp_init_flood
//#define EXTF_m2dp_open
//#define EXTF_m2dp_parse_dspkt
//#define EXTF_m2dp_mark_dsframe
//#define EXTF_m2dp_dsproc






/// M2QP Module EXTFs
//#define EXTF_m2qp_put_beacon
//#define EXTF_m2qp_put_na2ptmpl
//#define EXTF_m2qp_put_a2ptmpl
//#define EXTF_m2qp_set_suppliedid
//#define EXTF_m2qp_put_isfs
//#define EXTF_m2qp_put_isf
//#define EXTF_m2qp_sigresp_null
//#define EXTF_m2qp_init
//#define EXTF_m2qp_parse_frame
//#define EXTF_m2qp_parse_dspkt
//#define EXTF_m2qp_mark_dsframe
//#define EXTF_m2qp_isf_comp
//#define EXTF_m2qp_isf_call
//#define EXTF_m2qp_load_isf





/// MPipe EXTFs
//#define EXTF_mpipe_footerbytes
//#define EXTF_mpipe_init
//#define EXTF_mpipe_kill
//#define EXTF_mpipe_wait
//#define EXTF_mpipe_setspeed
//#define EXTF_mpipe_status
//#define EXTF_mpipe_sig_txdone
//#define EXTF_mpipe_sig_rxdone
//#define EXTF_mpipe_sig_rxdetect
//#define EXTF_mpipe_txndef
//#define EXTF_mpipe_rxndef
//#define EXTF_mpipe_isr





/// NDEF module EXTFs
//#define EXTF_ndef_new_msg
//#define EXTF_ndef_new_record
//#define EXTF_ndef_send_msg
//#define EXTF_ndef_load_msg
//#define EXTF_ndef_parse_record





/// OT Utils EXTFs
//#define EXTF_otutils_calc_timeout
//#define EXTF_otutils_encode_timeout





/// OTAPI C EXTFs
//#define EXTF_otapi_sysinit
//#define EXTF_otapi_new_session
//#define EXTF_otapi_open_request
//#define EXTF_otapi_close_request
//#define EXTF_otapi_start_flood
//#define EXTF_otapi_start_dialog
//#define EXTF_otapi_session_number
//#define EXTF_otapi_flush_sessions
//#define EXTF_otapi_is_session_blocked
//#define EXTF_otapi_put_command_tmpl
//#define EXTF_otapi_put_dialog_tmpl
//#define EXTF_otapi_put_query_tmpl
//#define EXTF_otapi_put_ack_tmpl
//#define EXTF_otapi_put_error_tmpl
//#define EXTF_otapi_put_isf_comp
//#define EXTF_otapi_put_isf_call
//#define EXTF_otapi_put_isf_return
//#define EXTF_otapi_put_reqds
//#define EXTF_otapi_put_propds
//#define EXTF_otapi_put_shell_tmpl





/// OTAPI EXTFs
//#define EXTF_otapi_ndef_idle
//#define EXTF_otapi_ndef_proc
//#define EXTF_otapi_alpext_proc
//#define EXTF_otapi_log_direct
//#define EXTF_otapi_log
//#define EXTF_otapi_log_msg
//#define EXTF_otapi_log_hexmsg
//#define EXTF_otapi_log_code





/// Queue EXTFs
//#define EXTF_q_init
//#define EXTF_q_rebase
//#define EXTF_q_copy
//#define EXTF_q_empty
//#define EXTF_q_start
//#define EXTF_q_markbyte
//#define EXTF_q_writebyte
//#define EXTF_q_writeshort
//#define EXTF_q_writeshort_be
//#define EXTF_q_writelong
//#define EXTF_q_readbyte
//#define EXTF_q_readshort
//#define EXTF_q_readshort_be
//#define EXTF_q_readlong
//#define EXTF_q_writestring
//#define EXTF_q_readstring





/// Radio EXTFs
//#define EXTF_radio_init
//#define EXTF_radio_rssi
//#define EXTF_radio_buffer
//#define EXTF_radio_off
//#define EXTF_radio_gag
//#define EXTF_radio_sleep
//#define EXTF_radio_idle
//#define EXTF_radio_flush_tx
//#define EXTF_radio_flush_rx
//#define EXTF_radio_putbyte
//#define EXTF_radio_putfourbytes
//#define EXTF_radio_getbyte
//#define EXTF_radio_getfourbytes
//#define EXTF_radio_rxopen
//#define EXTF_radio_rxopen_4
//#define EXTF_radio_txopen
//#define EXTF_radio_txopen_4
//#define EXTF_rm2_default_tgd
//#define EXTF_rm2_pkt_duration
//#define EXTF_rm2_scale_codec
//#define EXTF_rm2_rxinit_ff
//#define EXTF_rm2_rxinit_bf
//#define EXTF_rm2_txinit_ff
//#define EXTF_rm2_txinit_bf
//#define EXTF_rm2_txstop_flood
//#define EXTF_rm2_txcsma
//#define EXTF_rm2_kill
//#define EXTF_rm2_rxsync_isr
//#define EXTF_rm2_rxtimeout_isr
//#define EXTF_rm2_rxdata_isr
//#define EXTF_rm2_rxend_isr
//#define EXTF_rm2_txdata_isr






/// Session EXTFs
//#define EXTF_session_init
//#define EXTF_session_refresh
//#define EXTF_session_new
//#define EXTF_session_occupied
//#define EXTF_session_pop
//#define EXTF_session_flush
//#define EXTF_session_drop
//#define EXTF_session_count
//#define EXTF_session_top



/// System EXTFs
//#define EXTF_sys_init
//#define EXTF_sys_refresh
//#define EXTF_sys_change_settings
//#define EXTF_sys_goto_off
//#define EXTF_sys_goto_sleep
//#define EXTF_sys_goto_hold
//#define EXTF_sys_panic
//#define EXTF_sys_idle
//#define EXTF_sys_default_csma
//#define EXTF_sys_quit_rf
//#define EXTF_sys_set_mutex
//#define EXTF_sys_clear_mutex
//#define EXTF_sys_get_mutex
//#define EXTF_sys_event_manager
//#define EXTF_sys_sig_loadapp
#define EXTF_sys_sig_panic
#define EXTF_sys_sig_rfainit
#define EXTF_sys_sig_rfaterminate
//#define EXTF_sys_sig_btsprestart
//#define EXTF_sys_sig_hssprestart
//#define EXTF_sys_sig_sssprestart
#define EXTF_sys_sig_extprocess




/// Veelite Core EXTFs
//#define EXTF_vas_check
//#define EXTF_vworm_format
//#define EXTF_vworm_init
//#define EXTF_vworm_save
//#define EXTF_vworm_read
//#define EXTF_vworm_write
//#define EXTF_vworm_mark
//#define EXTF_vworm_mark_physical
//#define EXTF_vworm_get
//#define EXTF_vworm_print_table
//#define EXTF_vworm_wipeblock
//#define EXTF_vsram_read
//#define EXTF_vsram_mark
//#define EXTF_vsram_mark_physical
//#define EXTF_vsram_get



/// Veelite Module EXTFs
//#define EXTF_vl_init
//#define EXTF_vl_get_fp
//#define EXTF_vl_get_fd
//#define EXTF_vl_new
//#define EXTF_vl_delete
//#define EXTF_vl_getheader_vaddr
//#define EXTF_vl_getheader
//#define EXTF_vl_open_file
//#define EXTF_vl_open
//#define EXTF_GFB_open_su
//#define EXTF_ISFS_open_su
//#define EXTF_ISF_open_su
//#define EXTF_GFB_open
//#define EXTF_ISFS_open
//#define EXTF_ISF_open
//#define EXTF_vl_chmod
//#define EXTF_GFB_chmod_su
//#define EXTF_ISFS_chmod_su
//#define EXTF_ISF_chmod_su
//#define EXTF_vl_read
//#define EXTF_vl_write
//#define EXTF_vl_close
//#define EXTF_vl_checklength
//#define EXTF_vl_checkalloc
//#define EXTF_ISF_syncmirror
//#define EXTF_ISF_loadmirror




#endif 
