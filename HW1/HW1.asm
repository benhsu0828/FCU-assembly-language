HW1	START	0
READ	TD	input
		JEQ	READ
		RD	input
		COMP	endmark
		JEQ		A
		COMP exchange
		JGT	WRITE
		COMP	NUM
		JGT	UP
		J	WRITE
UP		ADD UpToLow
		J WRITE
		.
		.
WRITE	TD	output
		JEQ	WRITE
		WD output
		J READ
		.
		.
		.
FIRST	JSUB READ
A		STCH	tmp
		END	FIRST
		.
		.
		.
input	BYTE X'F1'
output	BYTE X'F2'
tmp	RESB	1
tmpin	RESB	1
endmark	WORD 36
exchange	WORD 96
UpToLow	WORD 32
NUM	WORD 58