CXXFLAGS += -O2 -frtti -ggdb3
LDFLAGS += -frtti

test: Skip_test
	ddd Skip_test

Skip_simple.o: Skip_simple.cc SkipList.hh

Skip_test: Skip_test.o
	g++ -frtti Skip_test.o -o Skip_test -lpthread

Skip_test.o: SkipList.hh

QsTest: QsTest.o
	g++ -frtti QsTest.o -o QsTest -lpthread

QsTest.o: Qs.hh

doc:Qs.hh
	-mkdir doc
	-rm doc/*
	doc++ -H -a -f -M -d doc Qs.hh

clean:
	-rm *.o *~ QsTest core doc/*

tar: clean
	cd ..; tar czvf SkipList.tgz --exclude skiplist/backup skiplist/
