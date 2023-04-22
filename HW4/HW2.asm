HW2		START	0
PRINT	COMP #10
		JLT	L1
		DIV	#10
		STA	temp2
G1		TD	outdev
		JEQ G1
		ADD	#48
		WD	outdev
		LDA temp2
		MUL	#10
		STA	temp2
		LDA	temp
		SUB	temp2
G2		TD outdev
		JEQ G2
		ADD #48
		WD	outdev
		J	OUTLP3
L1		TD outdev
		JEQ	L1
		LDCH	SPACE
		WD outdev
L2		TD	outdev
		JEQ	L2
		LDA temp
		ADD	#48
		WD	outdev
OUTLP3	TD outdev
		JEQ	OUTLP3
		LDCH	SPACE
		WD outdev
		RSUB
		.
		.
		.
FIRST	LDX	#1
		LDT #10
		LDS #29
		STX X1
		STX	X2
		LDX #0
OUTLP1	TD outdev
		JEQ	OUTLP1
		LDCH	line1,X
		WD outdev
		TIXR	S
		JLT	OUTLP1	做29次
LOOP1	TD outdev
		JEQ	LOOP1
		LDA	Nline
		WD outdev
		LDA X1
		STA temp
		JSUB	PRINT
		LDX #1
		STX	X2
LOOP2	LDA	X2
		MUL X1
		STA	temp
		JSUB	PRINT
		TIXR	T
		STX	X2
		JLT	LOOP2
		LDX	X1
		TIXR	T
		STX X1
		JLT	LOOP1
		END	FIRST
		.
		.
		.
		.
X1	RESW 	1
X2	RESW	1
line1	BYTE	C'    1  2  3  4  5  6  7  8  9'
Nline	WORD	10
SPACE	BYTE	C' '
outdev	BYTE	X'F2'
temp	RESW	1
temp2	RESW	1	