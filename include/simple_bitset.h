/*
  The MIT License

  Copyright (c) 2015-2016 Paul Kirth pk1574@gmail.com

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/**
 * @author: Paul Kirth
 * @file: bitset.h
 */

#ifndef SIMPLE_BITSET_H_
#define SIMPLE_BITSET_H_

#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

/**
 * returns the value of the indexth bit in a 32 bit bitset
 * starting at buff
 * @param buff a pointer to the beginning of a bitset
 * @param index the index of the bit to be queried
 * @param size	the number of items in the bitset
 * @return the value of the indexth bit in the bitset (o or 1)
 */
static inline int get_bs_32(uint32_t* buff, size_t index, size_t size)
{
    if(index >= size)
        return -1;
    int offset    = index / 32;
    uint32_t* ptr = (uint32_t*)buff;
    ptr += offset;
    uint32_t mask = 1 << ((index) % 32);
    return (*ptr) & mask;
}

/**
 * creates a bitset of num_items using 32 bit integers
 * @param num_items the number of items in the bitset
 * @return a pointer to the beginning of the bitset
 */
static inline uint32_t* make_bs_32(size_t num_items)
{
    int len = (int)ceil(num_items / 32.0);
    return (uint32_t*)calloc(len, sizeof(uint32_t));
}

/**
 * clears the bit at index
 * @param buff pointer to the beginning of the bitset
 * @param index the bit to be cleared
 * @param size the size of the bitset
 * @return 0 for success, -1 for failure/error
 */
static inline int clear_bs_32(uint32_t* buff, size_t index, size_t size)
{
    if(index >= size)
        return -1;
    int offset    = index / 32;
    uint32_t* ptr = (uint32_t*)buff;
    ptr += offset;
    uint32_t mask = 1 << (index % 32);
    *ptr &= ~mask;
    return 0;
}

/**
 * sets the bit at index to 1
 * @param buff pointer to the beginning of the bitset
 * @param index the bit to be cleared
 * @param size the size of the bitset
 * @return 0 for success, -1 for failure/error
 */
static inline int set_bs_32(uint32_t* buff, size_t index, size_t size)
{
    if(index > size)
        return -1;
    uint32_t* ptr = (uint32_t*)buff;
    // int offset = index /32;
    ptr += (index >> 5);
    uint32_t mask = 1 << ((index % 32));
    // uint32_t mask = 1 << (index & 31);
    (*ptr) |= mask;
    return 0;
}

#endif