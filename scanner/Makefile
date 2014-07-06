CXX = g++
CXXFLAGS = -Wall -g -MMD

EXEC = wlp4scan
OBJECTS = wlp4scan.o kind.o lexer.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${DEPENDS} ${EXEC} 2> /dev/null
