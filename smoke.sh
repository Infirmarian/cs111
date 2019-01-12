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
    err_count+=1
fi

if [ $(<res.txt) != "test" ]; then
    echo "Error with stdin/stdout"
    err_count+=1
fi
# One input file
../lab0 --input=file1.txt > res.txt
if [ $? -ne 0 ]; then
    echo "Bad RC"
    err_count+=1
fi
if [[ $(<res.txt) != $(<file1.txt) ]]; then
    echo "Error with input file"
    err_count+=1
fi

# One output file
echo "test" | ../lab0 --output=res.txt;
if [ $? -ne 0 ]; then
    echo "Bad RC"
    err_count+=1
fi

if [ $(<res.txt) != "test" ]; then
    echo "Error with output file"
    err_count+=1
fi

# One input, one output file
../lab0 --input=file1.txt --output=res.txt;
if [ $? -ne 0 ]; then
    echo "Bad RC"
    err_count+=1
fi

if [[ $(<res.txt) != $(<file1.txt) ]]; then
    echo "Error with input and output file"
    err_count+=1
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