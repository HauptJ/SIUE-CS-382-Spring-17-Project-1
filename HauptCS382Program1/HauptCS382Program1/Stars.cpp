/***********************************************************************/
/* Filename: Stars_Prekiminary.cpp                                     */
/* Generates a dozen pulsating star-shaped polygons that float in a 2D */
/* environment, bouncing off of any window borders with which they     */
/* collide. Mouse operations are used to temporarily "freeze" stars.   */
/* All stars maintain the same color and spin rate.                    */
/***********************************************************************/

#include <gl/freeglut.h>
#include <cmath>			// Header File For Math Library
#include <ctime>			// Header File For Accessing System Time
#include <atltime.h>        // Header File For Processing Time Intervals
#include <cstring>          // Header File For Accessing String Type
#include <sys/types.h>
#include <fstream>
#include <iostream>			// Header File for debug print messages
using namespace std;





//////////////////////
// Global Constants //
//////////////////////
const int   INIT_WINDOW_POSITION[2] = { 100, 100 };          // Window offset (in pixels).       //
const float PI_OVER_180 = 0.0174532925f;         // One degree (in radians).         //

const int   FREEZE_BEEP_DURATION = 25;                    // # msec per beep for freezing.    //
const int   FREEZE_BEEP_FREQUENCY = 1000;                  // Freezing beep audio frequency.   //
const int   UNFREEZE_BEEP_DURATION = 25;                    // # msec per beep for unfreezing.  //
const int   UNFREEZE_BEEP_FREQUENCY = 400;                   // Unfreezing beep audio frequency. //

const int   NBR_STARS = 12;                    // # stars in game.                 //
const int   NBR_STAR_TIPS = 5;                     // # points per star.               //
const int   MAX_STATE_INDEX = 5;                     // Maximum state index for stars.   //
const float STAR_RADIUS = 0.075f;                 // Normal radius of star.           //
const float STAR_COLOR[NBR_STARS][3] = { { 0.9f, 0.4f, 0.4f },   // Red
{ 0.9f, 0.7f, 0.4f },    // Orange
{ 0.9f, 0.9f, 0.4f },   // Yellow
{ 0.4f, 0.9f, 0.4f },   // Green
{ 0.4f, 0.9f, 0.9f },    // Cyan
{ 0.9f, 0.4f, 0.9f },   // Magenta
{ 0.9f, 0.9f, 0.9f },   // White
{ 0.4f, 0.4f, 0.9f },   // Blue
{ 0.9f, 0.7f, 0.9f },   // Pink
{ 0.0f, 0.6f, 0.9f },   // Turquoise
{ 0.9f, 0.0f, 0.6f },   // Violet
{ 0.6f, 0.6f, 0.0f } }; // Brown
const float PULSATION_FACTOR = 2.5f;                   // Extent of pulsation enlargement. //
const int   FREEZE_INTERVAL = 6;                      // INITIAL Freeze interval (in seconds).    //
const float STAR_SPEED = 0.015f;                 // Star velocity.                   //
const float STAR_SPIN_INC = 0.3f;                   // Star rotation rate.              //
const float PULSATION_INC = 0.03f;                  // Star pulsation rate.             //

//NEW
const int COLLISION_LIMIT = 6;					// Max possible collions for a star. //

int TOTAL_COLLISIONS = 0;						// Counter for total number of collisions. Used to determine if game is over //
int YELLOW_STARS = 0;							// Counter for total number of yellow stars. //


													/////////////////////////////////////////////////////
													// 2D star-shaped polygon class (for convenience). //
													/////////////////////////////////////////////////////
class Star
{
	/* Local function to generate random value in parameterized range. */
	float GenerateRandomNumber(float lowerBound, float upperBound)
	{
		static bool firstTime = true;
		static time_t randomNumberSeed;
		if (firstTime)
		{
			time(&randomNumberSeed);
			firstTime = false;
			srand(unsigned int(randomNumberSeed));
		}
		return (lowerBound + ((upperBound - lowerBound) * (float(rand()) / RAND_MAX)));
	}

public:
	float x;              // Star center's current x-coordinate (image space). //
	float y;              // Star center's current y-coordinate (image space). //
	float xInc;           // Star's motion increment in x-dimension.           //
	float yInc;           // Star's motion increment in y-dimension.           //
	float color[3];       // Star's color.                                     //
	float spin;           // Star's current rotated orientation.               //
	float spinInc;        // Star's rotation increment.                        //
	float maxPulsation;   // Star's maximum expansion.                         //
	float minPulsation;   // Star's minimum contraction.                       //
	float pulsation;      // Star's current pulsation value.                   //
	float pulsationInc;   // Star's current pulsation increment.               //
	int   freezeLimit;    // Star's current freeze time limit.                 //
	CTime freezeTime;     // Snapshot of time when star was frozen.            //

