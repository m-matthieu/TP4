#include "trianglewindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <time.h>
//#include <sys/time.h>
#include <iostream>

#include <QtCore>
#include <QtGui>

//#include <omp.h>

int numParticules = 1000;
int minP = 0;
int maxP = 360;


using namespace std;

TriangleWindow::TriangleWindow()
{
    nbTick = 0;
    m_frame = 0;
    maj = 20;
    QString s ("FPS : ");
    s += QString::number(1000/maj);
    s += "(";
    s += QString::number(maj);
    s += ")";
    setTitle(s);
    timer = new QTimer();
    timer->connect(timer, SIGNAL(timeout()),this, SLOT(renderNow()));
    timer->start(maj);

    master = true;
}
TriangleWindow::TriangleWindow(int _maj)
{
    nbTick = 0;
    m_frame = 0;
    maj = _maj;
    QString s ("FPS : ");
    s += QString::number(1000/maj);
    s += "(";
    s += QString::number(maj);
    s += ")";
    setTitle(s);
    timer = new QTimer();
    timer->connect(timer, SIGNAL(timeout()),this, SLOT(renderNow()));
    timer->start(maj);
}

void TriangleWindow::setSeason(int i)
{
    season = i;

    if (i==0) day=80;
    else if (i==1) day = 170;
    else if (i==2) day = 260;
    else if (i==3) day = 350;
}

void TriangleWindow::updateSeason()
{
    day = (day + 1) % 365;

    if (day==80) season = 0;
    else if (day==170) season = 1;
    else if (day==260) season = 2;
    else if (day==350) season = 3;
}


void TriangleWindow::initialize()
{
    const qreal retinaScale = devicePixelRatio();


    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);


	//if( master )
		//loadMap( ":/heightmap-1.png" );
	//else
		loadFromFile( "saveTest.tst" );

    particules = new point[numParticules];


    for(int i = 0; i < numParticules; i++)
    {
        int angle =minP + (rand() % (int)(maxP - minP + 1));
        int dist = (rand() % (int)(100 ));
        int alt = (rand() % (int)(100));
        float x = sin(
                      ((3.14159 * 2) *
                       angle
                       )/360
                      )*dist;
        float y = cos(
                      ((3.14159 * 2) *
                       angle
                       )/360
                      )*dist;

        particules[i].x = 0.5f + (float)+x/(m_image.width()) - ((float)m_image.width()/2.0)/m_image.width();
        particules[i].y = 0.5f + (float)y/(m_image.height()) - ((float)m_image.height()/2.0)/m_image.height();
        particules[i].z = (float)(alt)/100;
    }

}

void TriangleWindow::loadMap(QString localPath)
{

    if (QFile::exists(localPath)) {
        m_image = QImage(localPath);
		strcpy_s( picName, localPath.toStdString().c_str() );
    }


    uint id = 0;
    p = new point[m_image.width() * m_image.height()];
    QRgb pixel;
    for(int i = 0; i < m_image.width(); i++)
    {
        for(int j = 0; j < m_image.height(); j++)
        {

            pixel = m_image.pixel(i,j);

            id = i*m_image.width() +j;

            p[id].x = (float)i/(m_image.width()) - ((float)m_image.width()/2.0)/m_image.width();
            p[id].y = (float)j/(m_image.height()) - ((float)m_image.height()/2.0)/m_image.height();
            p[id].z = 0.001f * (float)(qRed(pixel));

        }
    }

	//if( master )
	//	saveToFile( "saveTest.tst" );
}

