#include <stdio.h>
#include <sys/types.h>
#include <MessageAB.h>
/* MessageAB ASN.1 type */
int main(int ac, char **av)
{
    char buf[1024];
    /* Temporary buffer
     */
    asn_dec_rval_t rval;        /* Decoder return value */
    MessageAB_t *ab = 0; /* Type to decode. Note this 0 1 ! */
    FILE *fp;
    size_t size;
    char *filename;
    /* Input file handler
    /* Number of bytes read
    /* Input file name */
    /* Require a single filename argument */
    if (ac != 2)
    {
        fprintf(stderr, "Usage: %s <file.ber>\n", av[0]);
        exit(1);
    }
    else
    {
        filename = av[1];
    }
    /* Open input file as read ି only binary */
    fp = fopen(filename, "rb");
    if (!fp)
    {
        perror(filename);
        exit(1);
    }
    /* Read up to the buffer size */
    size = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);
    if (!size)
    {
        fprintf(stderr, "%s: Empty or broken\n", filename);
        exit(1);
    }
    /* Decode the input buffer as MessageAB type */
    asn_dec_rval_t decRet;
    char errBuf[10000] = {};
    size_t errlen = 10000;
     MessageAB_t *decab=0; 

	/*decode msg into NGAP PDU*/
	decRet = aper_decode_complete(0, &asn_DEF_MessageAB,
				(void**) &decab, (char*) buf, 100000);

	if(asn_check_constraints(&asn_DEF_MessageAB, decab,
					errBuf, &errlen) < 0)
		printf("check constraints failed: %s", errBuf);

	if(decRet.code == RC_OK) {
		printf("%s", "Decode succeeded");
        printf("%d \n",decab->unqID);
        printf("%s \n",decab->msgType.buf);
        printf("%s \n",decab->msg.buf);

		// asnLog(&asn_DEF_MessageAB, decab);
		// *ngapPdu_p = ngapPdu;
	} else {
		printf("Decode failed, Err %s", decRet.code == RC_FAIL ?
						"RC_FAIL" : "RC_WMORE");
		// ngapFree(ngapPdu);
		// *ngapPdu_p = NULL;
		// return FAILURE;
	}
    /* Print the decoded MessageAB type as XML */
    xer_fprint(stdout, &asn_DEF_MessageAB, decab);
    return 0; /* Decoding finished successfully */
}