	//NEW
	int starNbr;		// Star number //
	int collisionCnt;	// Number of times star has collided. //
	float speed;		// Star speed
	float radius;		// Star radius

						  /* Default constructor. */
	Star::Star()
	{
		// Randomly generated initial position (inside window). //
		x = GenerateRandomNumber(-1.0f + STAR_RADIUS, 1.0f - STAR_RADIUS);
		y = GenerateRandomNumber(-1.0f + STAR_RADIUS, 1.0f - STAR_RADIUS);

		// Randomly generated velocity. //
		//speed = STAR_SPEED; // default speed
		speed = GenerateRandomNumber(0.010f, 0.045f); // random speed
		xInc = GenerateRandomNumber(speed / 4.0, speed);
		yInc = sqrt(speed * speed - xInc * xInc);
		float randNbr = GenerateRandomNumber(-1.0, 1.0);
		if (randNbr < 0.0f)
			xInc *= -1.0f;
		randNbr = GenerateRandomNumber(-1.0, 1.0);
		if (randNbr < 0.0f)
			yInc *= -1.0f;

		// Initial orientation: zero. //
		spin = 0.0f;
		//spinInc = STAR_SPIN_INC;
		spinInc = GenerateRandomNumber(0.15f, 0.55f);


		// Initial pulsation status: 100%. //
		/*
		pulsation = 1.0f;
		pulsationInc = PULSATION_INC;
		*/
		pulsation = 1.0f;
		//pulsationInc = 0.15f;
		pulsationInc = GenerateRandomNumber(0.02f, 0.045f); // unique pulsation rate for each star
		//pulsationInc = 0.0f; // disable pulsation
		//pulsationInc = PULSATION_INC; // default

		// Star initialized in unfrozen state. //
		freezeLimit = 0;

		// Initialize collision count
		collisionCnt = 0;

		// Initialize color cyan
		color[0] = 0.4f; //
		color[1] = 0.9f; //  initialize star color as cyan
		color[2] = 0.9f; //
	}

	/* Render the star-shaped polygon. */
	void Star::draw()
	{
		GLfloat theta;
		glColor3fv(color);
		glBegin(GL_LINE_LOOP);
		for (int j = 0; j < 2 * NBR_STAR_TIPS; j++)
		{
			theta = spin + 360 * j * PI_OVER_180 / (2 * NBR_STAR_TIPS);
			if (j % 2 != 0)
				glVertex2f(x + pulsation * 0.5f * STAR_RADIUS * cos(theta),
					y + pulsation *  0.5f * STAR_RADIUS * sin(theta));
			else
				glVertex2f(x + pulsation * STAR_RADIUS * cos(theta),
					y + pulsation * STAR_RADIUS * sin(theta));
		}
		glEnd();
	}
};

/////////////////////////
// Function Prototypes //
/////////////////////////
void MouseClick(int mouseButton, int mouseState, int mouseXPosition, int mouseYPosition);
int  FindMouseHit(GLfloat mouseX, GLfloat mouseY);
void TimerFunction(int value);
void AdjustToWindow(Star &currentStar);
void Display();
void ResizeWindow(GLsizei w, GLsizei h);
void ConvertToCharacterArray(int value, char valueArray[]);
void UpdateTitleBar();

// NEW Collision detection
// based off of FindMouseHit()
//int DetectCollision(GLfloat posX, GLfloat posY);
int DetectCollision(Star &currentStar);

// Collision effects
void CollisionEffects(Star &currentStar);

