#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include "GL/glut.h"
#include <unistd.h>
    // Ambient light values. These will be normalized on program start.
GLfloat amb_light[3] = {.5f, .5f, .5f};
    // Direction for diffuse lighting
GLfloat light_dir[3] = {5.3f, 10.2f, 10.1f};

void drawCube(float x, float y, float z, float size);
void renderScene();
void keyUp (unsigned char key, int x, int y);
void changeSize(int w, int h);
void reset(int layer);
uint8_t decide_cell(int x, int y);
float clamp(float in, float lo, float hi);
void quad(int a,int b,int c,int d, float x, float y, float z, float size, GLfloat col[3]);
void colorcube(float x, float y, float z, float size, GLfloat col[3]);


#define min_x (1)
#define min_y (-22)
#define max_x (33)
#define max_y (10)
#define depth (40)

typedef struct {
    uint8_t active = 0;
    GLfloat col[3] = {1.0f, 1.0f, 1.0f};
} block_state_t;

int idx = 0;
block_state_t blocks[depth][max_x - min_x][max_y - min_y];


int main(int argc, char **argv) {
    srand((unsigned int)std::time(nullptr));

        // normalize light_direction so abs(light_dir) = 1
    float ldir_len = 0;
    for(int i = 0; i < 3; i++) {
        ldir_len += light_dir[i] * light_dir[i];
    }

    ldir_len = sqrt(ldir_len);
    for(int i = 0; i < 3; i++) {
        light_dir[i] = light_dir[i] / ldir_len;
    }
        // Create a rainbow spectrum from three-phase sines.
        // Note that this is calculated only once and repeated, so it repeats every -depth- iterations.
    for(int i = 0; i < depth; i++) {
        GLfloat rgb_gen[] = {
            std::sin(((float)i)/((float)depth) * 2 * 3.1415f + 0 * 3.1415f/3)/2 + 0.5f,
            std::sin(((float)i)/((float)depth) * 2 * 3.1415f + 2 * 3.1415f/3)/2 + 0.5f,
            std::sin(((float)i)/((float)depth) * 2 * 3.1415f + 4 * 3.1415f/3)/2 + 0.5f
        };
        float len = sqrt(rgb_gen[0] * rgb_gen[0] + rgb_gen[1] * rgb_gen[1] + rgb_gen[2] * rgb_gen[2]);
        rgb_gen[0] /= len;
        rgb_gen[1] /= len;
        rgb_gen[2] /= len;
        for(int x = 0; x < max_x - min_x; x++) {
            for(int y = 0; y < max_y - min_y; y++) {
                blocks[i][x][y].col[0] = rgb_gen[0];
                blocks[i][x][y].col[1] = rgb_gen[1];
                blocks[i][x][y].col[2] = rgb_gen[2];
            }
        }
    }

        // init first layer randomly.
    reset(idx);
    idx++;

	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(1600,1200);
	glutCreateWindow("3D game of life");
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
    glutKeyboardUpFunc(keyUp);
    glEnable(GL_DEPTH_TEST);    // make vertices draw at the position they're drawn from
    glutMainLoop();
    return 0;
}

void reset(int layer) {
    for(int x = 0; x < max_x - min_x; x++) {
        for(int y = 0; y < max_y - min_y; y++) {
            if((rand() % 50) > 35) {
                blocks[layer][x][y].active = 1;
            }
        }
    }
}

void keyUp (unsigned char key, int x, int y) {  
        // Very rarely it gets stuck due an unresolved bug. For now just press r to solve it.
    if(key == 'r') {
        reset(idx);
        idx++;
    }
}  

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    gluLookAt(52, 20, 80, 
              2, -10, -1, 
              1, 1, 2.1);

        // Set next layer of blocks
    for(int x = 0; x < max_x - min_x; x++) {
        for(int y = 0; y < max_y - min_y; y++) {
            blocks[idx][x][y].active = decide_cell(x, y);
        }
    }

    // check if generated == the same:
    if(idx == depth -1) {
        int same = 1;
        for(int x = 0; x < max_x - min_x && same; x++) {
            for(int y = 0; y < max_y - min_y && same; y++) {
                same = (blocks[idx][x][y].active == blocks[9][x][y].active || blocks[idx][x][y].active == blocks[8][x][y].active || blocks[idx][x][y].active == blocks[7][x][y].active);
            }
        }
        if(same) {
            reset(idx);
        }
    }

        // Generate the image
    for(int i = 0; i < depth; i++) {
        if((i - idx + depth) % depth == 0) continue;
        for(int x = 0; x < max_x - min_x; x++) {
            for(int y = 0; y < max_y - min_y; y++) {
                if(blocks[i][x][y].active == 1) {
                    GLfloat rel = (float)(((i - idx + depth) % depth)/(0.5*depth));
                    GLfloat bias_col[] = {blocks[i][x][y].col[0] * rel, blocks[i][x][y].col[1] * rel, blocks[i][x][y].col[2] * rel};
                    colorcube((float)x + min_x, float(y) + min_y, 
                    (float)((i - idx + depth) % depth), 
                    0.5f, bias_col);
                }
            }
        }
    }
    idx++;
    idx = idx % depth;

    glutSwapBuffers(); 
    usleep(20000);
}

    // From http://www.lighthouse3d.com/tutorials/glut-tutorial/preparing-the-window-for-a-reshape/
