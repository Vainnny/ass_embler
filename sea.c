#include <GL/freeglut.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <memory.h>
 
#define GRID_SIZE 10
#define CELL_SIZE 50
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 650
 
#define PLAYER_GRID_OFFSET_X 50
#define PLAYER_GRID_OFFSET_Y 50
#define BOT_GRID_OFFSET_X 650
#define BOT_GRID_OFFSET_Y 50
 
typedef enum { MENU_SCREEN, PLACEMENT_SCREEN, GAME_SCREEN, GAME_OVER_SCREEN } GameState;
 
typedef struct {
    bool is_ship;
    bool is_hit;
    bool is_miss;
} Cell;
 
typedef struct {
    int size;
    int count;
} ShipType;
 
GameState gameState = MENU_SCREEN;
Cell playerGrid[GRID_SIZE][GRID_SIZE];
Cell botGrid[GRID_SIZE][GRID_SIZE];
bool horizontal = true;
bool playerTurn = true;
bool gameOver = false;
bool showError = false;
 
const ShipType shipTypes[] = {{4,1}, {3,2}, {2,3}, {1,4}};
int currentShipType = 0;
int currentShipCount = 0;
 
typedef struct {
    int x;
    int y;
} Point;
 
typedef struct {
    Point hits[4];
    int count;
    int direction;
    Point lastShot;
} BotAI;
 
BotAI botAI = {0};
 
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Sea Battle");
 
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
 
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeyPress);
    glutMouseFunc(handleMouse);
 
    srand(time(NULL));
    initializeGame();
 
    glutMainLoop();
    return 0;
}
 
void initializeGame() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            playerGrid[i][j] = (Cell){false, false, false};
            botGrid[i][j] = (Cell){false, false, false};
        }
    }
    resetBotAI();
}
 
void resetBotAI() {
    memset(&botAI, 0, sizeof(BotAI));
}
 
void startNewGame() {
    initializeGame();
    gameState = PLACEMENT_SCREEN;
    currentShipType = 0;
    currentShipCount = 0;
    playerTurn = true;
    gameOver = false;
    showError = false;
    horizontal = true;
    botPlaceShips();
    glutPostRedisplay();
}
 
bool canPlaceShip(int x, int y, int size, bool horizontal, Cell grid[GRID_SIZE][GRID_SIZE]) {
    if (x < 0 || y < 0 || (horizontal && x+size > GRID_SIZE) || (!horizontal && y+size > GRID_SIZE))
        return false;
 
    for (int i = x-1; i <= x+(horizontal?size:1); i++) {
        for (int j = y-1; j <= y+(horizontal?1:size); j++) {
            if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && grid[i][j].is_ship)
                return false;
        }
    }
    return true;
}
 
void placeShip(int x, int y) {
    if (currentShipType >= sizeof(shipTypes)/sizeof(shipTypes[0])) {
        return;
    }
 
    int size = shipTypes[currentShipType].size;
    
    if (!canPlaceShip(x, y, size, horizontal, playerGrid)) {
        showError = true;
        glutPostRedisplay();
        return;
    }
 
    showError = false;
    
    for (int i = 0; i < size; i++) {
        int nx = horizontal ? x + i : x;
        int ny = horizontal ? y : y + i;
        playerGrid[nx][ny].is_ship = true;
    }
 
    currentShipCount++;
    
    if (currentShipCount >= shipTypes[currentShipType].count) {
        currentShipType++;
        currentShipCount = 0;
    }
 
    if (currentShipType >= sizeof(shipTypes)/sizeof(shipTypes[0])) {
        gameState = GAME_SCREEN;
    }
    
    glutPostRedisplay();
}
 
void botPlaceShips() {
    for (int type = 0; type < sizeof(shipTypes)/sizeof(shipTypes[0]); type++) {
        for (int ship = 0; ship < shipTypes[type].count; ship++) {
            bool placed = false;
            int attempts = 0;
            
            while (!placed && attempts < 100) {
                int x = rand() % GRID_SIZE;
                int y = rand() % GRID_SIZE;
                bool dir = rand() % 2 == 0;
 
                if (canPlaceShip(x, y, shipTypes[type].size, dir, botGrid)) {
                    for (int i = 0; i < shipTypes[type].size; i++) {
                        int nx = dir ? x+i : x;
                        int ny = dir ? y : y+i;
                        botGrid[nx][ny].is_ship = true;
                    }
                    placed = true;
                }
                attempts++;
            }
        }
    }
}
 
