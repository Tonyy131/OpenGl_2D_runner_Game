#include <stdlib.h>
#include <glut.h>
#include <math.h>
#include <string.h>
#include <cstdio>
#include <time.h>
#include <iostream>
#include <irrKlang.h>


void Anim(void);
// Window dimensions
const int windowWidth = 800;
const int windowHeight = 600;
const char* windowTitle = "2D Infinite Runner";
irrklang::ISoundEngine* engine2 = irrklang::createIrrKlangDevice();

bool isFullscreen = true;  // Start in fullscreen mode

// Player state variables
bool isJumping = false;
bool isDucking = false;
float playerY = 150;  // Player's Y position (ground level)
float jumpHeight = 100;  // Height of the jump
float jumpSpeed = 5;  // Speed of jumping
float playerX = 300;  // Player's X position
float playerHeight = 50;
int lives = 5;
int score = 0;
int timer = 30;
bool done = false;



// Obstacle structure
struct Obstacle {
    float x;
    float y;
    bool isMissile; // True if missile, false if box
    bool collisionDetected = false; // true if obstacle is already detected
    Obstacle* next;
};
// Head of the linked list of obstacles
Obstacle* obstacleHead = nullptr;

double collecatbleRotation = 0.0;


float obstacleSpeed = 5.0f;  // Initial speed of obstacles
float speedIncrement = 0.5f;  // Amount to increase the speed by

struct Collectable {
    float x;
    float y;
    bool isCollected = false;
    Collectable* next;
    bool unique;
};

Collectable* collectableHead = nullptr;

struct PowerUp {
    float x;
    float y;
    bool isCollected = false;
    bool type;
    PowerUp* next;
};
float powerUpHeight = 100;
PowerUp* powerupHead = nullptr;
int powerupIsActive = 0;

//to check if game is over
bool isOver = false;

// Function to initialize OpenGL
void initOpenGL() {
    // Set the background color (R, G, B, A)
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);  // Light blue sky background
    glMatrixMode(GL_PROJECTION);            // Setup projection matrix
    glLoadIdentity();
    gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);  // Set 2D orthographic projection
}

int randomInRange(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void TimeAndScore(float x, float y, const char* text) {
    glColor3f(0.0f, 0.0f, 0.0f); // Set text color to white 
    glRasterPos2f(x, y);


    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *text);
        text++;
    }
}

void drawHeart(float centerX, float centerY, float size) {
    glBegin(GL_POLYGON);  // Begin drawing the heart as a filled polygon

    // Loop through values of t to generate vertices
    for (float t = 0.0f; t <= 2 * 3.14; t += 0.01f) {
        float x = 16 * pow(sin(t), 3);  // Parametric equation for x
        float y = 13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t);  // Parametric equation for y

        // Scale and translate the heart to desired position
        glVertex2f(centerX + x * size, centerY + y * size);
    }

    glEnd();  // End drawing the heart
}


