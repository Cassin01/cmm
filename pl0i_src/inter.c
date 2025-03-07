/* -------------------------- inter.c ------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include "code.h"

#define BSIZE 100

/* global variables for interpreter */
#define STACKSIZE 500
int s[STACKSIZE]; /* run-time stack */
int p, b, t;      /* program counter, pointer to stack, top of stack */

int label[CMAX];		/* label table for backpatching */
int cl = 0;			/* current label number using O_LAB */ 

instruction code[CMAX];		/* code table             */
instruction code2[CMAX];	/* non labeled code table */
int cx  = 1;			/* code index for code    */
int cx2 = 1;			/* code index for code2   */

opecode mnemonic2i(char* f);
instruction getcode(char* buf);

static void wrcode(int c, instruction c2[]);

void interpreter()        /* interpreter */
{
    int tmp;
    instruction i;
    opecode f; /* operation code */
    int l, a;  /* l = level difference or 0
                  a = displacement or sub-operation or number */
    printf( "start PL/0\n" );
    t = 0;  b = 1;  p = 1;
    s[1] = s[2] = s[3] = 0; /* static link, dynamic link, ret. addr. of main */
    do {
        i = code[p++]; /* get an instruction */
        f = i.f;  l = i.l;  a = i.a;
        switch( f ) { /* switch by op-code */
            case O_LIT : s[++t] = a;  break;
            case O_OPR : 
                         switch( (oprcode)a ) { /* P_RET = return from procedure */
                             case P_RET: t = b - 1;  p = s[t+3];	 b = s[t+2];  break;
                             case P_NEG:	   ;  s[t] = -s[t]		   ;  break;
                             case P_ADD: --t;  s[t] = s[t] + s[t+1]	   ;  break; 
                             case P_SUB: --t;  s[t] = s[t] - s[t+1]	   ;  break; 
                             case P_MUL: --t;  s[t] = s[t] * s[t+1]	   ;  break; 
                             case P_DIV: --t;  s[t] = s[t] / s[t+1]	   ;  break; 
                             case P_ODD:	   ;  s[t] = s[t] % 2		   ;  break;
                             case P_EQ:	--t;  s[t] = ( s[t] == s[t+1] )	   ;  break;
                             case P_NE:	--t;  s[t] = ( s[t] != s[t+1] )	   ;  break;
                             case P_LT:	--t;  s[t] = ( s[t] <  s[t+1] )	   ;  break;
                             case P_GE:	--t;  s[t] = ( s[t] >= s[t+1] )	   ;  break;
                             case P_GT:	--t;  s[t] = ( s[t] >  s[t+1] )	   ;  break;
                             case P_LE:	--t;  s[t] = ( s[t] <= s[t+1] )	   ;  break;
                         }
                         break;
            case O_LOD: s[++t] = s[base(l)+a];  break;
            case O_STO: s[base(l)+a] = s[t--];  break;
            case O_CAL: s[t+1] = base(l) /* static link */ ;
                        s[t+2] = b /* dynamic link */; s[t+3] = p /* ret. addr. */;
                        b = t + 1;	p = a;	break;
            case O_INT: t += a;  break;
            case O_RET: tmp = s[t];
                        t = b - 1;
                        p = s[t+3];
                        b = s[t+2];
                        t-=a;
                        s[++t] = tmp;
                        break;
            case O_JMP: p = a;  break;
            case O_JPC: if( s[t--] == 0 ) p = a;  break;
            case O_CSP:
                            switch( (cspcode)a ) {
                                case RDI: if( scanf("%d",&s[++t])!=1 ) printf( "error read\n" );
                                              break; /* read(var) */
                                case WRI: printf( "%10d", s[t--] );  break; /* write(exp) */
                                case WRL: putchar( '\n' );	break; /* writeln */
                            }
                            break;
        }
    } while( p != 0 );
    printf( "end PL/0\n" );
}

int base( l ) /* trace static link l times, where l is the difference */
    int l;      /* of current and referenced static nesting levels */
{
    int b1;

    b1	= b; /* b points to the area of static link */
    while( l>0 ) {
        b1 = s[b1];
        l--;
    }
    return b1;
}

