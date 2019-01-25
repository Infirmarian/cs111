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
touch $DIR/a10.txt
touch $DIR/a10e.txt
touch $DIR/0
touch $DIR/1
touch $DIR/2
touch $DIR/3
touch $DIR/4
touch $DIR/5
touch $DIR/6
touch $DIR/7
touch $DIR/8
touch $DIR/9
touch $DIR/10
touch $DIR/11

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

sleep 1s
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
sleep 0.5s
# Test 10: Continues past bad file descriptor
./$1 --rdonly $DIR/a1.txt --wronly $DIR/DOES_NOT_EXIST --wronly $DIR/a10.txt --wronly $DIR/a10e.txt --command 0 2 3 cat &> $DIR/empty10.txt
if [ $? -ne 1 ] || [ ! -s $DIR/empty10.txt ] || [ -s $DIR/a10e.txt ] || [[ $(<$DIR/a1.txt) != $(<$DIR/a10.txt) ]]; then
    let "error_count+=1"
    echo "Test 10 FAILED"
fi

# Test 11: Many many file descriptors
./$1 --rdonly $DIR/0 --rdonly $DIR/1 --rdonly $DIR/2 --wronly $DIR/3 --wronly $DIR/4 \
    --wronly $DIR/5 --wronly $DIR/6 --wronly $DIR/7 --wronly $DIR/8 --wronly $DIR/9 \
    --wronly $DIR/10 --wronly $DIR/11 --command 0 3 4 ls --command 1 7 8 ls &> $DIR/empty11.txt
if [ $? -ne 0 ] || [ -s $DIR/empty1.txt ]; then
    let "error_count+=1"
    echo "Test 11 FAILED"
fi

# Test 12: Verbose flag
./$1 --verbose --rdonly $DIR/a1.txt 1> $DIR/command.txt
if [ $? -ne 0 ] || [[ $(<$DIR/command.txt) != "--rdonly $DIR/a1.txt" ]]; then
    let "error_count+=1"
    echo "Test 12 FAILED"
fi

# Test 13: Create file
./$1 --creat --rdonly $DIR/new.txt 1> $DIR/test13o 2> $DIR/test13e
if [ $? -ne 0 ] || [ ! -e $DIR/new.txt ] || [ -s $DIR/test13o ] || [ -s $DIR/test13e ]; then
    let "error_count+=1"
    echo "Test 13 FAILED"
fi

# Test 14: Create and excl
touch $DIR/exists.txt
./$1 --creat --excel --rdonly $DIR/exists.txt 1> $DIR/test14o 2> $DIR/test14e
if [ $? -ne 1 ] || [ -s $DIR/test14o ] || [ ! -s $DIR/test14e ]; then
    let "error_count+=1"
    echo "Test 14 FAILED"
fi

# Test 16: Abort
./$1 --abort 1> $DIR/test16o 2> $DIR/test16e
if [ $? -ne 139 ] || [ -s $DIR/test16o ] || [ -s $DIR/test16e ]; then
    let "error_count+=1"
    echo "Test 16 FAILED"
fi

# Test 17: Abort and Catch
./$1 --catch 11 --abort 1> $DIR/test17o 2> $DIR/test17e
if [ $? -ne 11 ] || [ -s $DIR/test17o ] || [ ! -s $DIR/test17e ] || ! grep -q "11 caught" $DIR/test17e; then
    let "error_count+=1"
    echo "Test 17 FAILED"
fi

# Test 18: Abort and fail to catch
./$1 --abort --catch 11 1> $DIR/test18o 2> $DIR/test18e
if [ $? -ne 139 ] || [ -s $DIR/test18o ] || [ -s $DIR/test18e ]; then
    let "error_count+=1"
    echo "Test 18 FAILED"
fi

# Test 19: Catch, reset to default and abort
./$1 --catch 11 --default 11 --abort 1> $DIR/test19o 2> $DIR/test19e
if [ $? -ne 139 ] || [ -s $DIR/test19o ] || [ -s $DIR/test19e ]; then
    let "error_count+=1"
    echo "Test 19 FAILED"
fi

# Test 20: Pause
./$1 --pause 1> $DIR/test20o 2> $DIR/test20e & 
passed=1
sleep 0.5
if ! ps --pid $! > /dev/null; then
  passed=0
fi
kill -10 $!
sleep 0.5
if ps --pid $! > /dev/null; then
passed = 0
fi
if [ $passed -ne 1 ] || [ -s $DIR/test20o ] || [ -s $DIR/test20e ]; then
    let "error_count+=1"
    echo "Test 20 FAILED"
fi

# Test 21: Pause and Ignore
./$1 --ignore 10 --pause 1> $DIR/test21o 2> $DIR/test21e & 
passed=1
sleep 0.5
if ! ps --pid $! > /dev/null; then
  passed=0
fi
kill -10 $!
sleep 0.5
if ! ps --pid $! > /dev/null; then
passed = 0
fi
kill -11 $!
sleep 0.5
if ps --pid $! > /dev/null; then
passed = 0
fi
if [ $passed -ne 1 ] || [ -s $DIR/test21o ] || [ -s $DIR/test21e ]; then
    let "error_count+=1"
    echo "Test 21 FAILED"
fi

# Test 22: Wait (for nothing)
./$1 --wait > $DIR/test22o 2> $DIR/test22e
if [ $? -ne 0 ] || [ -s $DIR/test22o ] || [ -s $DIR/test22e ]; then
    let "error_count+=1"
    echo "Test 22 FAILED"
fi

# Test 23: Wait for cat
echo "SAMPLE TEXT" > $DIR/23i.txt
./$1 --rdonly $DIR/23i.txt --creat --wronly $DIR/23o.txt --creat --wronly $DIR/23e.txt --command 0 1 2 cat --wait > $DIR/test23o 2> $DIR/test23e
if [ $? -ne 0 ] || [ ! -s $DIR/test23o ] || [ -s $DIR/test23e ] || [[ $(<$DIR/23o.txt) != $(<$DIR/23i.txt) ]] || ! grep -q "exit 0 cat" $DIR/test23o; then
    let "error_count+=1"
    echo "Test 23 FAILED"
fi

# Test 24: pipe
./$1 --pipe > $DIR/test24o 2> $DIR/test24e
if [ $? -ne 0 ] || [ -s $DIR/test24o ] || [ -s $DIR/test24e ]; then
    let "error_count+=1"
    echo "Test 24 FAILED"
fi

# Test 25: pipe part 2
echo "Sample values" > $DIR/in25.txt
./$1 --pipe --rdonly $DIR/in25.txt --creat --wronly $DIR/out25.txt --creat --wronly $DIR/err25.txt \
--command 2 1 4 cat --command 0 3 4 cat --close 1 --wait > $DIR/test25o 2> $DIR/test25e
if [ $? -ne 0 ] || [ ! -s $DIR/test25o ] || [ -s $DIR/test25e ] || [[ $(<$DIR/in25.txt) != $(<$DIR/out25.txt) ]] \
|| ! grep -q "exit 0 cat" $DIR/test25o; then
    let "error_count+=1"
    echo "Test 25 FAILED"
fi



# Teardown
if [ $error_count -gt 0 ]; then
    echo "Tests finished with $error_count errors"
else
    echo "All tests passed"
    rm -rf $DIR
fi


