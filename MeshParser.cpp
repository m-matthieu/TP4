#include "MeshParser.h"


MeshParser::MeshParser() {
}


MeshParser::~MeshParser() {
}

void MeshParser::from_PLY_file( const char * fileName ) {
	char line[100];
	std::fstream file;

	nbVertex = 0;
	nbFaces = 0;

	file.open( fileName, std::fstream::in );

	if( file.is_open() ) {
		// -----------------------------------
		// HEADER
		// -----------------------------------
		file.getline( line, 100 ); // ply
		file.getline( line, 100 ); // format ascii 1.0
		file.getline( line, 100 ); // comment
		file.getline( line, 100, ' ' ); file.getline( line, 100, ' ' ); file.getline( line, 100 ); // element vertex X
		sscanf( line, "%i", &nbVertex );
		qDebug() << nbVertex << "vertices";
		file.getline( line, 100 ); // property float x
		file.getline( line, 100 ); // property float y
		file.getline( line, 100 ); // property float z
		file.getline( line, 100 ); // property float nx
		file.getline( line, 100 ); // property float ny
		file.getline( line, 100 ); // property float nz
		file.getline( line, 100, ' ' ); file.getline( line, 100, ' ' ); file.getline( line, 100 ); // element face X
		sscanf( line, "%i", &nbFaces );
		qDebug() << nbFaces << "faces";
		file.getline( line, 100 ); // property list uchar uint vertex_indices
		file.getline( line, 100 ); // end_header

		// -----------------------------------
		// VERTICES & NORMALS
		// -----------------------------------

		vertex = (float *)malloc( nbVertex * 3 * sizeof( float ) );
		normal = (float *)malloc( nbVertex * 3 * sizeof( float ) );

		float pX, pY, pZ, nX, nY, nZ;
		int vIdx = 0;
		int nIdx = 0;
		for( int i = 0; i != nbVertex; ++i ) {
			file.getline( line, 100 );
			sscanf( line, "%f %f %f %f %f %f", &pX, &pY, &pZ, &nX, &nY, &nZ );

			vertex[vIdx++] = pX;
			vertex[vIdx++] = pY;
			vertex[vIdx++] = pZ;

			normal[nIdx++] = nX;
			normal[nIdx++] = nY;
			normal[nIdx++] = nZ;
		}

		// -----------------------------------
		// FACES
		// -----------------------------------

		for( int i = 0; i != nbFaces; ++i ) {
			file.getline( line, 100, ' ' ); // nombre de points pour cette face (devrait toujours être 3)
			int nbPointPerFace = 0;
			sscanf( line, "%i", &nbPointPerFace );
			std::vector<int> facePoints;
			int pointIndex;
			for( int j = 0; j != nbPointPerFace - 1; ++j ) {
				file.getline( line, 100, ' ' );
				sscanf( line, "%i", &pointIndex );
				facePoints.push_back( pointIndex );
			}

			file.getline( line, 100 );
			sscanf( line, "%i", &pointIndex );
			facePoints.push_back( pointIndex );

			faces.push_back( facePoints );
		}
		file.close();
	} else {
		qDebug() << "file not found !";
	}

	qDebug() << "MESH LOADED";
}