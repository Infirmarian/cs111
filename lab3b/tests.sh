
if [ ! -d "tests" ]; then
    mkdir tests
fi
failed=0
testcount=22
for((i=1;i<=$testcount;i++)); do
    if [ ! -f "tests/P3B-test_$i.csv" ]; then
        curl http://web.cs.ucla.edu/classes/cs111/Samples/P3B-test_$i.csv > tests/P3B-test_$i.csv
    fi
    if [ ! -f "tests/P3B-test_$i.err" ]; then
        curl http://web.cs.ucla.edu/classes/cs111/Samples/P3B-test_$i.err > tests/P3B-test_$i.err
    fi
    
    if ./lab3b tests/P3B-test_$i.csv | diff -q - tests/P3B-test_$i.err;
    then
        echo "PASSED: $i"
    else
        echo "FAILED: $i"
        ./lab3b tests/P3B-test_$i.csv | diff - tests/P3B-test_$i.err;
        let "failed++"
    fi
done
echo -e "\n###########################\n#         RESULTS         #\n###########################\n"
if [ $failed -eq 0 ]; then
    echo "ALL TESTS PASSED"
else
    echo -e "$failed / $testcount TEST FAILED\n"
fi