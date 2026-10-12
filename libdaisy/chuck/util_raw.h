/*----------------------------------------------------------------------------
  ChucK Strongly-timed Audio Programming Language
    Compiler, Virtual Machine, and Synthesis Engine

  Copyright (c) 2003 Ge Wang and Perry R. Cook. All rights reserved.
    http://chuck.stanford.edu/
    http://chuck.cs.princeton.edu/

  This program is free software; you can redistribute it and/or modify
  it under the dual-license terms of EITHER the MIT License OR the GNU
  General Public License (the latter as published by the Free Software
  Foundation; either version 2 of the License or, at your option, any
  later version).

  This program is distributed in the hope that it will be useful and/or
  interesting, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  MIT Licence and/or the GNU General Public License for details.

  You should have received a copy of the MIT License and the GNU General
  Public License (GPL) along with this program; a copy of the GPL can also
  be obtained by writing to the Free Software Foundation, Inc., 59 Temple
  Place, Suite 330, Boston, MA 02111-1307 U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: util_raw.h
// desc: raw waves from STK
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2004
//-----------------------------------------------------------------------------
#ifndef __UTIL_RAW_H__
#define __UTIL_RAW_H__

#include "chuck_def.h"


#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

// begin the abuse of memory
extern const SAMPLE ahh_data[];
extern const SAMPLE britestk_data[];
extern const SAMPLE dope_data[];
extern const SAMPLE eee_data[];
extern const SAMPLE fwavblnk_data[];
extern const SAMPLE halfwave_data[];
extern const SAMPLE impuls10_data[];
extern const SAMPLE impuls20_data[];
extern const SAMPLE impuls40_data[];
extern const SAMPLE mand1_data[];
extern const SAMPLE mandpluk_data[];
extern const SAMPLE marmstk1_data[];
extern const SAMPLE ooo_data[];
extern const SAMPLE peksblnk_data[];
extern const SAMPLE ppksblnk_data[];
extern const SAMPLE silence_data[];
extern const SAMPLE sineblnk_data[];
extern const SAMPLE sinewave_data[];
extern const SAMPLE snglpeak_data[];
extern const SAMPLE twopeaks_data[];
extern const SAMPLE glot_ahh_data[];
extern const SAMPLE glot_eee_data[];
extern const SAMPLE glot_ooo_data[];
extern const SAMPLE glot_pop_data[];

extern t_CKUINT ahh_size;
extern t_CKUINT britestk_size;
extern t_CKUINT dope_size;
extern t_CKUINT eee_size;
extern t_CKUINT fwavblnk_size;
extern t_CKUINT halfwave_size;
extern t_CKUINT impuls10_size;
extern t_CKUINT impuls20_size;
extern t_CKUINT impuls40_size;
extern t_CKUINT mand1_size;
extern t_CKUINT mandpluk_size;
extern t_CKUINT marmstk1_size;
extern t_CKUINT ooo_size;
extern t_CKUINT peksblnk_size;
extern t_CKUINT ppksblnk_size;
extern t_CKUINT silence_size;
extern t_CKUINT sineblnk_size;
extern t_CKUINT sinewave_size;
extern t_CKUINT snglpeak_size;
extern t_CKUINT twopeaks_size;
extern t_CKUINT glot_ahh_size;
extern t_CKUINT glot_eee_size;
extern t_CKUINT glot_ooo_size;
extern t_CKUINT glot_pop_size;


#if defined(_cplusplus) || defined(__cplusplus)
}
#endif


#endif