// Function to draw the player model using shapes
void drawPlayer() {
    glPushMatrix();  // Save the current transformation matrix

    // Translate the shape to the player's position
    glTranslatef(playerX, playerY, 0);

    // If ducking, rotate the shape to simulate sliding
    if (isDucking) {
        // glRotatef(30.0f, 0.0f, 0.0f, 1.0f);  // Rotate by 30 degrees around the Z-axis
        playerHeight = 10;
    }
    else {
        playerHeight = 50;
    }

    glColor3f(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(-25, -35); // Bottom left
    glVertex2f(25, -35);  // Bottom right
    glVertex2f(25, playerHeight - 15);   // Top right
    glVertex2f(-25, playerHeight - 15);  // Top left
    glEnd();

    glLineWidth(5.0f);

    glBegin(GL_LINES);
    glVertex2f(-20, -50); // Start point of line
    glVertex2f(-20, -35); // End point of line
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(20, -50); // Start point of line
    glVertex2f(20, -35); // End point of line
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(25, playerHeight - 15);
    glVertex2f(-25, playerHeight - 15);
    glVertex2f(0, playerHeight);
    glEnd();
    glColor3f(1, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(0, playerHeight - 15);
    glVertex2f(-10, playerHeight - 20);
    glVertex2f(0, playerHeight - 30);
    glVertex2f(10, playerHeight - 20);
    glEnd();
    glColor3f(1, 1, 1);
    glBegin(GL_POINTS);
    glVertex2f(-5, playerHeight - 20);
    glVertex2f(5, playerHeight - 20);
    glEnd();

    glPopMatrix();  // Restore the previous transformation matrix
}

// Function to draw obstacles
void drawObstacles() {
    Obstacle* current = obstacleHead;

    while (current != nullptr) {
        glPushMatrix();
        glTranslatef(current->x, current->y, 0);

        if (current->isMissile) {
            float scale = 100; // Scale factor for size
            glRotatef(90, 0, 0, 1);
            // Draw the body of the missile using a rectangle
            glBegin(GL_QUADS);
            glColor3f(0.8f, 0.0f, 0.0f); // Red color for the missile body
            glVertex2f(-0.05f * scale, -0.3f * scale); // Bottom left
            glVertex2f(0.05f * scale, -0.3f * scale);  // Bottom right
            glVertex2f(0.05f * scale, 0.3f * scale);   // Top right
            glVertex2f(-0.05f * scale, 0.3f * scale);  // Top left
            glEnd();

            // Draw the nose of the missile using a triangle
            glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for the nose
            glVertex2f(-0.05f * scale, 0.3f * scale);    // Bottom left
            glVertex2f(0.05f * scale, 0.3f * scale);     // Bottom right
            glVertex2f(0.0f, 0.5f * scale);               // Top point
            glEnd();

            // Draw the fins of the missile using triangles
            glBegin(GL_TRIANGLES);
            glColor3f(0.0f, 0.0f, 0.0f); // Black color for the fins
            glVertex2f(-0.05f * scale, -0.3f * scale);   // Bottom left
            glVertex2f(-0.15f * scale, -0.2f * scale);   // Bottom left fin point
            glVertex2f(-0.05f * scale, -0.2f * scale);   // Top left fin point
            glEnd();

            glBegin(GL_TRIANGLES);
            glVertex2f(0.05f * scale, -0.3f * scale);    // Bottom right
            glVertex2f(0.15f * scale, -0.2f * scale);    // Bottom right fin point
            glVertex2f(0.05f * scale, -0.2f * scale);     // Top right fin point
            glEnd();
        }
        else {
            // Draw the box using quads
            glBegin(GL_QUADS);
            glColor3f(0, 0, 1.0f); // blue color for the box
            glVertex2f(-25, -35); // Bottom left
            glVertex2f(25, -35);  // Bottom right
            glVertex2f(25, 15);   // Top right
            glVertex2f(-25, 15);  // Top left
            glEnd();

            // Draw edges using lines
            glBegin(GL_LINE_LOOP);
            glColor3f(0.0f, 0.0f, 0.0f); // Black color for edges
            glVertex2f(-25, -35);
            glVertex2f(25, -35);
            glVertex2f(25, 15);
            glVertex2f(-25, 15);
            glEnd();
        }

        glPopMatrix();

        current = current->next;
    }
}

// function to darw collectables
void drawCollectables() {
    Collectable* current = collectableHead;

    while (current != nullptr) {
        glPushMatrix();
        glTranslatef(current->x, current->y, 0);
        glRotated(collecatbleRotation, 0, 0, 1); // collectable rotation
        glScalef(50, 50, 1);
        
       
        if (current->unique) {
            glColor3f(1, 0, 0);
            
            
        }
        else {
            glColor3f(1, 1, 0); // Yellow star
            
        }

        float radius = 0.5f;
        float centerX = 0.0f, centerY = 0.0f;

        // Draw Hexagon using triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < 6; ++i) {
            float theta1 = 2.0f * 3.14 * i / 6; // Angle for first vertex
            float theta2 = 2.0f * 3.14 * (i + 1) / 6; // Angle for second vertex

            glVertex2f(centerX, centerY); // Center of hexagon
            glVertex2f(centerX + radius * cos(theta1), centerY + radius * sin(theta1)); // First vertex
            glVertex2f(centerX + radius * cos(theta2), centerY + radius * sin(theta2)); // Second vertex
        }
        glEnd();

        // Draw Hexagon outline using lines
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 6; ++i) {
            float theta = 2.0f * 3.14 * i / 6;
            glVertex2f(centerX + radius * cos(theta), centerY + radius * sin(theta));
        }
        glEnd();

        // Draw center point
        glBegin(GL_POINTS);
        glVertex2f(centerX, centerY);
        glEnd();
        glPopMatrix();

        current = current->next;
    }
}

