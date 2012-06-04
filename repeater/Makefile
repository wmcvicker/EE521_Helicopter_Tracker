# makefile configuration

SIMPLICITI_COMPONENTS_PATH = ../
BSP_PATH     = ${SIMPLICITI_COMPONENTS_PATH}/bsp
NWK_APP_PATH = ${SIMPLICITI_COMPONENTS_PATH}/simpliciti/nwk_applications
NWK_SIM_PATH = ${SIMPLICITI_COMPONENTS_PATH}/simpliciti/nwk
MRFI_PATH    = ${SIMPLICITI_COMPONENTS_PATH}/mrfi

NAME     = repeater

OBJECTS  = mrfi.o bsp.o nwk_api.o            			\
	   nwk.o nwk_join.o nwk_frame.o nwk_link.o nwk_ioctl.o 	\
	   nwk_globals.o nwk_freq.o nwk_ping.o nwk_mgmt.o      	\
	   nwk_security.o nwk_QMgmt.o uartComm.o rfComm.o

CPU      = msp430x2274
INCLUDES = -I${MRFI_PATH}/         		\
	   -I${BSP_PATH}/                  	\
	   -I${BSP_PATH}/drivers/          	\
	   -I${BSP_PATH}/boards/${BOARD}/  	\
	   -I${NWK_SIM_PATH}/       	   	\
	   -I${NWK_APP_PATH}/
RF       = -DMRFI_CC2500
BOARD    = EZ430RF

#configuration de l'application
SMPL_NWK_CONFIG  += -DMAX_APP_PAYLOAD=50
SMPL_NWK_CONFIG  += -DMAX_NWK_PAYLOAD=50
SMPL_NWK_CONFIG  += -DNUM_CONNECTIONS=2
SMPL_NWK_CONFIG  += -DMAX_HOPS=3
SMPL_NWK_CONFIG  += -DEND_DEVICE
SMPL_NWK_CONFIG  += -DDEFAULT_JOIN_TOKEN=0x05060708
SMPL_NWK_CONFIG  += -DDEFAULT_LINK_TOKEN=0x01020304
SMPL_NWK_CONFIG  += -DMAX_HOPS_FROM_AP=1
SMPL_NWK_CONFIG  += -DSIZE_INFRAME_Q=2
SMPL_NWK_CONFIG  += -DSIZE_OUTFRAME_Q=2
SMPL_NWK_CONFIG  += -DTHIS_DEVICE_ADDRESS="{0x79, 0x56, 0x34, 0x13}" 
SMPL_NWK_CONFIG  += -DAPP_AUTO_ACK
SMPL_NWK_CONFIG  += -DEXTENDED_API

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

mrfi.o:	${MRFI_PATH}/mrfi.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

bsp.o: ${BSP_PATH}/bsp.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

nwk_api.o: ${NWK_SIM_PATH}/nwk_api.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

nwk.o: ${NWK_SIM_PATH}/nwk.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

nwk_join.o: ${NWK_APP_PATH}/nwk_join.c
	${CC} ${CFLAGS} -c ${NWK_APP_PATH}/nwk_join.c -o $@
	@echo CC $<

nwk_frame.o: ${NWK_SIM_PATH}/nwk_frame.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

nwk_link.o: ${NWK_APP_PATH}/nwk_link.c
	${CC} ${CFLAGS} -c ${NWK_APP_PATH}/nwk_link.c -o $@
	@echo CC $<

nwk_ioctl.o: ${NWK_APP_PATH}/nwk_ioctl.c
	${CC} ${CFLAGS} -c ${NWK_APP_PATH}/nwk_ioctl.c -o $@
	@echo CC $<

nwk_globals.o: ${NWK_SIM_PATH}/nwk_globals.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

nwk_freq.o: ${NWK_APP_PATH}/nwk_freq.c
	${CC} ${CFLAGS} -c ${NWK_APP_PATH}/nwk_freq.c -o $@
	@echo CC $<

nwk_ping.o: ${NWK_APP_PATH}/nwk_ping.c
	${CC} ${CFLAGS} -c ${NWK_APP_PATH}/nwk_ping.c -o $@
	@echo CC $<

nwk_mgmt.o: ${NWK_APP_PATH}/nwk_mgmt.c
	${CC} ${CFLAGS} -c ${NWK_APP_PATH}/nwk_mgmt.c -o $@
	@echo CC $<

nwk_security.o: ${NWK_APP_PATH}/nwk_security.c
	${CC} ${CFLAGS} -c ${NWK_APP_PATH}/nwk_security.c -o $@
	@echo CC $<

nwk_QMgmt.o: ${NWK_SIM_PATH}/nwk_QMgmt.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

uartComm.o: uartComm.c uartComm.h ../common.h ${BSP_PATH}/bsp.h
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

rfComm.o: rfComm.c rfComm.h ../common.h ${BSP_PATH}/bsp.h
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

-include ${DEPS}
