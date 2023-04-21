#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetBackgroundColor(ofColor::black);
	theCam = &mainCam;
	mainCam.setDistance(20);
	mainCam.setNearClip(1);

	sideCam.setPosition(glm::vec3(50, 0, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));

	previewCam.setPosition(glm::vec3(0, 0, 10));
	previewCam.lookAt(glm::vec3(0, 0, -1));

	firstLightCam.setPosition(lights[0]->position);
	firstLightCam.lookAt(glm::vec3(0, 0, 0));

	secondLightCam.setPosition(lights[1]->position);
	secondLightCam.lookAt(glm::vec3(0, 0, 0));

	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
	theCam->begin();

	for (SceneObject* s : scene) {
		ofSetColor(s->diffuseColor);
		s->draw();
	}

	for (Light* l : lights) {
		ofSetColor(ofColor::grey);
		l->draw();
	}

	theCam->end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &previewCam;
		rayTrace();
		break;
	case OF_KEY_F4:
		theCam = &previewCam;
		rmRayTrace();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

void ofApp::rayTrace() {

	for (int j = 0; j < imageHeight; j++) {
		for (int i = 0; i < imageWidth; i++) {

			// convert each i, j to (u, v)
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
				//ofColor color = lambert(intersectPt, intersectNorm, closestObject->diffuseColor);
				ofColor textureDiffuse = closestObject->textureLookup(intersectPt);
				ofColor color = phong(intersectPt, intersectNorm, textureDiffuse, closestObject->specularColor, 5000);
				image.setColor(i, imageHeight - j - 1, color);
			}
			else { image.setColor(i, imageHeight - j - 1, ofGetBackgroundColor()); }
		}
	
	}
	image.save(path, OF_IMAGE_QUALITY_BEST);
}

void ofApp::rmRayTrace() {
	for (int j = 0; j < imageHeight; j++) {
		for (int i = 0; i < imageWidth; i++) {

			// convert each i, j to (u, v)
			float u = (i + .5) / imageWidth;
			float v = (j + .5) / imageHeight;
			Ray ray = renderCam.getRay(u, v);
			bool hit = rayMarch(ray, point);

			if (hit) {
				SceneObject* rmHitObject = scene[hitObjectIndex];
				rmNormal = getNormalRM(point);
				ofColor color = phong(point, rmNormal, rmHitObject->diffuseColor, rmHitObject->specularColor, 500);
				image.setColor(i, imageHeight - j - 1, color);
			}
			else { image.setColor(i, imageHeight - j - 1, ofGetBackgroundColor()); }
		}
		if (j % 50 == 0) {
			cout << ".";
		}
	}
	image.save(path, OF_IMAGE_QUALITY_BEST);
}

ofColor ofApp::lambert(const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse) {
	ofColor color = ambient;
	std::vector < Light* > pointLights;
	for (Light* light : lights) {
		pointIntensity = light->intensity / pow(glm::distance(light->position, p), 2);
		color += (diffuse * pointIntensity * glm::dot(glm::normalize(norm), glm::normalize(light->position - p)));
	}
	return color;
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

float ofApp::sceneSDF(glm::vec3 point) {
	closestDistance = FLT_MAX;
	int i = 0;

	for (SceneObject* sObject : scene) {
		d = sObject->sdf(point);
		if (d < closestDistance) {
			closestDistance = d;
			hitObjectIndex = i;
		}
		i++;
	}
	return closestDistance;
}

bool ofApp::rayMarch(Ray r, glm::vec3 & p) {
	rmHit = false;
	p = r.p;
	for (int i = 0; i < 100; i++) {
		rmDist = sceneSDF(p);
		if (rmDist < 0.01) {
			rmHit = true;
			point = p;
			break;
		}
		else if(rmDist > 20) {
			break;
		}
		else {
			p = p + r.d*rmDist;
		}
	}
	return rmHit;
}

glm::vec3 ofApp::getNormalRM(const glm::vec3 &p) {
	float eps = 0.01;
	float dp = sceneSDF(p);
	glm::vec3 n(dp - sceneSDF(glm::vec3(p.x - eps, p.y, p.z)),
				dp - sceneSDF(glm::vec3(p.x, p.y - eps, p.z)),
				dp - sceneSDF(glm::vec3(p.x, p.y, p.z - eps)));
	return glm::normalize(n);
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