void drawPowerUps() {
    PowerUp* current = powerupHead;

    while (current != nullptr) {
        glPushMatrix();
        glTranslatef(current->x, powerUpHeight, 0);
        glScalef(50, 50, 1); // Scale
        if (current->type) {
            // Draw the top triangle of the hourglass
            glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.8f, 0.0f); // Yellow color
            glVertex2f(-0.3f, 0.5f); // Left vertex
            glVertex2f(0.0f, 0.2f);  // Apex
            glVertex2f(0.3f, 0.5f);  // Right vertex
            glEnd();

            // Draw the bottom triangle of the hourglass
            glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.8f, 0.0f); // Yellow color
            glVertex2f(-0.3f, -0.5f); // Left vertex
            glVertex2f(0.0f, -0.2f);  // Apex
            glVertex2f(0.3f, -0.5f);  // Right vertex
            glEnd();

            // Draw outline using lines
            glBegin(GL_LINE_LOOP);
            glColor3f(0.0f, 0.0f, 0.0f); // Black outline
            glVertex2f(-0.3f, 0.5f);
            glVertex2f(0.0f, 0.2f);
            glVertex2f(0.3f, 0.5f);
            glEnd();

            glBegin(GL_LINE_LOOP);
            glColor3f(0.0f, 0.0f, 0.0f); // Black outline
            glVertex2f(-0.3f, -0.5f);
            glVertex2f(0.0f, -0.2f);
            glVertex2f(0.3f, -0.5f);
            glEnd();

            // Draw additional decorative elements using quads (speed lines or sand)
            glBegin(GL_QUADS);
            glColor3f(1.0f, 0.6f, 0.0f); // Light yellow for sand or decoration
            glVertex2f(-0.1f, 0.2f); // Left top
            glVertex2f(0.1f, 0.2f);  // Right top
            glVertex2f(0.1f, 0.0f);  // Right bottom
            glVertex2f(-0.1f, 0.0f); // Left bottom
            glEnd();

            // Draw points for energy effect
            glBegin(GL_POINTS);
            glColor3f(1.0f, 0.0f, 0.0f); // Red color for sparkles
            glVertex2f(-0.1f, 0.0f);
            glVertex2f(0.1f, 0.0f);
            glVertex2f(-0.2f, -0.2f);
            glVertex2f(0.2f, -0.2f);
            glEnd();
        }
        else {
            glColor3f(1.0f, 0.0f, 0.0f); // Red color for the arrow
            float petalRadius = 0.3f;
            float centerX = 0.0f, centerY = 0.0f;

            // Draw flower stem as a quad
            glBegin(GL_QUADS);
            glColor3f(0.0f, 0.8f, 0.0f); // Green color for the stem
            glVertex2f(-0.05f, -1.0f);
            glVertex2f(0.05f, -1.0f);
            glVertex2f(0.05f, 0.0f);
            glVertex2f(-0.05f, 0.0f);
            glEnd();

            // Draw petals using triangles
            glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.5f, 0.0f); // Orange color for the petals
            for (int i = 0; i < 6; ++i) {
                float angle = 2.0f * 3.14 * i / 6;

                glVertex2f(centerX, centerY); // Center of flower
                glVertex2f(centerX + petalRadius * cos(angle), centerY + petalRadius * sin(angle)); // Petal outer point
                glVertex2f(centerX + petalRadius * cos(angle + 3.14 / 6), centerY + petalRadius * sin(angle + 3.14 / 6)); // Adjacent petal outer point
            }
            glEnd();

            // Draw flower outline using lines
            glBegin(GL_LINE_LOOP);
            glColor3f(0.0f, 0.0f, 0.0f); // Black color for outline
            for (int i = 0; i < 6; ++i) {
                float angle = 2.0f * 3.14 * i / 6;
                glVertex2f(centerX + petalRadius * cos(angle), centerY + petalRadius * sin(angle)); // Petal outer point
            }
            glEnd();

            // Draw center point
            glBegin(GL_POINTS);
            glColor3f(1.0f, 0.0f, 0.0f); // Red color for center point
            glVertex2f(centerX, centerY);
            glEnd();
        }

        glPopMatrix();

        current = current->next;
    }
}


// Function to handle jumping
void updatePlayer() {
    if (isJumping) {
        playerY += jumpSpeed;  // Move player up
        if (playerY >= jumpHeight + 150) {  // Reached peak
            isJumping = false;  // Start falling
        }
    }
    else if (playerY > 150 && powerupIsActive != 2) {  // Falling
        playerY -= jumpSpeed;  // Move player down
        if (playerY < 150) {  // Reset to ground level
            playerY = 150;
        }
    }

}

// Function to check if the player collides with the obstacle
bool playerCollidesWithObstacle(Obstacle* obstacle) {
    // Check for approximate collision based on proximity
    if (obstacle->isMissile) {
        const int collisionDistance = 40;
        if (abs(static_cast<int>(playerX - obstacle->x)) < collisionDistance &&
            (abs(static_cast<int>(playerY + playerHeight - obstacle->y)) < collisionDistance || (playerY > obstacle->y && powerupIsActive != 2))) {
            return true;
        }
        return false;
    }
    else {
        const int collisionDistance = 60;

        if (abs(static_cast<int>(playerX - obstacle->x)) < collisionDistance &&
            abs(static_cast<int>(playerY - obstacle->y)) < collisionDistance) {
            return true;
        }
        return false;
    }
}

//funtction to check if the player collects a collectabel
bool playerCollectsCollectable(Collectable* collectabele) {
    const int collisionDistance = 50;
    if (abs(static_cast<int>(playerX - collectabele->x)) < collisionDistance &&
        abs(static_cast<int>(playerY - collectabele->y)) < collisionDistance) {
        return true;
    }
    return false;
}

bool playerCollectsPowerup(PowerUp* powerup) {
    const int collisionDistance = 60;
    if (abs(static_cast<int>(playerX - powerup->x)) < collisionDistance &&
        abs(static_cast<int>(playerY - powerUpHeight)) < collisionDistance) {
        return true;
    }
    return false;
}

void checkCollisions() {
    Obstacle* current = obstacleHead;
    Obstacle* previous = nullptr;

    // Loop through all obstacles
    while (current != nullptr) {
        // Check if the player collides with the current obstacle
        if (!current->collisionDetected && playerCollidesWithObstacle(current)) {
            if (current->isMissile) {
                engine2->play2D("media/collision2.mp3", false);
            }
            else {
                engine2->play2D("media/collision.mp3", false);
            }
            // Deduct 1 life on collision

            if (lives > 0) {
                lives--;
                score--;
                current->collisionDetected = true;

                if (lives <= 0) {
                    isOver = true;
                }
            }

            playerX -= 70;
            current->x += 70;
            current->collisionDetected = false;

            // Since we removed an obstacle, exit the loop early
            break;
        }
        else {
            // Move to the next obstacle
            previous = current;
            current = current->next;
        }
    }
}


