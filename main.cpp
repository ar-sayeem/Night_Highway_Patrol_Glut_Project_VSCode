// main.cpp
// Night Highway Patrol - Enhanced Version
// Uses: DDA, Bresenham line, Midpoint circle, basic 2D transforms
// Controls: Left/Right arrows: move | S: siren | P: pause | R: restart | ESC: exit

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;

// Road boundaries and lanes
const float ROAD_LEFT = 200.0f;
const float ROAD_RIGHT = 600.0f;
const int LANE_COUNT = 3;
float LANE_X[LANE_COUNT];
const float LANE_CENTER = (ROAD_LEFT + ROAD_RIGHT) / 2.0f;

// Game state
bool gameOver = false;
bool paused = false;
int score = 0;
float gameSpeed = 1.0f;

// Score timing
float scoreTimer = 0.0f;
const float SCORE_INTERVAL = 0.8f; // Slightly faster scoring

// Track criminals caught
int criminalsCaught = 0;

// Base vehicle size
const float BASE_VEH_W = 44.0f;
const float BASE_VEH_H = 66.0f;

// Police car
struct PoliceCar {
    float x, y;
    float width, height;
    bool sirenOn;
    int sirenBlink;
    float vx;
    bool leftPressed;
    bool rightPressed;
    float maxVx;
} police = {WIDTH/2.0f, 80.0f, BASE_VEH_W, BASE_VEH_H, true, 0, 0.0f, false, false, 250.0f};

// Civilian vehicle types: 0=car, 1=bus, 2=bike
struct Car {
    float x, y;
    float width, height;
    float speed;
    int color;
    int type;
    int lane;
    bool active;
};

// Criminal car
struct CriminalCar {
    float baseX;
    float x, y;
    float width, height;
    float speed;
    float zigzag;
    bool active;
} criminal = {WIDTH/2.0f, WIDTH/2.0f, HEIGHT + 100.0f, BASE_VEH_W, BASE_VEH_H, 2.5f, 0.0f, true};

// Containers
std::vector<Car> civilianCars;
struct LaneMarker { float x, y; };
std::vector<LaneMarker> laneMarkers;
std::vector<std::pair<int,int>> stars;

// Random helpers
static float randFloat(float a, float b) {
    return a + (b - a) * (rand() / (float)RAND_MAX);
}
static int randInt(int a, int b) {
    if (a > b) return a;
    return a + rand() % (b - a + 1);
}

// ==================== ALGORITHM IMPLEMENTATIONS ====================

