#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetBackgroundColor(ofColor::black);
	theCam = &mainCam;
	mainCam.setDistance(20);
	mainCam.setNearClip(1);

	previewCam.setPosition(glm::vec3(0, 0, 10));
	previewCam.lookAt(glm::vec3(0, 0, -1));

	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	theCam->begin();

	//ofDrawGrid();
	for (SceneObject* s : scene) {
		ofSetColor(s->diffuseColor);
		s->draw();
	}

	theCam->end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &previewCam;
		rayMarchLoop();
		cout << totalHits << ": 1-" << hit1 << ", 2-" << hit2 << ", 3-" << hit3 << endl;
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

void ofApp::rayMarchLoop() {
	for (int i = 0; i < imageHeight; i++) {
		for (int j = 0; j < imageWidth; j++) {

			float u = (j + .5) / imageWidth;
			float v = (i + .5) / imageHeight;
			Ray ray = renderCam.getRay(u, v);
			bool hit = rayMarch(ray, point);

			if (hit) {
				SceneObject* rmHitObject = scene[hitObjectIndex];
				rmNormal = getNormalRM(point);
				ofColor color = phong(point, rmNormal, rmHitObject->diffuseColor, rmHitObject->specularColor, 500);
				//ofColor color = ofColor::blue;
				image.setColor(j, imageHeight - i - 1, color);
			}
			else {
				image.setColor(j, imageHeight - i - 1, ofColor::black);
			}
		}
	}
	image.save(path, OF_IMAGE_QUALITY_BEST);
}

bool ofApp::rayMarch(Ray r, glm::vec3 &p) {
	rmHit = false;
	p = r.p;
	for (int i = 0; i < 50; i++) {
		//rmDist = sceneSDF(p);
		rmDist = opRep(p, glm::vec3(20, 20, 20), &s1);
		totalHits++;
		if (rmDist < 0.001) {
			rmHit = true;
			point = p;
			hit1++;
			break;
		}
		else if (rmDist > 20) {
			hit2++;
			break;
		}
		else {
			hit3++;
			p = p + r.d*rmDist;
		}
	}
	return rmHit;
}

float ofApp::sceneSDF(glm::vec3 point) {
	closestDistance = FLT_MAX;
	int i = 0;


	for (SceneObject* sObject : scene) {
		tempDist = opRep(point, glm::vec3(5, 5, 5), sObject);
		//tempDist = sObject->sdf(point);
		if (tempDist < closestDistance) {
			closestDistance = tempDist;
			hitObjectIndex = i;
		}
		i++;
	}
	return closestDistance;
}

glm::vec3 ofApp::getNormalRM(const glm::vec3 &p) {
	float eps = 0.01;
	float dp = sceneSDF(p);
	glm::vec3 n(dp - sceneSDF(glm::vec3(p.x - eps, p.y, p.z)),
		dp - sceneSDF(glm::vec3(p.x, p.y - eps, p.z)),
		dp - sceneSDF(glm::vec3(p.x, p.y, p.z - eps)));
	return glm::normalize(n);
}

ofColor ofApp::phong(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, float power) {
	ofColor color = ambient;
	glm::vec3 v = normalize(renderCam.position - p);
	for (Light* light : lights) {
		glm::vec3 l = normalize(light->position - p);
		if (!inShadow(Ray((p + .0001*norm), l))) {
			pointIntensity = light->intensity / pow(glm::distance(light->position, p), 2);
			glm::vec3 h = normalize(v + l);
			color += (diffuse * pointIntensity * glm::dot(glm::normalize(norm), glm::normalize(light->position - p)));
			color += (specular * pointIntensity * pow(glm::dot(glm::normalize(norm), h), power));
		}
	}
	return color;
}

bool ofApp::inShadow(Ray r) {
	glm::vec3 intersectPtShade;
	glm::vec3 intersectNormShade;
	for (SceneObject* sObject : scene) {
		if (sObject->intersect(r, intersectPtShade, intersectNormShade)) { return true; }
	}
	return false;
}

float ofApp::opRep(glm::vec3 p, glm::vec3 c, SceneObject* prim) {
	glm::vec3 q = mod(p + 0.5*c, c) - 0.5*c;
	return prim->sdf(q);
}

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