void changeSize(int w, int h) {
    // 
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;
	float ratio = 1.0f* w / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

        // Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,1,1000);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

uint8_t decide_cell(int x, int y) {
    int prev_depth = (depth + idx - 1) % depth;
        // Discount itself from being counted.
    int neighcount = -(blocks[prev_depth][x][y].active);

    for(int x_ = x-1; x_ <= x+1; x_++) {   
        for(int y_ = y-1; y_ <= y+1; y_++) {
            neighcount += blocks[prev_depth][x_ % (max_x - min_x)][y_ % (max_y - min_y)].active;
        }
    }
    if(blocks[prev_depth][x][y].active == 1) {
        if(neighcount == 2 || neighcount == 3) return 1;
    } else if(neighcount == 3) return 1;
    return 0;
}

float clamp(float in, float lo, float hi) {
    if(isnan(in)) return 0;
    if(in < lo) return lo;
    if(in > hi) return hi;
    return in;
}

void quad(int a,int b,int c,int d, float x, float y, float z, float size, GLfloat col[3]) {
    float ver[8][3] = 
    {
        {-0.5f * size + x,-0.5f * size + y,0.5f * size + z},
        {-0.5f * size + x,0.5f * size + y,0.5f * size + z},
        {0.5f * size + x,0.5f * size + y,0.5f * size + z},
        {0.5f * size + x,-0.5f * size + y,0.5f * size + z},
        {-0.5f * size + x,-0.5f * size + y,-0.5f * size + z},
        {-0.5f * size + x,0.5f * size + y,-0.5f * size + z},
        {0.5f * size + x,0.5f * size + y,-0.5f * size + z},
        {0.5f * size + x,-0.5f * size + y,-0.5f * size + z},
    };
    GLfloat amb_col[] = {amb_light[0] * col[0], amb_light[1] * col[1], amb_light[2] * col[2]};

        // calculate normal
    GLfloat v2v1[] = {ver[b][0] - ver[a][0], ver[b][1] - ver[a][1], ver[b][2] - ver[a][2]};
    GLfloat v3v1[] = {ver[c][0] - ver[a][0], ver[c][1] - ver[a][1], ver[c][2] - ver[a][2]}; 
    GLfloat cross_product[] = {v2v1[0] * v3v1[1], v2v1[1] * v3v1[2], v2v1[2] * v3v1[0]};
    float crossp_len = 0;
    for(int i = 0; i < 3; i++) {
        crossp_len += cross_product[i] * cross_product[i];
    }
    crossp_len = sqrt(crossp_len);
    for(int i = 0; i < 3; i++) {
        cross_product[i] = cross_product[i] / crossp_len;
    }

        // calculate diffuse
    float mul = 1;
    float theta = clamp(cross_product[0] * light_dir[0] + cross_product[1] * light_dir[1] + cross_product[2] * light_dir[2], 0, 1);
    GLfloat dif_col[] = {col[0] * theta, col[1] * theta, col[2] * theta};
    GLfloat fin_col[] = {amb_col[0] + dif_col[0], amb_col[1] + dif_col[1], amb_col[2] + dif_col[2]};

        // draw this shitteru
    glBegin(GL_QUADS);
        glColor3fv(fin_col);
        glVertex3fv(ver[a]);

        glColor3fv(fin_col);
        glVertex3fv(ver[b]);

        glColor3fv(fin_col);
        glVertex3fv(ver[c]);

        glColor3fv(fin_col);
        glVertex3fv(ver[d]);
    glEnd();
}

void colorcube(float x, float y, float z, float size, GLfloat col[3]) {
    quad(0,3,2,1, x, y, z, size, col);
    quad(2,3,7,6, x, y, z, size, col);
    quad(0,4,7,3, x, y, z, size, col);
    quad(1,2,6,5, x, y, z, size, col);
    quad(4,5,6,7, x, y, z, size, col);
    quad(0,1,5,4, x, y, z, size, col);
}
