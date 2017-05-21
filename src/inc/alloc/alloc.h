/*
   Copyright 2017, Mike Krinkin <krinkin.m.u@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stdint.h>

struct myfs;

int myfs_reserve(struct myfs *myfs, uint64_t size, uint64_t *offs);
int myfs_cancel(struct myfs *myfs, uint64_t size, uint64_t offs);

#endif /*__ALLOC_H__*/