void TriangleWindow::render()
{
    nbTick += maj;

    if(nbTick >= 1000)
    {
        QString s ("FPS : ");
        s += QString::number(m_frame);
        s += "(th=";
        s += QString::number(1000/maj);
        s += "-";
        s += QString::number(maj);
        s += ")";
        s += " day ";
        s += QString::number(day);
        if (season==0) s += " Printemps ";
        else if (season==1) s += " Eté ";
        if (season==2) s += " Automne ";
        if (season==3) s += " Hiver ";
        setTitle(s);
        nbTick = 0;
        m_frame = 0;
    }
    glClear(GL_COLOR_BUFFER_BIT);


    glLoadIdentity();
    glScalef(c->ss,c->ss,c->ss);

    glRotatef(c->rotX,1.0f,0.0f,0.0f);
    if(c->anim == 0.0f)
    {
        glRotatef(c->rotY,0.0f,0.0f,1.0f);
    }
    else
    {
        glRotatef(c->anim,0.0f,0.0f,1.0f);
        if(master)
            c->anim +=0.05f;
    }



    switch(c->etat)
    {
        case 0:
            displayPoints();
            break;
        case 1:
            displayLines();
            break;
        case 2:
            displayTriangles();
            break;
        case 3:
            displayTrianglesC();
            break;
        case 4:
            displayTrianglesTexture();
            break;
        case 5:

            displayTrianglesTexture();
            displayLines();
            break;
        default:
            displayPoints();
            break;
    }
    if (season == 2)
        updateParticlesAut();
    else if (season == 3)
        updateParticlesHiv();

	for( int i = 0; i != mesh.size(); ++i ) {
		renderMesh( i );
	}

    m_frame++;

}

bool TriangleWindow::event(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::UpdateRequest:
            renderNow();
            return true;
        default:
            return QWindow::event(event);
    }
}

void TriangleWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {

        case 'C':
            if(c->anim == 0.0f)
                c->anim = c->rotY;
            else
                c->anim = 0.0f;
            break;
        case 'Z':
            c->ss += 0.10f;
            break;
        case 'S':
            c->ss -= 0.10f;
            break;
        case 'A':
            c->rotX += 1.0f;
            break;
        case 'E':
            c->rotX -= 1.0f;
            break;
        case 'Q':
            c->rotY += 1.0f;
            break;
        case 'D':
            c->rotY -= 1.0f;
            break;
        case 'W':
            c->etat ++;
            if(c->etat > 5)
                c->etat = 0;
            break;
        case 'P':
            maj++;
            timer->stop();
            timer->start(maj);
            break;
        case 'O':
            maj--;
            if(maj < 1)
                maj = 1;
            timer->stop();
            timer->start(maj);
            break;
        case 'L':
            maj = maj - 20;
            if(maj < 1)
                maj = 1;
            timer->stop();
            timer->start(maj);
            break;
        case 'M':
            maj = maj + 20;

            timer->stop();
            timer->start(maj);
            break;
        case 'X':
            carte ++;
            if(carte > 3)
                carte = 1;
            QString depth (":/heightmap-");
            depth += QString::number(carte) ;
            depth += ".png" ;

            loadMap(depth);
            break;


    }

}


void TriangleWindow::displayPoints()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    uint id = 0;
    for(int i = 0; i < m_image.width(); i++)
    {
        for(int j = 0; j < m_image.height(); j++)
        {
            id = i*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);

        }
    }
    glEnd();
}


void TriangleWindow::displayTriangles()
{
    if (season==0) glColor3f(0.5f, 0.8f, 0.5f);
    else if (season==1) glColor3f(0.8f,0.5f, 0.5f);
    else if (season==2) glColor3f(0.5f, 0.5f, 0.8f);
    else if (season==3) glColor3f(1.0f, 1.0f, 1.0f);


    glBegin(GL_TRIANGLES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);



            id = i*m_image.width() +(j+1);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
        }
    }

    glEnd();
}

void TriangleWindow::displayTrianglesC()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {
            glColor3f(0.0f, 1.0f, 0.0f);
            id = i*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);


            glColor3f(1.0f, 1.0f, 1.0f);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
        }
    }
    glEnd();
}


void TriangleWindow::displayLines()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);

            id = (i+1)*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = i*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);

            id = (i+1)*m_image.width() +j;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = i*m_image.width() +(j+1);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);

            id = i*m_image.width() +(j+1);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);

            id = (i+1)*m_image.width() +j+1;
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);

            id = (i+1)*m_image.width() +(j);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
        }
    }

    glEnd();
}

void TriangleWindow::displayTrianglesTexture()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            displayColor(p[id].z);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = i*m_image.width() +(j+1);
            displayColor(p[id].z);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j;
            displayColor(p[id].z);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);



            id = i*m_image.width() +(j+1);
            displayColor(p[id].z);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j+1;
            displayColor(p[id].z);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
            id = (i+1)*m_image.width() +j;
            displayColor(p[id].z);
            glVertex3f(
                       p[id].x,
                       p[id].y,
                       p[id].z);
        }
    }
    glEnd();
}


