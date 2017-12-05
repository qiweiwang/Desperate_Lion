// prucode.p

.origin 0
.entrypoint START

#include "prucode.hp"

#define GPIO1 0x4804c000
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194
#define SENDSIZE 4896
#define SENDADD 0x00010008
.macro DEL
DEL:
		MOV r1, 94

DEL1:
		SUB r1, r1, 1
		QBNE DEL1, r1, 0 // loop if we've not finished
.endm


.macro DEL13
DEL13:
                MOV r1, 6
DEL131:
                SUB r1, r1, 1
                QBNE DEL131, r1, 0
.endm



.macro DEL28
DEL28:
                MOV r1, 13
                MOV r1, 13

DEL281:
                SUB r1, r1, 1
                QBNE DEL281, r1, 0 // loop if we've not finished
.endm


.macro  DEL89
DEL89:
                MOV r1, 44


DEL891:
                SUB r1, r1, 1
                QBNE DEL891, r1, 0 // loop if we've not finished
.endm


.macro  DEL97
DEL97:
                MOV r1, 48


DEL971:
                SUB r1, r1, 1
                QBNE DEL971, r1, 0 // loop if we've not finished
.endm


.macro DEL34
DEL34:
                MOV r1, 16
                MOV r1, 16

DEL341:
                SUB r1, r1, 1
                QBNE DEL341, r1, 0 // loop if we've not finished
.endm

.macro DEL95
DEL95:
                MOV r1, 47


DEL951:
                SUB r1, r1, 1
                QBNE DEL951, r1, 0 // loop if we've not finished
.endm



.macro DEL38
DEL38:
                MOV r1, 18
                MOV r1, 18

DEL381:
                SUB r1, r1, 1
                QBNE DEL381, r1, 0 // loop if we've not finished
.endm

.macro DEL99
DEL99:
                MOV r1, 49

DEL991:
                SUB r1, r1, 1
                QBNE DEL991, r1, 0 // loop if we've not finished
.endm


.macro DEL5
DEL5:
		MOV r1, 2
DEL5S:
		SUB r1, r1, 1
		QBNE DEL5S, r1, 0
.endm

.macro DEL9
DEL9:
		MOV r1, 4
DEL9S:
		SUB r1, r1, 1
		QBNE DEL9S, r1, 0
.endm

.macro  DEL51
DEL51:
                MOV r1, 25


DEL511:
                SUB r1, r1, 1
                QBNE DEL511, r1, 0 // loop if we've not finished
.endm

.macro DEL22
DEL22:
                MOV r1, 10
                MOV r1, 10

DEL221:
		SUB r1, r1, 1
                QBNE DEL221, r1, 0
.endm

.macro DEL55
DEL55:
                MOV r1, 27
DEL551:
                SUB r1, r1, 1
                QBNE DEL551, r1, 0
.endm


.macro  DEL990
DEL990:
                MOV r1, 200

DEL9901:
                SUB r1, r1, 1
                QBNE DEL9901, r1, 0 // loop if we've not finished
.endm

.macro DEL1E
DEL1E:
                MOV r1, 48
                MOV r1, 48

DEL11E:
                SUB r1, r1, 1
                QBNE DEL11E, r1, 0 // loop if we've not finished
.endm

.macro DEL0E
DEL0E:
                MOV r1, 46
                MOV r1, 46

DEL01E:
                SUB r1, r1, 1
                QBNE DEL01E, r1, 0 // loop if we've not finished
.endm


.macro STARTDEL
STARTDEL:     MOV r15,20000

SDEL:         SUB r15, r15, 1
              QBNE SDEL, r15, 0
.endm

.macro DEL12
DEL12:
    		MOV r1, 5
		MOV r1, 5
DEL121:
		SUB r1, r1, 1
		QBNE DEL121, r1, 0
.endm

.macro DEL10
DEL10:		MOV r1, 4
		MOV r1, 4
DEL101:
		SUB r1, r1, 1
		QBNE DEL101, r1,0
.endm
.macro DEL25
DEL25:
		MOV r1, 12
DEL251:
		SUB r1, r1, 1
                QBNE DEL251, r1, 0
.endm


.macro DEL24
DEL24:
		MOV r1, 11
 		MOV r1, 11
DEL241:
		SUB r1, r1, 1
		QBNE DEL241, r1, 0
.endm

.macro DEL15
DEL15:
		MOV r1, 7
DEL151:		SUB r1, r1, 1
		QBNE DEL151, r1, 0
.endm


.macro DEL19
DEL19:
		MOV r1, 9

DEL191:
		SUB r1, r1, 1
		QBNE DEL191, r1, 0

.endm

.macro DEL90
DEL90:
		MOV r1, 44
		MOV r1, 44
DEL901:
		SUB r1, r1, 1
		QBNE DEL901, r1, 0
.endm

.macro DEL100
DEL100:
                MOV r1, 49
                MOV r1, 49
DEL1001:
                SUB r1, r1, 1
                QBNE DEL1001, r1, 0
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


START:
    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0120.  This will make C28 point to 0x00012000 (PRU shared RAM).
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
   // SBCO      r0, CONST_PRUSHAREDRAM, 0, 12

CORRECT:
   // SET r30.t14

//    STARTDEL

    MOV r7, SENDSIZE
    MOV r6, SENDADD
    MOV r17, 0 // head byte counter
    MOV r18, 0 // head bit counter
    MOV r19, 100
    MOV r20, 0x000113a8
    CLR r30.t15

//////////////////////////////////////////////////////
SENDBLANK:
    DEL5
    CLR r30.t14
    DEL9
    SET r30.t14
    LBCO r3, CONST_PRUSHAREDRAM, 0, 4
    QBNE SENDBLANK, r3, 2
   // DEL100
   // DEL100
///////////////////////////////////////////////////////

    MOV r20, r20
    MOV r20, r20
//LED2_OFF

   // LED2_OFF

SENDLOOP:

//    SET r30.t15
    LBBO r30.b1, r6, 0, 1         //read in the data at address r6
    SET r30.t15
    MOV r20, r20
    MOV r20, r20
    MOV r20, r20
    CLR r30.t15
//    DEL10


    ADD r6, r6, 1
    QBNE SENDLOOP, r6, r20

    MOV r1, 1
    SBCO r1, CONST_PRUSHAREDRAM, 0, 4





    MOV r7, SENDSIZE
    MOV r17, 0 // head byte counter
    MOV r18, 0 // head bit counter
    MOV r19, 100
    MOV r20, 0x12748

//////////////////////////////////////////////////////
SENDBLANK2:
    DEL5
    CLR r30.t14
    DEL9
    SET r30.t14
    LBCO r3, CONST_PRUSHAREDRAM, 4, 4
    QBNE SENDBLANK2, r3, 2
///////////////////////////////////////////////////////

   MOV r20, r20
   MOV r20, r20

SENDLOOP2:
 //   SET r30.t15
    LBBO r30.b1, r6, 0, 1         //read in the data at address r6
    SET r30.t15
    MOV r20,r20
    MOV r20,r20
    MOV r20,r20
    CLR r30.t15
 //   DEL10
    ADD r6, r6, 1
    QBNE SENDLOOP2, r6, r20


    MOV r1, 1
    SBCO r1, CONST_PRUSHAREDRAM, 4, 4

    JMP CORRECT
    SBBO r2, r4, 0, 4

    // Send notification to Host for program completion
    MOV       r31.b0, PRU0_ARM_INTERRUPT+16

    // Halt the processor
    HALT
