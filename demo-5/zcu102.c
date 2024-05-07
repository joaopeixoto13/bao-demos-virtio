#include <config.h>

// Frontend binary
VM_IMAGE(frontend, "PATH/TO/YOUR/frontend.bin");
// Backend binary
VM_IMAGE(backend, "PATH/TO/YOUR/backend.bin");

struct config config = {
    CONFIG_HEADER
    
    .shmemlist_size = 1,
    .shmemlist = (struct shmem[]) {
        [0] = { .size = 0x00010000, },
    },

    .vmlist_size = 2,
    .vmlist = {
    	// Backend
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
                        .size = 0x80000000,
                    },
                },
                
                .remote_io_dev_num = 1,                                                              
                .remote_io_pooling = false,                            
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .id = 0,
                        .is_backend = true,
                        .interrupt = 40,
                        .shmem = {
                            .base = 0x80000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    }
                },

                // Devices configuration
                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        // UART0
                        .pa = 0xFF000000,
                        .va = 0xFF000000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {54}              
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
     	// Frontend
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
                        .size = 0x40000000,
                    },
                },

                .remote_io_dev_num = 1,
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .id = 0,
                        .is_backend = false,
                        .size = 0x200,                                  
                        .va = 0xa003e00,                                
                        .interrupt = 79,                         
                        .shmem = {
                            .base = 0x80000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    }
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        // UART1
                        .pa = 0xFF010000,
                        .va = 0xFF010000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {54}
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
        }
    },
};
