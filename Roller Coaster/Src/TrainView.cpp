#include "TrainView.h"  



TrainView::TrainView(QWidget *parent) :
	QGLWidget(parent)
{

	resetArcball();
	arrow = new Model("train.obj", 3, Point3d(0, 0, 0));

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
	drawStuff();
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
	GLfloat lightPosition2[] = { 0, 1, 0, 0 };
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
	if(terrain)
		drawGround();

	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();
	interpos.clear();
	interorient.clear();
	lowestpoint = 0;
	for (int i = 0; i < this->m_pTrack->points.size(); i++) {
		if (this->m_pTrack->points[i].pos.y < lowestpoint)
			lowestpoint = this->m_pTrack->points[i].pos.y;
	}
	drawStuff();
	drawTrain(1);
	// this time drawing is for shadows (except for top view)
	if (this->camera != 1) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}
	if (isrun) {
		time += kinetic * 0.004f;
		float currentTime = time * speed *0.6f;
		int position = currentTime;
		Pnt3f nowPos = interpos[position%interpos.size()];
		Pnt3f nextPos = interpos[(position + 1) % interpos.size()];
		Pnt3f trainDir = nextPos + -1 * nowPos;
		float distanceL = sqrt(pow(trainDir.x, 2) + pow(trainDir.y, 2) + pow(trainDir.z, 2));

		time += 0.1 * interpos.size()/1400;
		//if (time > 1.0) time -= 1.0;
		if (time > 300000) time = 0.1;
	}
	else {
		energy = 0;
		kinetic = 0;
	}
	//printf("%f\n", time);
	interpos.clear();
	interorient.clear();
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
		glOrtho(-wi, wi, he, -he, 200, -200);
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
	else if (this->camera == 2) {
		float currentTime = time * speed *0.6f;
		int position = currentTime;
		float trainLength = 5.0f;
		Pnt3f nowPos = interpos[position%interpos.size()];
		Pnt3f nowOrt = interorient[position%interpos.size()];
		Pnt3f nextPos = interpos[(position + 1) % interpos.size()];
		Pnt3f trainDir = nextPos + -1 * nowPos;
		float distanceL = sqrt(pow(trainDir.x, 2) + pow(trainDir.y, 2) + pow(trainDir.z, 2));
		float scaleL = trainLength / distanceL;
		trainDir = trainDir * scaleL * 2;

		Pnt3f higher = nowOrt;
		higher.normalize();
		gluPerspective(90, aspect, 1, 300);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(nowPos.x+10*higher.x, nowPos.y+10*higher.y, nowPos.z+10*higher.z,
			nowPos.x+10*higher.x + trainDir.x, nowPos.y+10*higher.y + trainDir.y, nowPos.z+ 10 * higher.z + trainDir.z,
			nowOrt.x, nowOrt.y, nowOrt.z);
		update();

	}


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
	QMatrix4x4 cardinalM = {
		-1,2,-1,0,
		3,-5,0,2,
		-3,4,1,0,
		1,-1,0,0
	};
	cardinalM *= 0.5f;


	QMatrix4x4 bsplineM = {
		-1,3,-3,1,
		3,-6,0,4,
		-3,3,3,1,
		1,0,0,0
	};
	bsplineM /= 6;

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

	float arclength = 2.5f;
	Pnt3f lastpoints[2]; //first is the point second is the crosst
	Pnt3f firstpoint[2]; //first point computed second is crosst
	boolean first = false;
	Pnt3f lastcross_t;
	int distance = 0;
	Pnt3f orient_t, cross_t;
	for (size_t i = 0; i < this->m_pTrack->points.size(); ++i) {
		Pnt3f qt0, qt1, qt;
		//first two
		Pnt3f cp_pos_p1 = m_pTrack->points[i].pos;
		Pnt3f cp_pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;
		Pnt3f pointlength = (cp_pos_p1 + (-1 * cp_pos_p2));
		int inter = 10;
		float length = sqrt((pointlength.x * pointlength.x) + (pointlength.y * pointlength.y) + (pointlength.z * pointlength.z));
		if (doarc)
			DIVIDE_LINE = length * 70;
		else 
			DIVIDE_LINE = length;
		
		
		float percent = 1.0f / DIVIDE_LINE;
		float t = 0;
		
		int structure = 100;
		int upsidedownStruct=structure;
		
		int track_num = DIVIDE_LINE / inter;

		//for curved tracks
		Pnt3f cp_pos_p0 = m_pTrack->points[(i - 1) % m_pTrack->points.size()].pos;
		if (i == 0)
			cp_pos_p0 = m_pTrack->points[this->m_pTrack->points.size() - 1].pos;

		Pnt3f cp_pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;
		float tmatrix[4] = { pow(t,3),pow(t,2),t,1.0f };
		QMatrix4x4 qposMatrix = {
			cp_pos_p0.x,cp_pos_p1.x,cp_pos_p2.x,cp_pos_p3.x,
			cp_pos_p0.y,cp_pos_p1.y,cp_pos_p2.y,cp_pos_p3.y,
			cp_pos_p0.z,cp_pos_p1.z,cp_pos_p2.z,cp_pos_p3.z,
			0,0,0,0
		};


		Pnt3f cp_orient_p1 = m_pTrack->points[i].orient;
		Pnt3f cp_orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;
		//for curved tracks
		Pnt3f cp_orient_p0 = m_pTrack->points[(i - 1) % m_pTrack->points.size()].orient;
		if (i == 0)
			cp_orient_p0 = m_pTrack->points[m_pTrack->points.size() - 1].orient;
		Pnt3f cp_orient_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].orient;

		QMatrix4x4 qorientMatrix = {
			cp_orient_p0.x,cp_orient_p1.x,cp_orient_p2.x,cp_orient_p3.x,
			cp_orient_p0.y,cp_orient_p1.y,cp_orient_p2.y,cp_orient_p3.y,
			cp_orient_p0.z,cp_orient_p1.z,cp_orient_p2.z,cp_orient_p3.z,
			0,0,0,0
		};
		QMatrix4x4 QMcardinalorient = qorientMatrix*cardinalM;
		QMatrix4x4 QMcardinalpos = qposMatrix*cardinalM;
		QMatrix4x4 QMbsplineorient = qorientMatrix*bsplineM;
		QMatrix4x4 QMbsplinepos = qposMatrix*bsplineM;
		GLUquadricObj *quadObj;
		

		switch (curve) {
		case 0:
			qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
			break;
		case 1:
			qt.x = 0; qt.y = 0; qt.z = 0;
			for (int k = 0; k < 4; k++) {
				float temp = 0;
				qt.x += QMcardinalpos(0, k)*tmatrix[k];
				qt.y += QMcardinalpos(1, k)*tmatrix[k];
				qt.z += QMcardinalpos(2, k)*tmatrix[k];
			}
			break;
		case 2:
			qt.x = 0; qt.y = 0; qt.z = 0;
			for (int k = 0; k < 4; k++) {
				float temp = 0;
				qt.x += QMbsplinepos(0, k)*tmatrix[k];
				qt.y += QMbsplinepos(1, k)*tmatrix[k];
				qt.z += QMbsplinepos(2, k)*tmatrix[k];
			}
			break;
		}
		for (int j = 0; j < DIVIDE_LINE; j++) {

			qt0 = qt;
			
		
			if (!doingShadows) {
				glColor3ub(32, 32, 64);
			}

			if (j == 0 && i != 0) {
				glEnable(GL_LINE_SMOOTH);
				switch (track) {
				case 0:
					glLineWidth(8);
					glBegin(GL_LINES);
					glVertex3f(lastpoints[0].x, lastpoints[0].y, lastpoints[0].z);
					glVertex3f(qt0.x, qt0.y, qt0.z);
					glEnd();
					break;
				case 1:
					glLineWidth(8);
					glBegin(GL_LINES);
					glVertex3f(lastpoints[0].x + lastpoints[1].x, lastpoints[0].y + lastpoints[1].y, lastpoints[0].z + lastpoints[1].z);
					glVertex3f(qt0.x + lastcross_t.x, qt0.y + lastcross_t.y, qt0.z + lastcross_t.z);
					glVertex3f(lastpoints[0].x - lastpoints[1].x, lastpoints[0].y - lastpoints[1].y, lastpoints[0].z - lastpoints[1].z);
					glVertex3f(qt0.x - lastcross_t.x, qt0.y - lastcross_t.y, qt0.z - lastcross_t.z);
					glEnd();
					break;
				case 2:
					glLineWidth(8);
					if (!doingShadows) {
						glColor3ub(0, 0, 0);
					}
					glBegin(GL_QUADS);
					glVertex3f(lastpoints[0].x + lastpoints[1].x, lastpoints[0].y + lastpoints[1].y, lastpoints[0].z + lastpoints[1].z);
					glVertex3f(qt0.x + lastcross_t.x, qt0.y + lastcross_t.y, qt0.z + lastcross_t.z);
					glVertex3f(qt0.x - lastcross_t.x, qt0.y - lastcross_t.y, qt0.z - lastcross_t.z);
					glVertex3f(lastpoints[0].x - lastpoints[1].x, lastpoints[0].y - lastpoints[1].y, lastpoints[0].z - lastpoints[1].z);
					glEnd();
					break;
				}
			}


			switch (curve) {
			case 0:
				orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;
				break;
			case 1:
				orient_t.x = 0; orient_t.y = 0; orient_t.z = 0;
				for (int k = 0; k < 4; k++) {
					float temp = 0;
					orient_t.x += QMcardinalorient(0, k)*tmatrix[k];
					orient_t.y += QMcardinalorient(1, k)*tmatrix[k];
					orient_t.z += QMcardinalorient(2, k)*tmatrix[k];
				}
				break;
			case 2:
				orient_t.x = 0; orient_t.y = 0; orient_t.z = 0;
				for (int k = 0; k < 4; k++) {
					float temp = 0;
					orient_t.x += QMbsplineorient(0, k)*tmatrix[k];
					orient_t.y += QMbsplineorient(1, k)*tmatrix[k];
					orient_t.z += QMbsplineorient(2, k)*tmatrix[k];
				}
				break;
			}
			interpos.push_back(qt);
			
			
			
			while (true) {
				t += percent;
				tmatrix[0] = pow(t, 3);
				tmatrix[1] = pow(t, 2);
				tmatrix[2] = t;
				tmatrix[3] = 1;
				switch (curve) {
				case 0:
					qt = (1 - t) * cp_pos_p1 + t * cp_pos_p2;
					break;
				case 1:
					qt.x = 0; qt.y = 0; qt.z = 0;
					for (int k = 0; k < 4; k++) {
						float temp = 0;
						qt.x += QMcardinalpos(0, k)*tmatrix[k];
						qt.y += QMcardinalpos(1, k)*tmatrix[k];
						qt.z += QMcardinalpos(2, k)*tmatrix[k];
					}
					break;
				case 2:
					qt.x = 0; qt.y = 0; qt.z = 0;
					for (int k = 0; k < 4; k++) {
						float temp = 0;
						qt.x += QMbsplinepos(0, k)*tmatrix[k];
						qt.y += QMbsplinepos(1, k)*tmatrix[k];
						qt.z += QMbsplinepos(2, k)*tmatrix[k];
					}
					break;
				}
				Pnt3f trainDir = qt + -1 * qt0;
				float distanceL = sqrt(pow(trainDir.x, 2) + pow(trainDir.y, 2) + pow(trainDir.z, 2));
				distance += distanceL;
				if (distance >= arclength || j >= DIVIDE_LINE - 1)
					break;
				else {
					j++;
				}
			}
			if (distance >= arclength)
				distance = 0;
				
			qt1 = qt;
			orient_t.normalize();
			cross_t = (qt1 + -1 * qt0) * orient_t;
			cross_t.normalize();
			orient_t = -1 * ((qt1 + -1 * qt0) * cross_t);
			orient_t.normalize();
			interorient.push_back(orient_t);
			cross_t = (qt1 + -1 * qt0) * orient_t;
			cross_t.normalize();
			cross_t = cross_t * 2.5f;
			if (i == 0 && !first)
				lastcross_t = cross_t;
			if (i == 0 && !first) {
				firstpoint[0] = qt0;
				firstpoint[1] = cross_t;
				first = true;
			}

			switch (track) {
			case 0:
				glEnable(GL_LINE_SMOOTH);
				glLineWidth(8);
				glBegin(GL_LINES);
				glVertex3f(qt0.x, qt0.y, qt0.z);
				glVertex3f(qt1.x, qt1.y, qt1.z);
				glEnd();
				if (interpos.size() % inter == 0) {
					glLineWidth(5);
					glBegin(GL_LINES);
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glVertex3f(qt1.x + 1.5*cross_t.x + orient_t.x*0.5, qt1.y + 1.5*cross_t.y + orient_t.y*0.5, qt1.z + 1.5*cross_t.z + orient_t.z*0.5);
					glVertex3f(qt1.x - 1.5*cross_t.x + orient_t.x*0.5, qt1.y - 1.5*cross_t.y + orient_t.y*0.5, qt1.z - 1.5*cross_t.z + orient_t.z*0.5);
					glEnd();
					if (interpos.size() % upsidedownStruct== 0 && orient_t.y < 0) {
						glPushMatrix();
						if (!doingShadows) {
							glColor3ub(37, 77, 81);
						}
						glTranslatef(qt1.x + 1.5*cross_t.x + orient_t.x*0.5, 0, qt1.z + 1.5*cross_t.z + orient_t.z*0.5);
						glRotatef(90, -1, 0, 0);
	
						//glRotatef(30, 0, 0, 1);
						quadObj = gluNewQuadric();
						gluCylinder(quadObj, 1.5, 0.1, qt1.y + 1.5*cross_t.y + orient_t.y*0.5, 30, 30);
						glPopMatrix();
						glPushMatrix();
						if (!doingShadows) {
							glColor3ub(37, 77, 81);
						}
						glTranslatef(qt1.x - 1.5*cross_t.x - orient_t.x*0.5, 0, qt1.z - 1.5*cross_t.z - orient_t.z*0.5);
						glRotatef(90, -1, 0, 0);
						//glRotatef(30, 0, 0, -1);
						quadObj = gluNewQuadric();
						gluCylinder(quadObj, 1.5, 0.1, qt1.y - 1.5*cross_t.y + orient_t.z*0.5, 30, 30);
						glPopMatrix();
					}
				}
				if (interpos.size() % structure == 0 && orient_t.y>0) {
					//glLineWidth(5);

					glPushMatrix();
					if (!doingShadows) {
						glColor3ub(37, 77, 81);
					}
					glTranslatef(qt0.x, 0, qt0.z);
					glRotatef(90, -1, 0, 0);
					quadObj = gluNewQuadric();
					gluCylinder(quadObj, 1.5, 0.1, qt0.y, 30, 30);
					glPopMatrix();
				}
				
				break;
			case 1:
				glEnable(GL_LINE_SMOOTH);
				if (!doingShadows) {
					glColor3ub(32, 32, 64);
				}
				glLineWidth(8);
				glBegin(GL_LINES);
				glVertex3f(qt0.x + lastcross_t.x, qt0.y + lastcross_t.y, qt0.z + lastcross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
				glVertex3f(qt0.x - lastcross_t.x, qt0.y - lastcross_t.y, qt0.z - lastcross_t.z);
				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glEnd();

				if (interpos.size() % inter == 0) {
					glLineWidth(5);
					glBegin(GL_LINES);
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glVertex3f(qt1.x + 1.5*cross_t.x, qt1.y + 1.5*cross_t.y, qt1.z + 1.5*cross_t.z);
					glVertex3f(qt1.x - 1.5*cross_t.x, qt1.y - 1.5*cross_t.y, qt1.z - 1.5*cross_t.z);
					glEnd();
					if (interpos.size() % upsidedownStruct == 0 && orient_t.y < 0) {
						glPushMatrix();
						if (!doingShadows) {
							glColor3ub(37, 77, 81);
						}
						glTranslatef(qt1.x + 1.5*cross_t.x + orient_t.x*0.5, 0, qt1.z + 1.5*cross_t.z + orient_t.z*0.5);
						glRotatef(90, -1, 0, 0);

						//glRotatef(30, 0, 0, 1);
						quadObj = gluNewQuadric();
						gluCylinder(quadObj, 1.5, 0.1, qt1.y + 1.5*cross_t.y + orient_t.y*0.5, 30, 30);
						glPopMatrix();
						glPushMatrix();
						if (!doingShadows) {
							glColor3ub(37, 77, 81);
						}
						glTranslatef(qt1.x - 1.5*cross_t.x - orient_t.x*0.5, 0, qt1.z - 1.5*cross_t.z - orient_t.z*0.5);
						glRotatef(90, -1, 0, 0);
						//glRotatef(30, 0, 0, -1);
						quadObj = gluNewQuadric();
						gluCylinder(quadObj, 1.5, 0.1, qt1.y - 1.5*cross_t.y + orient_t.z*0.5, 30, 30);
						glPopMatrix();
					}
				}
				
				if (interpos.size() % structure == 0 && orient_t.y>0) {
					//glLineWidth(5);
					glPushMatrix();
					if (!doingShadows) {
						glColor3ub(37, 77, 81);
					}
					glTranslatef(qt0.x, 0, qt0.z);
					glRotatef(90, -1, 0, 0);
					GLUquadricObj *quadObj = gluNewQuadric();
					gluCylinder(quadObj, 1.5, 0.1, qt0.y, 30, 30);
					glPopMatrix();
				}
				break;
			case 2:
				glEnable(GL_LINE_SMOOTH);
				if (!doingShadows) {
					glColor3ub(0, 0, 0);
				}
				glLineWidth(8);
				glBegin(GL_QUADS);
				glVertex3f(qt0.x + lastcross_t.x, qt0.y + lastcross_t.y, qt0.z + lastcross_t.z);
				glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
				glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
				glVertex3f(qt0.x - lastcross_t.x, qt0.y - lastcross_t.y, qt0.z - lastcross_t.z);
				glEnd();
				if (interpos.size() % inter == 0) {
					glLineWidth(5);
					glBegin(GL_LINES);
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glVertex3f(qt1.x + 1.5*cross_t.x, qt1.y + 1.5*cross_t.y, qt1.z + 1.5*cross_t.z);
					glVertex3f(qt1.x - 1.5*cross_t.x, qt1.y - 1.5*cross_t.y, qt1.z - 1.5*cross_t.z);
					glEnd();
					if (interpos.size() % upsidedownStruct == 0 && orient_t.y < 0) {
						glPushMatrix();
						if (!doingShadows) {
							glColor3ub(37, 77, 81);
						}
						glTranslatef(qt1.x + 1.5*cross_t.x + orient_t.x*0.5, 0, qt1.z + 1.5*cross_t.z + orient_t.z*0.5);
						glRotatef(90, -1, 0, 0);

						//glRotatef(30, 0, 0, 1);
						quadObj = gluNewQuadric();
						gluCylinder(quadObj, 1.5, 0.1, qt1.y + 1.5*cross_t.y + orient_t.y*0.5, 30, 30);
						glPopMatrix();
						glPushMatrix();
						if (!doingShadows) {
							glColor3ub(37, 77, 81);
						}
						glTranslatef(qt1.x - 1.5*cross_t.x - orient_t.x*0.5, 0, qt1.z - 1.5*cross_t.z - orient_t.z*0.5);
						glRotatef(90, -1, 0, 0);
						//glRotatef(30, 0, 0, -1);
						quadObj = gluNewQuadric();
						gluCylinder(quadObj, 1.5, 0.1, qt1.y - 1.5*cross_t.y + orient_t.z*0.5, 30, 30);
						glPopMatrix();
					}
				}
				if (interpos.size() % structure == 0 && orient_t.y>0) {
					//glLineWidth(5);

					glPushMatrix();
					if (!doingShadows) {
						glColor3ub(37, 77, 81);
					}
					glTranslatef(qt0.x, 0, qt0.z);
					glRotatef(90, -1, 0, 0);
					GLUquadricObj *quadObj = gluNewQuadric();
					gluCylinder(quadObj, 1.5, 0.1, qt0.y, 30, 30);
					glPopMatrix();
				}


				break;
			}


			if (j >= DIVIDE_LINE-1) {
				lastpoints[0] = qt1;
				lastpoints[1] = cross_t;
			}

			//wraparound to the first point
			if (j>= DIVIDE_LINE-1 && i == this->m_pTrack->points.size() - 1) {
				switch (track) {
				case 0:
					if (!doingShadows) {
						glColor3ub(32, 32, 64);
					}
					glEnable(GL_LINE_SMOOTH);
					glLineWidth(8);
					glBegin(GL_LINES);
					glVertex3f(qt1.x, qt1.y, qt1.z);
					glVertex3f(firstpoint[0].x, firstpoint[0].y, firstpoint[0].z);
					glEnd();
					glLineWidth(5);
					glBegin(GL_LINES);
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glVertex3f(firstpoint[0].x + 1.5*firstpoint[1].x, firstpoint[0].y + 1.5*firstpoint[1].y, firstpoint[0].z + 1.5*firstpoint[1].z);
					glVertex3f(firstpoint[0].x - 1.5*firstpoint[1].x, firstpoint[0].y - 1.5*firstpoint[1].y, firstpoint[0].z - 1.5*firstpoint[1].z);
					glEnd();
					break;
				case 1:
					if (!doingShadows) {
						glColor3ub(32, 32, 64);
					}
					glEnable(GL_LINE_SMOOTH);
					glLineWidth(8);
					glBegin(GL_LINES);
					glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
					glVertex3f(firstpoint[0].x + firstpoint[1].x, firstpoint[0].y + firstpoint[1].y, firstpoint[0].z + firstpoint[1].z);
					glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
					glVertex3f(firstpoint[0].x - firstpoint[1].x, firstpoint[0].y - firstpoint[1].y, firstpoint[0].z - firstpoint[1].z);
					glEnd();
					glLineWidth(5);
					glBegin(GL_LINES);
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glVertex3f(firstpoint[0].x + 1.5*firstpoint[1].x, firstpoint[0].y + 1.5*firstpoint[1].y, firstpoint[0].z + 1.5*firstpoint[1].z);
					glVertex3f(firstpoint[0].x - 1.5*firstpoint[1].x, firstpoint[0].y - 1.5*firstpoint[1].y, firstpoint[0].z - 1.5*firstpoint[1].z);
					glEnd();
					break;
				case 2:
					if (!doingShadows) {
						glColor3ub(0, 0, 0);
					}
					glEnable(GL_LINE_SMOOTH);
					glLineWidth(8);
					glBegin(GL_QUADS);
					glVertex3f(qt1.x + cross_t.x, qt1.y + cross_t.y, qt1.z + cross_t.z);
					glVertex3f(firstpoint[0].x + firstpoint[1].x, firstpoint[0].y + firstpoint[1].y, firstpoint[0].z + firstpoint[1].z);
					glVertex3f(firstpoint[0].x - firstpoint[1].x, firstpoint[0].y - firstpoint[1].y, firstpoint[0].z - firstpoint[1].z);
					glVertex3f(qt1.x - cross_t.x, qt1.y - cross_t.y, qt1.z - cross_t.z);
					glEnd();
					glLineWidth(5);
					glBegin(GL_LINES);
					if (!doingShadows) {
						glColor3ub(255, 255, 255);
					}
					glVertex3f(firstpoint[0].x + 1.5*firstpoint[1].x, firstpoint[0].y + 1.5*firstpoint[1].y, firstpoint[0].z + 1.5*firstpoint[1].z);
					glVertex3f(firstpoint[0].x - 1.5*firstpoint[1].x, firstpoint[0].y - 1.5*firstpoint[1].y, firstpoint[0].z - 1.5*firstpoint[1].z);
					glEnd();
					break;
				}

			}
			lastcross_t = cross_t;
		}
		//first = false;
	}
	
	update();
}

