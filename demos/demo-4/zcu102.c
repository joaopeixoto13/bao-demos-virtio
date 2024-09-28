#include <config.h>

// Frontend #1 binary
VM_IMAGE(frontend1, "PATH/TO/YOUR/frontend1.bin");
// Frontend #2 binary
VM_IMAGE(frontend2, "PATH/TO/YOUR/frontend2.bin");
// Backend binary
VM_IMAGE(backend, "PATH/TO/YOUR/backend.bin");
// FreeRTOS binary
VM_IMAGE(freertos, "PATH/TO/YOUR/freertos.bin");

struct config config = {
    CONFIG_HEADER
    
    .shmemlist_size = 3,
    .shmemlist = (struct shmem[]) {
        [0] = { .size = 0x00010000, },
        [1] = { .size = 0x01000000, },
        [2] = { .size = 0x01000000, },
    },

    .vmlist_size = 4,
    .vmlist = (struct vm_config[]) {
    	// Backend (Linux)
        { 
            .image = {
                .base_addr = 0x00000000,
                .load_addr = VM_IMAGE_OFFSET(backend),
                .size = VM_IMAGE_SIZE(backend)
            },

            .entry = 0x00000000,

            .cpu_affinity = 0x1,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x00000000,
                        .size = 0x40000000,
                    },
                },

                .ipc_num = 1,
                .ipcs = (struct ipc[]) {
                    {
                        .base = 0x70000000,
                        .size = 0x00010000,
                        .shmem_id = 0,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {52}
                    }
                },
                
                .remio_dev_num = 2,                                                                                        
                .remio_devs = (struct remio_dev[]) {
                    {
                        .id = 0,
                        .type = REMIO_DEV_BACKEND,
                        .interrupt = 40,
                        .shmem = {
                            .base = 0x80000000,
                            .size = 0x01000000,
                            .shmem_id = 1,
                        }
                    },
                    {
                        .id = 1,
                        .type = REMIO_DEV_BACKEND,
                        .interrupt = 41,
                        .shmem = {
                            .base = 0x81000000,
                            .size = 0x01000000,
                            .shmem_id = 2,
                        }
                    }
                },

                .dev_num = 4,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        // MMC (SD Card)
                        .id = 0x871, /* smmu stream id */
                        .pa = 0xff170000,
                        .va = 0xff170000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {81}                     
                    },
                    {
                        // UART1
                        .pa = 0xFF010000,
                        .va = 0xFF010000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {54}              
                    },
                    {
                        // GEM3 (Ethernet)
                        .id = 0x877, /* smmu stream id */
                        .pa = 0xff0e0000,
                        .va = 0xff0e0000,
                        .size = 0x1000,
                        .interrupt_num = 2,
                        .interrupts = 
                            (irqid_t[]) {95, 96}                           
                    },
                    {   
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0xF9010000,    
                        .gicc_addr = 0xF9020000,
                    }
                }
            },
        },
     	// Frontend #1 (Linux)
        { 
            .image = {
                .base_addr = 0x10000000,
                .load_addr = VM_IMAGE_OFFSET(frontend1),
                .size = VM_IMAGE_SIZE(frontend1)
            },

            .entry = 0x10000000,
            
            .cpu_affinity = 0x2,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x10000000,
                        .size = 0x30000000,
                    },
                },

                .remio_dev_num = 1,
                .remio_devs = (struct remio_dev[]) {
                    {
                        .id = 0,
                        .type = REMIO_DEV_FRONTEND,
                        .size = 0x200,                                  
                        .va = 0xa003e00,                                
                        .interrupt = 79,                         
                        .shmem = {
                            .base = 0x80000000,
                            .size = 0x01000000,
                            .shmem_id = 1,
                        }
                    }
                },

                .dev_num = 1,
                .devs =  (struct vm_dev_region[]) 
                {
                    {   
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0xF9010000,    
                        .gicc_addr = 0xF9020000,
                    }
                }
            },
        },
        // Frontend #2 (Linux)
        { 
            .image = {
                .base_addr = 0x10000000,
                .load_addr = VM_IMAGE_OFFSET(frontend2),
                .size = VM_IMAGE_SIZE(frontend2)
            },

            .entry = 0x10000000,
            
            .cpu_affinity = 0x4,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x10000000,
                        .size = 0x30000000,
                    },
                },

                .remio_dev_num = 1,
                .remio_devs = (struct remio_dev[]) {
                    {
                        .id = 1,
                        .type = REMIO_DEV_FRONTEND,
                        .size = 0x200,                                  
                        .va = 0xa003e00,                                
                        .interrupt = 79,                         
                        .shmem = {
                            .base = 0x81000000,
                            .size = 0x01000000,
                            .shmem_id = 2,
                        }
                    }
                },

                .dev_num = 1,
                .devs =  (struct vm_dev_region[]) 
                {
                    {   
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0xF9010000,    
                        .gicc_addr = 0xF9020000,
                    }
                }
            },
        },
        // FreeRTOS
        {
            .image = {
                .base_addr = 0x0,
                .load_addr = VM_IMAGE_OFFSET(freertos),
                .size = VM_IMAGE_SIZE(freertos)
            },

            .entry = 0x0,

            .cpu_affinity = 0x8,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x0,
                        .size = 0x8000000
                    }
                },

                .ipc_num = 1,
                .ipcs = (struct ipc[]) {
                    {
                        .base = 0x70000000,
                        .size = 0x00010000,
                        .shmem_id = 0,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {52}
                    }
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) {
                    {   
                        .pa = 0xFF000000,
                        .va = 0xFF000000,
                        .size = 0x10000,
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {53}                         
                    },
                    {   
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0xF9010000,
                        .gicc_addr = 0xF9020000,
                    }
                }
            },
        },
    },
};