void checkCollects() {
    Collectable* current = collectableHead;
    Collectable* previous = nullptr;

    // Loop through all collectables
    while (current != nullptr) {
        // Check if the player collects the current collectable
        if (!current->isCollected && playerCollectsCollectable(current)) {
            // increase score
            if (current->unique) {
                score += 50;
            }
            else {
                score += 10;
            }
            if (!engine2) {
                return;
            }
            engine2->play2D("media/collect.mp3", false);

            // Remove the collected collectable from the linked list
            if (previous == nullptr) {
                // Removing the head of the list
                collectableHead = current->next;
                delete current;
                current = collectableHead;
            }
            else {
                // Removing an collectable in the middle or end
                previous->next = current->next;
                delete current;
                current = previous->next;
            }
            // Since we removed an collectable, exit the loop early
            break;
        }
        else {
            // Move to the next collectable
            previous = current;
            current = current->next;
        }
    }
}


void returnSpeed(int s) {
    // Restore original speed
    obstacleSpeed = s / 100.0f; // Ensure we are using float division
    jumpSpeed = obstacleSpeed;
    powerupIsActive = 0; // Deactivate the power-up
}

void returnPosition(int s) {
    // Restore original position
    playerY = s / 100.0f; // Ensure we are using float division
    powerupIsActive = 0; // Deactivate the power-up
}

void checkCollectsPowerup() {
    PowerUp* current = powerupHead;
    PowerUp* previous = nullptr;

    // Loop through all power-ups
    while (current != nullptr) {
        // Check if the player collects the current power-up
        if (!current->isCollected && playerCollectsPowerup(current)) {


            if (current->type) { // Speed power-up
                powerupIsActive = 1;
                powerupHead = nullptr;
                float s = obstacleSpeed * 100.0f; // Store original speed scaled
                obstacleSpeed = 2.5f; // Set to new speed
                jumpSpeed = 2.5f;
                glutTimerFunc(5 * 1000, returnSpeed, static_cast<int>(s)); // Restore original speed
            }
            else { // Position power-up
                powerupIsActive = 2;
                powerupHead = nullptr;
                float y = playerY * 100.0f; // Store original position scaled
                playerY = 350; // Set to new position
                glutTimerFunc(5 * 1000, returnPosition, static_cast<int>(y)); // Restore original position
            }


            break;
        }
        else {
            // Move to the next power-up
            previous = current;
            current = current->next;
        }
    }
}





void updateObstacles() {
    Obstacle* current = obstacleHead;
    Obstacle* previous = nullptr;

    while (current != nullptr) {
        // Move obstacles left
        current->x -= obstacleSpeed;


        // If the obstacle moves off the screen, remove it
        if (current->x < 0) {
            // Remove obstacle from the linked list
            if (previous == nullptr) {
                // Removing the head of the list
                obstacleHead = current->next;
                delete current;
                current = obstacleHead;
            }
            else {
                // Removing an obstacle in the middle or end
                previous->next = current->next;
                delete current;
                current = previous->next;
            }
        }
        else {
            previous = current;
            current = current->next;
        }
    }
}

void updateCollectables() {
    Collectable* current = collectableHead;
    Collectable* previous = nullptr;

    while (current != nullptr) {
        // Move collectables left
        current->x -= obstacleSpeed;


        // If the collectable moves off the screen, remove it
        if (current->x < 0) {
            // Remove collectable from the linked list
            if (previous == nullptr) {
                // Removing the head of the list
                collectableHead = current->next;
                delete current;
                current = collectableHead;
            }
            else {
                // Removing a collectable in the middle or end
                previous->next = current->next;
                delete current;
                current = previous->next;
            }
        }
        else {
            previous = current;
            current = current->next;
        }
    }
}