//////////////////////
// Global Variables //
//////////////////////
GLint   currWindowSize[2] = { 1000, 750 };            // Window size in pixels. //
GLfloat windowWidth = 4.0;                      // Resized window width.  //
GLfloat windowHeight = 3.0;                      // Resized window height. //
Star    polyList[NBR_STARS];                             // Current polygon list.  //
CTime   startTime = CTime::GetCurrentTime();  // Game start time.       //

bool gameOver = false;								// Global Bool to check if game has ended. It should be set to true when collision threshold is met.


											  /* The main function: uses the OpenGL Utility Toolkit to set */
											  /* the window up to display the window and its contents.     */
void main(int argc, char **argv)
{
	/* Set up the display window. */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(INIT_WINDOW_POSITION[0], INIT_WINDOW_POSITION[1]);
	glutInitWindowSize(currWindowSize[0], currWindowSize[1]);
	glutCreateWindow("PULSATING STARS");

	/* Initialize the set of stars. */
	for (int i = 0; i < NBR_STARS; i++)
	{
		Star newStar;

		polyList[i] = newStar;
		polyList[i].starNbr = i; // assign star number
	}

	/* Specify the resizing, displaying, and interactive routines. */
	glutReshapeFunc(ResizeWindow);
	glutDisplayFunc(Display);
	glutMouseFunc(MouseClick);
	glutTimerFunc(50, TimerFunction, 1);
	glutMainLoop();
}

/* Function to react to the pressing of a mouse button by the user, */
/* by determining whether the mouse is positioned within a star's   */
/* boundaries and, if so, by freezing (or unfreezing)that star.     */
void MouseClick(int mouseButton, int mouseState, int mouseXPosition, int mouseYPosition)
{
	ofstream mouseClickFile; 
	mouseClickFile.open("mouseClickFile.txt", std::ios_base::app);
	if (mouseClickFile.is_open()) {
		Star str;
		GLfloat x = windowWidth * mouseXPosition / currWindowSize[0] - 0.5f * windowWidth;
		GLfloat y = 0.5f * windowHeight - (windowHeight * mouseYPosition / currWindowSize[1]);
		int index = FindMouseHit(x, y);
		if ((mouseState == GLUT_DOWN) && (index >= 0))
		{
			if (polyList[index].freezeLimit == 0)
			{
				Beep(FREEZE_BEEP_FREQUENCY, FREEZE_BEEP_DURATION);
				polyList[index].freezeTime = CTime::GetCurrentTime();
				polyList[index].freezeLimit = (FREEZE_INTERVAL - polyList[index].collisionCnt); //Freeze time = Initial freeze limit - collision count
				mouseClickFile << "star: " << index << "freezeLimit: " << polyList[index].freezeLimit << endl;
			}
			else
			{
				Beep(UNFREEZE_BEEP_FREQUENCY, UNFREEZE_BEEP_DURATION);
				polyList[index].freezeLimit = 0;
			}
		}
	}
}


/* Function to traverse the star list until the current star contains the */
/* current mouse position, whereupon that star's index is returned. If no */
/* such star exists, an appropriate dummy index (-1) is returned.         */
int FindMouseHit(GLfloat mouseX, GLfloat mouseY)
{
	for (int i = 0; i < NBR_STARS; i++)
	{
		// Rather than determining whether the mouse-click occured precisely within the
		// star's boundaries, this function merely checks whether the click is within
		// 90% of the distance between the star's center and any of its tip vertices.
		if (sqrt(pow(mouseX - polyList[i].x, 2) + pow(mouseY - polyList[i].y, 2)) <
			0.9 * polyList[i].pulsation * STAR_RADIUS)
			return i;
	}
	return -1;
}

