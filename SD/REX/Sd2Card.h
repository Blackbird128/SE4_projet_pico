/** Definitions pour les cartes SD (adaptation de la bibliotheque Arduino) **/

/* Constantes */

#define	SD_DDR			DDRB
#define	SD_PORT			PORTB
#define SD_PIN			2

#define SD_PROTECT_BLOCK_ZERO	1	/* Protect block zero from write if nonzero */
#define SD_INIT_TIMEOUT		300	/* init timeout ms */
#define SD_ERASE_TIMEOUT	10000	/* erase timeout ms */
#define SD_READ_TIMEOUT		300	/* read timeout ms */
#define SD_WRITE_TIMEOUT	600	/* write time out ms */

#define SD_CARD_ERROR_CMD0		0x01	/* timeout error for command CMD0 */
#define SD_CARD_ERROR_CMD8		0x02	/* CMD8 was not accepted - not a valid SD card*/
#define SD_CARD_ERROR_CMD17		0x03	/* card returned an error response for CMD17 (read block) */
#define SD_CARD_ERROR_CMD24		0x04	/* card returned an error response for CMD24 (write block) */
#define SD_CARD_ERROR_CMD25		0x05	/*  WRITE_MULTIPLE_BLOCKS command failed */
#define SD_CARD_ERROR_CMD58		0x06	/* card returned an error response for CMD58 (read OCR) */
#define SD_CARD_ERROR_ACMD23		0x07	/* SET_WR_BLK_ERASE_COUNT failed */
#define SD_CARD_ERROR_ACMD41		0x08	/* card's ACMD41 initialization process timeout */
#define SD_CARD_ERROR_BAD_CSD		0x09	/* card returned a bad CSR version field */
#define SD_CARD_ERROR_ERASE		0x0a	/* erase block group command failed */
#define SD_CARD_ERROR_ERASE_SINGLE_BLOCK 0x0b	/* card not capable of single block erase */
#define SD_CARD_ERROR_ERASE_TIMEOUT	0x0c	/* Erase sequence timed out */
#define SD_CARD_ERROR_READ		0x0d	/* card returned an error token instead of read data */
#define SD_CARD_ERROR_READ_REG		0x0e	/* read CID or CSD failed */
#define SD_CARD_ERROR_READ_TIMEOUT	0x0f	/* timeout while waiting for start of read data */
#define SD_CARD_ERROR_STOP_TRAN		0x10	/* card did not accept STOP_TRAN_TOKEN */
#define SD_CARD_ERROR_WRITE		0x11	/* card returned an error token as a response to a write */
#define SD_CARD_ERROR_WRITE_BLOCK_ZERO	0x12	/* attempt to write protected block zero */
#define SD_CARD_ERROR_WRITE_MULTIPLE	0x13	/* card did not go ready for a multiple block write */
#define SD_CARD_ERROR_WRITE_PROGRAMMING 0x14	/* card returned error to a CMD13 status check after write */
#define SD_CARD_ERROR_WRITE_TIMEOUT	0x15	/* timeout occurred during write programming */
#define SD_CARD_ERROR_SCK_RATE		0x16	/* incorrect rate selected */

#define SD_CARD_TYPE_SD1	1	/* Standard capacity V1 SD card */
#define SD_CARD_TYPE_SD2	2	/* Standard capacity V2 SD card */
#define SD_CARD_TYPE_SDHC	3	/* High Capacity SD card */

/* Macros */

#define chipSelectInit()	SD_DDR |= (1<<SD_PIN)
#define chipSelectHigh()	SD_PORT |= (1<<SD_PIN)
#define chipSelectLow()		SD_PORT &= ~(1<<SD_PIN)
#define error(info,code)	{info->errorCode_ = code;}
#define type(info,type)		{info->type_ = type;}

/* Structures */

typedef struct{
  uint32_t block_;
  uint8_t errorCode_;
  uint8_t inBlock_;
  uint16_t offset_;
  uint8_t partialBlockRead_;
  uint8_t status_;
  uint8_t type_;
  } SD_info;

/* Prototypes */

void readEnd(SD_info *info);
inline uint8_t readCID(SD_info *info, cid_t* cid);
inline uint8_t readCSD(SD_info *info, csd_t* csd);
uint32_t cardSize(SD_info *info);
uint8_t eraseSingleBlockEnable(SD_info *info);
uint8_t erase(SD_info *info, uint32_t firstBlock, uint32_t lastBlock);
uint8_t sd_init(SD_info *info);
void partialBlockRead(SD_info *info, uint8_t value);
uint8_t readData(SD_info *info, uint32_t block, uint16_t offset, uint16_t count, uint8_t* dst);
uint8_t readBlock(SD_info *info, uint32_t block, uint8_t* dst);
uint8_t writeData(SD_info *info, const uint8_t* src);
uint8_t writeBlock(SD_info *info, uint32_t blockNumber, const uint8_t* src);
uint8_t writeStart(SD_info *info, uint32_t blockNumber, uint32_t eraseCount);
uint8_t writeStop(SD_info *info);