void updatePowerups() {
    PowerUp* current = powerupHead;
    PowerUp* previous = nullptr;

    while (current != nullptr) {
        // Move powerups left
        current->x -= obstacleSpeed;


        // If the powerup moves off the screen, remove it
        if (current->x < 0) {
            // Remove powerup from the linked list
            if (previous == nullptr) {
                // Removing the head of the list
                powerupHead = current->next;
                delete current;
                current = powerupHead;
            }
            else {
                // Removing a powrrup in the middle or end
                previous->next = current->next;
                delete current;
                current = previous->next;
            }
        }
        else {
            previous = current;
            current = current->next;
        }
    }
}
//Function to create a new obstacle
void createObstacle() {
    Obstacle* newObstacle = new Obstacle;

    newObstacle->isMissile = (randomInRange(0, 1) == 1);

    if (newObstacle->isMissile) {
        newObstacle->y = 200;  // Missile flies in the air
    }
    else {
        newObstacle->y = 135;  // Box on the ground
    }

    newObstacle->x = windowWidth;  // Start at the right edge of the screen

    // Check if there's enough distance from the last obstacle
    if (obstacleHead != nullptr) {
        Obstacle* current = obstacleHead;
        while (current->next != nullptr) {
            current = current->next;
        }

        // If the last obstacle is too close, push the new obstacle further away
        if (newObstacle->x - current->x < 500) {
            newObstacle->x = current->x + 500; // Set min distance between obstacles
        }
    }

    // Check distance from all collectables
    Collectable* collectableCurrent = collectableHead;
    while (collectableCurrent != nullptr) {
        float distance = fabs(newObstacle->x - collectableCurrent->x);  //fabs is float absolute
        if (distance < 500) { // Set the minimum distance between obstacles and collectables
            newObstacle->x = collectableCurrent->x + 500; // Adjust obstacle position
        }
        collectableCurrent = collectableCurrent->next;
    }

    // Check distance from all powerups
    PowerUp* powerupCurrent = powerupHead;
    while (powerupCurrent != nullptr) {
        float distance = fabs(newObstacle->x - powerupCurrent->x);
        if (distance < 500) { // Set the minimum distance between collectables and obstacles
            newObstacle->x = powerupCurrent->x + 200; // Adjust collectable position
        }
        powerupCurrent = powerupCurrent->next;
    }

    if (obstacleHead != nullptr) {
        Obstacle* lastObstacle = obstacleHead;
        while (lastObstacle->next != nullptr) {
            lastObstacle = lastObstacle->next;
        }
        lastObstacle->next = newObstacle; // Add the new obstacle to the end of the list
    }
    else {
        obstacleHead = newObstacle; // If it's the first obstacle, just add it
    }

    newObstacle->next = nullptr;  // New obstacle is the last one in the list
}

// Function to create a new collectable
void createCollectable() {
    Collectable* newCollectable = new Collectable;

    int rand = (randomInRange(0, 1) == 1);

    if (!done) {
        newCollectable->unique = true;
        done = true;
    }
    else {
        newCollectable->unique = false;
    }
    if (rand == 0) {
        newCollectable->y = 250;  // Air
    }
    else {
        newCollectable->y = 135;  // Ground
    }

    newCollectable->x = windowWidth;  // Start at the right edge of the screen

    // Check if there's enough distance from the last collectable
    if (collectableHead != nullptr) {
        Collectable* current = collectableHead;
        while (current->next != nullptr) {
            current = current->next;
        }

        // If the last collectable is too close, push the new collectable further away
        if (newCollectable->x - current->x < 500) {
            newCollectable->x = current->x + 500; // Set min distance between collectables
        }
    }

    // Check distance from all obstacles
    Obstacle* obstacleCurrent = obstacleHead;
    while (obstacleCurrent != nullptr) {
        float distance = fabs(newCollectable->x - obstacleCurrent->x);
        if (distance < 500) { // Set the minimum distance between collectables and obstacles
            newCollectable->x = obstacleCurrent->x + 500; // Adjust collectable position
        }
        obstacleCurrent = obstacleCurrent->next;
    }
    // Check distance from all powerups
    PowerUp* powerupCurrent = powerupHead;
    while (powerupCurrent != nullptr) {
        float distance = fabs(newCollectable->x - powerupCurrent->x);
        if (distance < 500) { // Set the minimum distance between collectables and obstacles
            newCollectable->x = powerupCurrent->x + 200; // Adjust collectable position
        }
        powerupCurrent = powerupCurrent->next;
    }

    if (collectableHead != nullptr) {
        Collectable* lastCollectable = collectableHead;
        while (lastCollectable->next != nullptr) {
            lastCollectable = lastCollectable->next;
        }
        lastCollectable->next = newCollectable; // Add the new collectable to the end of the list
    }
    else {
        collectableHead = newCollectable; // If it's the first collectable, just add it
    }

    newCollectable->next = nullptr;  // New collectable is the last one in the list
}

void createPowerup() {
    if (!powerupIsActive) {
        PowerUp* newPowerUp = new PowerUp;


        newPowerUp->type = (randomInRange(0, 1) == 1);

        newPowerUp->y = powerUpHeight;

        newPowerUp->x = windowWidth;  // Start at the right edge of the screen

        // Check if there's enough distance from the last powerup
        if (powerupHead != nullptr) {
            PowerUp* current = powerupHead;
            while (current->next != nullptr) {
                current = current->next;
            }

            // If the last powerup is too close, push the new powerup further away
            if (newPowerUp->x - current->x < 900) {
                newPowerUp->x = current->x + 900; // Set min distance between powerup
            }
        }

        // Check distance from all collectables
        Collectable* collectableCurrent = collectableHead;
        while (collectableCurrent != nullptr) {
            float distance = fabs(newPowerUp->x - collectableCurrent->x);  //fabs is float absolute
            if (distance < 500) { // Set the minimum distance between powerups and collectables
                newPowerUp->x = collectableCurrent->x + 200; // Adjust powerup position
            }
            collectableCurrent = collectableCurrent->next;
        }

        // Check distance from all obstacles
        Obstacle* obstacleCurrent = obstacleHead;
        while (obstacleCurrent != nullptr) {
            float distance = fabs(newPowerUp->x - obstacleCurrent->x);
            if (distance < 500) { // Set the minimum distance between powerups and obstacles
                newPowerUp->x = obstacleCurrent->x + 200; // Adjust powerup position
            }
            obstacleCurrent = obstacleCurrent->next;
        }

        if (powerupHead != nullptr) {
            PowerUp* lastPowerup = powerupHead;
            while (lastPowerup->next != nullptr) {
                lastPowerup = lastPowerup->next;
            }
            lastPowerup->next = newPowerUp; // Add the new collectable to the end of the list
        }
        else {
            powerupHead = newPowerUp; // If it's the first collectable, just add it
        }

        newPowerUp->next = nullptr;  // New collectable is the last one in the list
    }
}


