/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017,
 *               2018, 2019, 2020
 *   Jonathan Schleifer <js@nil.im>
 *
 * All rights reserved.
 *
 * This file is part of ObjFW. It may be distributed under the terms of the
 * Q Public License 1.0, which can be found in the file LICENSE.QPL included in
 * the packaging of this file.
 *
 * Alternatively, it may be distributed under the terms of the GNU General
 * Public License, either version 2 or 3, which can be found in the file
 * LICENSE.GPLv2 or LICENSE.GPLv3 respectively included in the packaging of this
 * file.
 */

#include <ObjFW/ObjFW.h>
#include <string.h>

#import "ObjFW/OFArray.h"
#import "ObjFW/OFData+MessagePackValue.h"
#import "ObjFW/OFDate.h"
#import "ObjFW/OFDictionary.h"
#import "ObjFW/OFMessagePackExtension.h"
#import "ObjFW/OFNull.h"
#import "ObjFW/OFNumber.h"
#import "ObjFW/OFString.h"

#import "ObjFW/OFInvalidArgumentException.h"
#import "ObjFW/OFInvalidFormatException.h"
#import "ObjFW/OFOutOfRangeException.h"
#import "ObjFW/OFTruncatedDataException.h"

static size_t parseObject (const unsigned char * buffer, size_t length,
                           id * object, size_t depthLimit);

static uint16_t readUInt16 (const unsigned char * buffer)
{
    return ((uint16_t)buffer[0] << 8) | buffer[1];
}

static uint32_t readUInt32 (const unsigned char * buffer)
{
    return ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) |
           ((uint32_t)buffer[2] << 8) | buffer[3];
}

static uint64_t readUInt64 (const unsigned char * buffer)
{
    return ((uint64_t)buffer[0] << 56) | ((uint64_t)buffer[1] << 48) |
           ((uint64_t)buffer[2] << 40) | ((uint64_t)buffer[3] << 32) |
           ((uint64_t)buffer[4] << 24) | ((uint64_t)buffer[5] << 16) |
           ((uint64_t)buffer[6] << 8) | buffer[7];
}

static size_t parseArray (const unsigned char * buffer, size_t length,
                          id * object, size_t count, size_t depthLimit)
{
    void * pool;
    size_t pos = 0;

    if (--depthLimit == 0)
        @throw [OFOutOfRangeException exception];

    /*
     * Don't use capacity! For data and strings, this is safe, as we can
     * check if we still have enough bytes left. For an array however, we
     * can't know this, as every child can be more than one byte.
     */
    *object = [OFMutableArray array];

    for (size_t i = 0; i < count; i++)
    {
        id child;

        pool = objc_autoreleasePoolPush ();

        pos += parseObject (buffer + pos, length - pos, &child, depthLimit);

        [*object addObject:child];

        objc_autoreleasePoolPop (pool);
    }

    return pos;
}

static size_t parseTable (const unsigned char * buffer, size_t length,
                          id * object, size_t count, size_t depthLimit)
{
    void * pool;
    size_t pos = 0;

    if (--depthLimit == 0)
        @throw [OFOutOfRangeException exception];

    /*
     * Don't use capacity! For data and strings, this is safe, as we can
     * check if we still have enough bytes left. For a dictionary however,
     * we can't know this, as every key / value can be more than one byte.
     */
    *object = [OFMutableDictionary dictionary];

    for (size_t i = 0; i < count; i++)
    {
        id key, value;

        pool = objc_autoreleasePoolPush ();

        pos += parseObject (buffer + pos, length - pos, &key, depthLimit);
        pos += parseObject (buffer + pos, length - pos, &value, depthLimit);

        [*object setObject:value forKey:key];

        objc_autoreleasePoolPop (pool);
    }

    return pos;
}

static OFDate * createDate (OFData * data)
{
    switch (data.count)
    {
    case 4:
    {
        uint32_t timestamp;

        memcpy (&timestamp, data.items, 4);
        timestamp = OF_BSWAP32_IF_LE (timestamp);

        return [OFDate dateWithTimeIntervalSince1970:timestamp];
    }
    case 8:
    {
        uint64_t combined;

        memcpy (&combined, data.items, 8);
        combined = OF_BSWAP64_IF_LE (combined);

        return [OFDate
            dateWithTimeIntervalSince1970:(double)(combined & 0x3FFFFFFFF) +
                                          (double)(combined >> 34) /
                                              1000000000];
    }
    case 12:
    {
        uint32_t nanoseconds;
        int64_t seconds;

        memcpy (&nanoseconds, data.items, 4);
        memcpy (&seconds, (char *)data.items + 4, 8);

        nanoseconds = OF_BSWAP32_IF_LE (nanoseconds);
        seconds = OF_BSWAP64_IF_LE (seconds);

        return [OFDate
            dateWithTimeIntervalSince1970:(double)seconds +
                                          (double)nanoseconds / 1000000000];
    }
    default:
        @throw [OFInvalidFormatException exception];
    }
}

