//
//  dynamic_perfect_hash.h
//  train_throttle
//
//  Created by Daniel Braun on 11/11/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef dynamic_perfect_hash_h
#define dynamic_perfect_hash_h

typedef struct {
    uint8_t p0;
    uint8_t p1;
    uint8_t p2;
    uint8_t p3;
    // ---
    int maxN0;
} dph64_def_t;


#define DPH_NPASS 1

static inline uint8_t rotateLeft8(uint8_t x, int n)
{
    return (x << (n & 7)) | (x >> (8 - (n & 7)));
}

static inline uint16_t rotateLeft16(uint16_t x, int n)
{
    return (x << (n & 0xF)) | (x >> (16 - (n & 0xF)));
}

// 256->64 hash function

static inline void dph64_register(dph64_def_t *dph, int npass, uint8_t v8, int numhash)
{
   
    
}

static inline uint8_t dph64_hash(dph64_def_t *dph, uint8_t v8, int mod)
{
    uint16_t v = (uint16_t)v8;
    uint16_t h1 = ((dph->p2+1) * v + dph->p1) % (dph->p0+100);
    //uint16_t h2 = v % mod;
    //uint16_t h3 = (v + dph->p0) / mod;;
    uint16_t h = h1; //+h2+h3;
    return h%mod;
#if 0
    //if (!dph->p0) return 0; // for tests
    // return (a*K) >> (w-m)
    
    uint16_t h = v8; //+ 0x143f*v8;
    //h += h*0x123;
    //h ^= ((~h) << 3) ;
    h ^= rotateLeft16(h, dph->p0) ;
    h ^= (h>>5);// ^(h>>5);
    h += rotateLeft16(h, dph->p2);
    //if (dph->p2) h ^= h % ;
    h ^= rotateLeft16(h, dph->p1);
    h += h>>dph->p3;
    //h &= 0x3FF;
    //h ^= v8;
    //h ^= h>>dph->p0;// rotateLeft16(h, dph->p0) ;
    h ^= (h>>7);
    //h = h % 1373;
    //h ^= rotateLeft16(h*0x1ef,dph->p2);
    //h ^= h >> dph->p2;
    //h ^= h << 7;
   
    
    
    return h%mod;
    /*
    float M = 0.5*(sqrt(5) - 1) + dph->p1/128.0;
    float z = (float)(v8+dph->p0) * M;
    float f = z - (int)z;
    int16_t h = floor(64*f);
    return h;
     */
    /*
    uint16_t h = v8+dph->p0;
    h ^= h >> 3;
    h *= (0x8ccd+dph->p2);
    h ^= h >> dph->p1;
    h *= 0xec53;
    h ^= h >> 3;
    return h % mod;
    */
    
    
    
    
    
#if 0
    uint16_t v = v8;
    uint16_t h = v8;
    h ^= (v8 & 0xF0)>>4;
    h ^= (v8 & 0x0F)<<4;
    h |= (v8<<8);

    h ^= rotateLeft16(h+dph->p3, dph->p0);
    h ^= rotateLeft16(h*(0x1785+dph->p2), dph->p1) ;
#endif
    
#if 0
    h = h + (h<<5) + (h>>2) + (h>>7);
    h = h*0x1785;
    h ^= rotateLeft16(h+dph->p2, dph->p0);
    h += (h+dph->p3)>>dph->p1;
#endif
#if 0
    h ^= h * 0x1321;
    h += ((h*0x3851)<<dph->p0);
    h ^= v*0x7C+0x27eb;
    h ^= ((h*0x137)>>dph->p1);
    h ^= v*dph->p2+87;
    //h ^= rotateLeft16(v*7d, dph->p1);
    //h ^= (h<<dph->p2);
    //h ^= v*0xad;
    //h += ((v^0xdead)<<3);
    //h ^= (h<<11);
    //h ^= (h>>15);
    //v += rotateLeft8(v ^ dph->p1, 3);
    //if (!dph->p1) return 0; // for tests
    //h ^= (v+0) >> dph->p1;
    //if (!dph->p2) return 0; // for tests
    //h ^= (v+0) << dph->p2;
    //h ^= (v<<3)+dph->p2;
    h = h ^ (h >>8) ^((h*0x1339)>>11);
#endif
#endif
}

static inline void dph64_start(dph64_def_t *dph)
{
    dph->p0 = 0;
    dph->p1 = 0;
    dph->p2 = 0;
    dph->p3 = 0;
}
#define MAX_P0  255
#define MAX_P1  255
#define MAX_P2  255
#define MAX_P3  0

static inline int dph64_next(dph64_def_t *dph)
{
    if (dph->p0 < MAX_P0) {
        dph->p0++;
        return 0;
    }
    dph->p0 = 0;
    
    if (dph->p1 < MAX_P1) {
        dph->p1++;
        return 0;
    }
    dph->p1 = 0;
    
    if (dph->p2 < MAX_P2) {
        dph->p2++;
        return 0;
    }
    dph->p2 = 0;
    
    if (dph->p3 < MAX_P3) {
        dph->p3++;
        return 0;
    }
    return -1;
}
#endif /* dynamic_perfect_hash_h */