void drawGameOver() {
    glColor3f(1.0f, 0.0f, 0.0f);  // Set color to red
    glRasterPos2f(windowWidth / 2 - 100, windowHeight / 2 + 20);
    obstacleSpeed = 0.0f;
    obstacleHead = nullptr;
    if (lives <= 0) {
        TimeAndScore(windowWidth / 2 - 100, windowHeight / 2, "Game Over!");
        engine2->play2D("media/lose.mp3", false);
    }
    else {
        if (timer <= 0) {
            TimeAndScore(windowWidth / 2 - 100, windowHeight / 2, "Game End!");
            engine2->play2D("media/win.mp3", false);
        }
    }

    glColor3f(1.0f, 1.0f, 1.0f);  // White color for score
    char scoreText[50];
    sprintf(scoreText, "Final Score: %d", score);
    TimeAndScore(windowWidth / 2 - 100, windowHeight / 2 - 20, scoreText);

    glColor3f(1.0f, 1.0f, 1.0f);  // Instructions
    TimeAndScore(windowWidth / 2 - 100, windowHeight / 2 - 60, "Press 'R' to Restart");
}
// Global variable to track cloud position
float cloudPosition = 0.0f;

void drawCloud(float x, float y, float size) {
    glColor3f(1.0f, 1.0f, 1.0f);  // White color for clouds

    // Draw cloud as a set of overlapping circles (simplified)
    for (int i = 0; i < 3; i++) {
        glBegin(GL_TRIANGLE_FAN);
        for (int j = 0; j <= 100; j++) {
            float angle = 2.0f * 3.1415926f * j / 100;
            float cloudX = (size + i * 10) * cos(angle);
            float cloudY = size * sin(angle);
            glVertex2f(x + cloudX, y + cloudY);
        }
        glEnd();
    }
}

void updateClouds() {
    // Update the cloud position (move to the left)
    cloudPosition -= 0.5f;  // Adjust speed of clouds

    // Wrap clouds back to the right side of the screen
    if (cloudPosition < -windowWidth) {
        cloudPosition = 0.0f;
    }
}

