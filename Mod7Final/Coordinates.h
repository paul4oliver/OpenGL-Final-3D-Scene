#pragma once
# include <vector>
# include <GL/glew.h> 

// Class to hold position, normal, and texture coordinates for each primitve object
class Coordinates
{
public:
	// Functions return coordinates for objects
	static std::vector<GLfloat>* getPlaneCoords();
	static std::vector<GLfloat>* getMilkBotCoords();
	static std::vector<GLfloat>* getMilkTopCoords();
	static std::vector<GLfloat>* getCapTopCoords();
	static std::vector<GLfloat>* getCapSideCoords();
	static std::vector<GLfloat>* getMilkPlaneCoords();
	static std::vector<GLfloat>* getBoxCoords();
	static std::vector<GLfloat>* getDonutCoords();
	static std::vector<GLfloat>* getGlassTopcoords();
	static std::vector<GLfloat>* getGlassSideCoords();
	static std::vector<GLfloat>* getLightCoords();
}; 
