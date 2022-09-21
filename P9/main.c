#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>

#define MEMSIZE 1000
#define WRDSIZE 12
#define HWRDSIZE 6

#define xn(v) xs3n[((v).zone << 4) + (v).code]

#define PCR printf("\n")

#define PULSE 2

FILE* uvconsole;

typedef struct {
	uint8_t p7 : 1;
	uint8_t zone : 2;
	uint8_t code : 4;
} uvchar;

typedef struct {
	char name[9];
	uint8_t index;
	uint8_t ilock;
	uint8_t density;
	int currpos;
} uvtape;

typedef uvchar uvword[WRDSIZE];
typedef uvchar uvhword[HWRDSIZE];

uvword memory[MEMSIZE];

/* arithmetic registers */
uvword rA;
uvword rX;
uvword rL;
uvword rF;

/* control registers */
uvword CC;
uvword CR;
uvhword SR;

/* transfer registers */
uvword rV[2];
uvword rY[10];
uvword rI[60];
uvword rO[60];

uvtape tape1 = { "tape1.tap",1,0,100,0 };
uvtape tape2 = { "tape2.tap",2,0,100,0 };
uvtape tape3 = { "tape3.tap",3,0,100,0 };
uvtape tape4 = { "tape4.tap",4,0,100,0 };
uvtape tape5 = { "tape5.tap",5,0,100,0 };
uvtape tape6 = { "tape6.tap",6,0,100,0 };
uvtape tape7 = { "tape7.tap",7,0,100,0 };
uvtape tape8 = { "tape8.tap",8,0,100,0 };
uvtape tape9 = { "tape9.tap",9,0,100,0 };

FILE* t1;
FILE* t2;
FILE* t3;
FILE* t4;
FILE* t5;
FILE* t6;
FILE* t7;
FILE* t8;
FILE* t9;


