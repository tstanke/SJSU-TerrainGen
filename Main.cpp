using namespace std;

#include "Main.h"
#include <math.h>

// aspect ratio of the window
float aspect;

// indexing for changing color schemes
int scheme = 0;

// height and width of the map
const int n = 1025;

// array to contain height map
float map[n][n];

// the greatest height in the height map
float maxHeight = 0;

// width and height of the screen
int w, h;

float pi = acos(-1);

float viewr = 500, viewphi = pi / 4., viewtheta = 0;
int mx, my;

/**
 * Function to map a height to a color.  Different
 * color schemes are available depending on the
 * state of Main.
 */
float *getColor(float height) {
	float *res = new float[3];
	float f = (float) height / (float) maxHeight;

	/**
	 * Default color scheme; black at height = 0
	 * with white peaks, and shades of blue in
	 * between.
	 */
	if (scheme == 0) {
		res[2] = f;
		res[1] = res[2] * f;
		res[0] = res[1] * f;
	}

	/**
	 * "Realistic" scheme; blue water, brown coast,
	 * different shades of green inland and mountains
	 * starting with gray at the bottom and becoming
	 * white at the top.
	 */
	else if (scheme == 1) {
		if (height == 0) {
			res[2] = 1;
			res[1] = 0;
			res[0] = 0;
		} else if (height < 8) {
			//139-69-19
			res[2] = 19. / 255;
			res[1] = 69. / 255;
			res[0] = 139. / 255;
		} else if (height < 23) {
			res[1] = f;
			res[2] = res[1] * f;
			res[0] = res[2] * f;
		} else {
			res[0] = f * f;
			res[1] = f * f;
			res[2] = f * f;
		}
	}

	/**
	 * Goofy color-phasing scheme.  It's kind of
	 * ugly but covers the spectrum decently.
	 */
	else if (scheme == 2) {
		res[0] = max(2. * f - 1., 0.);
		res[2] = max(0., 1. - (f * 2.));
		res[1] = 1. - res[0] - res[2];
	}

	return res;
}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//	glOrtho(0, n, 0, n, -1, 1);
	gluPerspective(60, 1 * aspect, 1, 10000);
	gluLookAt(viewr * cos(viewtheta) * sin(viewphi), viewr * sin(viewtheta)
			* sin(viewphi), viewr * cos(viewphi), 0, 0, 0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// the map is stored in a display list
	glCallList(1);

	glutSwapBuffers();
}

void init(void) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// remove rear-facing polygons
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glNewList(1, GL_COMPILE);
	glPushMatrix();
//	glTranslatef(-n / 2, -n / 2, -n/2);
	glBegin(GL_QUADS);
	/**
	 * The map is represented not as points but as
	 * triangles to support fancier behavior later.
	 * Corners of triangles get the color mapped
	 * to their height, creating a nice continuous
	 * look when zoomed in.
	 */

//	float di[] = { 1, 1, 0, 0, 0, 1 };
//	float dj[] = { 0, 1, 0, 1, 0, 1 };

	float di[] = {0, 0, 1, 1};
	float dj[] = {0, 1, 1, 0};

	float baser = 200;
	float dtheta = pi * 2. / (n - 1);
	float dphi = pi / (n - 1);

	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - 1; j++) {
			float *color;

			for (int k = 0; k < 4; k++) {
				int x = i + di[k];
				int y = j + dj[k];
				float z = map[x][y];

				// toggle bumpiness
//				float r = baser + z / 15.; // bumpy
				float r = baser; // not bumpy

				color = getColor(z);
				glColor3fv(color);
				delete color;
				glVertex3f(r * cos(dtheta * x) * sin(dphi * y), r * sin(dtheta * x) * sin(dphi * y), r * cos(dphi * y));
			}

		}
	}
	glEnd();
	glPopMatrix();
	glEndList();

}

void reshape(int width, int height) {
	w = width;
	h = height;

	glViewport(0, 0, w, h);

	aspect = (float) w / (float) h;
}

float randf(float error) {
	float f = rand() % 1000;
	f -= 500.;
	f /= 500.;
	f *= error;
	return f;
}

void doSquare(int x, int y, int w, float e) {

	if (w < 2)
		return;

	float height = map[x][y] + map[x + w][y] + map[x][y + w]
			+ map[x + w][y + w];
	height /= 4.;
	height += randf(e * w);

	int xc = x + (w >> 1);
	int yc = y + (w >> 1);
	int w2 = w >> 1;

	map[xc][y] = (map[x][y] + map[x + w][y]) / 2.;
	map[x][yc] = (map[x][y] + map[x][y + w]) / 2.;
	map[xc][y + w] = (map[x][y + w] + map[x + w][y + w]) / 2.;
	map[x + w][yc] = (map[x + w][y] + map[x + w][y + w]) / 2.;

	map[xc][yc] = height;

	doSquare(x, y, w2, e);
	doSquare(x + w2, y, w2, e);
	doSquare(x, y + w2, w2, e);
	doSquare(x + w2, y + w2, w2, e);
}

void loadMap(void) {

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			map[i][j] = -1;

	map[0][0] = map[0][n - 1] = map[n - 1][0] = map[n - 1][n - 1] = .2;
	doSquare(0, 0, n - 1, .5);

	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
			maxHeight = max(maxHeight, map[i][j]);
			if (map[i][j] == -1)
				std::cout << "(" << i << "," << j << ") uninitialized!\n";
			map[i][j] = max(map[i][j], (float) 0);
		}
}

/**
 * Cycles the coloring scheme if 'n' is pressed, then
 * rebuilds the display list.  There are three schemes
 * available.
 */
void keyboard(unsigned char key, int x, int y) {
	if (key == 'n') {
		scheme++;
		if (scheme == 3)
			scheme = 0;
		init();
	}

	if (key == '3')
		viewr /= .99;
	if (key == '4')
		viewr *= .99;

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT && state == GLUT_DOWN) {
		mx = x;
		my = y;
	}
}

void motion(int x, int y) {
	float dx = x - mx;
	float dy = y - my;
	viewtheta -= dx / 500.;
	viewphi -= dy / 500.;

	mx = x;
	my = y;

	glutPostRedisplay();
}

int main(int argc, char ** argv) {
	loadMap();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutCreateWindow(*argv);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	init();

	glutMainLoop();

	return 0;
}

Main::Main(void) {
}

Main::~Main(void) {
}
