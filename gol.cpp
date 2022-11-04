#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<cstdlib>
#include<dirent.h>
#include<SFML/Graphics.hpp>

using namespace std;

// global vars
int GRID_WIDTH = 30;
int GRID_HEIGHT = 30;

// global consts
const int CELL_SIZE = 30;
const int FRAME_RATE = 10;
const int8_t DX[] = {0,1,1,1,0,-1,-1,-1};
const int8_t DY[] = {-1,-1,0,1,1,1,0,-1};

// headers
bool fileEndsWith(string value);
void userLoadFile();
void generateRandomBoard(bool* A, float ratio);
int next_gen(bool* A, bool* B);
void drawBoard(bool* A, sf::RenderWindow& win);
void createBoard(string mode, string filename, float ratio);
void runGame(sf::RenderWindow& window, bool* A);

int main() {
    // 99% sure this command doesnt work on windows
    system("clear");
    
	int choice = 0;
	string options[3] = {
		"User preloaded file",
		"Use randomly generated board",
		"Create your own board!"
	};

	cout << "Welcome to Conway's Game of Life!\n" << endl;
	cout << "Please choose an option:" << endl;

	for (int i = 0; i < 3; i++) {
		cout << i << ": " << options[i] << endl;
	}
	cin >> choice;

	while (choice < 0 || choice > 2) {
		cout << "Invalid choice. Please choose again." << endl;
		cin >> choice;
	}

	switch (choice) {
		case 0:
            userLoadFile();
			break;
		case 1:
			cout << "Please enter a ratio of live cells to dead cells (0.0-1.0): ";
			double ratio;
			cin >> ratio;
			createBoard("random", "none", ratio);
			break;
		case 2:
			createBoard("user", "none", 0.0);
			break;
	}
    
    return 0;
}

// from gol_skeleton_sfml.cpp - prof. liang
bool* load(string filename) {
    ifstream fd;
    int length;
    char *Fbuf;
    fd.open(filename,ios::in|ios::binary);
    fd.seekg(0,ios::end);
    length = (int)fd.tellg();
    fd.seekg(0,ios::beg);
    Fbuf = new char[4];  // heap allocation
    fd.read(Fbuf,4); // read all data
    //fd.close();
    // decode first 4 bytes as row, column
    GRID_HEIGHT = ((uint16_t)Fbuf[0])*256 + (uint16_t)Fbuf[1];
    GRID_WIDTH = ((uint16_t)Fbuf[2])*256 + (uint16_t)Fbuf[3];
    // decode other bytes bit by bit
    int mblen = (GRID_HEIGHT*GRID_WIDTH)/8 + 1;
    char* Mbuf = new char[mblen];
    memset(Mbuf,0,mblen*sizeof(char));
    //fd.seekg(4,ios::beg);  //no effect
    fd.read(Mbuf,mblen);
    int bytecount = (int)fd.gcount();
    fd.close();
    bool* matrix = new bool[GRID_HEIGHT * GRID_WIDTH];
    int mi = GRID_HEIGHT * GRID_WIDTH - 1;
    mblen = bytecount;
    int bi = mblen-1; // byte index
    int bitcx = 0;    
    unsigned char onebyte = (unsigned char)(Mbuf[bi]);
    while (mi>=0) {
        matrix[mi] = onebyte%2;
        onebyte =onebyte/2;
        bitcx++;
        if (bitcx==8 && bi>0) {
        bitcx=0; bi--; onebyte = (unsigned char)Mbuf[bi];
        }
        mi--;
    }
    return matrix;
}

// yes, abstraction
bool fileEndsWith(string value) {
    string ending = ".gol";
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void userLoadFile() {
	string filename;
	DIR *dr;
	struct dirent *en;
	dr = opendir(".");

	while ((en = readdir(dr)) != NULL) {
        if (fileEndsWith(en->d_name)) {
            cout << en->d_name << endl;
        }
  	}

	closedir(dr);
	cout << "Enter the name of the file you want to load: ";
	cin >> filename;

    createBoard("file", filename, 0.0);
}

void generateRandomBoard(bool* A, float ratio) {
    srand(time(nullptr));
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
        int current = y * GRID_WIDTH + x;
        bool alive = rand() % 10 < ratio * 10;
        A[current] = alive;
        }
    }
}