uint8_t xs3n[13] = { 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

uvchar u0 = { 1, 0, 3 };

const char* bitr16[16] = {
	 [0] = "0000", [1] = "0001", [2] = "0010", [3] = "0011",
	 [4] = "0100", [5] = "0101", [6] = "0110", [7] = "0111",
	 [8] = "1000", [9] = "1001",[10] = "1010",[11] = "1011",
	[12] = "1100",[13] = "1101",[14] = "1110",[15] = "1111",
};

const char* bitr4[4] = {
	 [0] = "00",[1] = "01",[2] = "10",[3] = "11",
};

const char* bitr2[2] = {
	 [0] = "0",[1] = "1",
};

char xs3ascii[64] = {
						 0,32,45,48,49,50,51,52,53,54,//0
						55,56,57, 0, 0, 0,13,44,46,59,//1
						65,66,67,68,69,70,71,72,73, 0,//2
						 0, 0, 9, 0, 0,47,74,75,76,77,//3
						78,79,80,81,82,14, 0,15,11,12,//4
						 0, 0,43,83,84,85,86,87,88,89,//5
						90,14, 0, 0					  //6
};

//						 0  1  2  3  4  5  6  7  8  9
uint8_t asciixs3[256] = {
						 0, 0, 0, 0, 0, 0, 0, 0, 0,32,// 0
						 0,48,49,16,45,47, 0, 0, 0, 0,// 1 
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 2
						 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,// 3
						 0, 0, 0,52,17, 2,18,35, 3, 4,// 4
						 5, 6, 7, 8, 9,10,11,12, 0,19,// 5
						 0, 0, 0, 0, 0,20,21,22,23,24,// 6
						25,26,27,28, 0, 0, 0, 0, 0, 0,// 7
						 0, 0, 0, 0,36,37,38,39,40,41,// 8
						42,43,44,53,54,55,56,57,58,59,// 9
						60, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 10
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 11
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 12
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 13
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 14
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 15
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 16
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 17
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 18
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 19
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 20
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 21
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 22
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 23
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 24
						 0, 0, 0, 0, 0, 0			  // 25
};

void strrev(char *s)
{
	unsigned long n = strlen(s);
    for (int i=0; i < n/2; i++) { 
        s[i] ^= s[n-i-1]; 
    	s[n-i-1] ^= s[i]; 
    	s[i] ^= s[n-i-1]; 
    } 
}

uint8_t ps7(uint8_t v)
{
    uint8_t c = 0;

    for (c = 0; v; v >>= 1){
        c += v & 1;
    }
    if ((c % 2) == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

// init to NULL (internal use)
void _inituvw(uvword* v)
{
    for (int i = 0; i < 12; i++) {
        (*v)[i].p7 = 0;
        (*v)[i].zone = 0;
        (*v)[i].zone = 0;
    }
}

// init to NULL (internal use)
void _inituvhw(uvhword* v)
{
    for (int i = 0; i < 6; i++) {
        (*v)[i].p7 = 0;
        (*v)[i].zone = 0;
        (*v)[i].zone = 0;
    }
}

uint8_t uvweq(uvword* a, uvword* b) {
	uint8_t av = 0;
	uint8_t bv = 0;

	for (int i = 0; i < 12; i++) {
        av = xn((*a)[i]);
        bv = xn((*b)[i]);
		if (av != bv) {
			return 0;
		}
	}
	return 1;
}

uint8_t uvwgt(uvword* a, uvword* b) {
	uint8_t av = 0;
	uint8_t bv = 0;
	long va = 0;
	long vb = 0;
	uint8_t s = 1;
    uvchar ac;
    uvchar bc;

	// pos 0 is the sign
	for (int i = 1; i<12; i++) {
        ac = (*a)[11-i];
        bc = (*b)[11-i];
		av = xn(ac); bv = xn(ac);
		va = va + av * (long)pow(10, i);
		vb = vb + bv * (long)pow(10, i);
	}
	s = (xn((*a)[0]) == 1) ? -1 : 1;
	va = va * s;
	s = (xn((*b)[0]) == 1) ? -1 : 1;
	vb = vb * s;

	if (va > vb) {
		return 1;
	}

	return 0;
}

void shr_s(uint8_t n)
{
	for (int j = 1; j <= n; j++) {
		for (int i = 10; i <= 0; i--) {
			rA[i] = rA[i + 1];
		}
		rA[0] = u0;
	}
}

void shl_s(uint8_t n)
{
	for (int j = 1; j <= n; j++) {
		for (int i = 0; i < 11; i--) {
			rA[i] = rA[i+1];
		}
		rA[11] = u0;
	}
}

void shr(uint8_t n)
{
	for (int j = 1; j <= n; j++) {
		for (int i = 10; i <= 1; i--) {
			rA[i] = rA[i + 1];
		}
		rA[1] = u0;
	}
}

void shl(uint8_t n)
{
	for (int j = 1; j <= n; j++) {
		for (int i = 1; i < 11; i--) {
			rA[i] = rA[i + 1];
		}
		rA[11] = u0;
	}
}

uvchar dec2uvc(uint8_t v)
{
	uvchar r;
	uint8_t z = (v & 0b0110000) >> 4;
	uint8_t c = v & 0b0001111;

	r.p7 = ps7(v & 0b0111111);
	r.zone = z;
	r.code = c + 3;

	return r;
}

void inc(uvword* v)
{
    uint8_t carry = 0;
    uint8_t cc = 0;

    for (int i = 11; i >= 9; i--) {
        cc = (*v)[i].code;
        (*v)[i].code += carry + 1;
        (*v)[i].p7 = ps7((*v)[i].code);
        if ((cc + 1 + carry) > 15) {
            carry = 1;
        }
        else {
            carry = 0;
            break;
        }
    }
}

uvword* str2uvw(char* s)
{
    int slen = (int) strlen(s);
    uvword r;

    _inituvw(&r);
    
    for (int i = 0; i < slen; i++) {
        r[i].zone = asciixs3[s[i]] >> 4;
        r[i].code = asciixs3[s[i]] & 0x0f;
        r[i].p7 = ps7(asciixs3[s[i]]);
    }
    if (s[0] == '-') {
        r[0].zone = 0;
        r[0].code = 4;
        r[0].p7 = ps7(4);
    }

    return &r;
}


void uvadd(void)
{
	uint8_t va = 0;
	uint8_t vb = 0;
    uint8_t av = 0;
    uint8_t bv = 0;
	int s = 0;
	uint8_t sa = xs3n[rA[0].code];
	uint8_t sb = xs3n[rX[0].code];
    char ss[13];
    uint8_t ccf = 0;
    
    sa = sa ? -1 : 1;
    sb = sb ? -1 : 1;
    
    // find if one of them is not a number
    for (int i=11;i<=0;i--) {
        ccf += rA[i].zone + rX[i].zone;
    }
    
    if (ccf == 0) {
        // both numbers... easy way
        for (int i = 0; i<11; i++) {
            av = xn(rA[11-i]);
            bv = xn(rX[11-i]);
            va = va + av * (long)pow(10, i);
            vb = vb + bv * (long)pow(10, i);
        }
        va *= sa; vb *= sb;
        s = va + vb;
        sprintf(ss,"%012d",abs(s));
        memcpy(rA,str2uvw(ss),sizeof(uvword));
        
        if (s < 0) {
            rA[0].code = 4;
            rA[0].zone = 0;
            rA[0].p7 = ps7(4);
        }
    }
}

void uvmul(void) {
    int t = 0;
    int rLv = 0;
    int rXv = 0;
    int p = 0;
    char p1[13];
    char p2[13];
    uint8_t sa = xs3n[rL[0].code];
    uint8_t sb = xs3n[rX[0].code];
    
    char rFv[13];
    
    sa = sa ? -1 : 1;
    sb = sb ? -1 : 1;
    
    for (int i = 0; i < 11; i++) {
        t = t + rL[11-i].code * (uint8_t)pow(10, i);
    }
    rLv = t;
    t *= 3;
    
    sprintf("%012d",rFv,t);
    memcpy(rF,str2uvw(rFv),sizeof(uvword));
    
    t = 0;
    for (int i = 0; i < 11; i++) {
        t = t + rX[11-i].code * (uint8_t)pow(10, i);
    }
    rXv = t;
    p = rLv * rXv;
    
    sprintf(p1,"%012d",(p&0x3ff800)>>11);
    sprintf(p2,"%012d",(p&0x0007ff));
    memcpy(rA,str2uvw(p1),sizeof(uvword));
    memcpy(rX,str2uvw(p2),sizeof(uvword));
    
    if ((sa * sb) < 0) {
        rA[0].code = 4;
        rA[0].zone = 0;
        rA[0].p7 = ps7(4);
        rX[0].code = 4;
        rX[0].zone = 0;
        rX[0].p7 = ps7(4);
    }
}


void pruvc(uvchar a)
{
	printf("%s %s %s", bitr2[a.p7], bitr4[a.zone], bitr16[a.code]);
}

void pruwrd(uvword* a)
{
	for (int i = 0; i < 12; i++) {
		pruvc((*a)[i]);
        printf(" "); fflush(stdout);
        //usleep(PULSE*100);
	}
	printf("\n");
}

void pprtuvw(uvword* v)
{
	for (int i = 0; i < 12; i++) {
		uint8_t d = ((*v)[i].zone << 4) + (*v)[i].code;
        printf("%c", xs3ascii[d]);fflush(stdout);
        //usleep(PULSE*100);
	}
}


void read_tape(uint8_t n, uint8_t dir) {
	char c = 0;
	uint16_t pos = 0;
	int nw = 0;
	char* tarea = calloc(720, sizeof(char));
	char* tline = calloc(12, sizeof(char));

	switch (n) {
	case 1:
		while ((c = getc(t1)) != EOF) {
			if (pos < 720) {
				tarea[pos] = c;
				pos++;
			}
			else {
				break;
				// TAPE REMAIN OPEN FOR ANOTHER READ
			}
		}
		if (dir == 1) {
			// FORWARD
			for (int i = 0; i < 60; i++) {
				for (int j = 0; j < 12; j++) {
					tline[j] = tarea[i+j+nw];
				}
				memcpy(rI[i%60],str2uvw(tline),sizeof(uvword));
				nw += 12;
			}
		}
		else {
			// BACKWARD
			for (int i = 0; i < 60; i++) {
				for (int j = 0; j < 12; j++) {
					tline[j] = tarea[719 - (i + j + nw)];
				}
				strrev(tline);
				memcpy(rI[i], str2uvw(tline), sizeof(uvword));
				nw++;
			}
		}
	}
}

void write_tape(uint8_t n, uint8_t addr, uint8_t dens)
{

}

void exec(void)
{
	uint8_t addr = 0;
	uint8_t paddr = 0;
	char opc[3];

	for (int i = 0; i < 2; i++) {
		paddr = (SR[i].zone << 4) + SR[i].code;
		opc[i] = xs3ascii[paddr];
	}
	opc[2] = '\0';

	if (strcmp(opc, "00") == 0) {
		// SKIP
	}
	else if (strcmp(opc, "10") == 0) {
		char c = 0;
		uint16_t pos = 0;
		char line[12];

		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
			addr = addr + paddr * (uint8_t)pow(10, i);
		}
		// TYPE ONE WORD INTO addr
		// TODO: USING A SEPARATE PROGRAM TO CODE XS3 CHAR
		while ((c = getc(uvconsole)) != EOF) {
			if (c == 13) {
				if (pos == 11) {
					memcpy(memory[addr], str2uvw(line), sizeof(uvword));
				}
				pos = 0;
			}
			else {
				line[pos] = c;
			}
			pos++;
		}
	}
	else if (strcmp(opc, "30") == 0) {
		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
			addr = addr + paddr * (uint8_t)pow(10, i);
		}
		for (int j = 0; j < 60; j++) {
			for (int i = 0; i < WRDSIZE; i++) {
				memory[addr+j][i] = rI[j][i];
			}
		}
	}
	else if (strcmp(opc, "40") == 0) {
		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
			addr = addr + paddr * (uint8_t)pow(10, i);
		}
		for (int j = 0; j < 60; j++) {
			for (int i = 0; i < WRDSIZE; i++) {
				memory[addr + j][i] = rI[j][i];
			}
		}
	}
	else if (strcmp(opc, "50") == 0) {
		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
			addr = addr + paddr * (uint8_t)pow(10, i);
		}
		// PRINT ONE WORD FROM addr
		pprtuvw(&memory[addr]);
	}
	else if (opc[0] == '1') {
		// 60 WORDS FROM TAPE n, FORWARD
		uint8_t n = opc[1] - 48;
		read_tape(n, 1);
	}
	else if (opc[0] == '2') {
		// 60 WORDS FROM TAPE n, BACKWARD
		uint8_t n = opc[1] - 48;
		read_tape(n, -1);
	}
	else if (opc[0] == '3') {
		// (rI) to addr; 60 WORDS FROM TAPE n TO rI, FORWARD
		uint8_t n = opc[1] - 48;
		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
			addr = addr + paddr * (uint8_t)pow(10, i);
		}
		for (int j = 0; j < 60; j++) {
			for (int i = 0; i < WRDSIZE; i++) {
				memory[addr + j][i] = rI[j][i];
			}
		}
		read_tape(n, 1);
	}
	else if (opc[0] == '4') {
		// (rI) to addr; 60 WORDS FROM TAPE n TO rI, BACKWARD
		uint8_t n = opc[1] - 48;
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int j = 0; j < 60; j++) {
			for (int i = 0; i < WRDSIZE; i++) {
				memory[addr + j][i] = rI[j][i];
			}
		}
		read_tape(n, -1);
	}
	else if (opc[0] == '5') {
		// (addr) to TAPE n, 100 PULSES/INCH
		uint8_t n = opc[1] - 48;
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int j = 0; j < 60; j++) {
			for (int i = 0; i < WRDSIZE; i++) {
				rO[j][i] = memory[addr + j][i];
			}
		}
		write_tape(n, addr, 100);
	}
	else if (opc[0] == '6') {
		// REWIND TAPE n
		uint8_t n = opc[1] - 48;
	}
	else if (opc[0] == '7') {
		// (addr) to TAPE n, 20 PULSES/INCH
		uint8_t n = opc[1] - 48;
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int j = 0; j < 60; j++) {
			for (int i = 0; i < WRDSIZE; i++) {
				rO[j][i] = memory[addr + j][i];
			}
		}
		write_tape(n, addr, 20);
	}
	else if (opc[0] == '8') {
		// REWIND TAPE n; SET INTERLOCK
		uint8_t n = opc[1] - 48;
	}
	else if (opc[0] == '9') {
		// STOP COMPUTER
	}
	else if (opc[0] == ',') {
		// BREAKPOINT STOP
	}
	else if (opc[0] == '.') {
		// SHIFT (rA) RIGHT, WITH SIGN, n PLACES
		uint8_t n = opc[1] - 48;
		shr_s(n);
	}
	else if (opc[0] == ';') {
		// SHIFT (rA) LEFT, WITH SIGN, n PLACES
		uint8_t n = opc[1] - 48;
		shl_s(n);
	}
	else if (opc[0] == '-') {
		// SHIFT (rA) RIGHT, EXC. SIGN, n PLACES
		uint8_t n = opc[1] - 48;
		shr(n);
	}
	else if (opc[0] == '0') {
		// SHIFT (rA) LEFT, EXC. SIGN, n PLACES
		uint8_t n = opc[1] - 48;
		shl(n);
	}
	else if (opc[0] == 'H') {
		// HOLD
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 12; i++) {
			memory[addr][i] = rA[i];
		}
	}
	else if (opc[0] == 'C') {
		// CLEAR
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 12; i++) {
			memory[addr][i] = rA[i];
		}
		for (int i = 0; i < 12; i++) {
			rA[i] = u0;
		}
	}
	else if (opc[0] == 'B') {
		// BRING
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 12; i++) {
			rA[i] = memory[addr][i];
			rX[i] = memory[addr][i];
		}
	}
	else if (opc[0] == 'L') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 12; i++) {
			rL[i] = memory[addr][i];
			rX[i] = memory[addr][i];
		}
	}
	else if (opc[0] == 'F') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 12; i++) {
			rF[i] = memory[addr][i];
		}
	}
	else if (opc[0] == 'E') {
		uint8_t lb = 0;
		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
			addr = addr + paddr * (uint8_t)pow(10, i);
		}
		for (int i = 0; i < 12; i++) {
			lb = (rF[i].code) & 1;
			if (lb == 0) {
				rA[i] = memory[addr][i];
			}
		}
	}
	else if (opc[0] == 'G') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 12; i++) {
			memory[addr][i] = rF[i];
		}
	}
	else if (opc[0] == 'J') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 12; i++) {
			memory[addr][i] = rX[i];
		}
	}
	else if (opc[0] == 'V') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		// TODO: check constraint on addr
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 12; i++) {
				rV[j][i] = memory[addr][i];
			}
		}
	}
	else if (opc[0] == 'W') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		// TODO: check constraint on addr
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 12; i++) {
				memory[addr][i] = rV[j][i];
			}
		}
	}
	else if (opc[0] == 'Y') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		// TODO: check constraint on addr
		for (int j = 0; j < 10; j++) {
			for (int i = 0; i < 12; i++) {
				rY[j][i] = memory[addr][i];
			}
		}
	}
	else if (opc[0] == 'Z') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		// TODO: check constraint on addr
		for (int j = 0; j < 10; j++) {
			for (int i = 0; i < 12; i++) {
				memory[addr][i] = rY[j][i];
			}
		}
	}
	else if (opc[0] == 'K') {
		for (int i = 0; i < 12; i++) {
			rL[i] = rA[i];
		}
		for (int i = 0; i < 12; i++) {
			rA[i] = u0;
		}
	}
	else if (opc[0] == 'R') {
		uvword cc1;
		char cc2[13];
		char cc3[4];
		uint8_t icc1 = 0;

		memcpy(cc1, CC, sizeof(uvword));
		inc(&cc1);
		
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(cc1[5 - i].zone << 4) + cc1[5 - i].code];
			icc1 = icc1 + paddr * (uint8_t)pow(10, i);
		}
		strcat(cc2, "000000U00");
		sprintf(cc3, "%03d", icc1);
		strcat(cc2, cc3);
		memcpy(memory[addr], str2uvw(cc2), sizeof(uvword));
	}
	else if (opc[0] == 'U') {
		for (int i = 9; i < 12; i++) {
			CC[i] = CR[i];
		}
	}
	else if (opc[0] == 'Q') {
		if (uvweq(&rA,&rL)) {
			for (int i = 9; i < 12; i++) {
				CC[i] = CR[i];
			}
		}
	}
	else if (opc[0] == 'T') {
		if (uvwgt(&rA, &rL)) {
			for (int i = 9; i < 12; i++) {
				CC[i] = CR[i];
			}
		}
	}
	else if (opc[0] == 'A') {
		for (int i = 0; i < 3; i++) {
			paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
			addr = addr + paddr * (uint8_t)pow(10, i);
		}
		for (int i = 0; i < 12; i++) {
			rX[i] = memory[addr][i];
		}
        uvadd();
	}
    else if (opc[0] == 'X') {
        uvadd();
    }
    else if (opc[0] == 'S') {
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
        for (int i = 0; i < 12; i++) {
            rX[i] = memory[addr][i];
        }
        rX[0].zone=0;
        rX[0].code=4;
        rX[0].p7 = ps7(4);
        uvadd();
    }
    else if (opc[0] == 'P') {
        char rLv[13];
        
        // PRECISION MULTIPLY
        for (int i = 0; i < 3; i++) {
            paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
            addr = addr + paddr * (uint8_t)pow(10, i);
        }
        for (int i = 0; i < 12; i++) {
            rX[i] = memory[addr][i];
        }
    
        for (int i = 0; i < 11; i++) {
            addr = addr + rL[11-i].code * (uint8_t)pow(10, i);
        }
        addr *= 3;
        sprintf("%012d",rLv,addr);
        memcpy(rF,str2uvw(rLv),sizeof(uvword));
        
        uvmul();
    }
	else {
        printf("Illegal instruction!!!\n"); fflush(stdout);
		exit(-200);
	}
}

