# makefile configuration

NAME     = pwm 

OBJECTS  = uartComm.o

CPU      = msp430x2274
RF       = -DMRFI_CC2500
BOARD    = EZ430RF
CFLAGS   = ${RF} ${SMPL_NWK_CONFIG} -mmcu=${CPU} -O2 -Wall -g ${INCLUDES}

#switch the compiler (for the internal make rules)
CC       = msp430-gcc

.PHONY: all FORCE clean download-jtag dist

#all should be the first target. it's built when make is run without args
all: $(NAME).elf $(NAME).hex

%.o:	%.c ../common.h
	$(CC)  ${CFLAGS} -c -o $@ $<

#additional rules for files

$(NAME).elf: $(NAME).o $(OBJECTS)
	${CC} ${CFLAGS} -o $@ $^
	@echo
	@echo ">>>> Size of Firmware <<<<"
	@msp430-size $@
	@echo

%.hex: %.elf
	msp430-objcopy -O ihex $^ $@

%.lst: %.elf
	msp430-objdump -dSt $^ >$@

%.dmp: %.elf
	msp430-objdump -d $^ >$@

%.pp.c: %.c
	${CC} -E $(CFLAGS) $^ >$@

%.s: %.c
	${CC} -S $(CFLAGS) $^ >$@

download-jtag: all
	msp430-jtag -e ${NAME}.elf

distclean: clean
	rm -f *.elf *.hex *.lst

clean:
	rm -f *.o out.eps *~ *.hex *.lst *.elf ${NAME}

#backup archive
dist:
	tar czf dist.tgz *.c *.h *.txt makefile

#print all sources
print:
	enscript	-2 -G -r --toc --pretty-print=c --word-wrap\
			--mark-wrapped-lines=arrow\
			-o out.eps\
			*.h *.c

#project dependencies
%.dep:	%.c 
	@$(CC)  ${CFLAGS} -MM -MF  $@ $<
	@echo computing depencies for $<


#dummy target as dependecy if something has to be build everytime
FORCE:


uartComm.o: uartComm.c uartComm.h ../common.h ${BSP_PATH}/bsp.h
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

-include ${DEPS}
