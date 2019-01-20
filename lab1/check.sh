#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

if [ $# -eq 0 ]; then
    echo "Expected usage: ./check.sh executable_name"
    exit 1
fi

# Setup
DIR=050375b3-35bc-4e30-b998-c31a064a6122 # this is unlikely to exist
mkdir $DIR
echo "Some sample text" > $DIR/a1.txt
echo "Some sample text" > $DIR/a2.txt
touch $DIR/a8.txt
touch $DIR/a8e.txt
touch $DIR/DENIED
chmod 0 $DIR/DENIED
error_count=0

echo "Running tests"

# Test 0: No flags
./$1
if [ $? -ne 0 ]; then
    let "error_count+=1"
    echo "Test 0 FAILED"
fi

# Test 1: Input only (nothing to stdout or stderr)
./$1 --rdonly $DIR/a1.txt &> $DIR/empty.txt
if [ $? -ne 0 ] || [ -s $DIR/empty.txt ]; then
    let "error_count+=1"
    echo "Test 1 FAILED"
fi

# Test 2: Output only (nothing to stdout or stderr)
./$1 --wronly $DIR/a1.txt &> $DIR/empty2.txt
if [ $? -ne 0 ] || [ -s $DIR/empty2.txt ]; then
    let "error_count+=1"
    echo "Test 2 FAILED"
fi

# Test 3: Input and output (nothing to stdout or stderr)
./$1 --rdonly $DIR/a2.txt --wronly $DIR/a1.txt &> $DIR/empty3.txt
if [ $? -ne 0 ] || [ -s $DIR/empty3.txt ]; then
    let "error_count+=1"
    echo "Test 3 FAILED"
fi

# Test 4: Unreadable file
./$1 --rdonly $DIR/DENIED &> $DIR/err4.txt
if [ $? -ne 1 ] || [ ! -s $DIR/err4.txt ]; then
    let "error_count+=1"
    echo "Test 4 FAILED"
fi

sleep 1s
# Test 5: Unwriteable file
./$1 --wronly $DIR/DENIED &> $DIR/err5.txt
if [ $? -ne 1 ] || [ ! -s $DIR/err5.txt ]; then
    let "error_count+=1"
    echo "Test 5 FAILED"
fi

# Test 6: Nonexistant file
./$1 --rdonly $DIR/I_DONT_EXIST &> $DIR/err6.txt
if [ $? -ne 1 ] || [ ! -s $DIR/err6.txt ]; then
    let "error_count+=1"
    echo "Test 6 FAILED"
fi

# Test 7: Bogus argument
./$1 --bogus &> $DIR/err7.txt
if [ $? -ne 1 ] || [ ! -s $DIR/err7.txt ]; then
    let "error_count+=1"
    echo "Test 7 FAILED"
fi

# Test 8: Ordinary command
./$1 --rdonly $DIR/a1.txt --wronly $DIR/a8.txt --wronly $DIR/a8e.txt --command 0 1 2 cat &> $DIR/empty8.txt
if [ $? -ne 0 ] || [ -s $DIR/empty8.txt ] || [ -s $DIR/a8e.txt ] || [[ $(<$DIR/a1.txt) != $(<$DIR/a8.txt) ]]; then
    let "error_count+=1"
    echo "Test 8 FAILED"
fi

# Test 9: Bad file descriptor
./$1 --command 0 1 2 ls 2> $DIR/err7.txt
if [ $? -ne 1 ] || [ ! -s $DIR/err7.txt ]; then
    let "error_count+=1"
    echo "Test 9 FAILED"
fi



# Teardown

rm -rf $DIR

if [ $error_count -gt 0 ]; then
    echo "Tests finished with $error_count errors"
else
    echo "All tests passed"
fi


