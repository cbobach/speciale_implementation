#!/usr/bin/python3.5

import sys, getopt

def parseFile(ifile):
	#Reading gates from input file
	with open(ifile) as f:
		numGates = 0
		numXORGates = 0

		numWires = 0

		for line in f:
			line = line.rstrip()
			line = line.split(' ')

			inputl = ''
			inputr = ''
			output = ''
			gateType = ''

			dict = {}

			if len(line) == 4:
				numGates += 1

				inputl = line[0]
				inputr = line[1]
				output = line[2]
				gateType = line[3]

				if gateType == '0110':
					numXORGates += 1

				if int(output) > numWires:
					numWires = int(output)

		print("File: " + ifile)
		print("Number of wires: " + str(numWires))
		print("Number of gates: " + str(numGates))
		print("Number of free XOR-gates: " + str(numXORGates))
		print("Number of non-free gates: " + str(numGates - numXORGates))

def main():
	inputfile = ''

	try:
		opts, args = getopt.getopt(sys.argv[1:],"hi:",["ifile="])
	except getopt.GetoptError:
		print('usage: ./count-gate-types.py -i <inputfile>')
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print('usage: ./count-gate-types.py -i <inputfile>')
			sys.exit()
		elif opt in ("-i", "--ifile"):
			inputfile = arg
		else:
			print("usage: ./count-gate-types.py -i <inputfile>")

	parseFile(inputfile)


if __name__ == '__main__':
	main()