void display() {
    // Clear the screen with the background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the sky with a light blue background
    glBegin(GL_QUADS);
    glColor3f(0.7f, 0.9f, 1.0f);  // Light blue color
    glVertex2f(0.0f, windowHeight);  // Top left corner
    glVertex2f(windowWidth, windowHeight);  // Top right corner
    glColor3f(0.5f, 0.8f, 1.0f);  // Slightly darker blue towards the bottom
    glVertex2f(windowWidth, 0.0f);  // Bottom right corner
    glVertex2f(0.0f, 0.0f);  // Bottom left corner
    glEnd();

    // Draw moving clouds
    drawCloud(200 + cloudPosition, windowHeight - 150, 30.0f); // Cloud 1
    drawCloud(600 + cloudPosition, windowHeight - 120, 25.0f); // Cloud 2
    drawCloud(1000 + cloudPosition, windowHeight - 170, 35.0f); // Cloud 3

    updateClouds();  // Update cloud positions

    glColor3f(0.0f, 0.0f, 0.0f);  // Black color for the frame
    glBegin(GL_QUADS);
    glVertex2f(0.0f, windowHeight);          // Top left corner
    glVertex2f(windowWidth, windowHeight);   // Top right corner
    glVertex2f(windowWidth, windowHeight - 50); // Bottom right corner
    glVertex2f(0.0f, windowHeight - 50);     // Bottom left corner
    glEnd();

    // Draw the bottom black rectangle (frame)
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 30.0f);           // Bottom left corner
    glVertex2f(windowWidth, 30.0f);    // Bottom right corner
    glVertex2f(windowWidth, 0.0f);      // Top right corner
    glVertex2f(0.0f, 0.0f);            // Top left corner
    glEnd();

    // Draw red shapes inside the top rectangle
    glColor3f(1.0f, 0.0f, 0.0f);  // Red color for shapes

    // Draw the first rectangle (inside the top black rectangle)
    glBegin(GL_QUADS);
    glVertex2f(50.0f, windowHeight - 10.0f); // Top left corner
    glVertex2f(150.0f, windowHeight - 10.0f); // Top right corner
    glVertex2f(150.0f, windowHeight - 40.0f); // Bottom right corner
    glVertex2f(50.0f, windowHeight - 40.0f); // Bottom left corner
    glEnd();

    // Draw a line
    glBegin(GL_LINES);
    glVertex2f(220.0f, windowHeight - 30.0f); // Start point of line
    glVertex2f(350.0f, windowHeight - 30.0f); // End point of line
    glEnd();

    // Draw a five-sided polygon (pentagon)
    glBegin(GL_POLYGON);
    for (int i = 0; i < 5; i++) {
        float angle = i * (2.0f * 3.14 / 5); // Calculate angle for each vertex
        float x = 380.0f + cos(angle) * 30.0f; // X coordinate of vertex
        float y = windowHeight - 25.0f + sin(angle) * 20.0f; // Y coordinate of vertex
        glVertex2f(x, y); // Add vertex to polygon
    }
    glEnd();

    // Draw another line
    glBegin(GL_LINES);
    glVertex2f(windowWidth - 380, windowHeight - 30.0f); // Start point of line
    glVertex2f(windowWidth - 230, windowHeight - 30.0f); // End point of line
    glEnd();

    // Draw another rectangle
    glBegin(GL_QUADS);
    glVertex2f(windowWidth - 150, windowHeight - 10.0f); // Top left corner
    glVertex2f(windowWidth - 50, windowHeight - 10.0f); // Top right corner
    glVertex2f(windowWidth - 50, windowHeight - 40.0f); // Bottom right corner
    glVertex2f(windowWidth - 150, windowHeight - 40.0f); // Bottom left corner
    glEnd();


    // Draw the bottom black rectangle (frame)
    glColor3f(0.0f, 0.0f, 0.0f);  // Black color for the frame

    glBegin(GL_QUADS);
    glVertex2f(0.0f, 30.0f);           // Bottom left corner
    glVertex2f(windowWidth, 30.0f);    // Bottom right corner
    glVertex2f(windowWidth, 0.0f);      // Top right corner
    glVertex2f(0.0f, 0.0f);             // Top left corner
    glEnd();
    // Draw red shapes inside the bottom rectangle
    glColor3f(1.0f, 0.0f, 0.0f);  // Red color for shapes

    // Draw the first rectangle 
    glBegin(GL_QUADS);
    glVertex2f(50.0f, 25.0f); // Top left corner
    glVertex2f(150.0f, 25.0f); // Top right corner
    glVertex2f(150.0f, 5.0f); // Bottom right corner
    glVertex2f(50.0f, 5.0f); // Bottom left corner
    glEnd();

    // Draw a line
    glBegin(GL_LINES);
    glVertex2f(220.0f, 18.0f); // Start point of line
    glVertex2f(350.0f, 18.0f); // End point of line
    glEnd();

    // Draw a five-sided polygon (pentagon)
    glBegin(GL_POLYGON);
    for (int i = 0; i < 5; i++) {
        float angle = i * (2.0f * 3.14 / 5); // Calculate angle for each vertex
        float x = 380.0f + cos(angle) * 30.0f; // X coordinate of vertex
        float y = 18.0f + sin(angle) * 10.0f; // Y coordinate of vertex
        glVertex2f(x, y); // Add vertex to polygon
    }
    glEnd();

    // Draw another line
    glBegin(GL_LINES);
    glVertex2f(windowWidth - 380, 18.0f); // Start point of line
    glVertex2f(windowWidth - 230, 18.0f); // End point of line
    glEnd();

    // Draw another rectangle
    glBegin(GL_QUADS);
    glVertex2f(windowWidth - 150, 25.f); // Top left corner
    glVertex2f(windowWidth - 50, 25.0f); // Top right corner
    glVertex2f(windowWidth - 50, 5.0f); // Bottom right corner
    glVertex2f(windowWidth - 150, 5.0f); // Bottom left corner
    glEnd();



    if (isOver) {
        drawGameOver();
    }
    else {

        // Draw boundaries (e.g., the ground and the sky)
        glColor3f(0.0f, 0.5f, 0.0f);  // Ground color (green)
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 100.0f);           // Bottom left corner
        glVertex2f(windowWidth, 100.0f);    // Bottom right corner
        glVertex2f(windowWidth, 30.0f);     // Top right corner
        glVertex2f(0.0f, 30.0f);            // Top left corner
        glEnd();

        // Draw Score and Time
        glColor3f(1.0f, 1.0f, 1.0f);  // Set color to white for text
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        TimeAndScore(10, windowHeight - 80, scoreText);  // Score
        char timerText[50];
        sprintf(timerText, "Time: %d", timer);
        TimeAndScore(120, windowHeight - 80, timerText);    // Time

        float rectX = 470;  // Starting X position (slightly before the first heart)
        float rectY = windowHeight - 100;  // Starting Y position (above the hearts)
        float rectWidth = (5 * 50) + 20;  // Total width for 5 hearts, with padding on both sides
        float rectHeight = 40;  // Height of the rectangle (covering the hearts)

        // Draw the big rectangle surrounding all hearts
        glColor3f(1.0f, 1.0f, 1.0f);  // White color for the rectangle
        glBegin(GL_LINE_LOOP);
        glVertex2f(rectX, rectY);  // Top-left corner
        glVertex2f(rectX + rectWidth, rectY);  // Top-right corner
        glVertex2f(rectX + rectWidth, rectY + rectHeight);  // Bottom-right corner
        glVertex2f(rectX, rectY + rectHeight);  // Bottom-left corner
        glEnd();
        // Draw hearts for health
        glColor3f(1.0f, 0.0f, 0.0f);  // Set heart color to red
        for (int i = 0; i < lives; i++) {
            drawHeart(500 + i * 50, windowHeight - 80, 1.0);  // Increased spacing between hearts
        }

        drawPlayer();

        // Move the obstacle update here
        updateObstacles();  // Update the position of obstacles
        drawObstacles();    // Draw obstacles

        updateCollectables();
        drawCollectables();

        updatePowerups();
        drawPowerUps();
        Anim();
    }
    // Swap the buffers to display the rendered content
    glutSwapBuffers();
}