void _alpha(void) {
	for (int i = 0; i < 6; i++) {
		SR[i] = CC[11 - i];
	}
}

void _beta(void) {
	uint8_t addr = 0;
	uint8_t paddr = 0;

	for (int i = 0; i < 3; i++) {
		paddr = xs3n[(SR[5 - i].zone << 4) + SR[5 - i].code];
		addr = addr + paddr * (uint8_t) pow(10, i);
	}

	for (int i = 0; i < 12; i++) {
		CR[i] = memory[addr][i];
	}

	inc(&CC);
}

void _gamma(void)
{
	for (int i = 0; i < 6; i++) {
		SR[i] = CR[i];
	}
	exec();
}

void _delta(void)
{
	for (int i = 0; i < 6; i++) {
		SR[5 - i] = CR[11 - i];
	}
	exec();
}

void reset(void)
{
    printf("Init memory and regs... "); fflush(stdout);
	
	_inituvw(&CC);
	_inituvw(&CR);
	_inituvhw(&SR);


	for (int i = 0; i < WRDSIZE; i++) {
		rA[i] = dec2uvc(0);
		rX[i] = u0;
		rL[i] = u0;
		rF[i] = u0;
        //usleep(PULSE);
	}
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < WRDSIZE; i++) {
			rV[j][i] = u0;
            //usleep(PULSE);
		}
	}
	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < WRDSIZE; i++) {
			rY[j][i] = u0;
            //usleep(PULSE);
		}
	}
	for (int j = 0; j < 60; j++) {
		for (int i = 0; i < WRDSIZE; i++) {
			rI[j][i] = u0;
			rO[j][i] = u0;
            //usleep(PULSE);
		}
	}
	for (int j = 0; j < 1000; j++) {
		for (int i = 0; i < WRDSIZE; i++) {
			memory[j][i] = u0;
            //usleep(PULSE);
		}
	}
    printf("done.\n"); fflush(stdout);
}