/* Detect if two stars collide */ //WORKS!!!
//try passing current star
int DetectCollision(Star &currentStar) {
	//debug
	int colcnt = 0;

	ofstream collisionFile;
	collisionFile.open("collisionFile.txt", std::ios_base::app);
	if (collisionFile.is_open()) {
		for (int i = 0; i < NBR_STARS; i++)
		{
			// Rather than determining whether the collision occured precisely within the
			// star's boundaries, this function merely checks whether the colision is within
			// 90% of the distance between the star's center and any of its tip vertices.
			//cout << sqrt(pow(currentStar.x - polyList[i].x, 2) + pow(currentStar.y - polyList[i].y, 2)) << " : " << 0.9 * polyList[i].pulsation * STAR_RADIUS << endl;
			if (currentStar.starNbr != polyList[i].starNbr && sqrt(pow(currentStar.x - polyList[i].x, 2) + pow(currentStar.y - polyList[i].y, 2)) < 0.9 * polyList[i].pulsation * STAR_RADIUS) { //we cannot have a star collide with itself duh.
				
				//swap inverse trajectories on collision
				currentStar.xInc = polyList[i].xInc * -1;
				currentStar.yInc = polyList[i].yInc * -1;
				currentStar.collisionCnt = currentStar.collisionCnt + 1;
				
				if (currentStar.collisionCnt < COLLISION_LIMIT) { // make sure collision limit is not exceeded
					currentStar.collisionCnt = currentStar.collisionCnt + 1;
					//CollisionEffects(currentStar);
				}
				CollisionEffects(currentStar);

				polyList[i].xInc = currentStar.xInc * -1;
				polyList[i].yInc = currentStar.yInc * -1;
				polyList[i].collisionCnt = polyList[i].collisionCnt + 1;
				if (polyList[i].collisionCnt < COLLISION_LIMIT) { // make sure collision limit is not exceeded
					polyList[i].collisionCnt = polyList[i].collisionCnt + 1;
					//CollisionEffects(polyList[i]);
				}
				CollisionEffects(polyList[i]);
				
				//DEBUG
				collisionFile << "Collision Detected: " << i << " collisions: " << currentStar.collisionCnt << endl;
				colcnt++;
				collisionFile << "collision: " << colcnt << endl;
				collisionFile << "Total collisions: " << TOTAL_COLLISIONS << endl;
				collisionFile << "hit" << endl;
				//increment total collisions and check if total collision limit is reached. If it is, end game.
				//total collision limit = NBR_STARS * COLLISION_LIMIT
				TOTAL_COLLISIONS = TOTAL_COLLISIONS + 2;
				return i;
			}
			collisionFile << "miss" << endl;
			return -1;
		}
		collisionFile << "miss2" << endl;
		return -1;
	}
	collisionFile << "miss3" << endl;
	return -1;
}

// Collision effects

void CollisionEffects(Star &currentStar) {
	
	// 1 collision
	if (currentStar.collisionCnt == 1) { 
		currentStar.color[0] = 0.4f; //
		currentStar.color[1] = 0.4f; // set color to blue
		currentStar.color[2] = 0.9f; //
		currentStar.pulsation

	} 

	// 2 collisions
	if (currentStar.collisionCnt == 2) {
		currentStar.color[0] = 0.9f; //
		currentStar.color[1] = 0.0f; // set color to violet
		currentStar.color[2] = 0.6f; //

	}

	// 3 collisions
	if (currentStar.collisionCnt == 3) {
		currentStar.color[0] = 0.9f; //
		currentStar.color[1] = 0.4f; // set color to red
		currentStar.color[2] = 0.4f; //

	}

	// 4 collisions
	if (currentStar.collisionCnt == 4) {
		currentStar.color[0] = 0.9f; //
		currentStar.color[1] = 0.7f; // set color to orange
		currentStar.color[2] = 0.4f; //

	}

	// 5 collisions
	if (currentStar.collisionCnt >= 5) {
		currentStar.color[0] = 0.9f; //
		currentStar.color[1] = 0.9f; // set color to yellow
		currentStar.color[2] = 0.4f; //
		
		if (currentStar.collisionCnt == 5) { // increment number of yellow stars
			YELLOW_STARS = YELLOW_STARS + 1;
		}

		// end game if all stars are yellow
		if (YELLOW_STARS == NBR_STARS) {
			gameOver = true;
		}
	}


}

