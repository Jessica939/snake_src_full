snakegame: main.o game.o snake.o map.o
	g++ -o snakegame main.o game.o snake.o map.o -lcurses
main.o: src/main.cpp src/game.cpp
	g++ -c src/main.cpp -Iinclude
game.o: src/game.cpp src/snake.cpp src/map.cpp
	g++ -c src/game.cpp -Iinclude
snake.o: src/snake.cpp src/map.cpp
	g++ -c src/snake.cpp -Iinclude
map.o: src/map.cpp
	g++ -c src/map.cpp -Iinclude
clean:
	rm -f *.o 
	rm -f snakegame
	rm -f record.dat