void TrainView::drawTrain(float) {

	// orient
	
	float currentTime = time * speed *0.6f;
	int position = currentTime ;
	//kinetic = 0;
	//printf("%d\n", position);
	
	//Pnt3f nowPos = interpos[(position + 1) % interpos.size()];
	//Pnt3f prePos = interpos[position % interpos.size()];
	//Pnt3f nextPos = interpos[(position + 2) % interpos.size()];

	//Pnt3f nowOrt = interorient[position % interpos.size()];
	//nowOrt.normalize();

	//Pnt3f firstVec = nowPos + -1 * prePos;
	//Pnt3f secondVec = nextPos + -1 * nowPos;

	/*glPushMatrix();
	glColor3ub(255, 0, 0);
	glTranslatef(nowPos.x, nowPos.y, nowPos.z);
	glRotatef(90, nowOrt.x, nowOrt.y, nowOrt.z);
	glScalef(5, 10, 10);
	arrow->render();
	glPopMatrix();
*/
	for (int i = 0; i < cars; i++) {
		Pnt3f nowPos = interpos[position%interpos.size()];
		Pnt3f nowOrt = interorient[position%interpos.size()];

		Pnt3f nextPos = interpos[(position + 1) % interpos.size()];
		Pnt3f nextOrt = interorient[(position + 1) % interpos.size()];
		
			
		nowOrt.normalize();
		float trainLength = 5.0f;
		Pnt3f trainDir = nextPos + -1 * nowPos;
		float distanceL = sqrt(pow(trainDir.x, 2) + pow(trainDir.y, 2) + pow(trainDir.z, 2));
		float scaleL = trainLength / distanceL;
		trainDir = trainDir * scaleL;
		Pnt3f cross_t = (nextPos + -1 * nowPos) * nowOrt;
		cross_t.normalize();
		cross_t = cross_t * 4.0f;

		Pnt3f higher = nowOrt;
		higher.normalize();
		higher = higher*2;
		Pnt3f p1(nowPos.x + cross_t.x + trainDir.x + higher.x, nowPos.y + cross_t.y + trainDir.y + higher.y, nowPos.z + cross_t.z + trainDir.z + higher.z);
		Pnt3f p2(nowPos.x + cross_t.x - trainDir.x + higher.x, nowPos.y + cross_t.y - trainDir.y + higher.y, nowPos.z + cross_t.z - trainDir.z + higher.z);
		Pnt3f p3(nowPos.x - cross_t.x - trainDir.x + higher.x, nowPos.y - cross_t.y - trainDir.y + higher.y, nowPos.z - cross_t.z - trainDir.z + higher.z);
		Pnt3f p4(nowPos.x - cross_t.x + trainDir.x + higher.x, nowPos.y - cross_t.y + trainDir.y + higher.y, nowPos.z - cross_t.z + trainDir.z + higher.z);

		float trainHeight = 5.0f;
		float distanceH = sqrt(pow(nowOrt.x, 2) + pow(nowOrt.y, 2) + pow(nowOrt.z, 2));
		float scaleH = trainHeight / distanceH;
		nowOrt = nowOrt * scaleH;

		Pnt3f p5(p1.x + nowOrt.x, p1.y + nowOrt.y, p1.z + nowOrt.z);
		Pnt3f p6(p2.x + nowOrt.x, p2.y + nowOrt.y, p2.z + nowOrt.z);
		Pnt3f p7(p3.x + nowOrt.x, p3.y + nowOrt.y, p3.z + nowOrt.z);
		Pnt3f p8(p4.x + nowOrt.x, p4.y + nowOrt.y, p4.z + nowOrt.z);


		glColor3ub(80, 200, 80);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(p1.x, p1.y, p1.z);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(p2.x, p2.y, p2.z);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(p3.x, p3.y, p3.z);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(p4.x, p4.y, p4.z);
		glEnd();

		glColor3ub(200, 40, 40);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(p5.x, p5.y, p5.z);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(p6.x, p6.y, p6.z);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(p7.x, p7.y, p7.z);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(p8.x, p8.y, p8.z);
		glEnd();

		glColor3ub(200, 200, 80);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(p1.x, p1.y, p1.z);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(p4.x, p4.y, p4.z);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(p8.x, p8.y, p8.z);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(p5.x, p5.y, p5.z);
		glEnd();

		glColor3ub(80, 200, 200);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(p1.x, p1.y, p1.z);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(p2.x, p2.y, p2.z);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(p6.x, p6.y, p6.z);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(p5.x, p5.y, p5.z);
		glEnd();

		glColor3ub(200, 80, 200);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(p2.x, p2.y, p2.z);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(p3.x, p3.y, p3.z);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(p7.x, p7.y, p7.z);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(p6.x, p6.y, p6.z);
		glEnd();

		glColor3ub(200, 160, 80);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(p3.x, p3.y, p3.z);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(p4.x, p4.y, p4.z);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(p8.x, p8.y, p8.z);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(p7.x, p7.y, p7.z);
		glEnd();

		const float RADDEG = 57.29578;

		float az = atan2(nowOrt.y, nowOrt.x)       * RADDEG;
		float el = asin(nowOrt.z / 15) * RADDEG;

		

		/*glPushMatrix();
		glColor3ub(255, 0, 0);
		nowOrt.normalize();
		glTranslatef(nowPos.x, nowPos.y, nowPos.z);
		glRotatef();
		glScalef(5, 10, 10);
		arrow->render();
		glPopMatrix();
*/
		if (i == 0) {
			if (nextPos.y - nowPos.y > 0) {
				float newenergy = (nowPos.y - lowestpoint)*9.8;
				if (newenergy - energy > 0)
					kinetic -= sqrt((newenergy - energy) * 1.5);
				energy = newenergy; //Potential Energy
				
			}
			else if (nextPos.y - nowPos.y < 0) {

				float newenergy = (nowPos.y - lowestpoint)*9.8;
				
				if (energy - newenergy > 0)
					kinetic += sqrt((energy - newenergy) * 1.5);
				energy = newenergy;
			}
		}
		

		position -= 15;
		if (position < 0) 
			position += interpos.size();
		position %= interpos.size();
	}

	energy = energy < 0 ? 0 : energy;
	kinetic = kinetic < 3.5f ? 0 : kinetic - kinetic*0.002 - 3.5f;//friction
	printf("%d %d\n", position, interpos.size());
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
	for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
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

void TrainView::drawGround() {
	Terrain terrain;
	
	int h = terrain.ground.size();
	int w = 0;
	if (h > 0)
		w = terrain.ground[0].size();
	float h1 = 0.0f;
	float h2 = 0.0f;
	for (int x = 1; x < h; x++){
		glBegin(GL_TRIANGLE_STRIP);
		for (int z = 1; z < w; z++){
			
			h1 = terrain.ground[x][z];
			int temp = (x* z)*(int)h1 % 5;
			switch (temp) {
			case 0:
				glColor3ub(34, 139, 34);
				break;
			case 1:
				glColor3ub(0, 100, 0);
				break;
			case 2:
				glColor3ub(0, 128, 0);
				break;
			case 3:
				glColor3ub(32, 178, 170);
				break;
			case 4:
				glColor3ub(46, 139, 87);
				break;
			}
			if (x + 1 < h)
				h2 = terrain.ground[x + 1][z];
			else
				h2 = 0.0f;
			glVertex3f(x * 10.0f - h*5.0f, h1, z * 10.0f - w*5.0f);
			glVertex3f((x + 1) * 10.0f - h*5.0f, h2, z * 10.0f - w*5.0f);
			
		}
		glEnd();
	}
	

}

void TrainView::loadModel(std::string path) {
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "stone02.obj", NULL, true);


	
}
