#pragma once

#include "ofMain.h"
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/noise.hpp>

class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};


class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { /*cout << "SceneObject::intersect" << endl;*/ return false; }
	virtual float sdf(const glm::vec3 &p) { return 0; }

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
};


class PerlinWater : public SceneObject {
public:
	PerlinWater(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::white, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
	}

	PerlinWater() {
		normal = glm::vec3(0, 1, 0);
	}

	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }

	float sdf(const glm::vec3 & p) override {
		//float a = glm::perlin()
		return 0;
	}
	
	glm::vec3 normal;

	float width = 20;
	float height = 20;
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

	float sdf(const glm::vec3 &p) override {

		//glm::vec3 tempP = glm::vec3(glm::mod(p.x, 20.0f), glm::mod(p.y, 20.0f), glm::mod(p.z, 20.0f));
		//return glm::distance( tempP, position) - radius;
		return glm::distance(p, position) - radius;
	}

	float radius;
};


class RoundedCylinder : public SceneObject {
public:
	RoundedCylinder(glm::vec3 p, float ra, float rb, float h, ofColor diffuse = ofColor::lightGray) {
		position = p; radiusA = ra; radiusB = rb; height = h; diffuseColor = diffuse;
	}

	void draw() {
		ofDrawSphere(position, radiusB);
	}

	float sdf(const glm::vec3 &p) override {
		glm::vec2 d = glm::vec2(glm::distance(glm::vec2(p.x,p.z),glm::vec2(position.x,position.z)) - 2.0*radiusA + radiusB, abs(p.y) - height);
		return min(max(d.x, d.y), 0.0f) + glm::distance(max(d, 0.0f),glm::vec2(position.x,position.y)) - radiusB;
	}
	float radiusA, radiusB, height;
};



class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}

	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	float sdf(const glm::vec3 & p) override {
		if (abs(p.x) < width / 2 && abs(p.z) < height) {
			return glm::dot(p, glm::normalize(normal)) + (p.y + 2);
		}
		else if (abs(p.x) > width / 2 && !(abs(p.z) > height / 2)) {
			// know that width is over but height(depth) is within
			// z doesn't matter, distance of x and y to 
			// 
			return glm::distance(glm::vec2(abs(p.x), p.y), glm::vec2(width / 2, position.y));
		}
		else if (abs(p.z) > height / 2 && !(abs(p.x) > width / 2)) {
			return glm::distance(glm::vec2(abs(p.z), p.y), glm::vec2(height / 2, position.y));
		}
		else {
			// abs of both x and y, distance to point
			return glm::distance(glm::vec3(abs(p.x), p.y, abs(p.z)), glm::vec3(width / 2, position.y, height / 2));
		}
		//return glm::dot(glm::normalize(normal), (p - position));
	}



	ofPlanePrimitive plane;
	glm::vec3 normal;

	float width = 20;
	float height = 20;
};

// view plane for render camera
// 
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

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
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
	Light(glm::vec3 p, float intensity) { this->position = p; this->intensity = intensity; }
	void draw() { ofDrawSphere(position, .2); };

	glm::vec3 position;
	float intensity;
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
		void rayMarchLoop();
		bool rayMarch(Ray r, glm::vec3 &p);
		float sceneSDF(glm::vec3 point);
		glm::vec3 getNormalRM(const glm::vec3 &p);
		ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power);
		bool inShadow(Ray r);
		float ofApp::opRep(glm::vec3 p, glm::vec3 c, SceneObject* prim);


		ofCamera *theCam;
		ofEasyCam mainCam;
		ofCamera previewCam;
		RenderCam renderCam;

		Sphere s1 = Sphere(glm::vec3(2.5, 2.5, 2.5), 2, ofColor::blue);
		//RoundedCylinder rc1 = RoundedCylinder(glm::vec3(3.5, 1.5, 2.5), .1, .75, 1.5, ofColor::yellow);
		//Plane floor = Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor(238, 238, 238));
		SceneObject* scene[1] = { &s1 };

		Light light1 = Light(glm::vec3(3, 4, 2.5), 500.0);
		Light* lights[1] = { &light1 };

		glm::vec3 point, rmNormal;
		bool rmHit;
		float rmDist, closestDistance, tempDist;
		int hitObjectIndex;
		float pointIntensity;
		ofColor ambient = ofColor(100,100,0);

		int totalHits, hit1, hit2, hit3;

		ofImage image;
		float imageHeight = 800;
		float imageWidth = 1200;
		filesystem::path path = "images/image1.png";
};