void markDestroyedShip(Cell grid[GRID_SIZE][GRID_SIZE], int x, int y) {
    if (grid[x][y].is_miss) return;
 
    bool visited[GRID_SIZE][GRID_SIZE] = {false};
    int shipCells[GRID_SIZE*GRID_SIZE][2];
    int count = 0;
    
    void findShipParts(int cx, int cy) {
        if (cx < 0 || cx >= GRID_SIZE || cy < 0 || cy >= GRID_SIZE || visited[cx][cy] || !grid[cx][cy].is_ship)
            return;
        
        visited[cx][cy] = true;
        shipCells[count][0] = cx;
        shipCells[count][1] = cy;
        count++;
        
        findShipParts(cx+1, cy);
        findShipParts(cx-1, cy);
        findShipParts(cx, cy+1);
        findShipParts(cx, cy-1);
    }
    
    findShipParts(x, y);
    
    for (int i = 0; i < count; i++) {
        grid[shipCells[i][0]][shipCells[i][1]].is_miss = true;
        
        for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
                int ni = shipCells[i][0] + di;
                int nj = shipCells[i][1] + dj;
                if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE && !grid[ni][nj].is_hit) {
                    grid[ni][nj].is_hit = true;
                    grid[ni][nj].is_miss = true;
                }
            }
        }
    }
}
 
void checkDestroyedShips(Cell grid[GRID_SIZE][GRID_SIZE], int x, int y) {
    if (!grid[x][y].is_ship) return;
    
    grid[x][y].is_hit = true;
    
    bool visited[GRID_SIZE][GRID_SIZE] = {false};
    int shipCells[GRID_SIZE*GRID_SIZE][2];
    int count = 0;
    
    void findShipParts(int cx, int cy) {
        if (cx < 0 || cx >= GRID_SIZE || cy < 0 || cy >= GRID_SIZE || visited[cx][cy] || !grid[cx][cy].is_ship)
            return;
        
        visited[cx][cy] = true;
        shipCells[count][0] = cx;
        shipCells[count][1] = cy;
        count++;
        
        findShipParts(cx+1, cy);
        findShipParts(cx-1, cy);
        findShipParts(cx, cy+1);
        findShipParts(cx, cy-1);
    }
    
    findShipParts(x, y);
    
    bool allHit = true;
    for (int i = 0; i < count; i++) {
        if (!grid[shipCells[i][0]][shipCells[i][1]].is_hit) {
            allHit = false;
            break;
        }
    }
    
    if (allHit) {
        markDestroyedShip(grid, x, y);
    }
}
 
bool checkVictory(Cell grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j].is_ship && !grid[i][j].is_hit) {
                return false;
            }
        }
    }
    return true;
}
 
void botTurn() {
    if (gameOver) return;
 
    int x = -1, y = -1;
    bool targetFound = false;
    bool shipDestroyed = false;
    int attempts = 0;
 
    if (botAI.count > 0) {
        const int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        for (int i = 0; i < botAI.count && !targetFound; i++) {
            for (int j = 0; j < 4 && !targetFound; j++) {
                x = botAI.hits[i].x + dirs[j][0];
                y = botAI.hits[i].y + dirs[j][1];
                
                if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE &&
                    !playerGrid[x][y].is_hit) {
                    targetFound = true;
                }
            }
        }
        
        if (!targetFound) {
            markDestroyedShip(playerGrid, botAI.hits[0].x, botAI.hits[0].y);
            resetBotAI();
            shipDestroyed = true;
        }
    }
 
    if (!targetFound && !shipDestroyed) {
        do {
            x = rand() % GRID_SIZE;
            y = rand() % GRID_SIZE;
            attempts++;
        } while (playerGrid[x][y].is_hit && attempts < 200);
    }
 
    if (!shipDestroyed) {
        playerGrid[x][y].is_hit = true;
        
        if (playerGrid[x][y].is_ship) {
            if (botAI.count < 4) {
                botAI.hits[botAI.count].x = x;
                botAI.hits[botAI.count].y = y;
                botAI.count++;
            }
            
            checkDestroyedShips(playerGrid, x, y);
            
            if (playerGrid[x][y].is_miss) {
                shipDestroyed = true;
                resetBotAI();
            }
        } else {
            playerTurn = true;
        }
    }
 
    if (checkVictory(playerGrid)) {
        gameOver = true;
        gameState = GAME_OVER_SCREEN;
    }
    
    glutPostRedisplay();
    
    if (!playerTurn && !gameOver) {
        glutTimerFunc(1000, (void (*)(int))botTurn, 0);
    }
}
 
void drawText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    for (const char *c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}
 
