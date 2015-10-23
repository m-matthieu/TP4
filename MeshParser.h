#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <QtCore>


class MeshParser {
public:
	MeshParser();
	~MeshParser();

	void from_PLY_file( const char * );

	float posX, posY, posZ;
	float rotX, rotY, rotZ;
	float scaleX, scaleY, scaleZ;

	int nbVertex;
	int nbFaces;

	float *vertex;
	float *normal;

	std::vector<std::vector<int>> faces;
protected:
private:
};

