#include <config.h>

VM_IMAGE(frontend1, "/path/to/your/imgs/frontend1.bin");
VM_IMAGE(frontend2, "/path/to/your/imgs/frontend2.bin");
VM_IMAGE(backend, "/path/to/your/imgs/backend.bin");

struct config config = {
    
    CONFIG_HEADER
    
    .shmemlist_size = 2,
    .shmemlist = (struct shmem[]) {
        [0] = { .size = 0x01000000},
        [1] = { .size = 0x01000000},
    },
    
    .vmlist_size = 3,
    .vmlist = (struct vm_config[]) {
    	// BACKEND (Linux)
        { 
            .image = {
                .base_addr = 0x80000000,
                .load_addr = VM_IMAGE_OFFSET(backend),
                .size = VM_IMAGE_SIZE(backend)
            },

            .entry = 0x80000000,
            .cpu_affinity = 0x1,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x80000000,
                        .size = 0x80000000,
                        .place_phys = true,
                        .phys = 0x80000000,
                    },
                },
                
                .remio_dev_num = 2,                                                                                    
                .remio_devs = (struct remio_dev[]) {
                    {
                        .id = 0,
                        .type = REMIO_DEV_BACKEND,
                        .interrupt = 40,
                        .shmem = {
                            .base = 0x20000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    },
                    {
                        .id = 1,
                        .type = REMIO_DEV_BACKEND,
                        .interrupt = 41,
                        .shmem = {
                            .base = 0x21000000,
                            .size = 0x01000000,
                            .shmem_id = 1,
                        }
                    }
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        .pa = 0xa003000, 
                        .va = 0xa003000, 
                        .size = 0x1000, 
                        .interrupt_num = 8, 
                        .interrupts = (irqid_t[]) {72,73,74,75,76,77,78,79}
                    },
                    {   
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0x8000000,    
                        .gicr_addr = 0x80A0000,
                    }
                }
            },
        },
     	// FRONTEND #1 (Linux)
        { 
            .image = {
                .base_addr = 0x30000000,
                .load_addr = VM_IMAGE_OFFSET(frontend1),
                .size = VM_IMAGE_SIZE(frontend1)
            },

            .entry = 0x30000000,
            .cpu_affinity = 0x2,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x30000000,
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
                            .base = 0x20000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    }
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        // PL011
                        .pa = 0x9001000,                    
                        .va = 0x9001000,                    
                        .size = 0x10000,                    
                        .interrupt_num = 1,                 
                        .interrupts = (irqid_t[]) {42}      
                    },
                    {   
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0x8000000,    
                        .gicr_addr = 0x80A0000,
                    }
                }
            },
        },
        // FRONTEND #2 (Linux)
        { 
            .image = {
                .base_addr = 0x30000000,
                .load_addr = VM_IMAGE_OFFSET(frontend2),
                .size = VM_IMAGE_SIZE(frontend2)
            },

            .entry = 0x30000000,
            .cpu_affinity = 0x4,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x30000000,
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
                            .base = 0x21000000,
                            .size = 0x01000000,
                            .shmem_id = 1,
                        }
                    }
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        .pa = 0x9002000,                    
                        .va = 0x9002000,                   
                        .size = 0x10000,                    
                        .interrupt_num = 1,                
                        .interrupts = (irqid_t[]) {43}      
                    },
                    {
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                .arch = {
                    .gic = {
                        .gicd_addr = 0x8000000,    
                        .gicr_addr = 0x80A0000,
                    }
                }
            },
        },
    },
};