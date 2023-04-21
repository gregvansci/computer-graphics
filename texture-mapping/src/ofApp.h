#pragma once

#include "ofMain.h"
#include <glm/gtx/intersect.hpp>
#include <algorithm>

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { /*cout << "SceneObject::intersect" << endl;*/ return false; }
	virtual ofColor textureLookup(glm::vec3 p) { return diffuseColor; }
	virtual float sdf(const glm::vec3 &p) { return 0; }

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {

public:
	Sphere(glm::vec3 p, float r, filesystem::path t, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; texture.load(t); }
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofDrawSphere(position, radius);
	}
	ofColor textureLookup(glm::vec3 p) override {

		// calculate u, v
		glm::vec3 sRay = ofVec3f(p.x - position.x, p.y - position.y, p.z - position.z).getNormalized();
		float pi = atan(1) * 4;
		float u = 0.5 + atan2(sRay.z, sRay.x) / (2 * pi);
		float v = 0.5 - asin(sRay.y) / pi;

		// convert u, v to i, j
		int i = texture.getWidth() - (u  * texture.getWidth()) - .5;
		int j = (v  * texture.getHeight()) - .5;

		// return color at i,j texel in texture1
		return texture.getColor(i, j);
		//return ofColor::white; 
	}

	float sdf(const glm::vec3 &p) override {
		return glm::distance(p, position) - radius;
	}

	ofImage texture;
	float radius;
};


class Torus : public SceneObject {

public:
	Torus(glm::vec3 p, float r1, float r2, ofColor diffuse = ofColor::lightGray) 
	{ position = p; t = glm::vec2(r1, r2); diffuseColor = diffuse; }
	Torus() {}
	
	void draw() {
		ofDrawCircle(position, t.x);
	}
	

	float sdf(const glm::vec3 &p1) override {

		glm::mat4 m = glm::translate(glm::mat4(1.0), position);
		glm::mat4 M = glm::rotate(m, glm::radians(45.0f), glm::vec3(1, 0, 0));
		glm::vec3 p = glm::inverse(M)*glm::vec4(p1, 1);
		glm::vec2 q = glm::vec2(glm::length(glm::vec2(p.x, p.z)) - t.x, p.y);
		return glm::length(q) - t.y;
	}

	glm::vec2 t;

};

class Box : public SceneObject {

public:
	Box(glm::vec3 p, glm::vec3 b, ofColor diffuse = ofColor::lightGray)
	{
		position = p; bound=b; diffuseColor = diffuse;
	}
	Box() {}

	void draw() {
		//ofDrawCircle(position, radius);
	}


	float sdf(const glm::vec3 &p) override {
		glm::vec3 value = abs(p) - bound;
		if (value.x < 0.) {
			value.x = 0.;
		}
		if (value.y < 0.) {
			value.y = 0.;
		}
		if (value.z < 0.) {
			value.z = 0.;
		}
		return glm::distance(value, position);
	}

	glm::vec3 bound;

};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
		texture.load("images/texture2.jpg");
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}

	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	ofColor textureLookup(glm::vec3 p) override {
		float textureLength = 2.5;
		// find u,v
		float u = p.x + (width / 2);
		float v = p.z + (height / 2);
		// use fmod() to get i,j 
		int i = fmod(u, textureLength) / textureLength * texture.getWidth();
		int j = fmod(v, textureLength) / textureLength * texture.getHeight();
		// return color at i,j texel in texture1
		return texture.getColor(i, j);
		//return ofColor::white; 
	}
	float sdf(const glm::vec3 & p) override {
		
		return p.y - position.y;
		//return glm::dot(glm::normalize(normal), (p - position));
	}



	ofPlanePrimitive plane;
	glm::vec3 normal;
	ofImage texture;

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


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
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
	void rmRayTrace();
	void drawGrid();
	void drawAxis(glm::vec3 position);
	ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse);
	ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power);
	bool inShadow(Ray r);
	float sceneSDF(glm::vec3 point);
	bool rayMarch(Ray r, glm::vec3 &p);
	glm::vec3 getNormalRM(const glm::vec3 &p);




	bool bHide = true;
	bool bShowImage = false;

	ofEasyCam  mainCam;
	RenderCam renderCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera firstLightCam;
	ofCamera secondLightCam;
	ofCamera  *theCam;

	Sphere sOne = Sphere(glm::vec3(-3.5, 0, -3), 1.5, filesystem::path("images/texture5.jpg"), ofColor(255,109,105));
	Sphere sTwo = Sphere(glm::vec3(0, -1.25, 2.5), .75, filesystem::path("images/texture13.jpg"), ofColor(1,11,139));
	Sphere sThree = Sphere(glm::vec3(3, -1, -1), 1, filesystem::path("images/texture8.png"), ofColor(30,5,33));
	Torus tOne = Torus(glm::vec3(0, 2.5, 0), 1, .25, ofColor(254,204,80));
	Box lOne = Box(glm::vec3(0, 0, 0), glm::vec3(1,1,1), ofColor(11,231,251));
	//Sphere sFour = Sphere(glm::vec3(0, 4, -15), 1.75, filesystem::path("images/texture15.jpg"), ofColor(119, 217, 112));
	//Sphere sFive = Sphere(glm::vec3(0, 3.5, -12), .5, filesystem::path("images/texture14.jpg"), ofColor(119, 217, 112));

	Plane floorPlane = Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor(238, 238, 238));

	Light light1 = Light(glm::vec3(3, 8, 3), 75.0);
	Light light2 = Light(glm::vec3(-5, 2, 4), 60.0);
	Light light3 = Light(glm::vec3(0, 1, -12), 30.0);


	SceneObject* scene[6] = { &sOne, &sTwo, &sThree, &floorPlane, &lOne, &tOne};
	Light* lights[3] = { &light1, &light2, &light3 };

	ofImage image;
	ofImage texture1;
	glm::vec3 intersectPt;
	glm::vec3 intersectNorm;
	float pointIntensity;
	ofColor ambient = ofColor(0, 0, 0);

	float closestDistance, d, rmDist;
	bool rmHit;
	int hitObjectIndex;
	glm::vec3 point, rmNormal;


	int imageWidth = 1200;
	int imageHeight = 800;
	filesystem::path path = "C:/Users/gregv/Documents/of_v0.11.2_vs2017_release/apps/myApps/BProject2/src/images/image1.png";
};