mnemonic mntbl[] = { /* table for mnemonic code */
    { "LIT", O_LIT }, { "OPR", O_OPR },
    { "LOD", O_LOD }, { "STO", O_STO },
    { "CAL", O_CAL }, { "INT", O_INT },
    { "JMP", O_JMP }, { "JPC", O_JPC },
    { "CSP", O_CSP }, { "LAB", O_LAB },
    { "   ", O_BAD }, { "RET", O_RET }
};

main(int argc, char *argv[]){
    FILE *codef;
    char buf[BSIZE];

    if (argc != 2) {
        perror("コマンドライン引数が正しくありません");
        exit(EXIT_FAILURE);

    }

    codef = fopen(argv[1], "r");
    if(codef == NULL){
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    while(fgets(buf, BSIZE, codef) != 0){
        code2[cx2] = getcode(buf);
        cx2++;
    }

    if (ferror(codef)!=0){
        perror("getcode");
        exit(EXIT_FAILURE);
    }

    if (fclose(codef)!=0){
        perror("code.output");
        exit(EXIT_FAILURE);
    }

    linearize(code2);
    interpreter();
}

opecode 
mnemonic2i(char* f){
    int i;

    for(i=0; i<12;i++){
        if (strcmp(mntbl[i].sym, f)==0){
            /*      printf("m2i: %d\n", mntbl[i].cd);*/
            return mntbl[i].cd;
        }
    }
}

instruction getcode(char* buf){
    instruction i;
    char f[4];
    char d[BSIZE];
    int pos, j;

    for(pos = 2; pos < 5; pos++){
        f[pos - 2] = buf[pos];
    }
    f[pos -2] = '\0';

    i.f = mnemonic2i(f);
    pos++;

    for(j = 0; buf[pos] != ','; pos++, j++){
        d[j] = buf[pos];
    }
    d[j] = '\0';
    pos++;
    i.l = atoi(d);

    for(j = 0; buf[pos] != ')'; pos++, j++){
        d[j] = buf[pos];
    }
    d[j] = '\0';
    i.a = atoi(d);

    return i;
}


/***************************************************************/
/* code from code.c                                            */
/***************************************************************/

/* print code stored in array code[] */
static void wrcode(int c, instruction c2[]) 
{
    int i, f, l, a;

    printf( "code table\n\n" );

    for( i=1; i<=c; i++ ) {
        f = c2[i].f;  l = c2[i].l;  a = c2[i].a;
        printf( "%4d [ %s, %3d, %3d ]\n", i, mntbl[f].sym, l, a );
    }
    printf( "\f" );
}


/* transform list structure code cp to array code[] */
/* also do backpatching */

/* codeptr */
void linearize(instruction c[]){
    codestr *wp;
    opecode f;
    int l, a;
    int c1, c2;
    int tmpcx;

    cleararray( code, sizeof(code) );
    cleararray( label, sizeof(label)); /* label is the label table */
    if( cx2 == 1 ) return;
    for( tmpcx = 1; tmpcx != cx2; tmpcx ++) {
        f = code2[tmpcx].f;
        l = code2[tmpcx].l;
        a = code2[tmpcx].a;

        switch( f ) {
            case O_LAB:
                if( label[a] == 0 ) { /* label 'a' was unused */
                    label[a] = cx;    /* let it be defined. 
                                         let label[a] point to its address */
                } else if( label[a] < 0 ) { /* label 'a' was used before def. */
                    c1 = -label[a];         /* backpatching */
                    while( c1 != 0 ) {      /* trace forward reference chain */
                        c2 = code[c1].a;
                        code[c1].a = cx;
                        c1 = c2;
                    }
                    label[a] = cx;
                } else {
                    error( "label already defined" );
                    exit( 1 );
                }
                break; /* do not increment cx (delete LAB instruction) */
            case O_JMP:
            case O_JPC:
            case O_CAL:
                code[cx].f = f;
                code[cx].l = l;
                if( label[a] <= 0 ) { /* label 'a' was undefined. 
                                         make forward reference chain */
                    code[cx].a = -label[a];
                    label[a] = -cx;
                } else {              /* label 'a' was defined */
                    code[cx].a = label[a];
                }
                cx++;
                break;
            default:
                code[cx].f = f;  code[cx].l = l;  code[cx].a = a;
                cx++;
                break;
        }
        if( cx >= CMAX ) {
            error( "program is too large" );
            exit( 1 );
        }
    }
    wrcode( cx - 1 , code);
    /*    return cp; */
}
