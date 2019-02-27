#NAME: Robert Geil
#EMAIL: rgeil@ucla.edu
#ID: 104916969

exe=lab3b
distName=lab3b-104916969.tar.gz

all: lab3.py
	echo 'python3 lab3.py $$1 $$2' > $(exe)
	chmod +x $(exe)

dist: lab3.py Makefile
	tar -czf $(distName) lab3.py Makefile

clean:
	$(RM) $(exe)
	$(RM) $(distName)