// Lab 4: DDA Line Algorithm
void drawLineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = fabsf(dx) > fabsf(dy) ? fabsf(dx) : fabsf(dy);

    if (steps <= 0.0f) {
        glBegin(GL_POINTS);
        glVertex2i((int)roundf(x1), (int)roundf(y1));
        glEnd();
        return;
    }

    float xInc = dx / steps;
    float yInc = dy / steps;
    float x = x1, y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= (int)steps; ++i) {
        glVertex2i((int)roundf(x), (int)roundf(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// Lab 5: Bresenham Line Algorithm
void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while(true) {
        glVertex2i(x1, y1);
        if(x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    glEnd();
}

// Lab 6: Midpoint Circle Algorithm
void drawCircleMidpoint(int xc, int yc, int r) {
    int x = 0, y = r;
    int p = 1 - r;

    auto plotCirclePoints = [xc, yc](int x, int y) {
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);
        glVertex2i(xc + y, yc + x);
        glVertex2i(xc - y, yc + x);
        glVertex2i(xc + y, yc - x);
        glVertex2i(xc - y, yc - x);
    };

    glBegin(GL_POINTS);
    while(x <= y) {
        plotCirclePoints(x, y);
        x++;
        if(p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
    }
    glEnd();
}

// Filled circle helper
void drawFilledCircle(int xc, int yc, int r) {
    if (r < 1) r = 1;
    for(int dy = -r; dy <= r; ++dy) {
        int span = (int)floorf(sqrtf((float)(r*r - dy*dy)));
        glBegin(GL_POINTS);
        for(int dx = -span; dx <= span; ++dx) {
            glVertex2i(xc + dx, yc + dy);
        }
        glEnd();
    }
}

// ==================== UTILITIES ====================

static float getScaleForY(float /*y*/) {
    return 1.0f;
}

// Rectangle overlap detection
bool rectOverlap(float x1, float y1, float w1, float h1,
                 float x2, float y2, float w2, float h2, float margin = 8.0f) {
    float leftA = x1 - w1/2 - margin;
    float rightA = x1 + w1/2 + margin;
    float bottomA = y1;
    float topA = y1 + h1;
    float leftB = x2 - w2/2 - margin;
    float rightB = x2 + w2/2 + margin;
    float bottomB = y2;
    float topB = y2 + h2;
    return !(leftA > rightB || rightA < leftB || topA < bottomB || bottomA > topB);
}

// Collision detection
bool checkCollisionScaled(float x1, float y1, float w1, float h1,
                          float x2, float y2, float w2, float h2) {
    float s1 = getScaleForY(y1);
    float s2 = getScaleForY(y2);
    float ew1 = w1 * s1;
    float eh1 = h1 * s1;
    float ew2 = w2 * s2;
    float eh2 = h2 * s2;
    return (x1 - ew1/2.0f < x2 + ew2/2.0f && x1 + ew1/2.0f > x2 - ew2/2.0f &&
            y1 < y2 + eh2 && y1 + eh1 > y2);
}

// Compute lane center X
float laneX(int lane) {
    if (lane < 0) lane = 0;
    if (lane >= LANE_COUNT) lane = LANE_COUNT - 1;
    return LANE_X[lane];
}

// Draw centered text
void drawCenteredText(int y, void* font, const std::string &text) {
    int pixelWidth = glutBitmapLength(font, (const unsigned char*)text.c_str());
    int x = WIDTH/2 - pixelWidth/2;
    glRasterPos2i(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

// ==================== DRAWING FUNCTIONS ====================

void drawRoad() {
    // Gradient road
    glBegin(GL_QUADS);
    glColor3f(0.18f, 0.18f, 0.22f);
    glVertex2f(ROAD_LEFT, 0);
    glVertex2f(ROAD_RIGHT, 0);
    glColor3f(0.12f, 0.12f, 0.16f);
    glVertex2f(ROAD_RIGHT, HEIGHT);
    glVertex2f(ROAD_LEFT, HEIGHT);
    glEnd();

    // Road boundaries (DDA)
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3);
    drawLineDDA(ROAD_LEFT, 0, ROAD_LEFT, HEIGHT);
    drawLineDDA(ROAD_RIGHT, 0, ROAD_RIGHT, HEIGHT);

    // Yellow edge lines
    glColor3f(1.0f, 0.9f, 0.1f);
    glLineWidth(2);
    drawLineDDA(ROAD_LEFT + 3, 0, ROAD_LEFT + 3, HEIGHT);
    drawLineDDA(ROAD_RIGHT - 3, 0, ROAD_RIGHT - 3, HEIGHT);
    glLineWidth(1);
}

void drawLaneMarkers() {
    glColor3f(1.0f, 0.95f, 0.3f);
    for(const auto& marker : laneMarkers) {
        glBegin(GL_QUADS);
        glVertex2f(marker.x - 3, marker.y);
        glVertex2f(marker.x + 3, marker.y);
        glVertex2f(marker.x + 3, marker.y + 32);
        glVertex2f(marker.x - 3, marker.y + 32);
        glEnd();
    }
}

void drawPoliceCar() {
    float scale = getScaleForY(police.y);
    float w = police.width * scale;
    float h = police.height * scale;

    // Body
    glColor3f(0.05f, 0.08f, 0.65f);
    glBegin(GL_QUADS);
    glVertex2f(police.x - w/2, police.y);
    glVertex2f(police.x + w/2, police.y);
    glVertex2f(police.x + w/2, police.y + h * 0.65f);
    glVertex2f(police.x - w/2, police.y + h * 0.65f);
    glEnd();

    // Top cabin
    glColor3f(0.08f, 0.12f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(police.x - w * 0.35f, police.y + h * 0.65f);
    glVertex2f(police.x + w * 0.35f, police.y + h * 0.65f);
    glVertex2f(police.x + w * 0.3f, police.y + h);
    glVertex2f(police.x - w * 0.3f, police.y + h);
    glEnd();

    // Outline (Bresenham)
    glColor3f(1.0f, 1.0f, 1.0f);
    int x1 = (int)roundf(police.x - w/2);
    int x2 = (int)roundf(police.x + w/2);
    int y1 = (int)roundf(police.y);
    int y2 = (int)roundf(police.y + h);
    drawLineBresenham(x1, y1, x2, y1);
    drawLineBresenham(x2, y1, x2, y2);
    drawLineBresenham(x2, y2, x1, y2);
    drawLineBresenham(x1, y2, x1, y1);

    // Windshield
    glColor3f(0.5f, 0.7f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(police.x - w * 0.28f, police.y + h * 0.68f);
    glVertex2f(police.x + w * 0.28f, police.y + h * 0.68f);
    glVertex2f(police.x + w * 0.25f, police.y + h * 0.9f);
    glVertex2f(police.x - w * 0.25f, police.y + h * 0.9f);
    glEnd();

    // Police stripe
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(police.x - w * 0.4f, police.y + h * 0.42f);
    glVertex2f(police.x + w * 0.4f, police.y + h * 0.42f);
    glVertex2f(police.x + w * 0.4f, police.y + h * 0.48f);
    glVertex2f(police.x - w * 0.4f, police.y + h * 0.48f);
    glEnd();

    // Wheels (Midpoint Circle)
    glColor3f(0.08f, 0.08f, 0.08f);
    int wheelRadius = (int)std::max(4.0f, 6.0f * scale);
    drawFilledCircle((int)roundf(police.x - w * 0.35f), (int)roundf(police.y + h * 0.15f), wheelRadius);
    drawFilledCircle((int)roundf(police.x + w * 0.35f), (int)roundf(police.y + h * 0.15f), wheelRadius);

    // Siren lights (alternating)
    if (police.sirenOn) {
        if (police.sirenBlink < 15) {
            glColor3f(1.0f, 0.1f, 0.1f);
            drawFilledCircle((int)roundf(police.x - w * 0.2f), (int)roundf(police.y + h - 4.0f * scale), (int)std::max(3.0f, 5.0f * scale));
        } else {
            glColor3f(0.1f, 0.2f, 1.0f);
            drawFilledCircle((int)roundf(police.x + w * 0.2f), (int)roundf(police.y + h - 4.0f * scale), (int)std::max(3.0f, 5.0f * scale));
        }
    }
}

void drawCivilianCar(const Car& car) {
    if(!car.active) return;

    float scale = getScaleForY(car.y);
    float w = car.width * scale;
    float h = car.height * scale;

    // Color by type
    switch(car.color) {
        case 0: glColor3f(0.05f, 0.45f, 0.8f); break;   // Blue
        case 1: glColor3f(0.05f, 0.65f, 0.2f); break;   // Green
        case 2: glColor3f(0.9f, 0.75f, 0.05f); break;   // Yellow
        case 3: glColor3f(0.65f, 0.25f, 0.75f); break;  // Purple
        default: glColor3f(0.85f, 0.35f, 0.15f); break; // Orange
    }

    // Body
    glBegin(GL_QUADS);
    glVertex2f(car.x - w/2, car.y);
    glVertex2f(car.x + w/2, car.y);
    glVertex2f(car.x + w/2, car.y + h * 0.65f);
    glVertex2f(car.x - w/2, car.y + h * 0.65f);
    glEnd();

    // Top
    glBegin(GL_QUADS);
    glVertex2f(car.x - w * 0.35f, car.y + h * 0.65f);
    glVertex2f(car.x + w * 0.35f, car.y + h * 0.65f);
    glVertex2f(car.x + w * 0.3f, car.y + h);
    glVertex2f(car.x - w * 0.3f, car.y + h);
    glEnd();

    // Wheels
    glColor3f(0.08f, 0.08f, 0.08f);
    int wheelR = (int)std::max(2.0f, 5.0f * scale);
    drawFilledCircle((int)roundf(car.x - w * 0.35f), (int)roundf(car.y + h * 0.15f), wheelR);
    drawFilledCircle((int)roundf(car.x + w * 0.35f), (int)roundf(car.y + h * 0.15f), wheelR);

    // Window
    glColor3f(0.25f, 0.3f, 0.4f);
    glBegin(GL_QUADS);
    glVertex2f(car.x - w * 0.25f, car.y + h * 0.68f);
    glVertex2f(car.x + w * 0.25f, car.y + h * 0.68f);
    glVertex2f(car.x + w * 0.22f, car.y + h * 0.9f);
    glVertex2f(car.x - w * 0.22f, car.y + h * 0.9f);
    glEnd();

    // Bus special features
    if (car.type == 1) {
        glColor3f(0.95f, 0.95f, 0.95f);
        for (int i = 0; i < 3; ++i) {
            float wx = car.x - w * 0.3f + i * (w * 0.3f);
            glBegin(GL_QUADS);
            glVertex2f(wx, car.y + h * 0.5f);
            glVertex2f(wx + w * 0.15f, car.y + h * 0.5f);
            glVertex2f(wx + w * 0.15f, car.y + h * 0.62f);
            glVertex2f(wx, car.y + h * 0.62f);
            glEnd();
        }
    }

    // Tail lights
    glColor3f(0.7f, 0.05f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(car.x - w * 0.38f, car.y + h * 0.12f);
    glVertex2f(car.x - w * 0.32f, car.y + h * 0.12f);
    glVertex2f(car.x - w * 0.32f, car.y + h * 0.22f);
    glVertex2f(car.x - w * 0.38f, car.y + h * 0.22f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(car.x + w * 0.32f, car.y + h * 0.12f);
    glVertex2f(car.x + w * 0.38f, car.y + h * 0.12f);
    glVertex2f(car.x + w * 0.38f, car.y + h * 0.22f);
    glVertex2f(car.x + w * 0.32f, car.y + h * 0.22f);
    glEnd();
}

void drawCriminalCar() {
    if(!criminal.active) return;

    float scale = getScaleForY(criminal.y);
    float w = criminal.width * scale;
    float h = criminal.height * scale;

    // Body (aggressive red)
    glColor3f(0.95f, 0.05f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(criminal.x - w/2, criminal.y);
    glVertex2f(criminal.x + w/2, criminal.y);
    glVertex2f(criminal.x + w/2, criminal.y + h * 0.65f);
    glVertex2f(criminal.x - w/2, criminal.y + h * 0.65f);
    glEnd();

    // Top
    glColor3f(0.8f, 0.05f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(criminal.x - w * 0.35f, criminal.y + h * 0.65f);
    glVertex2f(criminal.x + w * 0.35f, criminal.y + h * 0.65f);
    glVertex2f(criminal.x + w * 0.3f, criminal.y + h);
    glVertex2f(criminal.x - w * 0.3f, criminal.y + h);
    glEnd();

    // Racing stripes
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(criminal.x - 4, criminal.y);
    glVertex2f(criminal.x + 4, criminal.y);
    glVertex2f(criminal.x + 4, criminal.y + h * 0.8f);
    glVertex2f(criminal.x - 4, criminal.y + h * 0.8f);
    glEnd();

    // Danger stripe
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(criminal.x - w * 0.4f, criminal.y + h * 0.45f);
    glVertex2f(criminal.x + w * 0.4f, criminal.y + h * 0.45f);
    glVertex2f(criminal.x + w * 0.4f, criminal.y + h * 0.5f);
    glVertex2f(criminal.x - w * 0.4f, criminal.y + h * 0.5f);
    glEnd();

    // Wheels
    glColor3f(0.05f, 0.05f, 0.05f);
    int wheelR = (int)std::max(3.0f, 6.0f * scale);
    drawFilledCircle((int)roundf(criminal.x - w * 0.35f), (int)roundf(criminal.y + h * 0.15f), wheelR);
    drawFilledCircle((int)roundf(criminal.x + w * 0.35f), (int)roundf(criminal.y + h * 0.15f), wheelR);

    // Tinted window
    glColor3f(0.1f, 0.1f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(criminal.x - w * 0.28f, criminal.y + h * 0.68f);
    glVertex2f(criminal.x + w * 0.28f, criminal.y + h * 0.68f);
    glVertex2f(criminal.x + w * 0.25f, criminal.y + h * 0.9f);
    glVertex2f(criminal.x - w * 0.25f, criminal.y + h * 0.9f);
    glEnd();
}

void drawBackground() {
    // Gradient sky
    glBegin(GL_QUADS);
    glColor3f(0.04f, 0.04f, 0.14f);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glColor3f(0.02f, 0.02f, 0.08f);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    // Stars
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    for (const auto &s : stars) {
        glVertex2i(s.first, s.second);
        glVertex2i(s.first + 1, s.second);
        glVertex2i(s.first, s.second + 1);
    }
    glEnd();

    // Buildings (left side)
    glColor3f(0.08f, 0.08f, 0.14f);
    for(int i = 0; i < 3; i++) {
        int x = 30 + i * 60;
        int h = 100 + (i % 3) * 80;
        glBegin(GL_QUADS);
        glVertex2f(x, 0);
        glVertex2f(x + 45, 0);
        glVertex2f(x + 45, h);
        glVertex2f(x, h);
        glEnd();

        // Windows
        glColor3f(1.0f, 0.9f, 0.4f);
        for(int j = 0; j < h/25; j++) {
            if((i + j) % 3 != 0) {
                glBegin(GL_QUADS);
                glVertex2f(x + 5, 10 + j * 20);
                glVertex2f(x + 15, 10 + j * 20);
                glVertex2f(x + 15, 16 + j * 20);
                glVertex2f(x + 5, 16 + j * 20);
                glEnd();
                glBegin(GL_QUADS);
                glVertex2f(x + 25, 10 + j * 20);
                glVertex2f(x + 35, 10 + j * 20);
                glVertex2f(x + 35, 16 + j * 20);
                glVertex2f(x + 25, 16 + j * 20);
                glEnd();
            }
        }
        glColor3f(0.08f, 0.08f, 0.14f);
    }

    // Buildings (right side)
    for(int i = 0; i < 3; i++) {
        int x = WIDTH - 175 + i * 60;
        int h = 120 + (i % 3) * 70;
        glBegin(GL_QUADS);
        glVertex2f(x, 0);
        glVertex2f(x + 45, 0);
        glVertex2f(x + 45, h);
        glVertex2f(x, h);
        glEnd();
    }
}

void drawUI() {
    // Control panel (left)
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(10, HEIGHT - 20);
    std::string title = "CONTROLS";
    for(char c: title) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glRasterPos2i(10, HEIGHT - 42);
    std::string line1 = "Arrows: Move";
    for(char c: line1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

    glRasterPos2i(10, HEIGHT - 60);
    std::string line2 = "S: Siren | P: Pause";
    for(char c: line2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

    glRasterPos2i(10, HEIGHT - 78);
    std::string line3 = "R: Restart | ESC: Exit";
    for(char c: line3) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

    // Siren indicator
    glRasterPos2i(10, HEIGHT - 105);
    std::string sirenText = "Siren: ";
    for(char c: sirenText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    if (police.sirenOn) {
        glColor3f(1.0f, 0.2f, 0.2f);
        drawFilledCircle(55, HEIGHT - 100, 5);
    } else {
        glColor3f(0.4f, 0.4f, 0.4f);
        drawFilledCircle(55, HEIGHT - 100, 5);
    }

    // Score panel (right side with background)
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex2f(WIDTH - 220, HEIGHT - 90);
    glVertex2f(WIDTH - 10, HEIGHT - 90);
    glVertex2f(WIDTH - 10, HEIGHT - 10);
    glVertex2f(WIDTH - 220, HEIGHT - 10);
    glEnd();
    glDisable(GL_BLEND);

    // Score
    glColor3f(1.0f, 1.0f, 0.2f);
    glRasterPos2i(WIDTH - 210, HEIGHT - 25);
    std::string scoreText = "Score: " + std::to_string(score);
    for(char c: scoreText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    // Criminals caught
    glColor3f(1.0f, 0.4f, 0.4f);
    glRasterPos2i(WIDTH - 210, HEIGHT - 48);
    std::string caughtText = "Caught: " + std::to_string(criminalsCaught);
    for(char c: caughtText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

    // Speed
    glColor3f(0.4f, 1.0f, 0.4f);
    glRasterPos2i(WIDTH - 210, HEIGHT - 70);
    std::string speedText = "Speed: " + std::to_string((int)(gameSpeed * 100)) + "%";
    for(char c: speedText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

    if(paused) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WIDTH, 0);
        glVertex2f(WIDTH, HEIGHT);
        glVertex2f(0, HEIGHT);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 1.0f, 0.0f);
        drawCenteredText(HEIGHT/2, GLUT_BITMAP_TIMES_ROMAN_24, "PAUSED");
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(HEIGHT/2 - 30, GLUT_BITMAP_HELVETICA_12, "Press P to Resume");
    }

    if(gameOver) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WIDTH, 0);
        glVertex2f(WIDTH, HEIGHT);
        glVertex2f(0, HEIGHT);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.1f, 0.1f);
        drawCenteredText(HEIGHT/2 + 50, GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER!");

        glColor3f(1.0f, 1.0f, 1.0f);
        std::string totalScore = "Final Score: " + std::to_string(score);
        drawCenteredText(HEIGHT/2 + 15, GLUT_BITMAP_HELVETICA_18, totalScore);

        std::string caughtTotal = "Criminals Caught: " + std::to_string(criminalsCaught);
        drawCenteredText(HEIGHT/2 - 10, GLUT_BITMAP_HELVETICA_18, caughtTotal);

        drawCenteredText(HEIGHT/2 - 45, GLUT_BITMAP_HELVETICA_18, "Press R to Restart");
    }
}

// ==================== GAME LOGIC ====================

// ==================== GAME LOGIC ====================

Car generateRandomCivilianTemplate() {
    Car car;
    car.type = randInt(0, 2);
    car.color = randInt(0, 4);
    car.active = true;

    if (car.type == 0) { // Regular car
        car.width = (float)randInt(30, 42);
        car.height = (float)randInt(45, 60);
        car.speed = 1.6f + randFloat(0.0f, 1.2f);
    } else if (car.type == 1) { // Bus
        car.width = (float)randInt(48, 62);
        car.height = (float)randInt(55, 70);
        car.speed = 0.9f + randFloat(0.0f, 0.6f);
    } else { // Bike
        car.width = (float)randInt(18, 24);
        car.height = (float)randInt(30, 42);
        car.speed = 2.6f + randFloat(0.0f, 1.0f);
    }
    return car;
}

bool canPlaceAt(float cx, float cy, float cw, float ch, int ignoreIndex = -1) {
    for (int i = 0; i < (int)civilianCars.size(); ++i) {
        if (i == ignoreIndex) continue;
        const Car &c = civilianCars[i];
        if (!c.active) continue;
        if (rectOverlap(cx, cy, cw, ch, c.x, c.y, c.width, c.height, 10.0f)) return false;
    }
    if (criminal.active) {
        if (rectOverlap(cx, cy, cw, ch, criminal.x, criminal.y, criminal.width, criminal.height, 10.0f)) return false;
    }
    return true;
}

void spawnCivilianAtIndex(int idx, int tries = 50) {
    if (idx < 0 || idx >= (int)civilianCars.size()) return;

    Car templateCar = generateRandomCivilianTemplate();
    Car car = templateCar;

    int attempt = 0;
    bool placed = false;

    while (attempt < tries && !placed) {
        int lane = randInt(0, LANE_COUNT - 1);
        car.lane = lane;
        float jitter = randFloat(-12.0f, 12.0f);
        car.x = laneX(lane) + jitter;

        // Vary spawn distances
        if (idx % 2 == 0) {
            car.y = HEIGHT + 30.0f + randFloat(0.0f, 180.0f) + attempt * 35.0f;
        } else {
            car.y = HEIGHT + 180.0f + randFloat(0.0f, 400.0f) + attempt * 55.0f;
        }

        if (canPlaceAt(car.x, car.y, car.width, car.height, idx)) {
            placed = true;
        }
        ++attempt;
    }

    // Fallback: push higher
    if (!placed) {
        for (int k = 0; k < 80; ++k) {
            car.y += 65.0f;
            if (canPlaceAt(car.x, car.y, car.width, car.height, idx)) {
                placed = true;
                break;
            }
        }
    }

    civilianCars[idx] = car;
}

void spawnCriminalOriginal() {
    float xleft = ROAD_LEFT + 60.0f;
    float xrange = (ROAD_RIGHT - ROAD_LEFT) - 120.0f;
    criminal.baseX = xleft + randFloat(0.0f, xrange);
    criminal.x = criminal.baseX;
    criminal.y = HEIGHT + 250.0f + randFloat(0.0f, 250.0f);
    criminal.width = BASE_VEH_W;
    criminal.height = BASE_VEH_H;
    criminal.speed = 2.4f + randFloat(0.0f, 0.4f);
    criminal.zigzag = randFloat(0.0f, 3.14f);
    criminal.active = true;
}

void initGame() {
    // Compute lane centers
    float segment = (ROAD_RIGHT - ROAD_LEFT) / (float)LANE_COUNT;
    for (int i = 0; i < LANE_COUNT; ++i) {
        LANE_X[i] = ROAD_LEFT + segment * 0.5f + i * segment;
    }

    // Lane markers
    laneMarkers.clear();
    for(int i = -100; i < HEIGHT + 200; i += 65) {
        laneMarkers.push_back({LANE_X[0], (float)i});
        laneMarkers.push_back({LANE_X[2], (float)i});
    }

    // Civilian cars
    civilianCars.clear();
    const int CIV_COUNT = 7;
    civilianCars.resize(CIV_COUNT);
    for (int i = 0; i < CIV_COUNT; ++i) {
        spawnCivilianAtIndex(i, 50);
    }

    // Reset police
    police.x = WIDTH / 2.0f;
    police.y = 80.0f;
    police.vx = 0.0f;
    police.leftPressed = police.rightPressed = false;
    police.sirenOn = true;
    police.sirenBlink = 0;
    police.maxVx = 250.0f;

    // Spawn criminal
    spawnCriminalOriginal();

    // Stars
    stars.clear();
    for (int i = 0; i < 100; ++i) {
        int sx = rand() % WIDTH;
        int sy = rand() % HEIGHT;
        if (sx >= (int)ROAD_LEFT - 15 && sx <= (int)ROAD_RIGHT + 15) {
            sx = (sx < WIDTH/2) ? sx - 180 : sx + 180;
            if (sx < 0) sx += WIDTH;
            if (sx >= WIDTH) sx -= WIDTH;
        }
        stars.emplace_back(sx, sy);
    }

    score = 0;
    scoreTimer = 0.0f;
    criminalsCaught = 0;
    gameOver = false;
    paused = false;
    gameSpeed = 1.0f;
}

// ==================== GAME UPDATE ====================

void updateGame() {
    if (gameOver || paused) return;

    const float dt = 16.0f / 1000.0f;

    // Gradually increase max speed (difficulty)
    police.maxVx += 2.5f * dt * 60.0f;
    if (police.maxVx > 650.0f) police.maxVx = 650.0f;

    // Police movement with acceleration and damping
    const float ACC = 1200.0f;
    const float DAMP = 6.0f;

    if (police.leftPressed && !police.rightPressed) {
        police.vx -= ACC * dt;
        if (police.vx < -police.maxVx) police.vx = -police.maxVx;
    } else if (police.rightPressed && !police.leftPressed) {
        police.vx += ACC * dt;
        if (police.vx > police.maxVx) police.vx = police.maxVx;
    } else {
        police.vx -= police.vx * DAMP * dt;
        if (fabsf(police.vx) < 0.5f) police.vx = 0.0f;
    }

    police.x += police.vx * dt;

    // Check if police hits road edge -> game over
    float halfw = police.width * 0.5f;
    if (police.x - halfw <= ROAD_LEFT || police.x + halfw >= ROAD_RIGHT) {
        gameOver = true;
        return;
    }

    // Update lane markers
    for(auto& marker : laneMarkers) {
        marker.y -= 3.5f * gameSpeed;
        if(marker.y < -100) marker.y = HEIGHT + 100;
    }

    // Move civilian cars and maintain lane alignment
    for (auto &car : civilianCars) {
        if (!car.active) continue;
        car.y -= car.speed * gameSpeed;
        float targetX = laneX(car.lane);
        float dx = targetX - car.x;
        car.x += dx * 0.08f; // Smooth return to lane
    }

    // Overlap resolution in lanes (vertical spacing)
    for (int lane = 0; lane < LANE_COUNT; ++lane) {
        std::vector<int> laneVehicles;
        for (int i = 0; i < (int)civilianCars.size(); ++i) {
            if (!civilianCars[i].active) continue;
            if (civilianCars[i].lane == lane) laneVehicles.push_back(i);
        }

        // Check if criminal is in this lane
        bool crimInLane = false;
        if (criminal.active) {
            float dist = fabsf(criminal.x - laneX(lane));
            if (dist < 70.0f) crimInLane = true;
        }

        struct VehicleItem {
            float y;
            float h;
            int type; // 0=civilian, 1=criminal
            int idx;
        };

        std::vector<VehicleItem> items;
        for (int i : laneVehicles) {
            items.push_back({civilianCars[i].y, civilianCars[i].height, 0, i});
        }
        if (crimInLane) {
            items.push_back({criminal.y, criminal.height, 1, -1});
        }

        std::sort(items.begin(), items.end(), [](const VehicleItem& a, const VehicleItem& b) {
            return a.y < b.y;
        });

        // Adjust spacing
        for (size_t j = 1; j < items.size(); ++j) {
            float prevY = items[j-1].y;
            float prevH = items[j-1].h;
            float curY = items[j].y;
            float curH = items[j].h;
            float minGap = std::max(prevH, curH) * 0.85f + 20.0f;

            if (curY - prevY < minGap) {
                float desiredY = prevY + minGap;
                if (items[j].type == 0) {
                    civilianCars[items[j].idx].y = desiredY;
                } else {
                    criminal.y = desiredY;
                }
                items[j].y = desiredY;
            }
        }
    }

    // Respawn civilians that fell off
    for (int i = 0; i < (int)civilianCars.size(); ++i) {
        Car &car = civilianCars[i];
        if (car.y < -350.0f) {
            spawnCivilianAtIndex(i, 40);
            score += 10;
        }
    }

    // Update criminal (zigzag pattern)
    if (criminal.active) {
        criminal.y -= criminal.speed * gameSpeed;
        criminal.zigzag += 0.10f * gameSpeed;
        criminal.x = criminal.baseX + sinf(criminal.zigzag) * 20.0f;

        // Keep criminal on road
        if (criminal.x < ROAD_LEFT + 35.0f) criminal.x = ROAD_LEFT + 35.0f;
        if (criminal.x > ROAD_RIGHT - 35.0f) criminal.x = ROAD_RIGHT - 35.0f;

        // Check if police caught criminal
        if (checkCollisionScaled(police.x, police.y, police.width, police.height,
                                 criminal.x, criminal.y, criminal.width, criminal.height)) {
            score += 50;
            criminalsCaught++;

            // Increase difficulty every 2 criminals caught
            if (criminalsCaught % 2 == 0) {
                police.maxVx += 45.0f;
                if (police.maxVx > 850.0f) police.maxVx = 850.0f;
                gameSpeed *= 1.15f;
                if (gameSpeed > 4.5f) gameSpeed = 4.5f;
            }

            spawnCriminalOriginal();
        }

        // Respawn if off screen
        if (criminal.y < -350.0f) {
            spawnCriminalOriginal();
        }
    }

    // Update siren blink
    police.sirenBlink = (police.sirenBlink + 1) % 30;

    // Check police vs civilian collisions
    for (const auto &car : civilianCars) {
        if (!car.active) continue;
        if (checkCollisionScaled(police.x, police.y, police.width, police.height,
                                 car.x, car.y, car.width, car.height)) {
            gameOver = true;
            return;
        }
    }

    // Score increment over time
    scoreTimer += dt;
    while (scoreTimer >= SCORE_INTERVAL) {
        score += 1;
        scoreTimer -= SCORE_INTERVAL;
    }
}

// ==================== GLUT CALLBACKS ====================

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBackground();
    drawRoad();
    drawLaneMarkers();

    // Sort and draw civilians (back to front)
    std::vector<const Car*> sorted;
    for (const auto &c : civilianCars) {
        if (c.active) sorted.push_back(&c);
    }
    std::sort(sorted.begin(), sorted.end(), [](const Car* a, const Car* b) {
        return a->y > b->y;
    });

    for (const auto* cp : sorted) {
        drawCivilianCar(*cp);
    }

    drawCriminalCar();
    drawPoliceCar();
    drawUI();

    glutSwapBuffers();
}

void timer(int value) {
    updateGame();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27: // ESC
            exit(0);
            break;
        case 's':
        case 'S':
            police.sirenOn = !police.sirenOn;
            break;
        case 'p':
        case 'P':
            paused = !paused;
            break;
        case 'r':
        case 'R':
            initGame();
            break;
    }
}

void specialKeyDown(int key, int x, int y) {
    if (gameOver) return;
    switch(key) {
        case GLUT_KEY_LEFT:
            police.leftPressed = true;
            break;
        case GLUT_KEY_RIGHT:
            police.rightPressed = true;
            break;
    }
}

void specialKeyUp(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            police.leftPressed = false;
            break;
        case GLUT_KEY_RIGHT:
            police.rightPressed = false;
            break;
    }
}

// ==================== INIT / MAIN ====================

void init() {
    glClearColor(0.04f, 0.04f, 0.14f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);

    srand((unsigned int)time(NULL));
    initGame();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Night Highway Patrol - Enhanced Edition");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
