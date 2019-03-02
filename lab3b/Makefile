#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

exe=lab3b
distName=lab3b-104916969.tar.gz

all: lab3b

dist: lab3.py Makefile README tests.sh
	tar -czf $(distName) lab3.py Makefile README tests.sh

check: lab3b tests.sh
	./tests.sh

lab3b: lab3.py
	echo 'python3 lab3.py $$1 $$2' > $(exe)
	chmod +x $(exe)
clean:
	$(RM) $(exe)
	$(RM) $(distName)
	$(RM) -r tests