// Function to handle game timer
void updateTime() {
    static int lastUpdate = 0;
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    if (currentTime - lastUpdate >= 1000) {  // Update every second
        timer--;
        if (!isOver) {
            score += 2;
        }
        lastUpdate = currentTime;
        if (timer <= 0) {
            isOver = true;
        }
    }
}






// Function to update game state and redraw
void update(int value) {
    updatePlayer();  // Update player position based on state
    updateObstacles(); //update the obstacles
    updateCollectables();
    updatePowerups();
    static int lastSpeedUpdate = 0;
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    if (currentTime - lastSpeedUpdate >= 10000) {  // Every 10 seconds
        obstacleSpeed += speedIncrement;  // Increase speed
        jumpSpeed += speedIncrement;
        lastSpeedUpdate = currentTime;
    }

    // Create a new obstacle periodically
    if (rand() % 100 < 5) {  // 5% chance to create a new obstacle each frame
        createObstacle();
    }

    if (rand() % 100 < 3) {  // 3% chance to create a new collectable each frame
        createCollectable();
    }

    if (rand() % 100 < 0.001) {  // 0.001% chance to create a new collectable each frame
        createPowerup();
    }
    glutPostRedisplay();  // Request display update
    updateTime();
    checkCollisions();
    checkCollects();
    checkCollectsPowerup();
    glutTimerFunc(16, update, 0);  // Call update every 16 milliseconds (approx 60 FPS)
}


// Function to toggle between fullscreen and windowed mode
void toggleFullscreen() {
    if (isFullscreen) {
        glutReshapeWindow(windowWidth, windowHeight);  // Switch back to windowed mode
        glutPositionWindow(100, 100);  // Position the window
        isFullscreen = false;
    }
    else {
        glutFullScreen();  // Switch to fullscreen mode
        isFullscreen = true;
    }
}

// Function to handle key presses
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  // Press ESC to exit
        exit(0);
    }
    else if (key == 'f') {  // Press 'f' to toggle fullscreen
        toggleFullscreen();
    }
    else if (key == 'r' && isOver) {  // Restart game
        lives = 5;               // Reset lives
        score = 0;              // Reset score
        timer = 60;              // Reset timer
        isOver = false;     // Reset game over state
        obstacleSpeed = 5.0f;
        obstacleHead = nullptr;  // Reset obstacles
        collectableHead = nullptr; // reset collectables
        jumpSpeed = 5;  // Speed of jumping
        playerX = 300;
    }
}

void keyboard2(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        if (!isJumping && playerY == 150) {  // Only jump if on the ground
            isJumping = true;
        }
        break;
    case GLUT_KEY_DOWN:
        isDucking = true;  // Ducking state
        break;
    }
}

void keyboard2Up(int key, int x, int y) {
    if (key == GLUT_KEY_DOWN) {  // Stop ducking when down arrow released
        isDucking = false;
    }
}

// Main function to set up GLUT and OpenGL
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    srand(static_cast<unsigned int>(time(0)));
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);  // Initial window size
    glutInitWindowPosition(100, 100);  // Initial window position
    glutCreateWindow(windowTitle);
    irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();
    if (!engine) {
        return 0;
    }
    engine->play2D("media/main.mp3", true);

    glutFullScreen();  // Start in fullscreen mode

    initOpenGL();

    //generateObstacles();

    glutDisplayFunc(display);  // Set the display callback for rendering
    glutKeyboardFunc(keyboard);  // Register keyboard handler for key presses
    glutSpecialFunc(keyboard2);
    glutSpecialUpFunc(keyboard2Up);  // Register special key release handler
    glutTimerFunc(0, update, 0);  // Start the update timerr
    glutMainLoop();
    engine->drop();// Enter the GLUT event processing loop
    engine2->drop();
    return 0;
}

void Anim() {
    collecatbleRotation += 1;
    if (collecatbleRotation >= 360.0f) {
        collecatbleRotation -= 360.0f; // Keep the angle within [0, 360)
    }
    powerUpHeight += 0.7;
    if (powerUpHeight >= 270) {
        powerUpHeight -= 100;
    }
}