/* Function to update each polygon's position, using "wraparound" */
/* to deal with the boundaries of the display window.             */
void TimerFunction(int value)
{
	// Loop through the list of polygons. //
	for (int i = 0; i <= NBR_STARS; i++)
	{
		polyList[i].pulsation += polyList[i].pulsationInc;
		if (polyList[i].pulsation > PULSATION_FACTOR)
		{
			polyList[i].pulsationInc *= -1.0;
			polyList[i].pulsation = PULSATION_FACTOR;
		}
		else if (polyList[i].pulsation < 1.0)
		{
			polyList[i].pulsationInc *= -1.0;
			polyList[i].pulsation = 1.0;
		}

		if (polyList[i].freezeLimit > 0)
		{
			CTimeSpan span = CTime::GetCurrentTime() - polyList[i].freezeTime;
			if (span.GetTotalSeconds() >= polyList[i].freezeLimit)
			{
				Beep(UNFREEZE_BEEP_FREQUENCY, UNFREEZE_BEEP_DURATION);
				polyList[i].freezeLimit = 0;
			}
		}
		else
		{
			// Update polygon position and orientation. //
			polyList[i].x += polyList[i].xInc;
			polyList[i].y += polyList[i].yInc;
			polyList[i].spin += polyList[i].spinInc;
			if (polyList[i].spin > 360 * PI_OVER_180)
				polyList[i].spin -= 360 * PI_OVER_180;

			AdjustToWindow(polyList[i]);
		}
	}
	UpdateTitleBar();

	// Force a redraw after 50 milliseconds. //
	glutPostRedisplay();
	glutTimerFunc(50, TimerFunction, 1);
}

/* Function to adjust the position of the parameterized polygon to ensure */
/* that the polygon remains inside the boundaries of the display window.  */
void AdjustToWindow(Star &currentStar)
{
	bool tooHigh, tooLow, tooLeft, tooRight;
	GLfloat theta, x, y;

	// Determine whether polygon exceeds window boundaries. //
	tooHigh = tooLow = tooLeft = tooRight = false;
	for (int j = 0; j < NBR_STAR_TIPS; j++)
	{
		theta = currentStar.spin + 360 * j * PI_OVER_180 / NBR_STAR_TIPS;
		x = currentStar.x + currentStar.pulsation * STAR_RADIUS * cos(theta);
		y = currentStar.y + currentStar.pulsation * STAR_RADIUS * sin(theta);
		if (x > windowWidth / 2.0)
			tooRight = true;
		else if (x < -windowWidth / 2.0)
			tooLeft = true;
		if (y > windowHeight / 2.0)
			tooHigh = true;
		else if (y < -windowHeight / 2.0)
			tooLow = true;
	}

	// Adjust position if window bounds exceeded. //
	if (tooRight)
	{
		currentStar.xInc *= -1.0f;
		currentStar.x = windowWidth / 2.0f - STAR_RADIUS;
	}
	else if (tooLeft)
	{
		currentStar.xInc *= -1.0f;
		currentStar.x = -windowWidth / 2.0f + STAR_RADIUS;
	}
	if (tooHigh)
	{
		currentStar.yInc *= -1.0f;
		currentStar.y = windowHeight / 2.0f - STAR_RADIUS;
	}
	else if (tooLow)
	{
		currentStar.yInc *= -1.0f;
		currentStar.y = -windowHeight / 2.0f + STAR_RADIUS;
	}
}

/* Function to update the window title bar to indicate */
/* the number of frozen and unfrozen stars.            */
void UpdateTitleBar()
{
	char label[100] = "PULSATING STARS: ";
	int frozenCount = 0;
	//int collisions = 0; // total number of collitions
	for (int i = 0; i < NBR_STARS; i++) {
		if (polyList[i].freezeLimit > 0)
			frozenCount++;
		/*if (polyList[i].collisionCnt > 0) // will not work
			collisions++;*/
	}
	
	char frozenLabel[5] = "";
	ConvertToCharacterArray(frozenCount, frozenLabel);
	strcat_s(label, 100, frozenLabel);
	strcat_s(label, 100, " FROZEN STARS; ");
	
	char unfrozenLabel[5] = "";
	ConvertToCharacterArray(NBR_STARS - frozenCount, unfrozenLabel);
	strcat_s(label, 100, unfrozenLabel);
	strcat_s(label, 100, " UNFROZEN STARS ");

	// Collisions
	/*char collisionLabel[5] = "";
	ConvertToCharacterArray(TOTAL_COLLISIONS, collisionLabel);
	strcat_s(label, 100, collisionLabel);
	strcat_s(label, 100, " Collisions");*/

	glutSetWindowTitle(label);
}