static id createExtension (int8_t type, OFData * data)
{
    switch (type)
    {
    case -1:
        return createDate (data);
    default:
        return [OFMessagePackExtension extensionWithType:type data:data];
    }
}

static size_t parseObject (const unsigned char * buffer, size_t length,
                           id * object, size_t depthLimit)
{
    size_t count;
    OFData * data;

    if (length < 1)
        @throw [OFTruncatedDataException exception];

    /* positive fixint */
    if ((buffer[0] & 0x80) == 0)
    {
        *object = [OFNumber numberWithUInt8:buffer[0] & 0x7F];
        return 1;
    }
    /* negative fixint */
    if ((buffer[0] & 0xE0) == 0xE0)
    {
        *object = [OFNumber numberWithInt8:((int8_t) (buffer[0] & 0x1F)) - 32];
        return 1;
    }

    /* fixstr */
    if ((buffer[0] & 0xE0) == 0xA0)
    {
        count = buffer[0] & 0x1F;

        if (length < count + 1)
            @throw [OFTruncatedDataException exception];

        *object = [OFString stringWithUTF8String:(const char *)buffer + 1
                                          length:count];
        return count + 1;
    }

    /* fixarray */
    if ((buffer[0] & 0xF0) == 0x90)
        return parseArray (buffer + 1,
                           length - 1,
                           object,
                           buffer[0] & 0xF,
                           depthLimit) +
               1;

    /* fixmap */
    if ((buffer[0] & 0xF0) == 0x80)
        return parseTable (buffer + 1,
                           length - 1,
                           object,
                           buffer[0] & 0xF,
                           depthLimit) +
               1;

    /* Prefix byte */
    switch (buffer[0])
    {
    /* Unsigned integers */
    case 0xCC: /* uint8 */
        if (length < 2)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithUInt8:buffer[1]];
        return 2;
    case 0xCD: /* uint 16 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithUInt16:readUInt16 (buffer + 1)];
        return 3;
    case 0xCE: /* uint 32 */
        if (length < 5)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithUInt32:readUInt32 (buffer + 1)];
        return 5;
    case 0xCF: /* uint 64 */
        if (length < 9)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithUInt64:readUInt64 (buffer + 1)];
        return 9;
    /* Signed integers */
    case 0xD0: /* int 8 */
        if (length < 2)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithInt8:buffer[1]];
        return 2;
    case 0xD1: /* int 16 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithInt16:readUInt16 (buffer + 1)];
        return 3;
    case 0xD2: /* int 32 */
        if (length < 5)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithInt32:readUInt32 (buffer + 1)];
        return 5;
    case 0xD3: /* int 64 */
        if (length < 9)
            @throw [OFTruncatedDataException exception];

        *object = [OFNumber numberWithInt64:readUInt64 (buffer + 1)];
        return 9;
    /* Floating point */
    case 0xCA:; /* float 32 */
        union
        {
            unsigned char u8[4];
            float f;
        } f;

        if (length < 5)
            @throw [OFTruncatedDataException exception];

        memcpy (&f.u8, buffer + 1, 4);

        *object = [OFNumber numberWithFloat:OF_BSWAP_FLOAT_IF_LE (f.f)];
        return 5;
    case 0xCB:; /* float 64 */
        union
        {
            unsigned char u8[8];
            double d;
        } d;

        if (length < 9)
            @throw [OFTruncatedDataException exception];

        memcpy (&d.u8, buffer + 1, 8);

        *object = [OFNumber numberWithDouble:OF_BSWAP_DOUBLE_IF_LE (d.d)];
        return 9;
    /* nil */
    case 0xC0:
        *object = [OFNull null];
        return 1;
    /* false */
    case 0xC2:
        *object = [OFNumber numberWithBool:false];
        return 1;
    /* true */
    case 0xC3:
        *object = [OFNumber numberWithBool:true];
        return 1;
    /* Data */
    case 0xC4: /* bin 8 */
        if (length < 2)
            @throw [OFTruncatedDataException exception];

        count = buffer[1];

        if (length < count + 2)
            @throw [OFTruncatedDataException exception];

        *object = [OFData dataWithItems:buffer + 2 count:count];

        return count + 2;
    case 0xC5: /* bin 16 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        count = readUInt16 (buffer + 1);

        if (length < count + 3)
            @throw [OFTruncatedDataException exception];

        *object = [OFData dataWithItems:buffer + 3 count:count];

        return count + 3;
    case 0xC6: /* bin 32 */
        if (length < 5)
            @throw [OFTruncatedDataException exception];

        count = readUInt32 (buffer + 1);

        if (length < count + 5)
            @throw [OFTruncatedDataException exception];

        *object = [OFData dataWithItems:buffer + 5 count:count];

        return count + 5;
    /* Extensions */
    case 0xC7: /* ext 8 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        count = buffer[1];

        if (length < count + 3)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 3 count:count];
        @try
        {
            *object = createExtension (buffer[2], data);
        }
        @finally
        {
            //[data release];
        }

        return count + 3;
    case 0xC8: /* ext 16 */
        if (length < 4)
            @throw [OFTruncatedDataException exception];

        count = readUInt16 (buffer + 1);

        if (length < count + 4)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 4 count:count];
        @try
        {
            *object = createExtension (buffer[3], data);
        }
        @finally
        {
            //[data release];
        }

        return count + 4;
    case 0xC9: /* ext 32 */
        if (length < 6)
            @throw [OFTruncatedDataException exception];

        count = readUInt32 (buffer + 1);

        if (length < count + 6)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 6 count:count];
        @try
        {
            *object = createExtension (buffer[5], data);
        }
        @finally
        {
            //[data release];
        }

        return count + 6;
    case 0xD4: /* fixext 1 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 2 count:1];
        @try
        {
            *object = createExtension (buffer[1], data);
        }
        @finally
        {
            // [data release];
        }

        return 3;
    case 0xD5: /* fixext 2 */
        if (length < 4)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 2 count:2];
        @try
        {
            *object = createExtension (buffer[1], data);
        }
        @finally
        {
            //[data release];
        }

        return 4;
    case 0xD6: /* fixext 4 */
        if (length < 6)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 2 count:4];
        @try
        {
            *object = createExtension (buffer[1], data);
        }
        @finally
        {
            //[data release];
        }

        return 6;
    case 0xD7: /* fixext 8 */
        if (length < 10)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 2 count:8];
        @try
        {
            *object = createExtension (buffer[1], data);
        }
        @finally
        {
            //[data release];
        }

        return 10;
    case 0xD8: /* fixext 16 */
        if (length < 18)
            @throw [OFTruncatedDataException exception];

        data = [[OFData alloc] initWithItems:buffer + 2 count:16];
        @try
        {
            *object = createExtension (buffer[1], data);
        }
        @finally
        {
            //[data release];
        }

        return 18;
    /* Strings */
    case 0xD9: /* str 8 */
        if (length < 2)
            @throw [OFTruncatedDataException exception];

        count = buffer[1];

        if (length < count + 2)
            @throw [OFTruncatedDataException exception];

        *object = [OFString stringWithUTF8String:(const char *)buffer + 2
                                          length:count];
        return count + 2;
    case 0xDA: /* str 16 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        count = readUInt16 (buffer + 1);

        if (length < count + 3)
            @throw [OFTruncatedDataException exception];

        *object = [OFString stringWithUTF8String:(const char *)buffer + 3
                                          length:count];
        return count + 3;
    case 0xDB: /* str 32 */
        if (length < 5)
            @throw [OFTruncatedDataException exception];

        count = readUInt32 (buffer + 1);

        if (length < count + 5)
            @throw [OFTruncatedDataException exception];

        *object = [OFString stringWithUTF8String:(const char *)buffer + 5
                                          length:count];
        return count + 5;
    /* Arrays */
    case 0xDC: /* array 16 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        return parseArray (buffer + 3,
                           length - 3,
                           object,
                           readUInt16 (buffer + 1),
                           depthLimit) +
               3;
    case 0xDD: /* array 32 */
        if (length < 5)
            @throw [OFTruncatedDataException exception];

        return parseArray (buffer + 5,
                           length - 5,
                           object,
                           readUInt32 (buffer + 1),
                           depthLimit) +
               5;
    /* Maps */
    case 0xDE: /* map 16 */
        if (length < 3)
            @throw [OFTruncatedDataException exception];

        return parseTable (buffer + 3,
                           length - 3,
                           object,
                           readUInt16 (buffer + 1),
                           depthLimit) +
               3;
    case 0xDF: /* map 32 */
        if (length < 5)
            @throw [OFTruncatedDataException exception];

        return parseTable (buffer + 5,
                           length - 5,
                           object,
                           readUInt32 (buffer + 1),
                           depthLimit) +
               5;
    default:
        @throw [OFInvalidFormatException exception];
    }
}

@implementation OFMutableData (MV)
- (id)nextMessagePackValue
{
    return [self nextMessagePackValueWithDepthLimit:32];
}

- (id)nextMessagePackValueWithDepthLimit:(size_t)depthLimit
{
    void * pool = objc_autoreleasePoolPush ();
    size_t count = self.count, res;
    id object;

    if (self.itemSize != 1)
        @throw [OFInvalidArgumentException exception];

    @try
    {
        res = parseObject (self.items, count, &object, depthLimit);
    }
    @catch (OFTruncatedDataException * exception)
    {
        res = 0;
    }

    if (res)
    {
        [self removeItemsInRange:of_range (0, res)];
    }

    //[object retain];

    objc_autoreleasePoolPop (pool);

    return object; //[object autorelease];
}

@end