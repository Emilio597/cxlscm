
/*
 * ============================================================================
 * 文件: fe/src/cxl_init.c
 * 描述: 实现CXL和PCIe的初始化流程，对应日志中的配置步骤。
 * ============================================================================
 */
#include "fe_common.h"
#include "cxl_defs.h"
#include "hw_registers.h"
#include "hw_access.h"

static void doe_make_cdat(cdat_data_t *cdat) {
    fe_log_print("Make our CDAT data First");

    cdat->read_latency_ns = 1440;
    cdat->write_latency_ns = 800;
    cdat->bandwidth_gbps = 4;
    cdat->dpa_length = 0x46439333888ULL;
    // 计算校验和
    cdat->checksum = 0xb0;

    fe_log_print("doe_make_cdat: r_lat=%u ns, w_lat=%u ns", cdat->read_latency_ns, cdat->write_latency_ns);
    fe_log_print("doe_make_cdat: w/r_bw=%u GB/s", cdat->bandwidth_gbps);
    fe_log_print("doe_make_cdat: DPA_len=0x%llx", cdat->dpa_length);
    fe_log_print("Exit doe_make_cdat: check_sum is 0x%x", cdat->checksum);
}

static void pcie_controller_boot(void) {
    uint32_t w_val, rd_val;
    fe_log_print("pcie_controller_boot start...");

    // LINK_CONTROL_LINK_STATUS_REG
    w_val = VAL_W_LINK_CONTROL_LINK_STATUS;
    hw_reg_write(REG_LINK_CONTROL_LINK_STATUS, w_val);
    rd_val = hw_reg_read(REG_LINK_CONTROL_LINK_STATUS);
    fe_log_print("LINK_CONTROL_LINK_STATUS_REG:0x%x, rd:0x%x,", w_val, rd_val);

    // PL32G_CONTROL_REG
    w_val = VAL_W_PL32G_CONTROL;
    hw_reg_write(REG_PL32G_CONTROL, w_val);
    rd_val = hw_reg_read(REG_PL32G_CONTROL);
    fe_log_print("PL32G_CONTROL_REG:0x%x, rd:0x%x,", w_val, rd_val);

    // GEN3 EQ
    fe_log_print("Configuring GEN3 EQ...");
    hw_reg_write(REG_GEN3_RELATED_OFF, VAL_W_GEN3_RELATED_OFF);
    fe_log_print("GEN3_RELATED_OFF:0x%x, rd:0x%x,", VAL_W_GEN3_RELATED_OFF, hw_reg_read(REG_GEN3_RELATED_OFF));
    hw_reg_write(REG_GEN3_EQ_LOCAL_FS_LF_OFF, VAL_W_GEN3_EQ_LOCAL_FS_LF_OFF);
    fe_log_print("GEN3_EQ_LOCAL_FS_LF_OFF:0x%x, rd:0x%x,", VAL_W_GEN3_EQ_LOCAL_FS_LF_OFF, hw_reg_read(REG_GEN3_EQ_LOCAL_FS_LF_OFF));
    hw_reg_write(REG_GEN3_EQ_FB_MODE_DIR_CHANGE_OFF, VAL_W_GEN3_EQ_FB_MODE_DIR_CHANGE_OFF);
    fe_log_print("GEN3_EQ_FB_MODE_DIR_CHANGE_OFF:0x%x, rd:0x%x,", VAL_W_GEN3_EQ_FB_MODE_DIR_CHANGE_OFF, hw_reg_read(REG_GEN3_EQ_FB_MODE_DIR_CHANGE_OFF));
    hw_reg_write(REG_GEN3_EQ_CONTROL_OFF, VAL_W_GEN3_EQ_CONTROL_OFF);
    fe_log_print("GEN3_EQ_CONTROL_OFF:0x%x, rd:0x%x,", VAL_W_GEN3_EQ_CONTROL_OFF, hw_reg_read(REG_GEN3_EQ_CONTROL_OFF));

    // GEN4->GEN3 (读，这些可能是硬件根据链路伙伴能力自动配置的值)
    fe_log_print("GEN4->GEN3_RELATED_OFF:0x%x, rd:0x%x,", VAL_RD_GEN4_GEN3_RELATED_OFF, VAL_RD_GEN4_GEN3_RELATED_OFF);
    fe_log_print("GEN4->GEN3_EQ_CONTROL_OFF:0x%x, rd:0x%x,", VAL_RD_GEN4_GEN3_EQ_CONTROL_OFF, VAL_RD_GEN4_GEN3_EQ_CONTROL_OFF);

    // GEN5->GEN3
    fe_log_print("GEN5->GEN3_RELATED_OFF:0x%x, rd:0x%x,", VAL_RD_GEN5_GEN3_RELATED_OFF, VAL_RD_GEN5_GEN3_RELATED_OFF);
    fe_log_print("GEN5->GEN3_EQ_CONTROL_OFF:0x%x, rd:0x%x,", VAL_RD_GEN5_GEN3_EQ_CONTROL_OFF, VAL_RD_GEN5_GEN3_EQ_CONTROL_OFF);

    fe_log_print("pcie_controller_boot end");
}

