#pragma once

#include "ofMain.h"
#include <glm/gtx/intersect.hpp>
#include <fstream>

class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }
	glm::vec3 evalPoint(float t) { return (p + t * d); }

	glm::vec3 p, d;
};

class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { /*cout << "SceneObject::intersect" << endl;*/ return false; }
	ofColor getDiffuse() { return diffuseColor; }

	glm::vec3 position = glm::vec3(0, 0, 0);
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
	float reflectiveness = 0;
};

class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofDrawSphere(position, radius);
	}
	float radius = 1.0;
};

// Mirror Sphere, Sphere class with reflectiveness value

class MirrorSphere : public Sphere {
public:
	MirrorSphere(glm::vec3 p, float r, float refl, ofColor diffuse = ofColor(212,225,236)) {
		position = p; 
		radius = r; 
		reflectiveness = refl;
		diffuseColor = diffuse; 
	}
	MirrorSphere() {}	
};


class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);		
		if (normal == glm::vec3(1, 0, 0)) plane.rotateDeg(90, 0, 1, 0);
		if (normal == glm::vec3(-1, 0, 0)) plane.rotateDeg(-90, 0, 1, 0);
		if (normal == glm::vec3(0, 0, -1)) plane.rotateDeg(180, 0, 1, 0);
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	float sdf(const glm::vec3 & p);
	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	ofPlanePrimitive plane;
	glm::vec3 normal;

	float width = 20;
	float height = 20;
};


// Too slow, need to stop memory leak to use

class MirrorPlane : public Plane {
public:
	MirrorPlane(glm::vec3 p, glm::vec3 n, float refl, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		reflectiveness = refl;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
		if (normal == glm::vec3(1, 0, 0)) plane.rotateDeg(90, 0, 1, 0);
		if (normal == glm::vec3(-1, 0, 0)) plane.rotateDeg(-90, 0, 1, 0);
		if (normal == glm::vec3(0, 0, -1)) plane.rotateDeg(180, 0, 1, 0);
	}
	MirrorPlane() {
		normal = glm::vec3(0, 1, 0);
		reflectiveness = 1.0;
		plane.rotateDeg(90, 1, 0, 0);
	}
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	float sdf(const glm::vec3 & p);
	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	ofPlanePrimitive plane;
	glm::vec3 normal;

	float width = 20;
	float height = 20;
};

class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }
	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}
	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }
	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]
	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}
	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	glm::vec2 min, max;
};

class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10.0);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};

class Light : public SceneObject {
public:
	Light() {}
	Light(glm::vec3 p, float intensity) { this->position = p; this->intensity = intensity; isArea = false; }
	void draw() { ofDrawSphere(position, .2); };

	glm::vec3 position;
	float intensity;
	bool isArea;
	std::vector < glm::vec3 > verts;
};

class AreaLight : public SceneObject {
public:
	AreaLight(glm::vec3 p, float intensity, std::string objshape) {
		this->position = p;
		this->intensity = intensity;
		// go through file and save all vertices to a vector
		ifstream inStream;
		inStream.open(objshape);
		std::string line;
		while (std::getline(inStream, line)) {
			if (line.size() > 2) {
				string frontTwo = line.substr(0, 2);

				if (frontTwo == "v ") {
					float vx, vy, vz;
					int spIndex, nextspIndex;
					std::string temp;

					spIndex = line.find(' ');
					temp = line.substr(spIndex + 1);
					nextspIndex = temp.find(' ');
					vx = std::stof(temp.substr(0, nextspIndex));

					temp = temp.substr(nextspIndex + 1);
					nextspIndex = temp.find(' ');
					vy = std::stof(temp.substr(0, nextspIndex));

					vz = std::stof(temp.substr(nextspIndex + 1));
					// push back the 3d point of each vertex plus the offset
					verts.push_back(glm::vec3(vx + p.x, vy + p.y, vz + p.z));
				}
			}
		}
	}
	void draw() {
		for (glm::vec3 a : verts) {
			ofDrawSphere(a, 0.1);
		}
	};

	float intensity;
	std::vector < glm::vec3 > verts;
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void rayTrace();
	ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, const float reflectiveness, float power);
	bool inShadow(Ray r);

	ofEasyCam mainCam;
	RenderCam renderCam;
	ofCamera previewCam;
	ofCamera lightCam;
	ofCamera *theCam;

	std::vector < SceneObject* > scene;
	std::vector < AreaLight* > lights;

	ofImage image;
	glm::vec3 intersectPt;
	glm::vec3 intersectNorm;
	glm::vec3 v, l, h, n;
	glm::vec3 meshPt;
	Ray *reflectRay;
	float pointIntensity, totalIntensity;
	ofColor ambient = ofColor(40, 40, 40);
	ofColor reflColor;
	int samplePts = 100;


	int imageWidth = 3000;
	int imageHeight = 2000;
	filesystem::path path = "images/image.png";
	std::string ceilingLight = "C:/Users/gregv/Documents/of_v0.11.2_vs2017_release/apps/myApps/FinalProject/bin/data/models/ceilingLight.obj";

	float hexRad = 5.0;  // radius distance of hexagon to calculate spheres around the mirror
};