void drawGrid(int offsetX, int offsetY, bool showShips) {
    Cell (*grid)[GRID_SIZE] = showShips ? playerGrid : botGrid;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            float x = offsetX + i * CELL_SIZE;
            float y = offsetY + j * CELL_SIZE;
            
            if (grid[i][j].is_miss && grid[i][j].is_ship) {
                glColor3f(1.0, 0.0, 0.0);
            } else if (grid[i][j].is_hit && grid[i][j].is_ship) {
                glColor3f(1.0, 0.5, 0.0);
            } else if (grid[i][j].is_hit) {
                glColor3f(0.8, 0.8, 0.8);
            } else if (showShips && grid[i][j].is_ship) {
                glColor3f(0.3, 0.3, 0.3);
            } else {
                glColor3f(0.8, 0.8, 1.0);
            }
            
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + CELL_SIZE, y);
            glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
            glVertex2f(x, y + CELL_SIZE);
            glEnd();
            
            glColor3f(0.5, 0.5, 0.5);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + CELL_SIZE, y);
            glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
            glVertex2f(x, y + CELL_SIZE);
            glEnd();
        }
    }
    
    glColor3f(0.0, 0.0, 0.0);
    for (int i = 0; i < GRID_SIZE; i++) {
        char letter[2] = { 'A' + i, '\0' };
        drawText(offsetX + i * CELL_SIZE + CELL_SIZE/2 - 5, offsetY - 25, letter);
        
        char number[3];
        snprintf(number, sizeof(number), "%d", i + 1);
        drawText(offsetX - 25, offsetY + i * CELL_SIZE + CELL_SIZE/2 - 5, number);
    }
}
 
void drawShipCounts() {
    char text[50];
    glColor3f(0.0, 0.0, 0.0);
    
    int playerShips[4] = {0};
    int botShips[4] = {0};
    
    for (int type = 0; type < 4; type++) {
        playerShips[type] = shipTypes[type].count;
        botShips[type] = shipTypes[type].count;
        
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (playerGrid[i][j].is_ship && playerGrid[i][j].is_miss) {
                    bool visited[GRID_SIZE][GRID_SIZE] = {false};
                    int shipSize = 0;
                    
                    void checkSize(int x, int y) {
                        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE ||
                            visited[x][y] || !playerGrid[x][y].is_ship) return;
                        visited[x][y] = true;
                        shipSize++;
                        checkSize(x+1, y);
                        checkSize(x-1, y);
                        checkSize(x, y+1);
                        checkSize(x, y-1);
                    }
                    
                    checkSize(i, j);
                    
                    for (int t = 0; t < 4; t++) {
                        if (shipTypes[t].size == shipSize) {
                            playerShips[t]--;
                            break;
                        }
                    }
                }
                
                if (botGrid[i][j].is_ship && botGrid[i][j].is_miss) {
                    bool visited[GRID_SIZE][GRID_SIZE] = {false};
                    int shipSize = 0;
                    
                    void checkSize(int x, int y) {
                        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE ||
                            visited[x][y] || !botGrid[x][y].is_ship) return;
                        visited[x][y] = true;
                        shipSize++;
                        checkSize(x+1, y);
                        checkSize(x-1, y);
                        checkSize(x, y+1);
                        checkSize(x, y-1);
                    }
                    
                    checkSize(i, j);
                    
                    for (int t = 0; t < 4; t++) {
                        if (shipTypes[t].size == shipSize) {
                            botShips[t]--;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    snprintf(text, sizeof(text), "Your ships: 4(%d) 3(%d) 2(%d) 1(%d)",
            playerShips[0], playerShips[1], playerShips[2], playerShips[3]);
    drawText(PLAYER_GRID_OFFSET_X, PLAYER_GRID_OFFSET_Y + GRID_SIZE*CELL_SIZE + 20, text);
    
    snprintf(text, sizeof(text), "Enemy ships: 4(%d) 3(%d) 2(%d) 1(%d)",
            botShips[0], botShips[1], botShips[2], botShips[3]);
    drawText(BOT_GRID_OFFSET_X, BOT_GRID_OFFSET_Y + GRID_SIZE*CELL_SIZE + 20, text);
}
 
void drawPreviewShip() {
    if (gameState != PLACEMENT_SCREEN || currentShipType >= 4) return;
    
    int size = shipTypes[currentShipType].size;
    glColor4f(0.0, 1.0, 0.0, 0.5);
    glBegin(GL_QUADS);
    for (int i = 0; i < size; i++) {
        float x = PLAYER_GRID_OFFSET_X + GRID_SIZE*CELL_SIZE + 20 + (horizontal ? i : 0)*CELL_SIZE;
        float y = 160 + (horizontal ? 0 : i)*CELL_SIZE;
        glVertex2f(x, y);
        glVertex2f(x + CELL_SIZE, y);
        glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
        glVertex2f(x, y + CELL_SIZE);
    }
    glEnd();
    
    char info[50];
    snprintf(info, sizeof(info), "Ship: %d-decker", size);
    drawText(PLAYER_GRID_OFFSET_X + GRID_SIZE*CELL_SIZE + 20, 130, info);
    snprintf(info, sizeof(info), "Left: %d", shipTypes[currentShipType].count - currentShipCount);
    drawText(PLAYER_GRID_OFFSET_X + GRID_SIZE*CELL_SIZE + 20, 100, info);
}
 
void renderMenu() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(0.1, 0.1, 0.5);
    drawText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 50, "SEA BATTLE");
    
    glColor3f(0.0, 0.0, 0.0);
    drawText(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 + 50, "Press any key to start");
    drawText(WINDOW_WIDTH/2 - 200, WINDOW_HEIGHT/2 + 80, "Controls: R - Rotate, N - New game, Q - Quit");
    drawText(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 + 110, "Placement: LMB - Place ship");
    drawText(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 + 140, "Game: LMB - Shoot at enemy field");
    
    glutSwapBuffers();
}
 
