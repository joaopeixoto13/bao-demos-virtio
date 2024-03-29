#include <config.h>

VM_IMAGE(frontend, "/path/to/your/imgs/frontend.bin");
VM_IMAGE(backend, "/path/to/your/imgs/backend.bin");

struct config config = {
    
    CONFIG_HEADER
    
    .shmemlist_size = 2,
    .shmemlist = (struct shmem[]) {
        [0] = { .size = 0x01000000},
        [1] = { .size = 0x01000000},
    },
    
    .vmlist_size = 2,
    .vmlist = {
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
                    }
                },

                .remote_io_dev_num = 2,                             
                .remote_io_interrupt = 40,                             
                .remote_io_pooling = false,                           
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .remote_io_id = 0,
                        .is_back_end = true,
                        .device_type = 1,
                        .shmem = {
                            .base = 0x20000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    },
                    {
                        .remote_io_id = 1,
                        .is_back_end = true,
                        .device_type = 2,
                        .shmem = {
                            .base = 0x21000000,
                            .size = 0x01000000,
                            .shmem_id = 1,
                        }
                    }
                },

                .dev_num = 3,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        // Virtio devices
                        .pa = 0xa003000,                    			          
                        .va = 0xa003000,                    			        
                        .size = 0x1000,                     			            
                        .interrupt_num = 8,                 			        
                        .interrupts = (irqid_t[]) {72,73,74,75,76,77,78,79} 
                    },
                    {
                        // PL011
                        .pa = 0x9001000,                    
                        .va = 0x9001000,                    
                        .size = 0x10000,                    
                        .interrupt_num = 1,                 
                        .interrupts = (irqid_t[]) {42}      // Interrupt ID (32 + 10), where 10 is the VIRT_UART1 in QEMU
                    },
                    {   
                        // Arch timer interrupt
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                // GIC (Generic Interrupt Controller) configuration
                .arch = {
                    .gic = {
                        .gicd_addr = 0x8000000,    
                        .gicr_addr = 0x80A0000,
                    }
                }
            },
        },
        // FRONTEND (Linux)
        { 
            .image = {
                .base_addr = 0x30000000,
                .load_addr = VM_IMAGE_OFFSET(frontend),
                .size = VM_IMAGE_SIZE(frontend)
            },

            .entry = 0x30000000,
            .cpu_affinity = 0x2,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct vm_mem_region[]) {
                    {
                        .base = 0x30000000,
                        .size = 0x40000000,
                    },
                },
                
                .remote_io_dev_num = 2,
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .remote_io_id = 0,
                        .is_back_end = false,
                        .size = 0x200,                              
                        .va = 0xa003e00,                            
                        .device_interrupt = 79,                     
                        .shmem = {
                            .base = 0x20000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    },
                    {
                        .remote_io_id = 1,
                        .is_back_end = false,
                        .size = 0x200,                              
                        .va = 0xa003c00,                            
                        .device_interrupt = 78,                     
                        .shmem = {
                            .base = 0x21000000,
                            .size = 0x01000000,
                            .shmem_id = 1,
                        }
                    },
                },

                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) 
                {
                    {
                        // PL011
                        .pa = 0x9002000,                    
                        .va = 0x9002000,                   
                        .size = 0x10000,                    
                        .interrupt_num = 1,                 
                        .interrupts = (irqid_t[]) {43}      
                    },
                    {   
                        // Arch timer interrupt
                        .interrupt_num = 1,
                        .interrupts = 
                            (irqid_t[]) {27}                         
                    }
                },

                // GIC (Generic Interrupt Controller) configuration
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
