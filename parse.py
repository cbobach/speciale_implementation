#!/usr/bin/python3.4

import sys, getopt

def parseFile(ifile):
	inWires = 0
	outWires = 0
	numWires = 0
	nonFreeGates = 0
	numGates = 0


	zeroConstantOutWire = 0
	oneConstantOutWire = 0

	gates = []

	#Reading gates from input file
	with open(ifile) as f:
		for line in f:
			line = line.rstrip()
			line = line.split(' ')

			gateType = line[0]
			output = line[1]
			input1 = line[2]
			input2 = 'MISSING INPUT WIRE'

			if len(line) == 4: 
				input2 = line[3]
			
			gate = []
			gate.append(input1)
			gate.append(input2)
			gate.append(output)
			
			
			if int(output) > numWires:
				numWires = int(output)
			

			if gateType == "IN":
				inWires += 1

			elif gateType == 'OUT':
				outWires += 1

			# 0 CONSTANT using XOR on same input
			elif gateType == '0':
				zeroConstantOutWire = output
				gate.append('0110')
				gates.append(gate)
				numGates += 1

			elif gateType == '1':
				gate.append('0001')
				gates.append(gate)
				nonFreeGates += 1
				numGates += 1

			elif gateType == '2':
				gate.append('0010')
				gates.append(gate)
				nonFrreGates += 1
				numGates += 1

			elif gateType == '3':
				gate.append('0011')
				gates.append(gate)
				numGates += 1

			elif gateType == '4':
				gate.append('0100')
				gates.append(gate)
				nonFreeGates += 1
				numGates += 1

			elif gateType == '5':
				gate.append('0101')
				gates.append(gate)
				numGates += 1

			elif gateType == "6":
				gate.append('0110')
				gates.append(gate)
				numGates += 1

			elif gateType == "7":
				gate.append('0111')
				gates.append(gate)
				nonXOR += 1
				numGates += 1

			elif gateType == "8":
				gate.append('1000')
				gates.append(gate)
				nonFreeGates += 1
				numGates += 1

			elif gateType == "9":
				gate.append('1001')
				gates.append(gate)
				numGates += 1

			elif gateType == '10':
				gate.append('1010')
				gates.append(gate)
				nonXOR += 1
				numGates += 1

			elif gateType == '11':
				gate.append('1011')
				gates.append(gate)
				nonFreeGates += 1
				numGates += 1

			elif gateType == '12':
				gate.append('1100')
				gates.append(gate)
				numGates += 1

			elif gateType == '13':
				gate.append('1101')
				gates.append(gate)
				nonFreeGates += 1
				numGates += 1

			elif gateType == "14":
				gate.append('1110')
				gates.append(gate)
				nonFreeGates += 1
				numGates += 1

			# 1 CONSTANT using NXOR on same input
			elif gateType == "15":
				oneConstantOutWire = output
				gate.append('1001')
				gates.append(gate)
				numGates += 1

			elif "copy" in gateType:
				gate.append('copy')
				gates.append(gate)

			else:
				gate.append('NOT IMPLEMENTED YET') 
				gates.append(gate)

				print('\n\033[93mGATE NOT IMPLEMENTED YET:\033[0m got gate type ' + gateType)
	

	# Adding constant gates
	for gate in gates:
		gateType = gate[3];

		if gateType == 'copy':
			gate[1] = zeroConstantOutWire
			gate[3] = '0110'		
			numGates += 1


	# Wirting to out file
	ofile = ifile + '.dp'
	with open(ofile, 'w') as f:
		firstOutWire = inWires
		inputSize = int(inWires/2)
		outputSize = int(outWires/2)

		f.write('1 1\n')
		f.write(str(inputSize) + ' ' + str(inputSize) + ' ' + str(outWires) + ' ' + str(outWires) + ' ' + str(outWires) + '\n')
		f.write('\n')
		f.write('FN 1 ' + str(inWires) + ' ' + str(outWires) + ' ' + str(numWires + 1) + ' ' + str(nonFreeGates) + ' ' + str(numGates) + '\n')
		for gate in gates:
			f.write(gate[0] + ' ' + gate[1] + ' ' + gate[2] + ' ' + gate[3] + '\n')

		f.write('--end FN 1 --\n')
		f.write('\n')
		f.write('FN 2\n')
		f.write('\n')
		f.write('FN 1\n')
		for i in range(0, inWires):
			f.write(str(i) + ' ')
		
		f.write('\n')
		for i in range(inWires, inWires + outWires):
			f.write(str(i) + ' ')

		f.close()


def main():
	inputfile = ''
	outputfile = ''

	try:
		opts, args = getopt.getopt(sys.argv[1:],"hi:",["ifile="])
	except getopt.GetoptError:
		print('parse.py -i <inputfile>')
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print('parse.py -i <inputfile>')
			sys.exit()
		elif opt in ("-i", "--ifile"):
			inputfile = arg
		else:
			print("usage: ./parse.py -i <inputfile>")

	parseFile(inputfile)


if __name__ == '__main__':
	main()