void TriangleWindow::displayColor(float alt)
{
    if (alt > 0.2)
    {
        glColor3f(01.0f, 1.0f, 1.0f);
    }
    else     if (alt > 0.1)
    {
        glColor3f(alt, 1.0f, 1.0f);
    }
    else     if (alt > 0.05f)
    {
        glColor3f(01.0f, alt, alt);
    }
    else
    {
        glColor3f(0.0f, 0.0f, 1.0f);
    }

}


void TriangleWindow::updateParticlesAut()
{
    int id2;
#pragma omp parallel
    {
#pragma omp for
        for(int id = 0; id < numParticules; id++)
        {
            particules[id].z -= 0.0003f * ((float) minP + (rand() % (int)(maxP - minP + 1)));
            id2 = (particules[id].x)*m_image.width() + particules[id].y;
			if( id2 < 0 ) id2 = 0;
            if(particules[id].z < p[id2].z)
            {
                int angle =minP + (rand() % (int)(maxP - minP + 1));
                int dist = (rand() % (int)(100 ));
                int alt = (rand() % (int)(100));
                float x = sin(
                              ((3.14159 * 2) *
                               angle
                               )/360
                              )*dist;
                float y = cos(
                              ((3.14159 * 2) *
                               angle
                               )/360
                              )*dist;

                particules[id].x = 0.5f + (float)+x/(m_image.width()) - ((float)m_image.width()/2.0)/m_image.width();
                particules[id].y = 0.5f + (float)y/(m_image.height()) - ((float)m_image.height()/2.0)/m_image.height();
                particules[id].z = (float)(alt)/100;

            }
        }
    }

    glColor3f(0.2f, 0.2f, 1.0f);
    glPointSize(0.01f);
    glBegin(GL_POINTS);
    for(int id = 0; id < numParticules; id++)
    {
        glVertex3f(
                   particules[id].x,
                   particules[id].y,
                   particules[id].z);


    }
    glEnd();
}

