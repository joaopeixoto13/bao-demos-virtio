#include <config.h>

// Frontend binary
VM_IMAGE(frontend, "/path/to/your/imgs/frontend.bin")
// Backend binary
VM_IMAGE(backend, "/path/to/your/imgs/backend.bin")

struct config config = {
    
    CONFIG_HEADER
    
    .shmemlist_size = 1,
    .shmemlist = (struct shmem[]) {
        [0] = { .size = 0x01000000},
    },
    
    .vmlist_size = 2,
    .vmlist = (struct vm_config[]) {
        { 
            .image = {
                .base_addr = 0x90200000,
                .load_addr = VM_IMAGE_OFFSET(backend),
                .size = VM_IMAGE_SIZE(backend)
            },

            .entry = 0x90200000,
            .cpu_affinity = 0x1,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x90000000,
                        .size = 0x50000000,
                        .place_phys = true,
                        .phys = 0x90000000,
                    }
                },

                .remio_dev_num = 1,                                                      
                .remio_devs = (struct remio_dev[]) {
                    {
                        .id = 0,
                        .type = REMIO_DEV_BACKEND,
                        .interrupt = 36,                           
                        .shmem = {
                            .base = 0x70000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    }
                },

                .dev_num = 1,
                .devs =  (struct vm_dev_region[]) {
                    {
                        .pa = 0x10008000,                    			         
                        .va = 0x10008000,                    			        
                        .size = 0x1000,                                                     			            
                        .interrupt_num = 1,                 			        
                        .interrupts = (irqid_t[]) {8} 	                   
                    },
                },

                .arch = {
                    .irqc.plic = {
                        .base = 0xc000000,
                    }
                }
            },
        },
        {
            .image = {
                .base_addr = 0x20200000,
                .load_addr = VM_IMAGE_OFFSET(frontend),
                .size = VM_IMAGE_SIZE(frontend)
            },

            .entry = 0x20200000,
            .cpu_affinity = 0x2,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x20000000,
                        .size = 0x50000000,
                    },
                },
                
                .remio_dev_num = 1,
                .remio_devs = (struct remio_dev[]) {
                    {
                        .id = 0,
                        .type = REMIO_DEV_FRONTEND,
                        .size = 0x200,                                  
                        .va = 0x10001000,                            
                        .interrupt = 37,                             
                        .shmem = {
                            .base = 0x70000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    }
                },

                .dev_num = 1,
                .devs =  (struct vm_dev_region[]) {
                    {
                        .pa = 0x10000000,                  			         
                        .va = 0x10000000,                			        
                        .size = 0x100,                                                         			            
                        .interrupt_num = 1,                 			        
                        .interrupts = (irqid_t[]) {10} 	                       
                    },
                },

                .arch = {
                    .irqc.plic = {
                        .base = 0xc000000,
                    }
                }
            },
        },
    },
};