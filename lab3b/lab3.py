#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

import csv
import sys
import os
import math

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
        self.indirect = []
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
    
    def insert_indirect_block(self, indirect):
        self.indirect.append(indirect)

class Group():
    def __init__(self, line):
        self.group_number = int(line[1])
        self.total_blocks = int(line[2])
        self.total_inodes = int(line[3])
        self.free_blocks = int(line[4])
        self.free_inodes = int(line[5])
        self.free_block_bitmap = int(line[6])
        self.free_inode_bitmap = int(line[7])
        self.first_inode = int(line[8])

class Indirect():
    def __init__(self, line):
        self.inode = int(line[1])
        self.indirection = int(line[2])
        self.block_number = int(line[5])

def main():
    arguments = sys.argv
    if(len(arguments) != 2):
        print("Expected argument in the form ./lab3b <CSV FILE>", file=sys.stderr)
        exit(1)
    file = arguments[1]
    if(not os.path.exists(file)):
        print("Unable to open provided file", file=sys.stderr)
        exit(1)
    inodes = {}
    groups = []
    freeblocks = set()
    with open(file, "r") as f:
        reader = csv.reader(f)
        for line in reader:
            if line[0] == "SUPERBLOCK":
                superblock = Superblock(line)
            elif line[0] == "INODE":
                inodes[line[1]] = Inode(line)
            elif line[0] == "GROUP":
                groups.append(Group(line))
            elif line[0] == "BFREE":
                freeblocks.add(int(line[1]))
            elif line[0] == "INDIRECT":
                inodes[line[1]].insert_indirect_block(Indirect(line))
    
    error = validateInodes(inodes, superblock)
    error += checkUnreferencedBlocks(freeblocks, inodes, superblock, groups)
    exit(2 if error > 0 else 0)  # Exit from the system with the given status

def first_nonreserved_block(superblock):
    block = (1024+1024)/superblock.block_size
    block += math.ceil((32.0*1)/superblock.block_size) # Group Descriptor Table (Only 1 group guaranteed)
    block += 1 # Block bitmap
    block += 1 # Inode bitmap
    block += math.ceil((128.0 * superblock.inodes_per_group)/superblock.block_size)
    return int(block)

def validateInodes(inodes, superblock):
    error = 0
    first_free_block = first_nonreserved_block(superblock)
    # Check that the blocks for each inode aren't negative or above the total allocated blocks
    for _, inode in inodes.items():
        for i in range(0, len(inode.blocks)):
            if inode.blocks[i] < 0 or inode.blocks[i] >= superblock.block_count:
                print("INVALID BLOCK {0} IN INODE {1} AT OFFSET {2}".format(inode.blocks[i], inode.number, i))
                error = 2
            if inode.blocks[i] < first_free_block and inode.blocks[i] != 0:
                print("RESERVED BLOCK {0} IN INODE {1} AT OFFSET {2}".format(inode.blocks[i], inode.number, i))
                error = 2
        if inode.indirect_block < 0 or inode.indirect_block > superblock.block_count:
            print("INVALID INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 12".format(inode.indirect_block, inode.number))
            error = 2
        if inode.indirect_block < first_free_block and inode.indirect_block != 0:
            print("RESERVED INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 12".format(inode.indirect_block, inode.number))
            error = 2
        if inode.double_indirect < 0 or inode.double_indirect > superblock.block_count:
            print("INVALID DOUBLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 268".format(inode.double_indirect, inode.number))
            error = 2
        if inode.double_indirect < first_free_block and inode.double_indirect != 0:
            print("RESERVED DOUBLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 268".format(inode.double_indirect, inode.number))
            error = 2
        if inode.triple_indirect < 0 or inode.triple_indirect > superblock.block_count:
            print("INVALID TRIPLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 65804".format(inode.triple_indirect, inode.number)) 
            error = 2
        if inode.triple_indirect < first_free_block and inode.triple_indirect != 0:
            print("RESERVED TRIPLE INDIRECT BLOCK {0} IN INODE {1} AT OFFSET 65804".format(inode.triple_indirect, inode.number))
            error = 2
    return error

def checkUnreferencedBlocks(freeblocks, inodes, superblock, groups):
    referenced_blocks = set()
    error = 0
    for _, inode in inodes.items():
        for block in inode.blocks:
            referenced_blocks.add(block)
        for indirect in inode.indirect:
            referenced_blocks.add(indirect.block_number)
        referenced_blocks.add(inode.indirect_block)
        referenced_blocks.add(inode.double_indirect)
        referenced_blocks.add(inode.triple_indirect)
    first_block = first_nonreserved_block(superblock)
    for i in range(first_block, groups[0].total_blocks):
        if (i not in freeblocks) and (i not in referenced_blocks):
            print("UNREFERENCED BLOCK {0}".format(i))
            error = 2
    for allocated in referenced_blocks:
        if allocated in freeblocks:
            print("ALLOCATED BLOCK {0} ON FREELIST".format(allocated))
            error = 2
    return error

def checkDuplicateBlocks(inodes):
    return 0

if(__name__ == '__main__'):
    main()
