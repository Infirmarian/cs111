#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

# Setup
err_count=0
dir=3564ce1c-5706-438e-9b52-1c75d7666a6f # UUID, very unlikely another folder will be named this
mkdir $dir
cd $dir

touch DENIED
chmod 000 DENIED

echo "FILE1 CONTENTS" > file1.txt
touch empty.txt


# Basic functionality
# 
echo "test" | ../lab0 > res.txt;
if [ $? -ne 0 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi

if [ $(<res.txt) != "test" ]; then
    echo "Error with stdin/stdout"
    let "err_count+=1"
fi

# One input file
../lab0 --input=file1.txt > res.txt
if [ $? -ne 0 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi
if [[ $(<res.txt) != $(<file1.txt) ]]; then
    echo "Error with input file"
    let "err_count+=1"
fi

# Empty input file
../lab0 --input=empty.txt > res.txt
if [ $? -ne 0 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi
if [[ $(<empty.txt) != $(<res.txt) ]]; then
    echo "Error with input file"
    let "err_count+=1"
fi

# One output file
echo "test" | ../lab0 --output=res.txt;
if [ $? -ne 0 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi

if [ $(<res.txt) != "test" ]; then
    echo "Error with output file"
    let "err_count+=1"
fi

# One input, one output file
../lab0 --input=file1.txt --output=res.txt;
if [ $? -ne 0 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi

if [[ $(<res.txt) != $(<file1.txt) ]]; then
    echo "Error with input and output file"
    let "err_count+=1"
fi

# Input and output are the same file
../lab0 --input=file1.txt --output=file1.txt;
if [ $? -ne 0 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi

if [[ $(<empty.txt) != $(<file1.txt) ]]; then
    echo "Error with input and output file as the same file"
    let "err_count+=1"
fi

# Input problems
../lab0 --input=NOT_A_FILE
if [ $? -ne 2 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi

../lab0 --input=DENIED
if [ $? -ne 2 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi

# Output problems

../lab0 --output=DENIED
if [ $? -ne 3 ]; then
    echo "Bad RC"
    let "err_count+=1"
fi


# Bogus arguments
../lab0 --bogus
if [ $? -ne 1 ]; then
    echo "Failed to catch bogus argument"
    let "err_count+=1"
fi

../lab0 --input
if [ $? -ne 1 ]; then
    echo "Failed to catch bogus argument"
    let "err_count+=1"
fi

../lab0 --badarg --segfault
if [ $? -ne 1 ]; then
    echo "Failed to catch bogus argument"
    let "err_count+=1"
fi

../lab0 --segfault --badarg
if [ $? -ne 1 ]; then
    echo "Failed to catch bogus argument"
    let "err_count+=1"
fi

# Segfaults and catching
../lab0 --segfault
if [ $? -ne 139 ]; then
    echo "Failed to segfault"
    let "err_count+=1"
fi

../lab0 --dump-core --segfault
if [ $? -ne 139 ]; then
    echo "Failed to segfault"
    let "err_count+=1"
fi

../lab0 --segfault --catch
if [ $? -ne 139 ]; then
    echo "Failed to segfault"
    let "err_count+=1"
fi

# Segfault interrupts program without writing to output
../lab0 --input=file1.txt --segfault --output=out.txt
if [ $? -ne 139 ]; then
    echo "Failed to segfault"
    let "err_count+=1"
fi
if [ -f out.txt ]; then
    echo "Created a file unintentionally"
    let "err_count+=1"
fi

# Successful catch attempts
../lab0 --catch --segfault
if [ $? -ne 4 ]; then
    echo "Failed to catch segfault"
    let "err_count+=1"
fi

../lab0 --dump-core --catch --segfault
if [ $? -ne 4 ]; then
    echo "Failed to catch segfault"
    let "err_count+=1"
fi

# Cleanup
rm -f DENIED
cd ..
rm -r $dir

# Results
if [ $err_count -eq 0 ]; then
    echo "All tests passed"
else
    echo "$err_count tests failed"
fi