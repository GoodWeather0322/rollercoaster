#ifndef TRAINVIEW_H  
#define TRAINVIEW_H  
#include <QtOpenGL/QGLWidget>  
#include <QtGui/QtGui>  
#include <QtOpenGL/QtOpenGL>  
#include <GL/GLU.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib") 
#include "Utilities/ArcBallCam.H"
#include "Utilities/3DUtils.H"
#include "Track.H"
#include <LoadObjModel\Model.h>
#include <LoadObjModel\tiny_obj_loader.h>
#include <HeightMap.h>

class AppMain;
class CTrack;

//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


class TrainView : public QGLWidget  
{  
	Q_OBJECT  
public:  
	explicit TrainView(QWidget *parent = 0);  
	~TrainView();  

public:
	// overrides of important window things
	//virtual int handle(int);
	virtual void paintGL();

	// all of the actual drawing happens in this routine
	// it has to be encapsulated, since we draw differently if
	// we're drawing shadows (no colors, for example)
	void drawStuff(bool doingShadows=false);

	// setup the projection - assuming that the projection stack has been
	// cleared for you
	void setProjection();

	// Reset the Arc ball control
	void resetArcball();

	// pick a point (for when the mouse goes down)
	void doPick(int mx, int my);

	void drawTrain(float);
	void drawGround();

	void loadModel(std::string path);
	void loadTexture2D(QString str, GLuint &textureID);
	void initTextures();

public:
	ArcBallCam		arcball;			// keep an ArcBall for the UI
	int				selectedCube;  // simple - just remember which cube is selected

	CTrack*			m_pTrack;		// The track of the entire scene

	Model *arrow;
	int DIVIDE_LINE;
	int camera;
	int curve;
	int track;
	int cars = 1;
	float energy = 0;
	float kinetic = 0;
	float lowestpoint;
	bool isrun;
	bool terrain = true;
	bool doarc = true;
	bool music = true;
	bool showstruct = false;
	float time=0;
	bool init = false;
	GLuint train_ID;
	GLuint trainSide_ID;
	GLuint trainHead_ID;
	GLuint trainTop_ID;
	GLuint trainOneSide_ID;
	GLuint trainOneHead_ID;
	GLuint trainOneTop_ID;
	GLuint trainOneFront_ID;
	GLuint trainOneHeadFront_ID;

	int speed = 50;
	vector<Pnt3f> interpos;
	vector<Pnt3f> interorient;
	

};  
#endif // TRAINVIEW_H  