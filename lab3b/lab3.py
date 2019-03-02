#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

import csv
import sys
import os

class Superblock():
    def __init__(self, line):
        self.block_count = int(line[1])
        self.inode_count = int(line[2])
        self.block_size = int(line[3])
        self.inode_size = int(line[4])
        self.blocks_per_group = int(line[5])
        self.inodes_per_group = int(line[6])
        self.first_non_reserved_inode = int(line[7])

class Inode():
    def __init__(self, line):
        self.number = int(line[1])
        self.filetype = line[2]
        self.mode = int(line[3])
        self.owner = int(line[4])
        self.group = int(line[5])
        self.link_count = int(line[6])
        self.change_time = line[7]
        self.modification_time = line[8]
        self.access_time = line[9]
        self.file_size = int(line[10])
        self.block_count = int(line[11])
        self.blocks = []
        if self.filetype == 's' and self.file_size <= 60:
            self.indirect_block = 0
            self.double_indirect = 0
            self.triple_indirect = 0
        else:
            for i in range(12, 24):
                self.blocks.append(int(line[i]))
            self.indirect_block = int(line[24])
            self.double_indirect = int(line[25])
            self.triple_indirect = int(line[26])


def main():
    arguments = sys.argv
    if(len(arguments) != 2):
        print("Expected argument in the form ./lab3b <CSV FILE>", file=sys.stderr)
        exit(1)
    file = arguments[1]
    if(not os.path.exists(file)):
        print("Unable to open provided file", file=sys.stderr)
        exit(1)
    inodes = []
    with open(file, "r") as f:
        reader = csv.reader(f)
        for line in reader:
            if line[0] == "SUPERBLOCK":
                superblock = Superblock(line)
            elif line[0] == "INODE":
                inodes.append(Inode(line))
    error = validateInodes(inodes, superblock)

    exit(error)  # Exit from the system with the given status

def validateInodes(inodes, superblock):
    error = 0
    # Check that the blocks for each inode aren't negative or above the total allocated blocks
    for inode in inodes:
        for i in range(0, len(inode.blocks)):
            if inode.blocks[i] < 0 or inode.blocks[i] >= superblock.block_count:
                print("INVALID BLOCK {0} IN INODE {1} AT OFFSET {2}".format(inode.blocks[i], inode.number, i))
                error = 2
        if inode.indirect_block < 0 or inode.indirect_block > superblock.block_count:
            print("INVALID INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 12".format(inode.indirect_block, inode.number))
            error = 2
        if inode.double_indirect < 0 or inode.double_indirect > superblock.block_count:
            print("INVALID DOUBLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 268".format(inode.double_indirect, inode.number))
            error = 2
        if inode.triple_indirect < 0 or inode.triple_indirect > superblock.block_count:
            print("INVALID TRIPLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 65804".format(inode.triple_indirect, inode.number)) 
            error = 2
    return error
         
if(__name__ == '__main__'):
    main()
