#include "be_common.h"
#include "pcm_controller.h"  
#include "address_translation.h"
#define MAPPING_TABLE_SIZE 1024 // 仅为
static uint64_t g_mapping_table[MAPPING_TABLE_SIZE];

void at_init(void) {
    be_log_print("Address Translation (AT) module initialized.");
    // 在真实系统中，地址翻译层(FTL)非常复杂。它负责将主机的逻辑块地址(LBA)
    // 映射到NAND的物理地址(PBA)。这个映射是动态的，因为磨损均衡和垃圾回收
    // 会不断移动数据。这个表本身必须持久化存储。
    for(int i = 0; i < MAPPING_TABLE_SIZE; ++i) { g_mapping_table[i] = i; }
}

int at_update_mapping(uint64_t lba, uint64_t new_pba) {
    uint32_t index = lba % MAPPING_TABLE_SIZE;
    be_log_print("AT: Updating mapping for LBA 0x%llx to new PBA 0x%llx.", lba, new_pba);
    g_mapping_table[index] = new_pba;
    be_log_print("AT: (Simulating) Persisting mapping update to NAND.");
    return 0;
}
  
// 地址转换表读取  
be_status_t address_translation_read_table(uint32_t entry_index, address_map_entry_t *entry) {  
    if (!entry) {  
        return BE_STATUS_INVALID_PARAM;  
    }  
      
    // 计算地址转换表在PCM中的位置  
    uint64_t table_address = ADDRESS_TRANSLATION_TABLE_BASE + (entry_index * sizeof(address_map_entry_t));  
      
    // 直接从PCM读取地址转换表项  
    return pcm_read(table_address, (uint8_t*)entry, sizeof(address_map_entry_t));  
}  
  
// 地址转换表写入  
be_status_t address_translation_write_table(uint32_t entry_index, const address_map_entry_t *entry) {  
    if (!entry) {  
        return BE_STATUS_INVALID_PARAM;  
    }  
      
    // 计算地址转换表在PCM中的位置  
    uint64_t table_address = ADDRESS_TRANSLATION_TABLE_BASE + (entry_index * sizeof(address_map_entry_t));  
      
    // 直接写入PCM  
    return pcm_write(table_address, (const uint8_t*)entry, sizeof(address_map_entry_t));  
}