void renderPlacement() {
    glClear(GL_COLOR_BUFFER_BIT);
 
    drawText(PLAYER_GRID_OFFSET_X + (GRID_SIZE*CELL_SIZE)/2 - 50, 50, "Your field");
    drawGrid(PLAYER_GRID_OFFSET_X, PLAYER_GRID_OFFSET_Y, true);
    
    const int infoX = PLAYER_GRID_OFFSET_X + GRID_SIZE*CELL_SIZE + 20;
    const int lineHeight = 25;
    int currentY = 100;
 
    if (currentShipType < 4) {
        int size = shipTypes[currentShipType].size;
        
        char header[50];
        snprintf(header, sizeof(header), "Current: %d-decker", size);
        drawText(infoX, currentY, header);
        currentY += lineHeight;
        
        char remaining[50];
        snprintf(remaining, sizeof(remaining), "Left: %d/%d",
                shipTypes[currentShipType].count - currentShipCount,
                shipTypes[currentShipType].count);
        drawText(infoX, currentY, remaining);
        currentY += lineHeight;
        
        drawText(infoX, currentY, "Press R to rotate");
        currentY += lineHeight*2;
        
        glColor4f(0.0, 1.0, 0.0, 0.5);
        glBegin(GL_QUADS);
        for (int i = 0; i < size; i++) {
            float px = infoX + (horizontal ? i : 0)*CELL_SIZE;
            float py = currentY + (horizontal ? 0 : i)*CELL_SIZE;
            glVertex2f(px, py);
            glVertex2f(px + CELL_SIZE, py);
            glVertex2f(px + CELL_SIZE, py + CELL_SIZE);
            glVertex2f(px, py + CELL_SIZE);
        }
        glEnd();
        currentY += (horizontal ? CELL_SIZE : size*CELL_SIZE) + lineHeight;
    }
 
    if (showError) {
        glColor3f(1.0, 0.0, 0.0);
        drawText(infoX, currentY, "Invalid position!");
    }
    
    glutSwapBuffers();
}
 
void renderGame() {
    glClear(GL_COLOR_BUFFER_BIT);
 
    drawText(PLAYER_GRID_OFFSET_X + (GRID_SIZE*CELL_SIZE)/2 - 50, 50, "Your field");
    drawGrid(PLAYER_GRID_OFFSET_X, PLAYER_GRID_OFFSET_Y, true);
 
    drawText(BOT_GRID_OFFSET_X + (GRID_SIZE*CELL_SIZE)/2 - 50, 50, "Enemy field");
    drawGrid(BOT_GRID_OFFSET_X, BOT_GRID_OFFSET_Y, false);
    
    drawShipCounts();
    
    glColor3f(0.0, 0.0, 0.0);
    if (playerTurn) {
        drawText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT - 30, "Your turn - click on enemy field");
    } else {
        drawText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT - 30, "Enemy's turn - please wait...");
    }
    
    glutSwapBuffers();
}
 
