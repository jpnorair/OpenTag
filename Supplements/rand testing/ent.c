/*
    ENT  --  Entropy calculation and analysis of putative
         random sequences.

        Designed and implemented by John "Random" Walker in May 1985.

    Multiple analyses of random sequences added in December 1985.

    Bit stream analysis added in September 1997.

    Terse mode output, getopt() command line processing,
    optional stdin input, and HTML documentation added in
    October 1998.
    
    Documentation for the -t (terse output) option added
    in July 2006.
    
    Replaced table look-up for chi square to probability
    conversion with algorithmic computation in January 2008.

    For additional information and the latest version,
    see http://www.fourmilab.ch/random/

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>

#include "iso8859.h"
#include "randtest.h"

#define UPDATE  "14 October 2011"

#define FALSE 0
#define TRUE  1

#ifdef M_PI
#define PI   M_PI
#else
#define PI   3.14159265358979323846
#endif

extern double pochisq(const double ax, const int df);

/*  HELP  --  Print information on how to call  */

static void help(void)
{
        printf("ent --  Calculate entropy of file.  Call");
        printf("\n        with ent [options] [input-file]");
        printf("\n");
        printf("\n       Options:   -b   Treat input as a stream of bits");
        printf("\n                  -c   Print occurrence counts");
        printf("\n                  -f   Fold upper to lower case letters");
        printf("\n                  -t   Terse output in CSV format");
        printf("\n                  -u   Print this help message");
        printf("\n");
        printf("\n      Hex input:  --hex=lsb");
        printf("\n                  where [lsb] is 1-32\n");
         
        printf("\nBy John Walker, with hex-file update by JP Norair");
        printf("\n   http://www.fourmilab.ch/");
        printf("\n   http://www.indigresso.com/");
        printf("\n   %s\n", UPDATE);
}





/*  Main program  */