int main()
{
	uvword temp1;

	uvconsole = fopen("console.txt", "r");
	t1 = fopen(tape1.name,"rw");
	t2 = fopen(tape2.name,"rw");
	t3 = fopen(tape3.name,"rw");
	t4 = fopen(tape4.name,"rw");
	t5 = fopen(tape5.name,"rw");
	t6 = fopen(tape6.name,"rw");
	t7 = fopen(tape7.name,"rw");
	t8 = fopen(tape8.name,"rw");
	t9 = fopen(tape9.name,"rw");
	
	reset();

	printf("s uvchar: %zd byte(s)\n", sizeof(uvchar));
	printf("s uvword: %zd byte(s)\n", sizeof(uvword));
	printf("s memory: %zd byte(s)\n", sizeof(memory));

	pruwrd(&rA);

	memcpy(&temp1,str2uvw("C00234B00934"),sizeof(uvword));

	pruwrd(&temp1);
	pruwrd(&CC);
	pprtuvw(&temp1);
	
	memcpy(memory[0],str2uvw("B00100C00150"),sizeof(uvword));
	memcpy(memory[100],str2uvw("000100000150"),sizeof(uvword));

	_alpha();
	_beta();
	_gamma();
	_delta();
	
	PCR;
	pprtuvw(&memory[0]); PCR;
	pprtuvw(&memory[100]); PCR;
	pprtuvw(&memory[150]); PCR;
	
	fclose(uvconsole);
	fclose(t1);
	fclose(t2);
	fclose(t3);
	fclose(t4);
	fclose(t5);
	fclose(t6);
	fclose(t7);
	fclose(t8);
	fclose(t9);

	return 0;
}
