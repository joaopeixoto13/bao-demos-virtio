#include <config.h>

// Frontend binary
VM_IMAGE(frontend, "PATH/TO/YOUR/frontend.bin");
// Backend binary
VM_IMAGE(backend, "PATH/TO/YOUR/backend.bin");

struct config config = {
    
    CONFIG_HEADER
    
    // Shared memory configuration
    .shmemlist_size = 1,
    .shmemlist = (struct shmem[]) {
        [0] = { .size = 0x01000000}
    },
    
    .vmlist_size = 2,
    .vmlist = {
    	// Backend (Linux)
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
                    }
                },
                
                // VirtIO configuration
                .remote_io_dev_num = 1,                             	// Number of VirtIO devices
                .remote_io_interrupt = 40,                             	// Interrupt ID (32 + 8), where 8 is the bao-virtio-intc interrupt
                .remote_io_pooling = false,                            	// Interrupt mode
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        // Virtio device
                        .remote_io_id = 0,
                        .is_back_end = true,
                        .device_type = 26,
                        .shmem = {
                            .base = 0x10000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    }
                },

                // Devices configuration
                .dev_num = 3,
                .devs =  (struct vm_dev_region[]) {
                    {
                        // PL011 (UART1)
                        .pa = 0x9001000,
                        .va = 0x9001000,
                        .size = 0x10000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {42}              			// Interrupt ID (32 + 10), where 10 is the VIRT_UART1 IRQ on QEMU
                    },
                    {
                        // Virtio devices
                        .pa = 0xa003000,                    			        // Virtio devices physical address   
                        .va = 0xa003000,                    			        // Virtio devices virtual address
                        .size = 0x1000,                     			            
                        .interrupt_num = 8,                 			        
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
     	// Frontend (Linux)
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
                
                // VirtIO configuration
                .remote_io_dev_num = 1,
                .remote_io_devs = (struct remote_io_dev[]) {
                    {
                        .remote_io_id = 0,
                        .is_back_end = false,
                        .size = 0x200,                              // Size of the VirtIO device
                        .va = 0xa003e00,                            // VirtIO device address
                        .device_interrupt = 79,                     // Interrupt ID (32 + 47), where 47 (0x2f) is the virtio device IRQ associated with the VirtIO device
                        .shmem = {
                            .base = 0x10000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    }
                },

                // Devices configuration
                .dev_num = 2,
                .devs =  (struct vm_dev_region[]) {
                    {
                        // PL011 (UART2)
                        .pa = 0x9002000,
                        .va = 0x9002000,
                        .size = 0x10000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {43}              // Interrupt ID (32 + 11), where 11 is the VIRT_UART2 IRQ on QEMU
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