int main(int argc, char *argv[])
{
    int i, oc, opt;
    long ccount[256];         /* Bins to count occurrences of values */
    long totalc = 0;          /* Total character count */
    char *samp;
    double montepi, chip,
           scc, ent, mean, chisq;
    FILE *fp = stdin;
    
    int counts = FALSE,       /* Print character counts */
        fold = FALSE,         /* Fold upper to lower */
        binary = FALSE,       /* Treat input as a bitstream */
        hex = FALSE,
        terse = FALSE;        /* Terse (CSV format) output */

    int lsblen;
    static struct option long_options[] = {
        {"hex", 1, 0, 'h'},
        {0, 0, 0, 0}
    };


    while ((opt = getopt_long(argc, argv, "bcftu?BCFTU", long_options, NULL)) != -1) {
        switch (toISOlower(opt)) {
            case 'b':   binary = TRUE;          break;
            case 'c':   counts = TRUE;          break;
            case 'f':   fold = TRUE;            break;
            
            case 'h':   hex     = TRUE;
                        binary  = TRUE;
                        sscanf(optarg, "%d", &lsblen);
                        if ((1 <= lsblen) && (lsblen <= 32)) {
                            break;
                        }
                        help();
                        return 0;
            
            case 't':   terse = TRUE;           break;
            case '?':
            case 'u':   help();                 return 0;
        }
    }
    
    if (optind < argc) {
        if (optind != (argc - 1)) {
            printf("Duplicate file name.\n");
            help();
            return 2;
        }
        if ((fp = fopen(argv[optind], "rb")) == NULL) {
            printf("Cannot open file %s\n", argv[optind]);
            return 2;
        }
    }
    
    
    // Hex file parser by JP Norair
    if (hex == TRUE) {
        unsigned int hexbuf[32];
        unsigned int binbuf[32];
        char filename_bin[256];
        FILE* fp_bin;
        
        unsigned int lsbmask;
        int shift;
        int j;
        
        for (i=0; ((i<250) && (argv[optind][i] != 0)); i++) {
            filename_bin[i] = argv[optind][i];
        }
        if ((i == 250) && (argv[optind][i] != 0)) {
            filename_bin[i++] = '~';
        }
        filename_bin[i++]   = '.';
        filename_bin[i++]   = 'b';
        filename_bin[i++]   = 'i';
        filename_bin[i++]   = 'n';
        filename_bin[i]     = 0;
        
        fp_bin = fopen(filename_bin, "wb");
        if (fp_bin == NULL) {
            printf("Cannot create intermediate bin file %s\n", filename_bin);
            return 2;
        }
        
        lsbmask  = (1 << lsblen);
        lsbmask -= 1;
        
        while (1) {
            // Buffer-in 32 data values from the hex file
            for (i=0; i<32; i++) {
                if (fscanf(fp, "%X", &hexbuf[i]) != 1) {
                    break;
                }
                hexbuf[i] &= lsbmask;
                //printf("hexbuf[%d] = %d\n", i, hexbuf[i]);
            }
            if (i != 32) {
                break;
            }
            
            // Concatenate 32 data values into binary buffer, without padding
            memset(binbuf, 0, lsblen*4);
            for (i=0, j=0, shift=(32-lsblen); i<32; i++, shift-=lsblen) {
                if (shift < 0) {
                    binbuf[j++] |= (hexbuf[i] >> (0-shift));
                    shift       += 32;
                }
                binbuf[j] |= (hexbuf[i] << shift);
            }
            
            //for(i=0; i<=j; i++) {
            //    printf("binbuf[%d] = %d\n", i, binbuf[i]);
            //}
            
            // Write 'lsblen' * 4 bytes to binary file, which is the length in
            // bytes of the concatenated binary data in the binary buffer.
            // NOTE: This may not work if you have a machine where "int" is not
            //       defined as 32 bits.  UNIX C always has int as 32 bits.
            fwrite(binbuf, 4, lsblen, fp_bin);
        }
        
        fclose(fp);
        fclose(fp_bin);
        fp = fopen(filename_bin, "rb"); //reopen bin file in read mode
    }
    
    //return 0;
    
    samp = binary ? "bit" : "byte";
    memset(ccount, 0, sizeof ccount);

    /* Initialise for calculations */

    rt_init(binary);

    /* Scan input file and count character occurrences */

    while ((oc = fgetc(fp)) != EOF) {
        unsigned char ocb;

        if (fold && isISOalpha(oc) && isISOupper(oc)) {
            oc = toISOlower(oc);
        }
        ocb = (unsigned char) oc;
        totalc += binary ? 8 : 1;
        
        if (binary) {
            int b;
            unsigned char ob = ocb;

            for (b = 0; b < 8; b++) {
                ccount[ob & 1]++;
                ob >>= 1;
            }
        }
        else {
           ccount[ocb]++;         /* Update counter for this bin */
        }
        rt_add(&ocb, 1);
    }
    fclose(fp);

    /* Complete calculation and return sequence metrics */

    rt_end(&ent, &chisq, &mean, &montepi, &scc);

    if (terse) {
        printf("0,File-%ss,Entropy,Chi-square,Mean,Monte-Carlo-Pi,Serial-Correlation\n",
            binary ? "bit" : "byte");
        printf("1,%ld,%f,%f,%f,%f,%f\n", totalc, ent, chisq, mean, montepi, scc);
    }

    /* Calculate probability of observed distribution occurring from
       the results of the Chi-Square test */

        chip = pochisq(chisq, (binary ? 1 : 255));

    /* Print bin counts if requested */

    if (counts) {
        if (terse) {
            printf("2,Value,Occurrences,Fraction\n");
        }
        else {
            printf("Value Char Occurrences Fraction\n");
        }
        for (i = 0; i < (binary ? 2 : 256); i++) {
            if (terse) {
                printf("3,%d,%ld,%f\n", i,
                ccount[i], ((double) ccount[i] / totalc));
            } 
            else {
                if (ccount[i] > 0) {
                    printf("%3d   %c   %10ld   %f\n", i,
               /* The following expression shows ISO 8859-1
              Latin1 characters and blanks out other codes.
              The test for ISO space replaces the ISO
              non-blanking space (0xA0) with a regular
                          ASCII space, guaranteeing it's rendered
                          properly even when the font doesn't contain
              that character, which is the case with many
              X fonts. */
                       (!isISOprint(i) || isISOspace(i)) ? ' ' : i,
                    ccount[i], ((double) ccount[i] / totalc));
                }
            }
        }
        if (!terse) {
            printf("\nTotal:    %10ld   %f\n\n", totalc, 1.0);
        }
    }

    /* Print calculated results */

    if (!terse) {
           printf("Entropy = %f bits per %s.\n", ent, samp);
           printf("\nOptimum compression would reduce the size\n");
           printf("of this %ld %s file by %d percent.\n\n", totalc, samp,
            (short) ((100 * ((binary ? 1 : 8) - ent) /
                  (binary ? 1.0 : 8.0))));
       printf(
              "Chi square distribution for %ld samples is %1.2f, and randomly\n",
          totalc, chisq);
       if (chip < 0.0001) {
              printf("would exceed this value less than 0.01 percent of the times.\n\n");
       } else if (chip > 0.9999) {
              printf("would exceed this value more than than 99.99 percent of the times.\n\n");
       } else {
              printf("would exceed this value %1.2f percent of the times.\n\n",
         chip * 100);
       }
       printf(
              "Arithmetic mean value of data %ss is %1.4f (%.1f = random).\n",
          samp, mean, binary ? 0.5 : 127.5);
           printf("Monte Carlo value for Pi is %1.9f (error %1.2f percent).\n",
          montepi, 100.0 * (fabs(PI - montepi) / PI));
           printf("Serial correlation coefficient is ");
       if (scc >= -99999) {
              printf("%1.6f (totally uncorrelated = 0.0).\n", scc);
       } else {
              printf("undefined (all values equal!).\n");
       }
    }

    return 0;
}