int next_gen(bool* A, bool* B) {
    int changes = 0;
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int n = 0; n < GRID_WIDTH; n++) {
            int count = 0;
            for (int j = 0; j < 8; j++) {
                int x = i + DX[j];
                int y = n + DY[j];
                if (x >= 0 && x < GRID_HEIGHT && y >= 0 && y < GRID_WIDTH) {
                    if (A[x * GRID_WIDTH + y]) {
                        count++;
                    }
                }
            }
            if (A[i * GRID_WIDTH + n]) {
                if (count == 2 || count == 3) {
                    B[i * GRID_WIDTH + n] = true;
                } else {
                    B[i * GRID_WIDTH + n] = false;
                }
            } else {
                if (count == 3) {
                    B[i * GRID_WIDTH + n] = true;
                } else {
                    B[i * GRID_WIDTH + n] = false;
                }
            }
            if (A[i * GRID_WIDTH + n] != B[i * GRID_WIDTH + n]) {
                changes++;
            }
        }
    }

    return changes;
}

void drawBoard(bool* A, sf::RenderWindow& win) {
    sf::RectangleShape sqr(sf::Vector2f(CELL_SIZE, CELL_SIZE)); 
    for (int y = 0 ; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            sqr.setOutlineColor(sf::Color(0, 200, 0));
            sqr.setOutlineThickness(1);
            sqr.setPosition(x * CELL_SIZE, y * CELL_SIZE);
            if (A[y * GRID_WIDTH + x]) {
                sqr.setFillColor(sf::Color::Yellow);      
            } else {
                sqr.setFillColor(sf::Color::Black);
            }
            win.draw(sqr);
        }
    }
}

void createBoard(string mode, string filename = "", float ratio = 0.0) {
    sf::RenderWindow window(sf::VideoMode(CELL_SIZE * GRID_WIDTH, CELL_SIZE * GRID_HEIGHT), "Conway's Game of Life");
    window.setFramerateLimit(FRAME_RATE);
    if (mode == "user") {
        bool* A = new bool[GRID_WIDTH * GRID_HEIGHT];
        memset(A, 0, GRID_WIDTH * GRID_HEIGHT * sizeof(bool));

        while (window.isOpen()) {
            drawBoard(A, window);
            sf::Event event;
            window.setTitle("Creation Mode | Hold Shift + Click to create multiple cells | R to reset | Enter to start");
            
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                if (event.type == sf::Event::MouseButtonPressed) {
                    int x = double(event.mouseButton.x) / CELL_SIZE;
                    int y = double(event.mouseButton.y) / CELL_SIZE;

                    A[y * GRID_WIDTH + x] = !A[y * GRID_WIDTH + x];
                }
            if (event.type == sf::Event::MouseMoved && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                    int x = double(event.mouseMove.x) / CELL_SIZE;
                    int y = double(event.mouseMove.y) / CELL_SIZE;

                    if (!A[y * GRID_WIDTH + x]) {
                        A[y * GRID_WIDTH + x] = !A[y * GRID_WIDTH + x];
                    }
            }
            if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) {
                        cout << "Resetting board" << endl;
                        for (int i=0;i<GRID_WIDTH * GRID_HEIGHT;i++) {
                            A[i] = false;
                        }
                    }
                    if (event.key.code == sf::Keyboard::Enter) {
                        runGame(window, A);
                    }
            }
            }
            window.display();
        }
    } else if (mode == "random") {
		bool* A = new bool[GRID_HEIGHT * GRID_WIDTH];    
		generateRandomBoard(A, ratio);

        runGame(window, A);
    } else if (mode == "file") {
		bool* A = load(filename);
        runGame(window, A);
    }
}

void runGame(sf::RenderWindow& window, bool* A) {
    window.setTitle("Conway's Game of Life");
    window.setFramerateLimit(FRAME_RATE);

    bool* B = new bool[GRID_HEIGHT * GRID_WIDTH];
    memset(B, 0, GRID_HEIGHT * GRID_WIDTH);

    int changes = 1;
    int cx = 0;
	bool currentA = true;
	while (changes>0 && window.isOpen())
		{
            cx++;
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
		wait			window.close();
			}

			window.clear(sf::Color::Black);

	        int changes0 = changes;
			if (currentA) {
				drawBoard((bool*)A, window);
				changes = next_gen((bool*)A, (bool*)B);
			}
			else {
				drawBoard((bool*)B, window);
				changes = next_gen((bool*)B, (bool*)A);
			}
			currentA = !currentA;

            if (changes==0 && changes0!=0)
              cout << "Stable state after " << cx << " generations\n";
            
			window.display();
		}
		delete(A);
		delete(B);
}