void renderGameOver() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    bool playerWon = checkVictory(botGrid);
    const int verticalOffset = 120;
    
    glColor3f(0.1, 0.1, 0.5);
    if (playerWon) {
        drawText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - verticalOffset, "YOU WIN!");
    } else {
        drawText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - verticalOffset, "YOU LOSE!");
    }
    
    glColor3f(0.0, 0.0, 0.0);
    drawText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2 + 50 - verticalOffset, "Press 'N' for new game");
    drawText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2 + 100 - verticalOffset, "Press 'Q' to quit");
    
    const int miniCellSize = 20;
    const int fieldSpacing = 250;
    const int startY = WINDOW_HEIGHT/2 + 130 - verticalOffset;
    
    drawText(PLAYER_GRID_OFFSET_X + (GRID_SIZE*miniCellSize)/2 - 50, startY - 25, "Your field");
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            float x = PLAYER_GRID_OFFSET_X + i * miniCellSize;
            float y = startY + j * miniCellSize;
            
            if (playerGrid[i][j].is_miss && playerGrid[i][j].is_ship) {
                glColor3f(1.0, 0.0, 0.0);
            } else if (playerGrid[i][j].is_hit) {
                glColor3f(1.0, 0.5, 0.0);
            } else if (playerGrid[i][j].is_ship) {
                glColor3f(0.3, 0.3, 0.3);
            } else {
                glColor3f(0.8, 0.8, 1.0);
            }
            
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + miniCellSize, y);
            glVertex2f(x + miniCellSize, y + miniCellSize);
            glVertex2f(x, y + miniCellSize);
            glEnd();
        }
    }
    
    drawText(BOT_GRID_OFFSET_X + fieldSpacing + (GRID_SIZE*miniCellSize)/2 - 50, startY - 25, "Enemy field");
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            float x = BOT_GRID_OFFSET_X + fieldSpacing + i * miniCellSize;
            float y = startY + j * miniCellSize;
            
            if (botGrid[i][j].is_miss && botGrid[i][j].is_ship) {
                glColor3f(1.0, 0.0, 0.0);
            } else if (botGrid[i][j].is_hit) {
                glColor3f(1.0, 0.5, 0.0);
            } else if (botGrid[i][j].is_ship && playerWon) {
                glColor3f(0.3, 0.3, 0.3);
            } else {
                glColor3f(0.8, 0.8, 1.0);
            }
            
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + miniCellSize, y);
            glVertex2f(x + miniCellSize, y + miniCellSize);
            glVertex2f(x, y + miniCellSize);
            glEnd();
        }
    }
    
    glutSwapBuffers();
}
 
void display() {
    switch (gameState) {
        case MENU_SCREEN: renderMenu(); break;
        case PLACEMENT_SCREEN: renderPlacement(); break;
        case GAME_SCREEN: renderGame(); break;
        case GAME_OVER_SCREEN: renderGameOver(); break;
    }
}
 
void handleMouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN || gameOver) return;
    
    if (gameState == PLACEMENT_SCREEN && button == GLUT_LEFT_BUTTON) {
        int gridX = (x - PLAYER_GRID_OFFSET_X) / CELL_SIZE;
        int gridY = (y - PLAYER_GRID_OFFSET_Y) / CELL_SIZE;
        
        if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE) {
            placeShip(gridX, gridY);
        }
    }
    else if (gameState == GAME_SCREEN && button == GLUT_LEFT_BUTTON && playerTurn) {
        int gridX = (x - BOT_GRID_OFFSET_X) / CELL_SIZE;
        int gridY = (y - BOT_GRID_OFFSET_Y) / CELL_SIZE;
 
        if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE &&
            !botGrid[gridX][gridY].is_hit) {
            botGrid[gridX][gridY].is_hit = true;
            
            if (botGrid[gridX][gridY].is_ship) {
                checkDestroyedShips(botGrid, gridX, gridY);
                
                if (checkVictory(botGrid)) {
                    gameOver = true;
                    gameState = GAME_OVER_SCREEN;
                } else {
                    playerTurn = true;
                }
            } else {
                playerTurn = false;
                glutTimerFunc(1000, (void (*)(int))botTurn, 0);
            }
            glutPostRedisplay();
        }
    }
}
 
void handleKeyPress(unsigned char key, int x, int y) {
    switch (key) {
        case 'r': case 'R':
            if (gameState == PLACEMENT_SCREEN) {
                horizontal = !horizontal;
                glutPostRedisplay();
            }
            break;
        case 'n': case 'N':
            startNewGame();
            break;
        case 'q': case 'Q':
            exit(0);
            break;
        default:
            if (gameState == MENU_SCREEN) {
                startNewGame();
            }
            break;
    }
}