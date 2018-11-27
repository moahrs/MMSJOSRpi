//
// types.h
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _uspienv_types_h
#define _uspienv_types_h

#define __bitwise __attribute__((bitwise))
#define __force

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

typedef signed char			s8;
typedef signed short		s16;
typedef signed int			s32;
typedef signed long long	s64;

typedef unsigned char		le8;
typedef unsigned short		le16;
typedef unsigned int		le32;
typedef unsigned long long	le64;

typedef s16 /*__bitwise*/ sle16;
typedef s32 /*__bitwise*/ sle32;
typedef s64 /*__bitwise*/ sle64;

typedef int		boolean;

#define FALSE		0
#define TRUE		1

#endif