void TriangleWindow::updateParticlesHiv()
{
    int id2;
#pragma omp parallel
    {
#pragma omp for
        for(int id = 0; id < numParticules; id++)
        {
            particules[id].z -= 0.00001f * ((float) minP + (rand() % (int)(maxP - minP + 1)));
            id2 = (particules[id].x)*m_image.width() + particules[id].y;
			if( id2 < 0 ) id2 = 0;
			if( particules[id].z < p[id2].z )
            {
                int angle =minP + (rand() % (int)(maxP - minP + 1));
                int dist = (rand() % (int)(100 ));
                int alt = (rand() % (int)(100));
                float x = sin(
                              ((3.14159 * 2) *
                               angle
                               )/360
                              )*dist;
                float y = cos(
                              ((3.14159 * 2) *
                               angle
                               )/360
                              )*dist;

                particules[id].x = 0.5f + (float)+x/(m_image.width()) - ((float)m_image.width()/2.0)/m_image.width();
                particules[id].y = 0.5f + (float)y/(m_image.height()) - ((float)m_image.height()/2.0)/m_image.height();
                particules[id].z = (float)(alt)/100;
            }
        }
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(0.0001f);
    glBegin(GL_POINTS);
    for(int id = 0; id < numParticules; id++)
    {
        glVertex3f(
                   particules[id].x,
                   particules[id].y,
                   particules[id].z);


    }
    glEnd();
}

void TriangleWindow::addMesh( const char *c, float pX, float pY, float pZ, float rX, float rY, float rZ, float scX, float scY, float scZ ) {
	MeshParser *m = new MeshParser;
	m->from_PLY_file( c );

	m->posX = pX;
	m->posY = pY;
	m->posZ = pZ;

	m->rotX = rX;
	m->rotY = rY;
	m->rotZ = rZ;

	m->scaleX = scX;
	m->scaleY = scY;
	m->scaleZ = scZ;

	mesh.push_back( m );
}

void TriangleWindow::renderMesh( int i ) {
	// set matrix
	glRotatef( mesh[i]->rotX, 1, 0, 0 );
	glRotatef( mesh[i]->rotY, 0, 1, 0 );
	glRotatef( mesh[i]->rotZ, 0, 0, 1 );

	glTranslatef( mesh[i]->posX, mesh[i]->posY, mesh[i]->posZ );

	glScalef( mesh[i]->scaleX, mesh[i]->scaleY, mesh[i]->scaleZ );
	//render
	for( int j = 0; j != mesh[i]->nbFaces; ++j ) {
		glColor3f( mesh[i]->normal[mesh[i]->faces[j][0] * 3],
				   mesh[i]->normal[mesh[i]->faces[j][0] * 3 + 1],
				   mesh[i]->normal[mesh[i]->faces[j][0] * 3 + 2] );
		if( mesh[i]->faces[j].size() == 3 ) {
			glBegin( GL_TRIANGLES );
			for( int k = 0; k != 3; ++k ) {
				glVertex3f(
					mesh[i]->vertex[mesh[i]->faces[j][k] * 3],
					mesh[i]->vertex[mesh[i]->faces[j][k] * 3 + 1],
					mesh[i]->vertex[mesh[i]->faces[j][k] * 3 + 2] );
			}
			glEnd();
		} else if( mesh[i]->faces[j].size() == 4 ) {
			glBegin( GL_QUADS );
			for( int k = 0; k != 4; ++k ) {
				glVertex3f(
					mesh[i]->vertex[mesh[i]->faces[j][k] * 3],
					mesh[i]->vertex[mesh[i]->faces[j][k] * 3 + 1],
					mesh[i]->vertex[mesh[i]->faces[j][k] * 3 + 2] );
			}
			glEnd();
		}
	}
}

void TriangleWindow::saveToFile( const char *c ) {
	std::ofstream file;
	file.open( c, std::ofstream::binary );
	
	// save time
	file << season << " " << day << " ";
	
	// save camera
	file << this->c->ss << " " << this->c->anim << " " << this->c->etat << " " << this->c->rotX << " " << this->c->rotY << " ";

	// save data
	int nbVertex = m_image.width() * m_image.height();
	file << nbVertex << " ";
	file << picName << " ";
	uint id = 0;

	for( int i = 0; i < m_image.width( ); i++ ) {
		for( int j = 0; j < m_image.height( ); j++ ) {

			id = i*m_image.width( ) + j;

			file << p[id].x << " " << p[id].y << " " << p[id].z << " ";
		}
	}

	file.flush();
	file.close();
	qDebug() << "FILE SAVED";
}

void TriangleWindow::loadFromFile( const char *c ) {
	std::fstream file;
	char line[100];
	int nbVertex;

	file.open( c, std::fstream::in );

	file.getline( line, 100, ' ' );
	sscanf( line, "%i", &season );

	file.getline( line, 100, ' ' );
	sscanf( line, "%i", &day );

	file.getline( line, 100, ' ' );
	sscanf( line, "%f", &this->c->ss );

	file.getline( line, 100, ' ' );
	sscanf( line, "%f", &this->c->anim );

	file.getline( line, 100, ' ' );
	sscanf( line, "%i", &this->c->etat );

	file.getline( line, 100, ' ' );
	sscanf( line, "%f", &this->c->rotX );

	file.getline( line, 100, ' ' );
	sscanf( line, "%f", &this->c->rotY );

	file.getline( line, 100, ' ' );
	sscanf( line, "%i", &nbVertex );

	// load geometry
	file.getline( line, 100, ' ' );
	sscanf( line, "%s", picName );
	//loadMap( pName );
	m_image = QImage( picName );
	p = (point *)malloc( nbVertex * sizeof( point ) );

	for( int i = 0; i != nbVertex; ++i ) {
		file.getline( line, 100, ' ' );
		sscanf( line, "%f", &p[i].x );

		file.getline( line, 100, ' ' );
		sscanf( line, "%f", &p[i].y );

		file.getline( line, 100, ' ' );
		sscanf( line, "%f", &p[i].z );
	}


	file.close();

	qDebug() << "load map from file successfully";
}