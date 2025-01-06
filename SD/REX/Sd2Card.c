/** Bibliotheque pour les cartes SD (adaptation de la bibliotheque Arduino) **/

#include "string.h"
#include "avr/io.h"
#include "util/delay.h"

#include "spi.h"
#include "SdInfo.h"
#include "Sd2Card.h"

#define true	1
#define false	0

//------------------------------------------------------------------------------
// wait for card to go not busy
static uint8_t waitNotBusy(uint16_t timeoutMillis) {
int i;
for(i=0;i<timeoutMillis;i++){
  if(spi_exch(0xff)==0xff) return true;
  _delay_ms(1);
  }
return false;
}

//------------------------------------------------------------------------------
/** Skip remaining data in a block when in partial block read mode. */
void readEnd(SD_info *info) {
if(info->inBlock_){
  while(info->offset_++<514) spi_exch(0xff);
  chipSelectHigh();
  info->inBlock_=0;
  }
}

//------------------------------------------------------------------------------
// send command and return error code.  Return zero for OK
static uint8_t cardCommand(SD_info *info, uint8_t cmd, uint32_t arg) {
readEnd(info);			// end read if in partialBlockRead mode
chipSelectLow();		// select card
waitNotBusy(30);		// wait up to 300 ms if busy
spi_exch(cmd|0x40);		// send command
int8_t s;			// send argument
for(s=24;s>=0;s -= 8) spi_exch(arg >> s);
uint8_t crc=0xff;		// send CRC
if(cmd==CMD0) crc=0x95;	// correct crc for CMD0 with arg 0
if(cmd==CMD8) crc=0x87;	// correct crc for CMD8 with arg 0X1AA
spi_exch(crc);
uint8_t i;			// wait for response
for(i=0;((info->status_=spi_exch(0xff)) & 0x80) && i!=0xff;i++);
return info->status_;
}

//------------------------------------------------------------------------------
static uint8_t waitStartBlock(SD_info *info){
int cpt=0;
while((info->status_=spi_exch(0xff))==0xff){
  if(cpt>SD_READ_TIMEOUT){
    error(info, SD_CARD_ERROR_READ_TIMEOUT);
    goto fail;
    }
  cpt++;
  }
if(info->status_!=DATA_START_BLOCK){
  error(info, SD_CARD_ERROR_READ);
  goto fail;
  }
return true;

fail:
chipSelectHigh();
return false;
}

//------------------------------------------------------------------------------
/** read CID or CSR register */
static uint8_t readRegister(SD_info *info, uint8_t cmd, void* buf) {
uint8_t* dst=buf;
if(cardCommand(info, cmd, 0)){
  error(info, SD_CARD_ERROR_READ_REG);
  goto fail;
  }
if(!waitStartBlock(info)) goto fail;
// transfer data
uint16_t i;
for(i=0;i<16;i++) dst[i]=spi_exch(0xff);
spi_exch(0xff);  // get first crc byte
spi_exch(0xff);  // get second crc byte
chipSelectHigh();
return true;

fail:
chipSelectHigh();
return false;
}

//------------------------------------------------------------------------------
inline uint8_t readCID(SD_info *info, cid_t *cid){ return readRegister(info, CMD10, cid); }
inline uint8_t readCSD(SD_info *info, csd_t *csd){ return readRegister(info, CMD9, csd); }

//------------------------------------------------------------------------------
/**
 * Determine the size of an SD flash memory card.
 *
 * \return The number of 512 byte data blocks in the card
 *         or zero if an error occurs.
 */
uint32_t cardSize(SD_info *info){
csd_t csd;
if(!readCSD(info, &csd)) return 0;
if(csd.v1.csd_ver==0){
  uint8_t read_bl_len=csd.v1.read_bl_len;
  uint16_t c_size=(csd.v1.c_size_high << 10)
                  |(csd.v1.c_size_mid << 2)|csd.v1.c_size_low;
  uint8_t c_size_mult=(csd.v1.c_size_mult_high << 1)
                      |csd.v1.c_size_mult_low;
  return (uint32_t)(c_size+1) << (c_size_mult+read_bl_len-7);
  }
else if(csd.v2.csd_ver==1){
  uint32_t c_size=((uint32_t)csd.v2.c_size_high << 16)
                  |(csd.v2.c_size_mid << 8)|csd.v2.c_size_low;
  return (c_size+1) << 10;
  }
else{
  error(info, SD_CARD_ERROR_BAD_CSD);
  return 0;
  }
}

//------------------------------------------------------------------------------
/** Determine if card supports single block erase.
 *
 * \return The value one, true, is returned if single block erase is supported.
 * The value zero, false, is returned if single block erase is not supported.
 */
