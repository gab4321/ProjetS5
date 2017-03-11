	.def _enableInterruptGPIO4
	.def _enableInterruptTIMER1
	.def _disableInterruptTIMER1
	.def _enableInterruptDAC
	.def _disableInterruptDAC

	.text

_enableInterruptTIMER1
	.asmfunc

	MVKL 0x00000001, B5 ;mask pour mettre GIE a 1
	MVKH 0x00000001, B5
	MVC CSR, B8
	OR B5, B8, B8
	MVC B8, CSR ; enable global interrupt

	MVKL 0x00008002, B6 ;mask pour mettre NMIE a 1 /mettre IE15 a 1
	MVKH 0x00008002, B6
	MVC IER, B9
	OR B6, B9, B9
	MVC B9, IER ; enable  interrupt pour timer 1

	B B3
	NOP 5

	.endasmfunc

_disableInterruptTIMER1
	.asmfunc

	MVKL 0xFFFF7FFF, B6 ;mask pour mettre NMIE a 1 /mettre IE15 a 1
	MVKH 0xFFFF7FFF, B6
	MVC IER, B9
	AND B6, B9, B9
	MVC B9, IER ; enable  interrupt pour timer 1

	B B3
	NOP 5

	.endasmfunc

_enableInterruptGPIO4
	.asmfunc

	MVKL 0x00000001, B5 ;mask pour mettre GIE a 1
	MVKH 0x00000001, B5
	MVC CSR, B8
	OR B5, B8, B8
	MVC B8, CSR ; enable global interrupt

	MVKL 0x00000012, B6 ;mask pour mettre NMIE a 1 /mettre IE4 a 1
	MVKH 0x00000012, B6
	MVC IER, B9
	OR B6, B9, B9
	MVC B9, IER ; enable  interrupt pour GPIO 4

	B B3
	NOP 5

	.endasmfunc

_enableInterruptDAC
	.asmfunc

	MVKL 0x00000001, B5 ;mask pour mettre GIE a 1
	MVKH 0x00000001, B5
	MVC CSR, B8
	OR B5, B8, B8
	MVC B8, CSR ; enable global interrupt

	MVKL 0x00000802, B6 ;mask pour mettre NMIE a 1 /mettre IE4 a 1
	MVKH 0x00000802, B6
	MVC IER, B9
	OR B6, B9, B9
	MVC B9, IER ; enable  interrupt pour GPIO 4

	B B3
	NOP 5

	.endasmfunc

_disableInterruptDAC
	.asmfunc

	MVKL 0xFFFFF7FF, B6 ;mask pour mettre NMIE a 1 /mettre IE4 a 1
	MVKH 0xFFFFF7FF, B6
	MVC IER, B9
	AND B6, B9, B9
	MVC B9, IER ; enable  interrupt pour GPIO 4

	B B3
	NOP 5

	.endasmfunc
