// Source-only LZW integration. Complete actual decoder, bit reader and allocator
// objects are linked separately; no retail dispatch, mocks or redirected calls.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/reconstructed/Lzw.h"
#include "../../src/reconstructed/MemTag.h"

struct LzwDictionaryEntry { int codeValue; unsigned int parentCode; char character; };
extern unsigned int g_lzwNextCode;
extern unsigned int g_lzwNextBumpCode;
extern int g_lzwCodeBits;
extern LzwDictionaryEntry* g_lzwDictionary[137];
extern unsigned int g_BitReaderWord;
extern int g_BitReaderBitsUsed;
extern unsigned int* g_BitReaderCursor;

struct String { unsigned short length; unsigned char bytes[256]; };
static String dictionary[1024], previous;
static unsigned char packed[8192], original[8192], expected[65536], output[65568];
static unsigned int parents[35023];
static unsigned char tails[35023];
static void* banks[137];
static int bitCount, width, nextCode, previousCode, outputCount, first, scenarios;

static void Fail(const char* why) { printf("LZW oracle failure %d: %s\n", scenarios, why); exit(2); }
static void Put(unsigned int value, int bits) {
    if (bitCount + bits >= (int)sizeof(packed)*8-64) Fail("packed capacity");
    for (int b=bits-1;b>=0;--b,++bitCount)
        packed[bitCount/8] |= ((value>>b)&1) << (7-(bitCount&7));
}
static void ResetOracle() {
    width=9; nextCode=259; first=1;
    memset(dictionary,0,sizeof(dictionary));
    for (int i=0;i<256;++i) { dictionary[i].length=1; dictionary[i].bytes[0]=(unsigned char)i; }
}
static void Begin(unsigned int prefix) {
    memset(packed,0,sizeof(packed)); memset(expected,0,sizeof(expected));
    bitCount=0;outputCount=0;Put(prefix,32);ResetOracle();
}
static void Emit(unsigned int code) {
    if (code >= (1u<<width)) Fail("code width");
    Put(code,width);
    if (code==256) return;
    if (code==257) { if(first||width==15)Fail("bump domain");++width;return; }
    if (code==258) { if(first)Fail("flush domain");ResetOracle();return; }
    String current;
    if (code<(unsigned)nextCode && code!=257 && code!=258) current=dictionary[code];
    else if (!first && code==(unsigned)nextCode) {
        current=previous;
        if(current.length>=256)Fail("string capacity");
        current.bytes[current.length++]=previous.bytes[0];
    } else Fail("code domain");
    if (!current.length||outputCount+current.length>(int)sizeof(expected))Fail("output capacity");
    memcpy(expected+outputCount,current.bytes,current.length);outputCount+=current.length;
    if (!first) {
        if(nextCode>=1024||previous.length>=256)Fail("dictionary capacity");
        dictionary[nextCode]=previous;
        dictionary[nextCode].bytes[dictionary[nextCode].length++]=current.bytes[0];
        parents[nextCode]=previousCode;tails[nextCode]=current.bytes[0];++nextCode;
    }
    previous=current;previousCode=code;first=0;
}
static unsigned int Big(const unsigned char* p) {
    return ((unsigned int)p[0]<<24)|((unsigned int)p[1]<<16)|((unsigned int)p[2]<<8)|p[3];
}
static void Run() {
    ++scenarios;
    memcpy(original,packed,sizeof(packed));memset(output,0xa5,sizeof(output));
    int before=g_MemTagStack->ours[g_MemTagStack->current];
    ExpandLzw(output+16,packed,outputCount);
    if(memcmp(output+16,expected,outputCount)||memcmp(original,packed,sizeof(packed)))Fail("payload/input");
    for(int g=0;g<16;++g)if(output[g]!=0xa5)Fail("leading output guard");
    for(g=16+outputCount;g<(int)sizeof(output);++g)if(output[g]!=0xa5)Fail("trailing output guard");
    if(g_lzwNextCode!=(unsigned)nextCode||g_lzwCodeBits!=width||g_lzwNextBumpCode!=511)Fail("decoder scalars");
    if(g_BitReaderBitsUsed!=(bitCount&31)||g_BitReaderCursor!=(unsigned int*)(packed+4*(bitCount/32))||g_BitReaderWord!=Big(packed+4*(bitCount/32)))Fail("reader state/eager lookahead");
    for(int i=0;i<137;++i) {
        if(!g_lzwDictionary[i])Fail("missing bank");
        if(scenarios>1&&banks[i]!=g_lzwDictionary[i])Fail("bank reallocation");
        banks[i]=g_lzwDictionary[i];
    }
    for(i=0;i<137*256;++i) {
        LzwDictionaryEntry& item=g_lzwDictionary[i>>8][i&255];
        if(item.codeValue!=(i<35023?-1:0)||item.parentCode!=(i<35023?parents[i]:0)||(unsigned char)item.character!=(i<35023?tails[i]:0))Fail("complete dictionary contents");
        unsigned char* p=(unsigned char*)&item;
        if(p[9]||p[10]||p[11])Fail("dictionary padding changed");
    }
    if(g_MemTagStack->ours[g_MemTagStack->current]-before!=(scenarios==1?137*256*12:0))Fail("actual allocation accounting");
}
int main() {
    if(sizeof(LzwDictionaryEntry)!=12||!g_MemTagStack||g_MemTagStack->count<1||!g_MemTagStack->ours)return 80;
    Begin(0x12345678);Emit(256);Run();
    Begin(0);Emit('A');Emit(259);Emit(260);Emit(256);
    if(outputCount!=6||memcmp(expected,"AAAAAA",6))Fail("independent KwKwK example");Run();
    Begin(0xffffffff);Emit('A');Emit('B');Emit(259);Emit(260);Emit(258);Emit('C');Emit(259);Emit(256);
    if(outputCount!=9||memcmp(expected,"ABABBACCC",9))Fail("independent prior/flush example");Run();
    // Every byte as a literal; seven widths9..15, flush, dictionary reuse, and
    // repeating calls with existing banks. The encoder oracle keeps complete
    // forward strings, unlike the decoder's parent-linked reverse stack.
    for(int seed=0;seed<256;++seed)for(int mode=0;mode<8;++mode) {
        Begin(0x9e3779b9u*(seed+1));Emit(seed);
        for(int step=0;step<48;++step) {
            if(mode&1)if(step==8||step==24) { Emit(258);Emit((seed+step)&255); }
            if(mode&2)if(step%7==0&&width<15)Emit(257);
            unsigned int code;
            if((mode&4)&&step%3==0)code=nextCode;
            else if(step%5==0&&nextCode>259)code=259+(seed+step)%(nextCode-259);
            else code=(seed*17+step*29)&255;
            Emit(code);
        }
        Emit(256);Run();
    }
    printf("LzwSourceDecode: %d packed-stream, dictionary, allocation and reader-state cases PASS\n",scenarios);
    return 0;
}
