#include <stdint.h>
#include <memory.h>

#include "../misc.h"



#include "../../w25qxx/w25qxx.h"



void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v)
{

}

void oam_flash_init(void)
{
	int rc = W25qxx_Init();
	if (rc) {
		Error_Handler();
	}
	// PageSize = 256, PageCount = 8192, SectorSize = 4096, SectorCount = 512, BlockSize = 65536, BlockCount = 32, CapacityInKiloByte = 2048,
	itm_debug3(DBG_OAM, "FLASH s", w25qxx.PageSize, w25qxx.SectorSize, w25qxx.BlockSize);
	itm_debug3(DBG_OAM, "FLASH c", w25qxx.PageCount, w25qxx.SectorCount, w25qxx.BlockCount);





	if ((1)) {
		W25qxx_EraseSector(1);
		char *str = "Hello world";
		W25qxx_WritePage((uint8_t *) str, W25qxx_SectorToPage(1), 0, strlen(str));

	}



	if ((1)) {
		static uint8_t buf[256] = {0};
		W25qxx_ReadBytes(buf, 4096, 32);
		itm_debug3(DBG_OAM, "buf r", buf[0], buf[1], buf[2]);
	}


	if ((1)) {
		uint8_t t = 0xF;
		W25qxx_WritePage(&t, W25qxx_SectorToPage(1), 2, 1);
	}


	if ((1)) {
		static uint8_t buf[256] = {0};
		W25qxx_ReadBytes(buf, 4096, 32);
		itm_debug3(DBG_OAM, "buf r", buf[0], buf[1], buf[2]);
	}


}
