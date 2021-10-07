EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Switch:SW_SPST K3
U 1 1 5FEE1EBE
P 4600 2700
F 0 "K3" H 4600 2843 50  0000 C CNN
F 1 "SW_SPST" H 4600 2844 50  0001 C CNN
F 2 "" H 4600 2700 50  0001 C CNN
F 3 "~" H 4600 2700 50  0001 C CNN
	1    4600 2700
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_SPST K4
U 1 1 5FEE21AA
P 5800 2700
F 0 "K4" H 5800 2843 50  0000 C CNN
F 1 "SW_SPST" H 5800 2844 50  0001 C CNN
F 2 "" H 5800 2700 50  0001 C CNN
F 3 "~" H 5800 2700 50  0001 C CNN
	1    5800 2700
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_SPST K1
U 1 1 5FEE2600
P 4950 3300
F 0 "K1" H 4950 3443 50  0000 C CNN
F 1 "SW_SPST" H 4950 3444 50  0001 C CNN
F 2 "" H 4950 3300 50  0001 C CNN
F 3 "~" H 4950 3300 50  0001 C CNN
	1    4950 3300
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPST K2
U 1 1 5FEE2A9E
P 4950 3700
F 0 "K2" H 4950 3843 50  0000 C CNN
F 1 "SW_SPST" H 4950 3844 50  0001 C CNN
F 2 "" H 4950 3700 50  0001 C CNN
F 3 "~" H 4950 3700 50  0001 C CNN
	1    4950 3700
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPST K5
U 1 1 5FEE2E52
P 6300 2700
F 0 "K5" H 6300 2843 50  0000 C CNN
F 1 "SW_SPST" H 6300 2844 50  0001 C CNN
F 2 "" H 6300 2700 50  0001 C CNN
F 3 "~" H 6300 2700 50  0001 C CNN
	1    6300 2700
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4200 3300 4750 3300
Wire Wire Line
	4200 3700 4750 3700
Wire Wire Line
	4200 3500 4600 3500
Wire Wire Line
	4600 3500 4600 2900
Wire Wire Line
	5150 3300 5250 3300
Wire Wire Line
	4600 2500 4600 2400
Wire Wire Line
	4600 2400 5050 2400
$Comp
L power:LINE #PWR?
U 1 1 5FE7DFC7
P 5050 2300
F 0 "#PWR?" H 5050 2150 50  0001 C CNN
F 1 "LINE" H 5067 2473 50  0000 C CNN
F 2 "" H 5050 2300 50  0001 C CNN
F 3 "" H 5050 2300 50  0001 C CNN
	1    5050 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 2300 5050 2400
Connection ~ 5050 2400
$Comp
L Motor+:Induction_Motor Motor
U 1 1 5FE7EC80
P 5200 4400
F 0 "Motor" H 5200 4603 50  0000 C CNN
F 1 "Induction_Motor" H 5200 4604 50  0001 C CNN
F 2 "" H 5210 4390 50  0001 C CNN
F 3 "" H 5210 4390 50  0001 C CNN
	1    5200 4400
	1    0    0    -1  
$EndComp
Connection ~ 4600 3500
$Comp
L power:NEUT #PWR?
U 1 1 5FE7FE80
P 5200 4850
F 0 "#PWR?" H 5200 4700 50  0001 C CNN
F 1 "NEUT" H 5217 5023 50  0000 C CNN
F 2 "" H 5200 4850 50  0001 C CNN
F 3 "" H 5200 4850 50  0001 C CNN
	1    5200 4850
	1    0    0    1   
$EndComp
Wire Wire Line
	5200 4650 5200 4850
Wire Wire Line
	6300 2400 6300 2500
$Comp
L Device:Lamp LA?
U 1 1 5FE84406
P 6300 4300
F 0 "LA?" H 6428 4346 50  0001 L CNN
F 1 "Lamp" H 6428 4300 50  0000 L CNN
F 2 "" V 6300 4400 50  0001 C CNN
F 3 "~" V 6300 4400 50  0001 C CNN
	1    6300 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6300 2900 6300 4100
$Comp
L power:NEUT #PWR?
U 1 1 5FE86AEA
P 6300 4850
F 0 "#PWR?" H 6300 4700 50  0001 C CNN
F 1 "NEUT" H 6317 5023 50  0000 C CNN
F 2 "" H 6300 4850 50  0001 C CNN
F 3 "" H 6300 4850 50  0001 C CNN
	1    6300 4850
	1    0    0    1   
$EndComp
Text GLabel 4400 3300 1    50   Input ~ 0
B
Text GLabel 4400 3500 1    50   Input ~ 0
R
Text GLabel 4400 3700 1    50   Input ~ 0
G
Wire Wire Line
	5150 3700 5250 3700
Wire Wire Line
	6300 4500 6300 4850
Text Label 4600 4000 0    50   ~ 0
M1
Text Label 5800 4000 0    50   ~ 0
M2
Text Label 6300 4000 0    50   ~ 0
LMP
Wire Wire Line
	5500 4300 5800 4300
Wire Wire Line
	4900 4300 4600 4300
Wire Wire Line
	4600 3500 4600 4300
$Comp
L Device+:Dual_Capacitor C?
U 1 1 5FE7B06F
P 4200 3500
F 0 "C?" H 4367 3833 50  0000 C CNN
F 1 "Ceiling_Fan_Capacitor_3_wire" H 4575 3875 50  0001 C CNN
F 2 "local:Capacitor_3_wire" H 4200 3500 50  0001 C CNN
F 3 "" H 4200 3500 50  0001 C CNN
	1    4200 3500
	-1   0    0    -1  
$EndComp
Wire Notes Line
	3500 2300 6650 2300
Wire Notes Line
	6650 2300 6650 3850
Wire Notes Line
	3500 3850 3500 2300
Wire Wire Line
	5250 3300 5250 3500
Wire Wire Line
	5250 3500 5300 3500
Connection ~ 5250 3500
Wire Wire Line
	5250 3500 5250 3700
Connection ~ 5800 3500
Wire Wire Line
	5800 3500 5800 4300
Wire Wire Line
	5800 2500 5800 2400
Connection ~ 5800 2400
Wire Wire Line
	5050 2400 5250 2400
$Comp
L Jumper:Jumper_2_Bridged JP1
U 1 1 5FEABA01
P 5500 3500
F 0 "JP1" H 5500 3695 50  0000 C CNN
F 1 "Fan" H 5500 3604 50  0000 C CNN
F 2 "" H 5500 3500 50  0001 C CNN
F 3 "~" H 5500 3500 50  0001 C CNN
	1    5500 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 3500 5800 3500
$Comp
L Jumper:Jumper_2_Open JP2
U 1 1 5FEACB39
P 5250 2850
F 0 "JP2" V 5200 2950 50  0000 L CNN
F 1 "Gen" V 5295 2948 50  0000 L CNN
F 2 "" H 5250 2850 50  0001 C CNN
F 3 "~" H 5250 2850 50  0001 C CNN
	1    5250 2850
	0    1    -1   0   
$EndComp
Wire Wire Line
	5250 2650 5250 2400
Connection ~ 5250 2400
Wire Wire Line
	5250 2400 5800 2400
Wire Wire Line
	5250 3300 5250 3050
Connection ~ 5250 3300
Wire Wire Line
	5800 2400 6300 2400
Wire Wire Line
	5800 2900 5800 3500
Wire Notes Line
	3500 3850 6650 3850
$EndSCHEMATC