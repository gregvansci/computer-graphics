#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetBackgroundColor(ofColor::black);
	
	// floor
	scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor(238, 238, 238)));
	// back wall
	scene.push_back(new Plane(glm::vec3(0, 3, -10), glm::vec3(0, 0, 1), ofColor(238, 238, 238)));
	// right wall
	scene.push_back(new Plane(glm::vec3(10, 3, 0), glm::vec3(-1, 0, 0), ofColor(238, 238, 238)));
	// left wall
	scene.push_back(new Plane(glm::vec3(-10, 3, 0), glm::vec3(1, 0, 0), ofColor(238, 238, 238)));
	// front wall
	// wouldn't work

	


	scene.push_back(new MirrorSphere(glm::vec3(0, 0, -2), 1.75, 1.0, ofColor(212, 225, 236)));
	scene.push_back(new Sphere(glm::vec3(hexRad, -1.0, -2), 0.75, ofColor::red));
	scene.push_back(new Sphere(glm::vec3(hexRad / 2, -1.0, (sqrt(3) * hexRad / 2) - 2), 0.75, ofColor::orange));
	scene.push_back(new Sphere(glm::vec3(-hexRad / 2, -1.0, (sqrt(3) * hexRad / 2) - 2), 0.75, ofColor::yellow));
	scene.push_back(new Sphere(glm::vec3(-hexRad, -1.0, -2), 0.75, ofColor::green));
	scene.push_back(new Sphere(glm::vec3(-hexRad / 2, -1.0, (-sqrt(3) * hexRad / 2) - 2), 0.75, ofColor::blue));
	scene.push_back(new Sphere(glm::vec3(hexRad / 2, -1.0, (-sqrt(3) * hexRad / 2) - 2), 0.75, ofColor::purple));


	lights.push_back(new AreaLight(glm::vec3(0, 12.0, 0), 650.0, ceilingLight));

	theCam = &mainCam;
	mainCam.setDistance(20);
	mainCam.setNearClip(1);

	previewCam.setPosition(glm::vec3(0, 0, 10));
	previewCam.lookAt(glm::vec3(0, 0, -1));
	previewCam.setNearClip(1);

	lightCam.setPosition(lights[0]->position);
	lightCam.lookAt(glm::vec3(0, 0, 0));

	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	theCam->begin();
	//renderCam.draw();
	for (auto& obj : scene) {
		ofSetColor(obj->diffuseColor);
		obj->draw();
	}
	for (auto& light : lights) {
		ofSetColor(ofColor::grey);
		light->draw();
	}
	renderCam.draw();
	theCam->end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &lightCam;
		break;
	case OF_KEY_F3:
		theCam = &previewCam;
		rayTrace();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::rayTrace() {
	for (int j = 0; j < imageHeight; j++) {
		if (j % 200 == 0) cout << "y pixel: " << j << endl;
		for (int i = 0; i < imageWidth; i++) {
			float u = (i + .5) / imageWidth;
			float v = (j + .5) / imageHeight;
			Ray ray = renderCam.getRay(u, v);
			bool hit = false;
			float distance = std::numeric_limits<float>::infinity();
			SceneObject* closestObject = NULL;

			for (SceneObject* obj : scene) {
				// determine if we hit the object and save closest obj
				if (obj->intersect(ray, intersectPt, intersectNorm)) {
					// determine if object is closest
					float tempDistance = sqrt(pow(intersectPt.x - ray.p.x, 2) + pow(intersectPt.y - ray.p.y, 2) + pow(intersectPt.z - ray.p.z, 2) * 1.0);
					if (distance > tempDistance) {
						distance = tempDistance;
						closestObject = obj;
					}
				}
				if (closestObject) { hit = true; }
			}
			if (hit) {
				closestObject->intersect(ray, intersectPt, intersectNorm);
				ofColor color = phong(intersectPt, intersectNorm, closestObject->diffuseColor, closestObject->specularColor, closestObject->reflectiveness, 40.0);
				// add ambient lighting value ato phong color
				image.setColor(i, imageHeight - j - 1, color + (closestObject->diffuseColor * ambient));
			}
			else { image.setColor(i, imageHeight - j - 1, ofGetBackgroundColor()); }
		}
	}
	image.save(path, OF_IMAGE_QUALITY_BEST);
}

ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float reflectiveness, float power) {
	ofColor color = ambient * diffuse;
	v = normalize(renderCam.position - p);
	n = normalize(norm);
	// reflectiveness and diffuseAmount sum to 100%.
	float diffuseAmount = 1 - reflectiveness;
	totalIntensity = 0;

	for (AreaLight *light : lights) {
		
		for (int i = 0; i < samplePts; i++) {
			meshPt = light->verts.at(rand() % light->verts.size());
			l = normalize(meshPt - p);
			if (!inShadow(Ray((p + .0001*n), l))) {
				pointIntensity = (light->intensity / pow(glm::distance(meshPt, p), 2)) / samplePts;
				totalIntensity += pointIntensity;
				// add diffuse lighting
				color += (diffuseAmount * diffuse * pointIntensity * glm::dot(n, l));

				// add specular lighting, if not a mirror
				if (reflectiveness == 0) {
					h = normalize(v + l);
					color += (specular * pointIntensity * pow(glm::dot(n, h), power));
				}
			}
		}
	}
	// add mirror lighting, if a mirror
	if (reflectiveness != 0) {
		// calculate reflect ray
		reflectRay = new Ray(p, normalize(2 * glm::dot(n, v) * n - v));


		// ray trace it using above method

		bool hit = false;
		float distance = std::numeric_limits<float>::infinity();
		SceneObject* closestObject = NULL;

		for (SceneObject* obj : scene) {
			// determine if we hit the object and save closest obj
			if (obj->intersect(*reflectRay, intersectPt, intersectNorm)) {
				// determine if object is closest
				float tempDistance = sqrt(pow(intersectPt.x - reflectRay->p.x, 2) + pow(intersectPt.y - reflectRay->p.y, 2) + pow(intersectPt.z - reflectRay->p.z, 2) * 1.0);
				if (distance > tempDistance) {
					distance = tempDistance;
					closestObject = obj;
				}
			}
			if (closestObject) { hit = true; }
		}
		if (hit) {
			closestObject->intersect(*reflectRay, intersectPt, intersectNorm);
			color += reflectiveness * totalIntensity * phong(intersectPt, intersectNorm, closestObject->diffuseColor, closestObject->specularColor, closestObject->reflectiveness, 40.0);
		}
		else {
			// placeholder, nothing for now
		}
	}

	// L = L ambient + L diffuse + L specular + L mirror
	return color;

}




// use point sleightly above surface of object = .0001
// lift in normal direction
bool ofApp::inShadow(Ray r) {
	glm::vec3 intersectPtShade;
	glm::vec3 intersectNormShade;
	for (SceneObject* sObject : scene) {
		if (sObject->intersect(r, intersectPtShade, intersectNormShade)) { return true; }
	}
	return false;
}

// Intersect Ray with Plane  (wrapper on glm::intersect*
//
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}

bool MirrorPlane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}

// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}