uint8_t eraseSingleBlockEnable(SD_info *info) {
csd_t csd;
return readCSD(info, &csd)?csd.v1.erase_blk_en:0;
}

//------------------------------------------------------------------------------
/** Erase a range of blocks.
 *
 * \param[in] firstBlock The address of the first block in the range.
 * \param[in] lastBlock The address of the last block in the range.
 *
 * \note This function requests the SD card to do a flash erase for a
 * range of blocks.  The data on the card after an erase operation is
 * either 0 or 1, depends on the card vendor.  The card must support
 * single block erase.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t erase(SD_info *info, uint32_t firstBlock, uint32_t lastBlock) {
if(!eraseSingleBlockEnable(info)){
  error(info, SD_CARD_ERROR_ERASE_SINGLE_BLOCK);
  goto fail;
  }
if(info->type_!=SD_CARD_TYPE_SDHC){
  firstBlock <<= 9;
  lastBlock <<= 9;
  }
if(cardCommand(info, CMD32, firstBlock)
   || cardCommand(info, CMD33, lastBlock)
   || cardCommand(info, CMD38, 0)) {
  error(info, SD_CARD_ERROR_ERASE);
  goto fail;
  }
if(!waitNotBusy(SD_ERASE_TIMEOUT)){
  error(info, SD_CARD_ERROR_ERASE_TIMEOUT);
  goto fail;
  }
chipSelectHigh();
return true;

fail:
chipSelectHigh();
return false;
}

//------------------------------------------------------------------------------
static uint8_t cardAcmd(SD_info *info, uint8_t cmd, uint32_t arg) {
cardCommand(info, CMD55, 0);
return cardCommand(info, cmd, arg);
}

//------------------------------------------------------------------------------
/**
 * Initialize an SD flash memory card.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  The reason for failure
 * can be determined by calling errorCode() and errorData().
 */
