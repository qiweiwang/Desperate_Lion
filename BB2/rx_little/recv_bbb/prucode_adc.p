// prucode_adc.p
//
// BBB Schematic  BBB port Assign Bit
// -------------  -------- ------ ------------
// LCD_DATA0      P8.45    D0     PRU1_R31_0
// LCD_DATA1      P8.46    D1     PRU1_R31_1
// LCD_DATA2      P8.43    D2     PRU1_R31_2
// LCD_DATA3      P8.44    D3     PRU1_R31_3
// LCD_DATA4      P8.41    D4     PRU1_R31_4
// LCD_DATA5      P8.42    D5     PRU1_R31_5
// LCD_DATA6      P8.39    D6     PRU1_R31_6
// LCD_DATA7      P8.40    D7     PRU1_R31_7
// LCD_PCLK       P8.28    CLK    PRU1_R31_10
// LCD_DE         P8.30    *EN    PRU1_R30_11


.origin 0
.entrypoint START

#include "prucode_adc.hp"

#define GPIO1 0x4804c000
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

#define SHARED 0x10000

#define MASK0 0x000000ff

// Memory location of command:
#define CMD 0x00010000

// Memory location where to store the data to be acquired:
#define ACQRAM 0x00010008
#define ACQRAM2 0x00011390
// Length of acquisition:
#define RECORDS 4992

// *** LED routines, so that LED USR0 can be used for some simple debugging
// *** Affects: r2, r3
.macro LED4_OFF
		MOV r2, 1<<24
    MOV r3, GPIO1 | GPIO_CLEARDATAOUT
    SBBO r2, r3, 0, 4
.endm

.macro LED4_ON

		MOV r2, 1<<24
    MOV r3, GPIO1 | GPIO_SETDATAOUT
    SBBO r2, r3, 0, 4
.endm

.macro LED2_ON
      MOV r2, 1<<22
    MOV r3, GPIO1 | GPIO_SETDATAOUT
    SBBO r2, r3, 0, 4
.endm

.macro LED2_OFF
       MOV r2, 1<<22
	MOV r3, GPIO1 | GPIO_CLEARDATAOUT
	SBBO r2,r3,0,4
.endm


.macro LDEL
LDEL:
                MOV r18,2000000000
LDEL1:
                SUB r18, r18, 1
                QBNE LDEL1, r18, 0 // loop if we've not finished
.endm



.macro DEL11
DEL11:
               MOV r13, 5
DEL111:
		SUB r13, r13, 1
                QBNE DEL111, r13, 1
.endm



.macro UDEL
UDEL:
		MOV r13,3
UDEL1:
		SUB r13, r13, 1
		QBNE UDEL1, r13, 0 // loop if we've not finished
.endm

.macro DEL
DEL:
		MOV r13, 49

DEL1:
		SUB r13, r13, 1
		QBNE DEL1, r13, 0 // loop if we've not finished
.endm

.macro DEL0

DEL0:
		MOV r13, 4
DEL01:
		SUB r13, r13, 1
		QBNE DEL01, r13, 0

.endm

.macro DEL2

DEL2:
                MOV r13, 47
DEL21:
                SUB r13, r13, 1
                QBNE DEL21, r13, 0

.endm



.macro  DEL12

DEL12:
                MOV r13, 5
                MOV r13, 5
DEL121:
                SUB r13, r13, 1
                QBNE DEL121, r13, 0

.endm



.macro  DEL16

DEL16:
                MOV r13, 7
                MOV r13, 7
DEL161:
                SUB r13, r13, 1
                QBNE DEL161, r13, 0

.endm

.macro DEL15
DEL15:       	MOV r13, 7


DEL151:		SUB r13, r13, 1
		QBNE DEL151, r13,0
.endm

.macro DEL99
DEL99:
 		MOV r13, 49

DEL991:		SUB r13, r13, 1
                QBNE DEL991, r13, 0
.endm


.macro DEL89
DEL89:
                MOV r13, 4

DEL891:         SUB r13, r13, 1
                QBNE DEL891, r13, 0
.endm

.macro DEL64
DEL64:
                MOV r13, 31
                MOV r13, 31

DEL641:         SUB r13, r13, 1
                QBNE DEL641, r13, 0
.endm

.macro DEL25
DEL25:
                MOV r13, 12

