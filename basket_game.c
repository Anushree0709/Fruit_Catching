#define STB_IMAGE_IMPLEMENTATION
#include <windows.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include "stb_image.h"

// Game Variables
int basketX = 0;
int fruitX = 0;
int fruitY = 300;
int score = 0;
int basketImgW = 1, basketImgH = 1; // Will be set at runtime

// Texture IDs
GLuint basketTex, appleTex, orangeTex, pineappleTex, strawberryTex;

// Fruit types
typedef enum { FRUIT_APPLE, FRUIT_ORANGE, FRUIT_PINEAPPLE, FRUIT_STRAWBERRY, FRUIT_COUNT } FruitType;
FruitType currentFruit = FRUIT_APPLE;

// 1. Texture Loader with Clamping Fix
GLuint loadTexture(const char* filename, int* outW, int* outH) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(1); // Standard for OpenGL
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        if (outW) *outW = width;
        if (outH) *outH = height;
        stbi_image_free(data);
    } else {
        printf("Failed to load texture: %s\n", filename);
        stbi_image_free(data);
    }
    return textureID;
}

// 2. Drawing Function with Correct Vertex Mapping
void drawObject(GLuint textureID, int x, int y, int w, int h) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glColor3f(1.0f, 1.0f, 1.0f); 
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(x - w/2, y - h/2); // Bottom Left
        glTexCoord2f(1.0f, 0.0f); glVertex2i(x + w/2, y - h/2); // Bottom Right
        glTexCoord2f(1.0f, 1.0f); glVertex2i(x + w/2, y + h/2); // Top Right
        glTexCoord2f(0.0f, 1.0f); glVertex2i(x - w/2, y + h/2); // Top Left
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

// 3. UI: Draw Score
void drawScore() {
    char scoreStr[20];
    sprintf(scoreStr, "Score: %d", score);
    glColor3f(0.0, 0.0, 0.0); // Black text for the UI
    glRasterPos2i(-280, 260); 
    for (int i = 0; scoreStr[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreStr[i]);
    }
}

void myReshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w > h) {
        gluOrtho2D(-300.0 * w / h, 300.0 * w / h, -300.0, 300.0);
    } else {
        gluOrtho2D(-300.0, 300.0, -300.0 * h / w, 300.0 * h / w);
    }
    glMatrixMode(GL_MODELVIEW);
}

void myInit() {
    glClearColor(1.0, 1.0, 1.0, 0.0); // White background

    // Transparency support for PNGs
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void myDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate basket draw size based on image aspect ratio
    float basketDrawW = 160; // You can adjust this for desired size
    float aspect = (float)basketImgW / (float)basketImgH;
    float basketDrawH = basketDrawW / aspect;

    // Draw the basket
    drawObject(basketTex, basketX, -270, (int)basketDrawW, (int)basketDrawH);

    // Draw the current fruit
    GLuint fruitTex = appleTex;
    switch (currentFruit) {
        case FRUIT_APPLE:  fruitTex = appleTex; break;
        case FRUIT_ORANGE: fruitTex = orangeTex; break;
        case FRUIT_PINEAPPLE: fruitTex = pineappleTex; break;
        case FRUIT_STRAWBERRY: fruitTex = strawberryTex; break;
    }
    drawObject(fruitTex, fruitX, fruitY, 40, 40);

    drawScore();

    // Logic: Falling
    fruitY -= 7;

    // Logic: Collision (adjusted for new basket width)
    int basketHalfW = (int)(basketDrawW/2);
    if (fruitY <= -240 && fruitX >= basketX - basketHalfW && fruitX <= basketX + basketHalfW) {
        score++;
        fruitY = 300;
        fruitX = (rand() % 500) - 250;
        currentFruit = (FruitType)(rand() % FRUIT_COUNT);
    } 
    // Logic: Game Over
    else if (fruitY < -300) {
        printf("Game Over! Final Score: %d\n", score);
        exit(0);
    }

    glutSwapBuffers();
}

void timer(int v) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:  if (basketX > -240) basketX -= 25; break;
        case GLUT_KEY_RIGHT: if (basketX < 240)  basketX += 25; break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(600, 600);
    
    // 1. CREATE THE WINDOW FIRST
    int windowID = glutCreateWindow("Fruit Catcher OpenGL");

    // 2. NOW YOU CAN POSITION IT
    glutPositionWindow(100, 100);

    // 3. INITIALIZE OPENGL STATES
    myInit();

    // 4. LOAD TEXTURES (Window must exist for these to bind)
    basketTex = loadTexture("basket.jpg", &basketImgW, &basketImgH);
    appleTex = loadTexture("apple.jpg", NULL, NULL);
    orangeTex = loadTexture("orange.jpg", NULL, NULL);
    pineappleTex = loadTexture("pineapple.jpg", NULL, NULL);
    strawberryTex = loadTexture("strawberry.jpg", NULL, NULL);

    // 5. REGISTER CALLBACKS
    glutDisplayFunc(myDisplay);
    glutReshapeFunc(myReshape);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);
    
    glutMainLoop();
    return 0;
}

