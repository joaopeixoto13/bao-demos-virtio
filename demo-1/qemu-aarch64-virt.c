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
                    },
                },
                
                .remote_io_dev_num = 2,                             
                .remote_io_interrupt = 40,                             
                .remote_io_pooling = false,                           
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .remote_io_id = 0,
                        .is_back_end = true,
                        .device_type = 3,
                        .shmem = {
                            .base = 0x20000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    },
                    {
                        .remote_io_id = 1,
                        .is_back_end = true,
                        .device_type = 3,
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
                        // Virtio devices
                        .pa = 0xa003000,                    			        // Virtio devices physical address   
                        .va = 0xa003000,                    			        // Virtio devices virtual address
                        .size = 0x1000,                     			        // 4 KB
                        .interrupt_num = 8,                 			        // 8 interrupts
                        .interrupts = (irqid_t[]) {72,73,74,75,76,77,78,79} 		// Interrupt IDs
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

                .remote_io_dev_num = 1,                                                       
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .remote_io_id = 0,
                        .is_back_end = false,
                        .size = 0x200,                              // Size of the VirtIO device
                        .va = 0xa003e00,                            // VirtIO device address
                        .device_interrupt = 79,                     // Interrupt ID (32 + 47), where 47 (0x2f) is the virtio device IRQ associated with the VirtIO device
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
                        .size = 0x10000,                    // 64 KB
                        .interrupt_num = 1,                 // 1 interrupt
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

                .remote_io_dev_num = 1,                                                       
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .remote_io_id = 0,
                        .is_back_end = false,
                        .size = 0x200,                              // Size of the VirtIO device
                        .va = 0xa003e00,                            // VirtIO device address
                        .device_interrupt = 79,                     // Interrupt ID (32 + 47), where 47 (0x2f) is the virtio device IRQ associated with the VirtIO device
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
                        // PL011
                        .pa = 0x9002000,                    
                        .va = 0x9002000,                   
                        .size = 0x10000,                    // 64 KB
                        .interrupt_num = 1,                 // 1 interrupt
                        .interrupts = (irqid_t[]) {43}      // Interrupt ID (32 + 11), where 11 is the VIRT_UART2 in QEMU
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