DEL251:         SUB r13, r13, 1
                QBNE DEL251, r13, 0
.endm

.macro DEL10
DEL10:
                MOV r13, 4
                MOV r13,4
DEL101:
                SUB r13, r13, 1
                QBNE DEL101, r13, 0
.endm

START:

    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0100.  This will make C28 point to 0x00010000 (PRU shared RAM).
    MOV     r0, 0x00000100
    MOV       r1, CTPPR_0
    ST32      r0, r1

    // Configure the programmable pointer register for PRU0 by setting c31_pointer[15:0]
    // field to 0x0010.  This will make C31 point to 0x80001000 (DDR memory).
    MOV     r0, 0x00100000
    MOV       r1, CTPPR_1
    ST32      r0, r1

    //Load values from external DDR Memory into Registers R0/R1/R2
    LBCO      r0, CONST_DDR, 0, 12

    //Store values from read from the DDR memory into PRU shared RAM
    //SBCO      r0, CONST_PRUSHAREDRAM, 0, 12


		LED4_ON


START1:
		SET r30.t11	// disable the data bus
		MOV r6, ACQRAM // PRU shared RAM
		MOV r7, RECORDS  // This will be the loop counter to read the entire set of data

		// wait for command

CMDLOOP:


CAPTURE:
		CLR r30.t11	// enable data bus
CAPSECTION:
                LBCO r12, CONST_PRUSHAREDRAM, 0, 4
                QBNE CAPSECTION, r12, 2
                MOV r7, RECORDS
		MOV r6, ACQRAM
                MOV r8, 8
                MOV r9, 0
                MOV r11,0
                MOV r20, 0x54b94e65


HEADERCOUNTERS:
//                MOV r16, 63
//                MOV r17, 0  //1 counter
//                MOV r18, 0  //0 counter
//                MOV r19, 0
//                MOV r4, 1  //total counter to 4
//                //used for reset r7 and address  when one section complete





STARTCHK:


  //              WBC r31.t14
  //              DEL12




READ:
                WBS r31.t16
                MOV r2, r31
                LSR r2, r2, 14
                AND r2, r2, 1
                OR r11, r11, r2
                XOR r12, r11, r20
                LSL r11, r11, 1
                QBNE READ, r12, 0





CAP:

                WBS r31.t16
                MOV r2, r2
                SBBO r31.b1, r6, 0, 1
                ADD r6, r6, 1
                SUB r7, r7, 1
                QBNE CAP, r7, 0
                

ENDHERE:
                MOV r1, 1
                SBCO r1, CONST_PRUSHAREDRAM, 0, 4 // Put contents of r1 into shared RAM //set tag 1
                JMP CHECKREADY2

CHECKREADY2:	LBCO r12, CONST_PRUSHAREDRAM, 4, 4
                QBNE CHECKREADY2, r12, 3
                MOV r7, RECORDS
                MOV r11, 0
               // LED2_ON


HEADERCOUNTERS1:
    //            MOV r16, 63
    //            MOV r17, 0  //1 counter
    //            MOV r18, 0  //0 counter
    //            MOV r19, 0
    //            MOV r4, 1  //total counter to 4



STARTCHK1:



READ1:
                WBS r31.t16
                MOV r2, r31
                LSR r2, r2, 14
                AND r2, r2, 1
                OR r11, r11, r2
                XOR r12, r11, r20
                LSL r11, r11, 1
                QBNE READ1, r12, 0



CAP1:

                WBS r31.t16
                MOV r2, r2
                SBBO r31.b1, r6, 0, 1
                ADD r6, r6, 1
                SUB r7, r7, 1
                QBNE CAP1, r7, 0

ENDHERE1:
                MOV r1, 1
                SBCO r1, CONST_PRUSHAREDRAM, 4, 4 // Put contents of r1 into shared RAM //set tag 1
                JMP CHECKREADY1

CHECKREADY1:     LBCO r12, CONST_PRUSHAREDRAM, 0, 4
                QBNE CHECKREADY1, r12, 3
                MOV r7, RECORDS
                MOV r6, ACQRAM
                JMP HEADERCOUNTERS

EXIT:
    // Send notification to Host for program completion
    MOV       r31.b0, PRU1_ARM_INTERRUPT+16

    // Halt the processor
    HALT

ERR:
//	LED_ON
	JMP ERR
