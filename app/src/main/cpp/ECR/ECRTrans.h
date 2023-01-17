#ifndef __ECR_TRANS_H__
#define	__ECR_TRANS_H__

#ifdef	__cplusplus
extern "C" {
#endif

#define ECR_SEPARATOR			0x1C
#define ECR_AMOUNT_TAG			"40"
#define ECR_CASHBACK_TAG		"42"
//#define ECR_INV_TAG					"02"
#define ECR_INV_TAG				"65" 
#define ECR_PAN_TAG				"66" 
#define ECR_EXPDATE_TAG			"67" 

#define ECR_SALECUP_TAG			"31" 
#define ECR_VOIDCUP_TAG			"32" 
#define ECR_REFUNDCUP_TAG		"33" 
#define ECR_VOIDRFNDCUP_TAG		"34" 
#define ECR_OFFLINECUP_TAG  	"35"
#define ECR_BATCH_RETCUP_TAG	"38"
#define ECR_PREAUTHCUP_TAG		"36"
#define ECR_PRECOMPCUP_TAG		"37"
#define RETURN_LST_TXN			"39"
#define RETURN_BAT_TOT			"40"


#define ECR_ECHOTEST_TAG                "19"
#define ECR_SALE_TAG			"20"
#define ECR_VOID_TAG			"26"
#define ECR_REFUND_TAG		 	"27"

#define ECR_TIP_ADJUST_TAG		 "28"
#define ECR_INSTALLMENT_TAG		 "29"
#define ECR_RETRIEVAL_TAG		 "30"
#define ECR_TOTAL_TAG		 	 "31"
#define ECR_SETTLE_ALL_TAG		 "32"
#define ECR_SETTLEMENT_TAG		 "33"
#define ECR_REPRINT_TAG                  "60"
#define ECR_REPRINT_SETTLEMENT_TAG             "61"

#ifdef ECR_PREAUTH_AND_COMP
#define ECR_PREAUTH_TAG                  "62"
#define ECR_PREAUTHCOMP_TAG             "63"
#endif



#if 1
#define ECR_GET_TOTAL_TAG             "64"
#define ECR_GET_LAST_TOTAL_TAG        "65"
#define ECR_GET_BARCODE_TAG           "66"
#define ECR_PRINTING_TAG              "67"
#define ECR_READCARD_TAG              "68"
#endif

#ifdef ECR_PREAUTH_AND_COMP
#define ECR_VOID_PREAUTH_TAG           "69"
#endif


#define ECR_CASHBACK_TXN_TAG	"36"
#define ECR_CANCEL_TAG			"D1"

#define ECR_OCBC_IPP_TAG		"21" 
#define ECR_OFFLINE_TAG			"11" 

#define ECR_PRNT_GRS_TAG		"57" 

#define ECR_PRNT_DATA_TAG		"5D" 
#define ECR_PRNT_DATA_NO_TAG	"5E" 
#define ECR_CARD_ENQUIRY_TAG	"6A" 
#define ECR_CARD_SHA1_TAG		"6B" 
#define ECR_PRNT_NO_TAG			"5C" 
#define ECR_PRNT_HOST_TAG		"5A" 
#define ECR_PRNT_INV_TAG		"5B" 

#define ECR_AUTH_CODE_TAG		"01"
#define ECR_RESP_TEXT_TAG		"02"
#define ECR_MERCHANT_NAME_TAG	"D0"
#define ECR_RESP_INV_TAG		"65"
#define ECR_TID_TAG				"16"
#define ECR_MID_TAG				"D1"
#define ECR_ISSUER_TAG			"D2"
#define ECR_CARD_NO_TAG			"30"
#define ECR_EXPIRY_TAG			"31"
#define ECR_BATCH_NO_TAG		"50"
#define ECR_DATE_TAG			"03"
#define ECR_TIME_TAG			"04"
#define ECR_RREF_TAG			"D3"
#define ECR_CARD_NAME_TAG		"D5"
//wubin 090826 start
#define	ECR_AID_TAG				"9A"
#define	ECR_AP_TAG				"9B"
#define	ECR_CRY_INFO_TAG		"9C"
#define	ECR_APP_CRY_TAG			"9D"
#define	ECR_TVR_TAG				"9E"
//wubin 090826 end

#define	ECR_ENTRY_CODE_TAG		"7C"
#define	ECR_QPS_STT_TAG			"7D"

#define	ECR_PRNT_TEXT1_TAG		"7E"
#define	ECR_PRNT_TEXT2_TAG		"7F"
#define	ECR_PRNT_TEXT3_TAG		"7G"
#define	ECR_PRNT_TEXT4_TAG		"7H"

#define	ECR_TRAC_AMOUT_TAG		"40"
#define	ECR_CURRENCY_TAG		"2A"

#define	ECR_GRS_CODE_TAG		"8A"
#define	ECR_GRS_ORGAMT_TAG		"8B"
#define	ECR_GRS_REDAMT_TAG		"8C"
#define	ECR_GRS_MERAMT_TAG		"8D"
#define	ECR_GRS_DISCOUNT_TAG	"8H"

#define	ECR_DCC_CURRENCY_TAG	"8E"
#define	ECR_DCC_CONVRATE_TAG	"8F"
#define	ECR_DCC_CONVAMT_TAG		"8G"

#define ECR_PRNT_TEXT1_SIZE		40
#define ECR_PRNT_TEXT2_SIZE		40
#define ECR_PRNT_TEXT3_SIZE		40
#define ECR_PRNT_TEXT4_SIZE		40

#define ECR_JSON_CMD_SALE               "SALE"
#define ECR_JSON_CMD_VOID               "VOID"
#define ECR_JSON_CMD_TIPS               "TIPS"
#define ECR_JSON_CMD_RETRIEVAL          "RETRIEVAL"
#define ECR_JSON_CMD_TOTAL              "TOTAL"
#define ECR_JSON_CMD_SETTLEMENT         "SETTLEMENT"
#define ECR_JSON_CMD_ECHOTEST           "ECHOTEST"
    
    
#define ECR_JSON_TYPE_VMJ               "VMJ"
#define ECR_JSON_TYPE_AE                "AE"
#define ECR_JSON_TYPE_MPU               "MPU"
#define ECR_JSON_TYPE_DCC               "DCC"
#define ECR_JSON_TYPE_Alipay            "Alipay"
#define ECR_JSON_TYPE_WeChat            "WeChat"
#define ECR_JSON_TYPE_EWALLET           "EWALLET"
    

#define ECR_APPROVED			"00"
#define ECR_DECLINED_ERR		"ND"
#define ECR_REFERAL_ERR			"01"
#define ECR_TIMEOUT_ERR			"TO"
#define ECR_CANNOT_ERR			"ED"
#define ECR_COMMS_ERR			"EN"
#define ECR_UNKNOWN_ERR			"NA"

#define ECR_REMOVE_RESP			"TRANSACTION NOT SUCCESS                 "
#define ECR_APPROVED_RESP		"APPROVAL                                "

#define TRANSPORT_HEADER_SIZE	2
#define TRANSPORT_DEST_SIZE		4
#define TRANSPORT_SRC_SIZE		4

#define FORMAT_VERSION_SIZE		1
#define REQ_RESP_SIZE			1
#define TXN_CODE_SIZE			2
#define MORE_FOLLOW_SIZE		1
#define END_PRESENT_SIZE		1
#define ECR_ENTRY_CODE_SIZE		3
#define ECR_QPS_STT_SIZE		1


#define AMOUNT_VALUE_SIZE		12
#define CASHBACK_VALUE_SIZE		12
#define INVOICE_NUMBER_SIZE		6
#define MID_SIZE				15
#define ISSUER_SIZE				10
#define TAG_SIZE				2
#define LENGTH_SIZE				2
#define CARD_NAME_SIZE			26
#define ECR_RESP_CODE_SIZE		2
#define ECR_RESP_TEXT_SIZE		40
#define ECR_MERCHANT_NAME_SIZE	69

#define ECR_2GENAC_DECLINED    -99

#define DATE_SIZE              	6            /* Date in MMDDYY format        */
#define TIME_SIZE              	6            /* Time in HHMMSS format        */
#define RET_REF_SIZE            26       /* Retrieval reference num size */
#define EXP_DATE_SIZE          	4            /* Expiry date MMYY             */
#define AUTH_CODE_SIZE          6        /* Authorization code size      */
#define TID_SIZE                8        /* Terminal Identification size */
#define BATCH_NUM_SIZE     		6              /* current largest batch number possible          */

#define GEN_VER_SIZE          	16           /* information in config files   */

#define ECR_REQ_CMD_SALE                     "SALE"
#define ECR_REQ_CMD_VOID                     "VOID"
#define ECR_REQ_CMD_REFUND                   "REFUND"
#define ECR_REQ_CMD_SETTLE_ALL               "SETTLE_ALL"
#define ECR_REQ_CMD_SETTLE                   "SETTLE"
#define ECR_REQ_CMD_TIPS                      "TIPS"
#define ECR_REQ_CMD_RETRIEVAL                  "RETRIEVAL"
#define ECR_REQ_CMD_BATCH_TOTAL                 "TOTAL"
#define ECR_REQ_CMD_REPRINT_SETTLE              "REPRINTSETTLE"
#define ECR_REQ_CMD_PREAUTH                  "PREAUTH"
#define ECR_REQ_CMD_PREAUTH_COMP                "PREAUTHCOMP"
#define ECR_REQ_CMD_REPRINTLAST                   "REPRINTLAST"
#define ECR_REQ_CMD_REPRINTANY                   "REPRINTANY"
#define ECR_REQ_CMD_INSTALLMENT                   "INSTALLMENT"
#define ECR_REQ_CMD_ECHO_TEST                  "ECHOTEST"

#define ECR_REQ_CMD_GET_TOTAL				"GET_TOTAL"
#define ECR_REQ_CMD_GET_LAST_TOTAL			"GET_LAST_TOTAL"
#define ECR_REQ_CMD_GET_LAST_TOTAL			"GET_LAST_TOTAL"
#define ECR_REQ_CMD_BARCODE                 "BARCODE"
#define ECR_REQ_CMD_PRINTING                 "PRINTING"
#define ECR_REQ_CMD_READCARD                 "READCARD"
#define ECR_REQ_CMD_VOID_PREAUTH                  "VOID_PREAUTH"


typedef struct tagECR_REQ
{
	char req_resp[REQ_RESP_SIZE + 1];
	char txn_code[TXN_CODE_SIZE + 1];
	char amount[AMOUNT_VALUE_SIZE + 1];
	char cashback[CASHBACK_VALUE_SIZE + 1];
	char inv_no[INVOICE_NUMBER_SIZE + 1];
	char date[DATE_SIZE + 1];
	char rref[RET_REF_SIZE+ 1];
	char Pan[PAN_SIZE + 1];
	char Expiry[EXP_DATE_SIZE+ 1];
	char prnt_no[3+ 1];
	char szCMD[50 + 1];
	char szTYPE[10 + 1];
	char szECRREF[26 + 1];
	char szAuthCode[AUTH_CODE_DIGITS+1];
	char szMID[MERCHANT_ID_BYTES+1];
	BYTE  HDTid;
	char szTipAmount[AMOUNT_VALUE_SIZE + 1];
	char szPrint[1+1];
} ECR_REQ;

typedef struct tagECR_RESP
{
	char resp_code[ECR_RESP_CODE_SIZE + 1];
	char resp_text[ECR_RESP_TEXT_SIZE + 1];
	char merchant_name[ECR_MERCHANT_NAME_SIZE + 1];
	char auth_code[AUTH_CODE_SIZE  + 1];
	char inv_no[INVOICE_NUMBER_SIZE + 1];
	char tid[TID_SIZE + 1];
	char mid[MID_SIZE + 1];
	char issuer[ISSUER_SIZE + 1];
	char card_no[PAN_SIZE + 1];
	char exp_date[EXP_DATE_SIZE + 1];
	char batch_no[BATCH_NUM_SIZE + 1];
	char date[DATE_SIZE + 1];
    char time[TIME_SIZE + 1];
    char rref[RET_REF_SIZE+ 1];
    char card_name[CARD_NAME_SIZE + 1];
	char entry_code[ECR_ENTRY_CODE_SIZE + 1];
	char qps_stt[ECR_QPS_STT_SIZE + 1];
	char prnt_text1[ECR_PRNT_TEXT1_SIZE + 1];
	char prnt_text2[ECR_PRNT_TEXT2_SIZE + 1];
	char prnt_text3[ECR_PRNT_TEXT3_SIZE + 1];
	char prnt_text4[ECR_PRNT_TEXT4_SIZE + 1];
	char Trac_amout[12+1];
	char Currency[3+1];
	char GRS_code[16+1];
	char GRS_orgAmt[12+1];
	char GRS_RedAmt[12+1];
	char GRS_MerAmt[12+1];
	char GRS_DiscAmt[12+1];
	char DCC_Currency[3+1];
	char DCC_ConvRate[13+1];
	char DCC_ConvAmt[10+1];	
} ECR_RESP;

/*
typedef struct tagPAYMT_REQ
{
	unsigned char    	txnCode[2+1];
	unsigned  char 		txnAmount[12+1];
	signed char         issName[10+1];
	signed char         numInvoice[6+1];
	unsigned short  	crdLen;
	unsigned char    	crdPAN[19+1];
	unsigned char    	numBatch[6+1]; 
	unsigned char    	szRREF[12+1];
	unsigned char    	szDate[6+1];
	unsigned char    	rfu[10+1];
	unsigned char   	szOutEcrBuf1[20+1];
	unsigned char   	szOutEcrBuf2[20+1];
	unsigned char   	szOutEcrBuf3[20+1];
}PAYMT_REQ;

typedef struct tagPAYMT_RESP
{
	unsigned char    	txnCode[2+1];
	unsigned  char 		szDispRespText[25+1];
	signed char         ApprovalCode[6+1];
	unsigned char  		szRespCode[2+1];   
	signed char         numInvoice[6+1];
	unsigned char    	txnAmount[12+1];
	unsigned char  		OperatorID[2+1];
	unsigned char 		szTID[8+1];
	unsigned char 		szMID[15+1];
	unsigned char 		szPAN[19+1];
	unsigned short 		shPANlen;
	unsigned char 		szExpDate[6+1];
	unsigned char 		szTxnDate[6+1];
	unsigned char 		szTxnTime[6+1];
	unsigned char 		szBatchNum[6+1];
	unsigned char 		CHName[26+1];
	unsigned char 		issName[26+1];
	unsigned char    	szRREF[12+1];
	unsigned char    	szCardIssID[2+1];
	unsigned short shSalesCnt;
	double dbSaleTotal;
	unsigned short shVoidCnt;
	double dbVoidTotal;
	unsigned short shRefundCnt;
	double dbRefundTotal;
	unsigned short shVoidRefundCnt;
	double dbVoidRefundTotal;
	unsigned char szEMVAID[33+1];
	unsigned char szEMVTC[16+1];
	unsigned char szEMVTVR[10+1];
	unsigned char szEMVTSI[10+1];
	unsigned char szEMVAppName[15+1];
	unsigned char   szEMVCRY [2+1];
	unsigned char   szInEcrBuf1[20+1];
	unsigned char   szInEcrBuf2[20+1];
	unsigned char   szInEcrBuf3[20+1];
	unsigned char   szInEcrBuf4[20+1];
	unsigned char   szInEcrBuf5[20+1];
}PAYMT_RESP;
*/
	
ECR_REQ ECRReq;
ECR_RESP ECRResp;
char chECRFlag;

int inPrintECRPacket(char *szTitle, unsigned char *pucMessage, int inLen);
int inValidatePacket(char* pucPkt,int inSize);
int inValidateLastPacket(void);
int inECRReceiveAnalyse(void);
int inECRSendAnalyse(void);
int inECRSendResponse(void);
USHORT usCTOSS_ShareECRInitialize(void);
int inCTOSS_CheckECREvent(void);
int inValidateJSONPacket(char* pucPkt,int inSize);

char *getMD5(char* szData, char* szKey);
int getContentLength(char* szData, char* szKey);

#define HOST_UOB_VMJ 1
#define HOST_AMEX    14

int inGetAcqrHostIndex(int Index);
void setLCDPrint(int line,int position, char *pbBuf);
void vduiWarningSound(void);
void getTerminalSerialNo(BYTE* sNo);

#ifdef	__cplusplus
}
#endif

#endif	/* __ECR_TRANS_H__ */