/* Principal display routine: clears the frame   */
/* buffer and draws the stars within the window. */
void Display()
{
	ofstream DisplayFile;
	DisplayFile.open("displayFile.txt", std::ios_base::app);
	if (DisplayFile.is_open()) {
		int i;

		if (gameOver == false) {  // check if game has ended / collision threshold has been met
			glClear(GL_COLOR_BUFFER_BIT); // prevents trippy end effect. Do not call when all stars finish colliding. 
		}

		glLineWidth(2);

		// Display each polygon, applying its spin as needed. //
		for (i = 0; i < NBR_STARS; i++)
			polyList[i].draw();

		// call to collision detection fctn here?
		int collisionDetected;
		for (i = 0; i < NBR_STARS; i++) {
			collisionDetected = DetectCollision(polyList[i]);
			DisplayFile << "display: " << i << " return: " << collisionDetected << endl;
			//DEBUG
			/*if(collisionDetected != -1) {
				//polyList[i].collisionCnt;
				cout << "collision count for: " << i <<" is: "<< polyList[i].collisionCnt << endl;

			}*/
		}
		//DEBUG
		/*while (gameOver != true) {
			for (i = 0; i < NBR_STARS; i++)
				cout << "star " << i << " : " << polyList[0].collisionCnt << endl;
		}*/

		// Help game along if we get stuck
		if (TOTAL_COLLISIONS >= 100) { //end game if it goes on too long
			for (i = 0; i < NBR_STARS; i++) {
				if (polyList[i].collisionCnt <= 1) {
					polyList[i].collisionCnt = 1;
					CollisionEffects(polyList[i]);
				}

			}
		}

		if (TOTAL_COLLISIONS >= 150) { //end game if it goes on too long
			for (i = 0; i < NBR_STARS; i++) {
				if (polyList[i].collisionCnt <= 2) {
					polyList[i].collisionCnt = 2;
					CollisionEffects(polyList[i]);
				}

			}
		}
		
		// Help game along if we get stuck
		if (TOTAL_COLLISIONS >= 200) { //end game if it goes on too long
			for (i = 0; i < NBR_STARS; i++) {
				if (polyList[i].collisionCnt <= 3) {
					polyList[i].collisionCnt = 3;
					CollisionEffects(polyList[i]);
				}
				
			}
		}

		if (TOTAL_COLLISIONS >= 250) { //end game if it goes on too long
			for (i = 0; i < NBR_STARS; i++) {
				if (polyList[i].collisionCnt <= 4) {
					polyList[i].collisionCnt = 4;
					CollisionEffects(polyList[i]);
				}

			}
		}

		if (TOTAL_COLLISIONS >= 300) { //end game if it goes on too long
			for (i = 0; i < NBR_STARS; i++) {
				if (polyList[i].collisionCnt <= 5) {
					polyList[i].collisionCnt = 5;
					CollisionEffects(polyList[i]);
				}

			}
		}


		glutSwapBuffers();
		glFlush();
	}
}

/* Window-reshaping routine, to scale the rendered scene according */
/* to the window dimensions, setting the global variables so the   */
/* mouse operations will correspond to mouse pointer positions.    */
void ResizeWindow(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	currWindowSize[0] = w;
	currWindowSize[1] = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
	{
		windowWidth = 2.0f;
		windowHeight = 2.0f * (GLfloat)h / (GLfloat)w;
		glOrtho(-1.0f, 1.0f, -1.0f * (GLfloat)h / (GLfloat)w, (GLfloat)h / (GLfloat)w, -10.0f, 10.0f);
	}
	else
	{
		windowWidth = 2.0f * (GLfloat)w / (GLfloat)h;
		windowHeight = 2.0f;
		glOrtho(-1.0f * (GLfloat)w / (GLfloat)h, (GLfloat)w / (GLfloat)h, -1.0f, 1.0f, -10.0f, 10.0f);
	}
	glMatrixMode(GL_MODELVIEW);
}


// Convert the parameterized integer value into the
// corresponding character array representing that value.
void ConvertToCharacterArray(int value, char valueArray[])
{
	char digitStr[4] = "0";
	if (value < 10)
	{
		digitStr[0] = char(value + int('0'));
		strcat_s(valueArray, 4, digitStr);
	}
	else
	{
		ConvertToCharacterArray(value / 10, valueArray);
		digitStr[0] = char(value % 10 + int('0'));
		strcat_s(valueArray, 4, digitStr);
	}
}