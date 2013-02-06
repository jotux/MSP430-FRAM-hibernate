# What is this?

This repository is a proof-of-concept showing the hibernation of an MSP430FR**** device by storing the SRAM in unused FRAM memory and then restoring the SRAM on power-up.

# How do I run this demonstration?

You will need:

 * [MSP-EXP430FR5739](http://www.ti.com/tool/msp-exp430fr5739) experimeter board
 * [LaunchLib](https://github.com/jotux/LaunchLib) -- though it's not required, you could port the code to library-less C code
 * IAR MSP430 compiler -- this can be ported to other compilers but the var @ address syntax is IAR specific

To run the demo:

 1. Run the code and notice the LEDs turn on in order then wrap around and turn off in order
 2. While the program is running press SW1 -- this will interrupt the program while running and store the state in non-volatile FRAM
 3. Unplug the experimenter board
 4. Plug the experimenter board back into power
 5. Notice the program continues running at the point you originally pressed SW1
 6. Bask in the warmth that pours over you
 7. Wonder if this useful...probably not

# How does it work?

We attach Hibernate() to the falling edge of SW1 so it can interrupt the while loop that toggles the LEDs:

    InterruptAttach(SW1_PORT,SW1_PIN,Hibernate,FALLING_EDGE);

Hibernate() is pretty simple, memcpy all of the SRAM to a specific location in FRAM and memcpy *some* of the SFR to a specific location in FRAM. After the copy is complete set a flag in FRAM to indicate the deed is done and loop forever:

    void Hibernate(void)
    {
        // save sram
        memcpy((uint16_t*)RAM_BACKUP_LOC,(uint16_t*)RAM_LOC,RAM_SIZE);
        // save sfr
        memcpy((uint16_t*)SFR_BACKUP_LOC,(uint16_t*)SFR_LOC,SFR_SIZE);
        f_asleep = TRUE;
        while(1);
    }

Once power is restored we do the normal init at the beginning but then check to see if the flag is set, if it is we restore SRAM and SFR from its FRAM backup location:

    void Restore(void)
    {
        f_asleep = FALSE;
        // restore sram
        f_to = (uint8_t*)RAM_LOC;
        f_from = (uint8_t*)RAM_BACKUP_LOC;
        f_mem_cnt = RAM_SIZE;
        while(--f_mem_cnt != 0)
        {
            *f_to++ = *f_from++;
        }

Restoring from FRAM to SRAM is a little tricky and you may be wondering why I have the f_to, f_from, and f_mem_cnt variables. I'm recreating the memcpy function but using a few variables in FRAM to replace the temporary variables that would have been created on the stack. We don't want to push variables onto the stack because we're going to overwrite the stack with the values that were present during the Hibernate() call.

        // restore SFR
        f_to = (uint8_t*)SFR_LOC;
        f_from = (uint8_t*)SFR_BACKUP_LOC;
        f_mem_cnt = SFR_SIZE;
        while(--f_mem_cnt != 0)
        {
            *f_to++ = *f_from++;
        }

At this point you may also be wondering what the heck the SFR is, why we're restoring it, and why I picked the specific SFR location and size. The SFR are the Special-Function-Registers. They are a special part of memory that hold configurations for all of the user-accessible hardware on the device (if you've every configured hardware with a statement like P1DIR |= 0x01; you're writing to a register in the SFR). The specific range I'm restoring is the location of the PJ DIR and OUT registers and this happens to be the locations of the eight LEDs on the experimenter board. We restore the SFR to get the state of the IO back after we restore the SRAM.

    }

So now we're at the end of Restore(). What happens? Well, now your stack is not the same as it was at the beginning of the Restore() call since we've overwritten it with the contents of FRAM. So when we get to the end of the function we're going to pop the address that was on the stack when Hibernate() was called and return to that location. Since Hibernate() was called from an interrupt that location could be *anywhere* within the while(1) in main(). Not only that, when it returns the stack will be the same as it was before the interrupt, and since we restored the SFR for port J the IO will be configured in the same way. Neat.

# Why?

I thought it would be interesting and possibly useful for some odd applications.

# Questions you may have

### What's with the weird variable @ location syntax?

This is a proprietary syntax for allocations of variables at a location. It only works in the IAR compiler but the capability can be duplicated in other compilers by fiddling with your linker configuration.

### Why not copy all of the SFR?

The SFR contains **ALL** configuration for hardware. Clocks, interrupts, watchdogs, etc. A lot of stuff requires setting registers in a specific order and waiting a specific time for them to settle out. If you just overwrite the whole SFR you'll cause a bunch of unfortunate behavior.

### Is this useful?

Probably not, but it's interesting.

### Must I have FRAM to do this?

No, but FRAM makes it easy. You could probably do this on a FLASH micro if you fiddle with the device configuration and keep it from locking you out of the unused memory space but you'd have to deal with the page-erase and probably can't address single bytes.








