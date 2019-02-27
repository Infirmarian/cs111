#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

import csv
import sys
import os

def main():
    arguments = sys.argv
    if(len(arguments) != 2):
        print("Expected argument in the form ./lab3b <CSV FILE>", file=sys.stderr)
        exit(1)
    file = arguments[1]
    if(not os.path.exists(file)):
        print("Unable to open provided file", file=sys.stderr)
        exit(1)
    with open(file, "r") as f:
        reader = csv.reader(f)
        for line in reader:
            print(line[0])

if(__name__ == '__main__'):
    main()
