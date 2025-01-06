/** Definitions pour les cartes SD (adaptation de la bibliotheque Arduino) **/

/* Constantes */

/** GO_IDLE_STATE - init card in spi mode if CS low */
#define CMD0		0x00
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define CMD8		0x08
/** SEND_CSD - read the Card Specific Data (CSD register) */
#define CMD9		0x09
/** SEND_CID - read the card identification information (CID register) */
#define CMD10		0x0a
/** SEND_STATUS - read the card status register */
#define CMD13		0x0d
/** READ_BLOCK - read a single data block from the card */
#define CMD17		0x11
/** WRITE_BLOCK - write a single data block to the card */
#define CMD24		0x18
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
#define CMD25		0x19
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
#define CMD32		0x20
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
#define CMD33		0x21
/** ERASE - erase all previously selected blocks */
#define CMD38		0x26
/** APP_CMD - escape for application specific command */
#define CMD55		0x37
/** READ_OCR - read the OCR register of a card */
#define CMD58		0x3a
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
#define ACMD23		0x17
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
#define ACMD41		0x29

/** status for card in the ready state */
#define R1_READY_STATE		0x00
/** status for card in the idle state */
#define R1_IDLE_STATE		0x01
/** status bit for illegal command */
#define R1_ILLEGAL_COMMAND	0x04
/** start data token for read or write single block*/
#define DATA_START_BLOCK	0xfe
/** stop token for write multiple blocks*/
#define STOP_TRAN_TOKEN		0xfd
/** start data token for write multiple blocks*/
#define WRITE_MULTIPLE_TOKEN	0xfc
/** mask for data response tokens after a write block operation */
#define DATA_RES_MASK		0x1f
/** write data accepted token */
#define DATA_RES_ACCEPTED	0x05

/* Structures */

typedef struct{
  uint8_t mid;			// Manufacturer ID
  char oid[2];			// OEM/Application ID
  char pnm[5];			// Product name
  unsigned prv_m : 4;		// Product revision n.m
  unsigned prv_n : 4;
  uint32_t psn;			// Product serial number
  unsigned mdt_year_high : 4;	// Manufacturing date
  unsigned reserved : 4;
  unsigned mdt_month : 4;
  unsigned mdt_year_low :4;
  unsigned always1 : 1;
  unsigned crc : 7;
  } cid_t;

typedef struct{
  unsigned reserved1 : 6;
  unsigned csd_ver : 2;
  uint8_t taac;
  uint8_t nsac;
  uint8_t tran_speed;
  uint8_t ccc_high;
  unsigned read_bl_len : 4;
  unsigned ccc_low : 4;
  unsigned c_size_high : 2;
  unsigned reserved2 : 2;
  unsigned dsr_imp : 1;
  unsigned read_blk_misalign :1;
  unsigned write_blk_misalign : 1;
  unsigned read_bl_partial : 1;
  uint8_t c_size_mid;
  unsigned vdd_r_curr_max : 3;
  unsigned vdd_r_curr_min : 3;
  unsigned c_size_low :2;
  unsigned c_size_mult_high : 2;
  unsigned vdd_w_cur_max : 3;
  unsigned vdd_w_curr_min : 3;
  unsigned sector_size_high : 6;
  unsigned erase_blk_en : 1;
  unsigned c_size_mult_low : 1;
  unsigned wp_grp_size : 7;
  unsigned sector_size_low : 1;
  unsigned write_bl_len_high : 2;
  unsigned r2w_factor : 3;
  unsigned reserved3 : 2;
  unsigned wp_grp_enable : 1;
  unsigned reserved4 : 5;
  unsigned write_partial : 1;
  unsigned write_bl_len_low : 2;
  unsigned reserved5: 2;
  unsigned file_format : 2;
  unsigned tmp_write_protect : 1;
  unsigned perm_write_protect : 1;
  unsigned copy : 1;
  unsigned file_format_grp : 1;
  unsigned always1 : 1;
  unsigned crc : 7;
  } csd1_t;

typedef struct{
  unsigned reserved1 : 6;
  unsigned csd_ver : 2;
  uint8_t taac;
  uint8_t nsac;
  uint8_t tran_speed;
  uint8_t ccc_high;
  unsigned read_bl_len : 4;
  unsigned ccc_low : 4;
  unsigned reserved2 : 4;
  unsigned dsr_imp : 1;
  unsigned read_blk_misalign :1;
  unsigned write_blk_misalign : 1;
  unsigned read_bl_partial : 1;
  unsigned reserved3 : 2;
  unsigned c_size_high : 6;
  uint8_t c_size_mid;
  uint8_t c_size_low;
  unsigned sector_size_high : 6;
  unsigned erase_blk_en : 1;
  unsigned reserved4 : 1;
  unsigned wp_grp_size : 7;
  unsigned sector_size_low : 1;
  unsigned write_bl_len_high : 2;
  unsigned r2w_factor : 3;
  unsigned reserved5 : 2;
  unsigned wp_grp_enable : 1;
  unsigned reserved6 : 5;
  unsigned write_partial : 1;
  unsigned write_bl_len_low : 2;
  unsigned reserved7: 2;
  unsigned file_format : 2;
  unsigned tmp_write_protect : 1;
  unsigned perm_write_protect : 1;
  unsigned copy : 1;
  unsigned file_format_grp : 1;
  unsigned always1 : 1;
  unsigned crc : 7;
  } csd2_t;

typedef union {
  csd1_t v1;
  csd2_t v2;
  } csd_t;
