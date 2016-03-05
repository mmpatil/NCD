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
 * @file: ip_checksum.h
 */

#ifndef IP_CHECKSUM_H_
#define IP_CHECKSUM_H_ 1


#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/time.h>

/*  Just returns current time as double, with most possible precision...  */
double get_time()
{
    struct timeval tv;
    double d;
    gettimeofday(&tv, NULL);
    d = ((double)tv.tv_usec) / 1000000. + (unsigned long)tv.tv_sec;
    return d;
}



uint16_t ip_checksum(void* vdata, size_t length)
{
    /* Cast the data pointer to one that can be indexed. */
    char* data = (char*)vdata;

    /* Initialize the accumulator. */
    uint64_t acc = 0xffff;

    /* Handle any partial block at the start of the data. */
    unsigned int offset = ((uintptr_t)data) & 3;
    if(offset)
    {
        size_t count = 4 - offset;
        if(count > length)
            count     = length;
        uint32_t word = 0;
        memcpy(offset + (char*)&word, data, count);
        acc += ntohl(word);
        data += count;
        length -= count;
    }

    /* Handle any complete 32-bit blocks. */
    char* data_end = data + (length & ~3);
    while(data != data_end)
    {
        uint32_t word = 0;
        memcpy(&word, data, 4);
        acc += ntohl(word);
        data += 4;
    }
    length &= 3;

    /* Handle any partial block at the end of the data. */
    if(length)
    {
        uint32_t word = 0;
        memcpy(&word, data, length);
        acc += ntohl(word);
    }

    /* Handle deferred carries. */
    acc = (acc & 0xffffffff) + (acc >> 32);
    while(acc >> 16)
    {
        acc = (acc & 0xffff) + (acc >> 16);
    }

    /* If the data began at an odd byte address */
    /* then reverse the byte order to compensate. */
    if(offset & 1)
    {
        acc = ((acc & 0xff00) >> 8) | ((acc & 0x00ff) << 8);
    }

    /* Return the checksum in network byte order. */
    return htons(~acc);
}


#endif /* ifndef IP_CHECKSUM_H_ */
