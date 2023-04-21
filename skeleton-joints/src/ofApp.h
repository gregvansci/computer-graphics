//
//  Skeleton Builder
//
//  This file includes functionality that supports selection and translate/rotation
//  of scene objects using the mouse.
//
//  Modifer keys for rotatation are x, y and z keys (for each axis of rotation)
//
//  Gregory Van Sciver
//

#include "ofMain.h"
#include "box.h"
#include "Primitives.h"
#include <glm/gtx/intersect.hpp>
#include <fstream>
#include <glm/gtx/string_cast.hpp>


class Joint : public Sphere {
public:
	Joint(glm::vec3 p, std::string n, float r = 0.25, ofColor diffuse = ofColor::lightGray) { position = p; name = n; radius = r; diffuseColor = diffuse; deleted = false; }
	Joint() { 
		position = glm::vec3(0, 0, 0);
		radius = 0.25;
		name = "joint0";
		deleted = false;
	}

	void draw() {
		if (deleted) { return; }
		Sphere::draw();
		// push matrix
		// ofmultiplymatrix
		// in here draw the cone
		// pop matrix
		glm::vec3 ones = glm::vec3(1, 1, 1);
		// draw line test
		Cone bone;
		for (SceneObject *c : childList) {
			//ofDrawLine(this->getPosition(), c->getPosition());
			glm::vec3 segment = c->getPosition() - this->getPosition();
			bone = *new Cone(this->getPosition() + (segment/2), normalize(segment), ones);
			bone.height = (glm::distance(this->getPosition(), c->getPosition()) - (radius * 2));
			ofSetColor(ofColor::white);
			bone.draw();
		}
		
	}

	bool deleted;
};


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		static void drawAxis(glm::mat4 transform = glm::mat4(1.0), float len = 1.0);
		bool mouseToDragPlane(int x, int y, glm::vec3 &point);
		void printChannels(SceneObject *);
		bool objSelected() { return (selected.size() ? true : false ); };
		
		// Lights
		//
		ofLight light1;
	
		// Cameras
		//
		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera topCam;
		ofCamera  *theCam;    // set to current camera either mainCam or sideCam

		// Materials
		//
		ofMaterial material;


		// scene components
		//
		vector<SceneObject *> scene;
		vector<SceneObject *> selected;
		ofPlanePrimitive plane;

		// state
		bool bDrag = false;
		bool bHide = true;
		bool bAltKeyDown = false;
		bool bRotateX = false;
		bool bRotateY = false;
		bool bRotateZ = false;
		glm::vec3 lastPoint;

		Joint *parentJoint, *childJoint, *deleteJoint;
		vector<Joint *> joints;
		std::string jointName = "joint";
		int jointCount = 0;
};