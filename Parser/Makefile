SRC = .
OBJ = .
BIN = .
CC = g++
DEFINES =
CFLAGS = -I./

OBJECTS = ${OBJ}/Rule.opp ${OBJ}/Grammar.opp ${OBJ}/Terminal_NonTerminal.opp ${OBJ}/xtoLL1.opp ${OBJ}/ParserFunctions.opp ${OBJ}/Table.opp ${OBJ}/NonRecursivePredictiveParser.opp ${OBJ}/TopDownDriver.opp #${OBJ}/xtoLL1driver.opp


all: ${OBJECTS}
	${CC} $^ -g -o ${BIN}/TopDownDriver.outpp

${OBJ}/%.opp : ${SRC}/%.cpp
	${CC} -c -g ${DEFINES} ${CFLAGS} $< -o $@

grammar: $(OBJ)/Rule.opp $(OBJ)/Terminal_NonTerminal.opp $(OBJ)/Grammar.opp $(OBJ)/xtoLL1.opp $(OBJ)/ParserFunctions.opp
	

clean:
	rm -f ${OBJ}/*.opp
	rm -f ${BIN}/*.outpp
