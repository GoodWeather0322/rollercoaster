#include "TrainView.h"  


TrainView::TrainView(QWidget *parent) :
	QGLWidget(parent)
{
	resetArcball();
	arrow = new Model("Mickey_Mouse.obj", 1, Point3d(0, 0, 0));
}
TrainView::~TrainView()
{}

void TrainView::resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

void TrainView::paintGL()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	// Set up the view port
	glViewport(0, 0, width(), height());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

									// we need to clear out the stencil buffer since we'll use
									// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

							//######################################################################
							// TODO: 
							// you might want to set the lighting up differently. if you do, 
							// we need to set up the lights AFTER setting up the projection
							//######################################################################
							// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (this->camera == 1) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[] = { 0,1,1,0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { 1, 0, 0, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .1f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);



	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	setupFloor();
	glDisable(GL_LIGHTING);
	drawFloor(200, 10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();
	drawTrain(1);
	// this time drawing is for shadows (except for top view)
	if (this->camera != 1) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}
	if (isrun) {
		/*if (clock() - lastRedraw > CLOCKS_PER_SEC / 30) {
			lastRedraw = clock();
			this->advanceTrain();
			this->damageMe();
		}*/
	}
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(width()) / static_cast<float>(height());

	// Check whether we use the world camp
	if (this->camera == 0) {
		arcball.setProjection(false);
		update();
		// Or we use the top cam
	}
	else if (this->camera == 1) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		}
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
		update();
	}
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################



	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this, aspect);
#endif
		update();
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (this->camera != 2) {
		for (size_t i = 0; i < this->m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			this->m_pTrack->points[i].draw();
		}
		update();
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################
	for (size_t i = 0; i < this->m_pTrack->points.size(); ++i) {
		Pnt3f qt0, qt1, qt;
		Pnt3f lastcross_t;
		Pnt3f orient_t, cross_t;
		DIVIDE_LINE = 15;
		float percent = 1.0f / DIVIDE_LINE;
		float t = 0;
		//first two
		Pnt3f cp_pos_p1 = m_pTrack->points[i].pos;
		Pnt3f cp_pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;

		//for curved tracks
		Pnt3f cp_pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;
		Pnt3f cp_pos_p4 = m_pTrack->points[(i + 3) % m_pTrack->points.size()].pos;


		Pnt3f cp_orient_p1 = m_pTrack->points[i].orient;
		Pnt3f cp_orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
		qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
		for (size_t j = 0; j < DIVIDE_LINE; j++) {
			qt0 = qt;
			switch (curve) {
			case 0:
				orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;
				break;
			case 1:
				break;
			case 2:
				break;
			}
			t += percent;
			switch (curve) {
			case 0:
				qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
				break;
			case 1:
				break;
			case 2:
				break;
			}
			
			qt1 = qt;
			orient_t.normalize();
			cross_t = (qt1 + -1 * qt0) * orient_t;
			cross_t.normalize();
			cross_t = cross_t * 2.5f;
			if (j == 0)
				lastcross_t = cross_t;

			if (!doingShadows) {
				glColor3ub(32, 32, 64);
			}
			glEnable(GL_LINE_SMOOTH);
			glLineWidth(8);
			glBegin(GL_LINES);
			glVertex3f(qt0.x + lastcross_t.x, qt0.y + lastcross_t.y, qt0.z + lastcross_t.z);
			glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
			glVertex3f(qt0.x - lastcross_t.x, qt0.y - lastcross_t.y, qt0.z - lastcross_t.z);
			glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
			glEnd();

			glLineWidth(5);
			glBegin(GL_LINES);
			if (!doingShadows) {
				glColor3ub(255, 255, 255);
			}
			glVertex3f(qt1.x + 1.5*cross_t.x, qt1.y + 1.5*cross_t.y, qt1.z + 1.5*cross_t.z);
			glVertex3f(qt1.x - 1.5*cross_t.x, qt1.y - 1.5*cross_t.y, qt1.z - 1.5*cross_t.z);
			glEnd();
			lastcross_t = cross_t;
		}
	}
	update();
}

void TrainView::drawTrain(float) {

	glPushMatrix();
	glColor3ub(255, 0, 0);
	glTranslatef(0, 10, 0);
	glScalef(10, 10, 10);
	arrow->render();
	glPopMatrix();
	size_t t = 1;
	t *= m_pTrack->points.size();
	size_t i;
	for (i = 0; t > 1; t -= 1)
		i++;
	//pos
	Pnt3f cp_pos_p1 = m_pTrack->points[i].pos;
	Pnt3f cp_pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;

	// orient
	Pnt3f cp_orient_p1 = m_pTrack->points[i].orient;
	Pnt3f cp_orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;

	Pnt3f qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
	Pnt3f orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;

	glColor3ub(255, 255, 255);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(qt.x - 5, qt.y - 5, qt.z - 5);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(qt.x + 5, qt.y - 5, qt.z - 5);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(qt.x + 5, qt.y + 5, qt.z - 5);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(qt.x - 5, qt.y + 5, qt.z - 5);
	glEnd();

	update();
}
	


#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif


void TrainView::
doPick(int mx, int my)
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	makeCurrent();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPickMatrix((double)mx, (double)(viewport[3] - my),
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);


	// draw the cubes, loading the names as we go
	for (size_t i = 0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint)(i + 1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		selectedCube = -1;
}