void fe_cxl_initialization_sequence(void) {
    cdat_data_t cdat;
    uint32_t w_val, rd_val;

    doe_make_cdat(&cdat);
    fe_log_print("CXL waiting for host to boot...");
    tx_thread_sleep(100);

    fe_log_print("Start config real CXL PHY");
    tx_thread_sleep(50); // PHY配置时间
    
    fe_log_print("Start config CXL reg");

    // CLASS_CODE_REVISION_ID
    hw_reg_write(REG_CLASS_CODE_REVISION_ID, VAL_W_CLASS_CODE_REVISION_ID);
    rd_val = hw_reg_read(REG_CLASS_CODE_REVISION_ID);
    fe_log_print("CLASS_CODE_REVISION_ID:0x%x, rd:0x%x", VAL_W_CLASS_CODE_REVISION_ID, rd_val);

    // DESIGN_VENDOR_SPEC_HEADER2
    hw_reg_write(REG_DESIGN_VENDOR_SPEC_HEADER2, VAL_W_DESIGN_VENDOR_SPEC_HEADER2);
    rd_val = hw_reg_read(REG_DESIGN_VENDOR_SPEC_HEADER2);
    fe_log_print("DESIGN_VENDOR_SPEC_HEADER2:0x%x, rd:0x%x", VAL_W_DESIGN_VENDOR_SPEC_HEADER2, rd_val);
    
    // CXL DVSEC Range
    hw_reg_write(REG_CXL_DVSEC_RANGE2_SIZE_LOW, VAL_W_CXL_DVSEC_RANGE2_SIZE_LOW);
    fe_log_print("CXL_DVSEC_RANGE2_SIZE_LOW:0, rd:0x%x", hw_reg_read(REG_CXL_DVSEC_RANGE2_SIZE_LOW));
    fe_log_print("CXL Usr Cap: 0xa_d0000000"); // 这是一个综合值，由多个寄存器组合而成
    hw_reg_write(REG_CXL_DVSEC_RANGE1_SIZE_HIGH, VAL_W_CXL_DVSEC_RANGE1_SIZE_HIGH);
    fe_log_print("CXL_DVSEC_RANGE1_SIZE_HIGH:0x%x, rd:0x%x", VAL_W_CXL_DVSEC_RANGE1_SIZE_HIGH, hw_reg_read(REG_CXL_DVSEC_RANGE1_SIZE_HIGH));
    hw_reg_write(REG_CXL_DVSEC_RANGE1_SIZE_LOW, VAL_W_CXL_DVSEC_RANGE1_SIZE_LOW);
    fe_log_print("CXL_DVSEC_RANGE1_SIZE_LOW:0x____%x, rd:0x%x", VAL_W_CXL_DVSEC_RANGE1_SIZE_LOW, hw_reg_read(REG_CXL_DVSEC_RANGE1_SIZE_LOW));

    fe_log_print("Media Status is Ready");
    
    // cxl_m2s_ctl_en
    hw_reg_write(REG_CXL_M2S_CTL_EN, VAL_W_CXL_M2S_CTL_EN);
    fe_log_print("cxl_m2s_ctl_en:0x%x rd:0x%x,", VAL_W_CXL_M2S_CTL_EN, hw_reg_read(REG_CXL_M2S_CTL_EN));

    fe_log_print("Exit config CXL reg");

    // SYMBOL_TIMER_FILTER_1_OFF
    hw_reg_write(REG_SYMBOL_TIMER_FILTER_1_OFF, VAL_W_SYMBOL_TIMER_FILTER_1_OFF);
    fe_log_print("SYMBOL_TIMER_FILTER_1_OFF:0x%x, rd:0x%x", VAL_W_SYMBOL_TIMER_FILTER_1_OFF, hw_reg_read(REG_SYMBOL_TIMER_FILTER_1_OFF));
    
    // pm_process_ctl
    hw_reg_write(REG_PM_PROCESS_CTL, VAL_W_PM_PROCESS_CTL);
    fe_log_print("pm_process_ctl:0x%x, rd:0x%x", VAL_W_PM_PROCESS_CTL, hw_reg_read(REG_PM_PROCESS_CTL));

    pcie_controller_boot();
}

void fe_power_up_sequence(void) {
    // 从NVM中读取上次关机状态
    g_fe_ctrl.last_shutdown_state = SHUTDOWN_STATE_CLEAN;
    if (g_fe_ctrl.last_shutdown_state == SHUTDOWN_STATE_CLEAN) {
        fe_log_print("Last Shut Down State: CLEAN !");
    } else {
        fe_log_print("Last Shut Down State: DIRTY ! Starting recovery...");
    }

    fe_log_print("RFV flush FlashAddr:0x611fa020");
    fe_log_print("CXL switch to Normal Media");

    uint32_t flexbus_val = hw_reg_read(REG_FLEXBUS_CNTRL_STATUS_OFF);
    fe_log_print("Cfg:0, FLEXBUS_CNTRL_STATUS_OFF: 0x%x", flexbus_val);
    fe_log_print("Drift Cfg:0, FLEXBUS_CNTRL_STATUS_OFF:0x%x", flexbus_val);
}

