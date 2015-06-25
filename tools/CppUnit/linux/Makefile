FRAMEWORK = ../test/framework
EXTENSIONS = $(FRAMEWORK)/extensions
INCLUDE = -I$(FRAMEWORK) -I$(EXTENSIONS)

SOURCE = $(FRAMEWORK)/TestCase.cpp $(FRAMEWORK)/TestFailure.cpp $(FRAMEWORK)/TestResult.cpp $(FRAMEWORK)/TestSuite.cpp

all: culib.a

culib.a: $(SOURCE)
	g++ $(INCLUDE) -c $^
	ar r $@ *.o
	ranlib $@
