	.ORIG x1200;
	ADD R6, R6, #-2;
	STW R1, R6, #0;
	ADD R6, R6, #-2;
	STW R2, R6, #0;
	
	LEA R1, storage;
	LDW R1, R1, #0;
	ADD R1, R1, #1;
	LEA R2, storage;
	STW R1, R2, #0;
	
	LDW R2, R6, #0;
	ADD R6, R6, #2;
	LDW R1, R6, #0;
	ADD R6, R6, #2;
	
storage	.FILL x4000;
	.END