uint8_t sd_init(SD_info *info){
memset(info,0,sizeof(SD_info));

// Initialize CS
chipSelectInit();
chipSelectHigh();

// must supply min of 74 clock cycles with CS high.
uint8_t i;
for(i=0;i<10;i++) spi_exch(0xff);
chipSelectLow();

// command to go idle in SPI mode
int cpt=0;
while((info->status_=cardCommand(info, CMD0, 0))!=R1_IDLE_STATE){
  if(cpt>SD_INIT_TIMEOUT){
    error(info, SD_CARD_ERROR_CMD0);
    goto fail;
    }
  _delay_ms(1);
  cpt++;
  }

// check SD version
if((cardCommand(info, CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)){
  type(info, SD_CARD_TYPE_SD1);
  }
else{
  // only need last byte of r7 response
  uint8_t i;
  for(i=0;i<4;i++) info->status_=spi_exch(0xff);
  if(info->status_!=0xaa) {
    error(info, SD_CARD_ERROR_CMD8);
    goto fail;
    }
  type(info, SD_CARD_TYPE_SD2);
  }
// initialize card and send host supports SDHC if SD2
uint32_t arg;
arg=(info->type_==SD_CARD_TYPE_SD2)?0x40000000:0;

while((info->status_=cardAcmd(info, ACMD41, arg))!=R1_READY_STATE){
  // check for timeout
  if(cpt>SD_INIT_TIMEOUT){
    error(info, SD_CARD_ERROR_ACMD41);
    goto fail;
    }
  _delay_ms(1);
  cpt++;
  }
// if SD2 read OCR register to check for SDHC card
if(info->type_==SD_CARD_TYPE_SD2){
  if(cardCommand(info, CMD58, 0)){
    error(info, SD_CARD_ERROR_CMD58);
    goto fail;
    }
  if((spi_exch(0xff) & 0xc0) == 0xc0) type(info, SD_CARD_TYPE_SDHC);
  // discard rest of ocr - contains allowed voltage range
  uint8_t i;
  for(i=0;i<3;i++) spi_exch(0xff);
  }
chipSelectHigh();
return true;

fail:
chipSelectHigh();
return false;
}

//------------------------------------------------------------------------------
/**
 * Enable or disable partial block reads.
 *
 * Enabling partial block reads improves performance by allowing a block
 * to be read over the SPI bus as several sub-blocks.  Errors may occur
 * if the time between reads is too long since the SD card may timeout.
 * The SPI SS line will be held low until the entire block is read or
 * readEnd() is called.
 *
 * Use this for applications like the Adafruit Wave Shield.
 *
 * \param[in] value The value TRUE (non-zero) or FALSE (zero).)
 */
void partialBlockRead(SD_info *info, uint8_t value) {
readEnd(info);
info->partialBlockRead_ = value;
}

//------------------------------------------------------------------------------
/**
 * Read part of a 512 byte block from an SD card.
 *
 * \param[in] block Logical block to be read.
 * \param[in] offset Number of bytes to skip at start of block
 * \param[out] dst Pointer to the location that will receive the data.
 * \param[in] count Number of bytes to read
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t readData(SD_info *info, uint32_t block, uint16_t offset, uint16_t count, uint8_t* dst) {
if(count==0) return true;
if((count+offset)>512){ goto fail; }
if(!info->inBlock_||block!=info->block_||offset<info->offset_){
  info->block_=block;
  // use address if not SDHC card
  if(info->type_!=SD_CARD_TYPE_SDHC) block <<= 9;
  if(cardCommand(info, CMD17, block)){
    error(info, SD_CARD_ERROR_CMD17);
    goto fail;
    }
  if(!waitStartBlock(info)){ goto fail; }
  info->offset_=0;
  info->inBlock_=1;
  }

// skip data before offset
for(;info->offset_<offset;info->offset_++) spi_exch(0xff);
// transfer data
uint16_t i;
for(i=0;i<count;i++) dst[i]=spi_exch(0xff);

info->offset_ += count;
if(!info->partialBlockRead_||info->offset_>=512){
  // read rest of data, checksum and set chip select high
  readEnd(info);
  }
return true;

fail:
chipSelectHigh();
return false;
}

//------------------------------------------------------------------------------
/**
 * Read a 512 byte block from an SD card device.
 *
 * \param[in] block Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data.

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t readBlock(SD_info *info, uint32_t block, uint8_t* dst) {
return readData(info, block, 0, 512, dst);
}

//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
static uint8_t writeData_(SD_info *info, uint8_t token, const uint8_t* src) {
spi_exch(token);
uint16_t i;
for(i=0;i<512;i++) spi_exch(src[i]);
spi_exch(0xff);  // dummy crc
spi_exch(0xff);  // dummy crc
info->status_=spi_exch(0xff);
if((info->status_&DATA_RES_MASK)!=DATA_RES_ACCEPTED){
  error(info, SD_CARD_ERROR_WRITE);
  chipSelectHigh();
  return false;
  }
return true;
}

//------------------------------------------------------------------------------
/** Write one data block in a multiple block write sequence */
uint8_t writeData(SD_info *info, const uint8_t* src){
// wait for previous write to finish
if(!waitNotBusy(SD_WRITE_TIMEOUT)){
  error(info, SD_CARD_ERROR_WRITE_MULTIPLE);
  chipSelectHigh();
  return false;
  }
return writeData_(info, WRITE_MULTIPLE_TOKEN, src);
}

//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to an SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t writeBlock(SD_info *info, uint32_t blockNumber, const uint8_t* src) {
// use address if not SDHC card
if(info->type_!=SD_CARD_TYPE_SDHC) blockNumber <<= 9;
if(cardCommand(info, CMD24, blockNumber)){
  error(info, SD_CARD_ERROR_CMD24);
  goto fail;
  }
if(!writeData_(info, DATA_START_BLOCK, src)) goto fail;

// wait for flash programming to complete
if(!waitNotBusy(SD_WRITE_TIMEOUT)){
  error(info, SD_CARD_ERROR_WRITE_TIMEOUT);
  goto fail;
  }
// response is r2 so get and check two bytes for nonzero
if(cardCommand(info, CMD13, 0) || spi_exch(0xff)){
  error(info,SD_CARD_ERROR_WRITE_PROGRAMMING);
  goto fail;
  }
chipSelectHigh();
return true;

fail:
chipSelectHigh();
return false;
}

//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 * \param[in] eraseCount The number of blocks to be pre-erased.
 *
 * \note This function is used with writeData() and writeStop()
 * for optimized multiple block writes.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t writeStart(SD_info *info, uint32_t blockNumber, uint32_t eraseCount) {
// send pre-erase count
if(cardAcmd(info, ACMD23, eraseCount)){
  error(info, SD_CARD_ERROR_ACMD23);
  goto fail;
  }
// use address if not SDHC card
if(info->type_!=SD_CARD_TYPE_SDHC) blockNumber <<= 9;
if(cardCommand(info, CMD25, blockNumber)){
  error(info, SD_CARD_ERROR_CMD25);
  goto fail;
  }
return true;

fail:
chipSelectHigh();
return false;
}

//------------------------------------------------------------------------------
/** End a write multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t writeStop(SD_info *info) {
if(!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
spi_exch(STOP_TRAN_TOKEN);
if(!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
chipSelectHigh();
return true;

fail:
error(info, SD_CARD_ERROR_STOP_TRAN);
chipSelectHigh();
return false;
}
