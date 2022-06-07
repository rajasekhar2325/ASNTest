#include <stdio.h>
#include <sys/types.h>
#include <include/MessageAB.h>
/* MessageAB ASN.1 type */
/* Write the encoded output into some FILE stream. */
static int write_out(const void *buffer, size_t size, void *app_key)
{
    FILE *out_fp = app_key;
    size_t wrote = fwrite(buffer, 1, size, out_fp);
    return (wrote == size) ? 0 : -1;
}

void encode(MessageAB_t *ab, char** enc_msg)
{
    asn_enc_rval_t ec;
    char temp[100000] = {};
    ec = aper_encode_to_buffer(&asn_DEF_MessageAB, 0, ab, temp, 100000);
    if (ec.encoded == -1)
    {
        fprintf(stderr, "Could not encode MessageAB(at %s)\n",
                ec.failed_type ? ec.failed_type->name : "unknown");
        exit(1);
    }
    else
    {
        // fprintf(stderr, "Encoded Message is: %s\n", temp);
        fprintf(stderr, "Encoded %ld bits\n", ec.encoded);
    }
    *enc_msg=temp;
}

void decode(char *temp, MessageAB_t **dec_msg)
{
    MessageAB_t *dec = 0;
    asn_dec_rval_t decRet;
    char errBuf[10000] = {};
    size_t errlen = 10000;
    decRet = aper_decode_complete(0, &asn_DEF_MessageAB, (void **)&dec, (char *)temp, 100000);

    if (asn_check_constraints(&asn_DEF_MessageAB, dec, errBuf, &errlen) < 0)
        printf("check constraints failed: %s", errBuf);

    if (decRet.code == RC_OK)
    {
        printf("Decode Successfull\n");
        *dec_msg = dec;
    }
    else
    {
        printf("Decode failed, Err %s", decRet.code == RC_FAIL ? "RC_FAIL" : "RC_WMORE");
        *dec_msg = NULL;
    }
}

int main(int ac, char **av)
{
    MessageAB_t *ab; // Type to encode

    ab = calloc(1, sizeof(MessageAB_t)); /* not malloc! */
    if (!ab)
    {
        perror("calloc() failed");
        exit(1);
    }
    /* Initialize the MessageAB members */
    ab->msgType.buf = "uplink";                 /* any random value */
    ab->msgType.size = strlen(ab->msgType.buf); /* any random value */
    ab->unqID = 23;                             /* any random value */
    ab->msg.buf = "msg from a to b";            /* any random value */
    ab->msg.size = strlen(ab->msg.buf);         /* any random value */
    char *enc_msg;
    encode(ab, &enc_msg);
    /* BER encode the data if filename is given */
    // if (ac < 2)
    // {
    //     fprintf(stderr, "Specify filename for BER output\n");
    // }
    // else
    // {
    // const char *filename = av[1];
    // FILE *fp = fopen(filename, "wb");
    /* for BER output */
    // if (!fp)
    // {
    //     perror(filename);
    //     exit(1);
    // }
    /* Encode the MessageAB type as BER (DER) */
    // ec = der_encode(&asn_DEF_MessageAB, ab, write_out, fp);
    // char temp[100000] = {};
    // ec = aper_encode_to_buffer(&asn_DEF_MessageAB, 0, ab,
    //                            temp, 100000);
    // if (ec.encoded == -1)
    // {
    //     fprintf(stderr, "Could not encode MessageAB(at %s)\n",
    //             ec.failed_type ? ec.failed_type->name : "unknown");
    //     exit(1);
    // }
    // else
    // {
    //     fprintf(stderr, "Encoded Message %s\n", temp);
    //     fprintf(stderr, "Encoded %ld bits\n", ec.encoded);
    //     fprintf(fp, "%s", temp);
    // }
    // fclose(fp);

    // asn_dec_rval_t decRet;
    // char errBuf[10000] = {};
    // size_t errlen = 10000;
    // MessageAB_t *decab = 0;

    // /*decode msg into NGAP PDU*/
    // decRet = aper_decode_complete(0, &asn_DEF_MessageAB,
    //                               (void **)&decab, (char *)temp, 100000);

    // if (asn_check_constraints(&asn_DEF_MessageAB, decab,
    //                           errBuf, &errlen) < 0)
    //     printf("check constraints failed: %s", errBuf);
    MessageAB_t *dec_msg = NULL;
    decode(enc_msg, &dec_msg);
    printf("%ld \n", dec_msg->unqID);
    printf("%s \n", dec_msg->msgType.buf);
    printf("%s \n", dec_msg->msg.buf);

    /* Also print the constructed MessageAB XER encoded (XML) */
    xer_fprint(stdout, &asn_DEF_MessageAB, ab);
    return 0; /* Encoding finished successfully */
}